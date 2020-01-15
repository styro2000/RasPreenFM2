//
// serial.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2019  R. Stange <rsta2@o2online.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "serial_irq.h"
#include <circle/devicenameservice.h>
#include <circle/bcm2835.h>
#include <circle/memio.h>
#include <circle/machineinfo.h>
#include <circle/synchronize.h>
#include <assert.h>
#include "global_obj.h"

#undef USE_RPI_STUB_AT

#ifndef USE_RPI_STUB_AT

// Definitions from Raspberry PI Remote Serial Protocol.
//     Copyright 2012 Jamie Iles, jamie@jamieiles.com.
//     Licensed under GPLv2
#define DR_OE_MASK		(1 << 11)
#define DR_BE_MASK		(1 << 10)
#define DR_PE_MASK		(1 << 9)
#define DR_FE_MASK		(1 << 8)

#define FR_TXFE_MASK		(1 << 7)
#define FR_RXFF_MASK		(1 << 6)
#define FR_TXFF_MASK		(1 << 5)
#define FR_RXFE_MASK		(1 << 4)
#define FR_BUSY_MASK		(1 << 3)

#define LCRH_SPS_MASK		(1 << 7)
#define LCRH_WLEN8_MASK		(3 << 5)
#define LCRH_WLEN7_MASK		(2 << 5)
#define LCRH_WLEN6_MASK		(1 << 5)
#define LCRH_WLEN5_MASK		(0 << 5)
#define LCRH_FEN_MASK		(1 << 4)
#define LCRH_STP2_MASK		(1 << 3)
#define LCRH_EPS_MASK		(1 << 2)
#define LCRH_PEN_MASK		(1 << 1)
#define LCRH_BRK_MASK		(1 << 0)

#define CR_CTSEN_MASK		(1 << 15)
#define CR_RTSEN_MASK		(1 << 14)
#define CR_OUT2_MASK		(1 << 13)
#define CR_OUT1_MASK		(1 << 12)
#define CR_RTS_MASK		(1 << 11)
#define CR_DTR_MASK		(1 << 10)
#define CR_RXE_MASK		(1 << 9)
#define CR_TXE_MASK		(1 << 8)
#define CR_LBE_MASK		(1 << 7)
#define CR_UART_EN_MASK		(1 << 0)

#define IFLS_RXIFSEL_SHIFT	3
#define IFLS_RXIFSEL_MASK	(7 << IFLS_RXIFSEL_SHIFT)
#define IFLS_TXIFSEL_SHIFT	0
#define IFLS_TXIFSEL_MASK	(7 << IFLS_TXIFSEL_SHIFT)
	#define IFLS_IFSEL_1_8		0
	#define IFLS_IFSEL_1_4		1
	#define IFLS_IFSEL_1_2		2
	#define IFLS_IFSEL_3_4		3
	#define IFLS_IFSEL_7_8		4

#define INT_OE			(1 << 10)
#define INT_BE			(1 << 9)
#define INT_PE			(1 << 8)
#define INT_FE			(1 << 7)
#define INT_RT			(1 << 6)
#define INT_TX			(1 << 5)
#define INT_RX			(1 << 4)
#define INT_DSRM		(1 << 3)
#define INT_DCDM		(1 << 2)
#define INT_CTSM		(1 << 1)

CSerialDevice_irq::CSerialDevice_irq (CInterruptSystem *pInterruptSystem, boolean bUseFIQ)
:
#if SERIAL_GPIO_SELECT == 14
	// to be sure there is no collision with the Bluetooth controller
	m_GPIO32 (32, GPIOModeInput),
	m_GPIO33 (33, GPIOModeInput),
	m_TxDPin (14, GPIOModeAlternateFunction0),
	m_RxDPin (15, GPIOModeAlternateFunction0),
#elif SERIAL_GPIO_SELECT == 32
	m_TxDPin (32, GPIOModeAlternateFunction3),
	m_RxDPin (33, GPIOModeAlternateFunction3),
#elif SERIAL_GPIO_SELECT == 36
	m_TxDPin (36, GPIOModeAlternateFunction2),
	m_RxDPin (37, GPIOModeAlternateFunction2),
#else
	#error SERIAL_GPIO_SELECT must be 14, 32 or 36!
#endif
	m_pInterruptSystem (pInterruptSystem),
	m_bUseFIQ (bUseFIQ),
	m_bInterruptConnected (FALSE),
	m_nRxInPtr (0),
	m_nRxOutPtr (0),
	m_nRxStatus (0),
	m_nTxInPtr (0),
	m_nTxOutPtr (0),
	m_nOptions (SERIAL_OPTION_ONLCR),
	m_pMagic (0),
	m_SpinLock (bUseFIQ ? FIQ_LEVEL : IRQ_LEVEL)
#ifdef REALTIME
	, m_LineSpinLock (TASK_LEVEL)
#endif
{
}

CSerialDevice_irq::~CSerialDevice_irq (void)
{
	PeripheralEntry ();
	write32 (ARM_UART0_IMSC, 0);
	write32 (ARM_UART0_CR, 0);
	PeripheralExit ();

	if (m_bInterruptConnected)
	{
		assert (m_pInterruptSystem != 0);
		if (!m_bUseFIQ)
		{
			m_pInterruptSystem->DisconnectIRQ (ARM_IRQ_UART);
		}
		else
		{
			m_pInterruptSystem->DisconnectFIQ ();
		}
	}

	m_pInterruptSystem = 0;
}

boolean CSerialDevice_irq::Initialize (unsigned nBaudrate)
{
	unsigned nClockRate = CMachineInfo::Get ()->GetClockRate (CLOCK_ID_UART);
	assert (nClockRate > 0);

	assert (300 <= nBaudrate && nBaudrate <= 4000000);
	unsigned nBaud16 = nBaudrate * 16;
	unsigned nIntDiv = nClockRate / nBaud16;
	assert (1 <= nIntDiv && nIntDiv <= 0xFFFF);
	unsigned nFractDiv2 = (nClockRate % nBaud16) * 8 / nBaudrate;
	unsigned nFractDiv = nFractDiv2 / 2 + nFractDiv2 % 2;
	assert (nFractDiv <= 0x3F);


	if (m_pInterruptSystem != 0)
	{
		if (!m_bUseFIQ)
		{
			m_pInterruptSystem->ConnectIRQ (ARM_IRQ_UART, InterruptStub, this);
		}
		else
		{
			m_pInterruptSystem->ConnectFIQ (ARM_FIQ_UART, InterruptStub, this);
		}

		m_bInterruptConnected = TRUE;
	}

	PeripheralEntry ();

	write32 (ARM_UART0_IMSC, 0);
	write32 (ARM_UART0_ICR,  0x7FF);
	write32 (ARM_UART0_IBRD, nIntDiv);
	write32 (ARM_UART0_FBRD, nFractDiv);

	if (m_pInterruptSystem != 0)
	{
		write32 (ARM_UART0_IFLS,   IFLS_IFSEL_1_4 << IFLS_TXIFSEL_SHIFT
					 | IFLS_IFSEL_1_4 << IFLS_RXIFSEL_SHIFT);
		write32 (ARM_UART0_LCRH, LCRH_WLEN8_MASK | LCRH_FEN_MASK);		// 8N1
		write32 (ARM_UART0_IMSC, INT_RX | INT_RT | INT_OE);
	}
	else
	{
		write32 (ARM_UART0_LCRH, LCRH_WLEN8_MASK);	// 8N1
	}

	write32 (ARM_UART0_CR, CR_UART_EN_MASK | CR_TXE_MASK | CR_RXE_MASK);

	PeripheralExit ();

	CDeviceNameService::Get ()->AddDevice ("ttyS1", this, FALSE);
//    out_led3.Set(1);
//    out_led3.Invert();

	return TRUE;
}

int CSerialDevice_irq::Write (const void *pBuffer, size_t nCount)
{
	m_LineSpinLock.Acquire ();

	u8 *pChar = (u8 *) pBuffer;
	assert (pChar != 0);

	int nResult = 0;

	while (nCount--)
	{
		if (!Write (*pChar))
		{
			break;
		}

		if (*pChar++ == '\n')
		{
			if (m_nOptions & SERIAL_OPTION_ONLCR)
			{
				if (!Write ('\r'))
				{
					break;
				}
			}
		}

		nResult++;
	}

	m_LineSpinLock.Release ();

	if (m_pInterruptSystem != 0)
	{
		m_SpinLock.Acquire ();

		if (m_nTxInPtr != m_nTxOutPtr)
		{
			PeripheralEntry ();

			while (m_nTxInPtr != m_nTxOutPtr)
			{
				if (!(read32 (ARM_UART0_FR) & FR_TXFF_MASK))
				{
					write32 (ARM_UART0_DR, m_TxBuffer[m_nTxOutPtr++]);
					m_nTxOutPtr &= SERIAL_BUF_MASK;
				}
				else
				{
					write32 (ARM_UART0_IMSC, read32 (ARM_UART0_IMSC) | INT_TX);

					break;
				}
			}

			PeripheralExit ();
		}

		m_SpinLock.Release ();
	}

	return nResult;
}

int CSerialDevice_irq::Read (void *pBuffer, size_t nCount)
{
	u8 *pChar = (u8 *) pBuffer;
	assert (pChar != 0);

	int nResult = 0;

	if (m_pInterruptSystem != 0)
	{
		m_SpinLock.Acquire ();

		if (m_nRxStatus < 0)
		{
			nResult = m_nRxStatus;
			m_nRxStatus = 0;
		}
		else
		{
			while (nCount > 0)
			{
				if (m_nRxInPtr == m_nRxOutPtr)
				{
					break;
				}

				*pChar++ = m_RxBuffer[m_nRxOutPtr++];
				m_nRxOutPtr &= SERIAL_BUF_MASK;

				nCount--;
				nResult++;
			}
		}

		m_SpinLock.Release ();
	}
	else
	{
		PeripheralEntry ();

		while (nCount > 0)
		{
			if (read32 (ARM_UART0_FR) & FR_RXFE_MASK)
			{
				break;
			}

			u32 nDR = read32 (ARM_UART0_DR);
			if (nDR & DR_BE_MASK)
			{
				nResult = -SERIAL_ERROR_BREAK;

				break;
			}
			else if (nDR & DR_OE_MASK)
			{
				nResult = -SERIAL_ERROR_OVERRUN;

				break;
			}
			else if (nDR & DR_FE_MASK)
			{
				nResult = -SERIAL_ERROR_FRAMING;

				break;
			}

			*pChar++ = nDR & 0xFF;

			nCount--;
			nResult++;
		}

		PeripheralExit ();
	}

	return nResult;
}

unsigned CSerialDevice_irq::GetOptions (void) const
{
	return m_nOptions;
}

void CSerialDevice_irq::SetOptions (unsigned nOptions)
{
	m_nOptions = nOptions;
}

void CSerialDevice_irq::RegisterMagicReceivedHandler (const char *pMagic, TMagicReceivedHandler *pHandler)
{
	assert (m_pInterruptSystem != 0);
	assert (m_pMagic == 0);

	assert (pMagic != 0);
	assert (*pMagic != '\0');
	assert (pHandler != 0);

	m_pMagicReceivedHandler = pHandler;

	m_pMagicPtr = pMagic;
	m_pMagic = pMagic;		// enables the scanner
}

unsigned CSerialDevice_irq::AvailableForWrite (void)
{
	assert (m_pInterruptSystem != 0);

	m_SpinLock.Acquire ();

	unsigned nResult;
	if (m_nTxOutPtr <= m_nTxInPtr)
	{
		nResult = SERIAL_BUF_SIZE+m_nTxOutPtr-m_nTxInPtr-1;
	}
	else
	{
		nResult = m_nTxOutPtr-m_nTxInPtr-1;
	}

	m_SpinLock.Release ();

	return nResult;
}

unsigned CSerialDevice_irq::AvailableForRead (void)
{
	assert (m_pInterruptSystem != 0);

	m_SpinLock.Acquire ();

	unsigned nResult;
	if (m_nRxInPtr < m_nRxOutPtr)
	{
		nResult = SERIAL_BUF_SIZE+m_nRxInPtr-m_nRxOutPtr;
	}
	else
	{
		nResult = m_nRxInPtr-m_nRxOutPtr;
	}

	m_SpinLock.Release ();

	return nResult;
}

int CSerialDevice_irq::Peek (void)
{
	assert (m_pInterruptSystem != 0);

	m_SpinLock.Acquire ();

	int nResult = -1;
	if (m_nRxInPtr != m_nRxOutPtr)
	{
		nResult = m_RxBuffer[m_nRxOutPtr];
	}

	m_SpinLock.Release ();

	return nResult;
}

void CSerialDevice_irq::Flush (void)
{
	PeripheralEntry ();

	while (read32 (ARM_UART0_FR) & FR_BUSY_MASK)
	{
		// just wait
	}

	PeripheralExit ();
}

boolean CSerialDevice_irq::Write (u8 uchChar)
{
	boolean bOK = TRUE;

	if (m_pInterruptSystem != 0)
	{
		m_SpinLock.Acquire ();

		if (((m_nTxInPtr+1) & SERIAL_BUF_MASK) != m_nTxOutPtr)
		{
			m_TxBuffer[m_nTxInPtr++] = uchChar;
			m_nTxInPtr &= SERIAL_BUF_MASK;
		}
		else
		{
			bOK = FALSE;
		}

		m_SpinLock.Release ();
	}
	else
	{
		PeripheralEntry ();

		while (read32 (ARM_UART0_FR) & FR_TXFF_MASK)
		{
			// do nothing
		}

		write32 (ARM_UART0_DR, uchChar);

		PeripheralExit ();
	}

	return bOK;
}

void CSerialDevice_irq::InterruptHandler_ext(RingBuffer<u8,200> *datx){
//	out_led3.Invert();
}
 

void CSerialDevice_irq::InterruptHandler (void)
{
	boolean bMagicReceived = FALSE;


//	out_led3.Invert();


	m_SpinLock.Acquire ();

	PeripheralEntry ();

	// acknowledge pending interrupts
	write32 (ARM_UART0_ICR, read32 (ARM_UART0_MIS));



	while (!(read32 (ARM_UART0_FR) & FR_RXFE_MASK))
	{
		u32 nDR = read32 (ARM_UART0_DR);
		if (nDR & DR_BE_MASK)
		{
			if (m_nRxStatus == 0)
			{
				m_nRxStatus = -SERIAL_ERROR_BREAK;
			}
		}
		else if (nDR & DR_OE_MASK)
		{
			if (m_nRxStatus == 0)
			{
				m_nRxStatus = -SERIAL_ERROR_OVERRUN;
			}
		}
		else if (nDR & DR_FE_MASK)
		{
			if (m_nRxStatus == 0)
			{
				m_nRxStatus = -SERIAL_ERROR_FRAMING;
			}
		}

		if (m_pMagic != 0)
		{
			if ((char) (nDR & 0xFF) == *m_pMagicPtr)
			{
				if (*++m_pMagicPtr == '\0')
				{
					bMagicReceived = TRUE;
				}
			}
			else
			{
				m_pMagicPtr = m_pMagic;
			}
		}

		usartBufferIn.insert( nDR & 0xFF);
		// if (m_RxFifo.insert( nDR & 0xFF))
		// {
		// 	m_nRxStatus = -SERIAL_ERROR_OVERRUN;
		// }
		InterruptHandler_ext(&usartBufferIn);
	}

	while (!(read32 (ARM_UART0_FR) & FR_TXFF_MASK))
	{
		if (usartBufferOut.getCount() >0)
		{
			write32 (ARM_UART0_DR, usartBufferOut.remove());

		}
		else
		{
			write32 (ARM_UART0_IMSC, read32 (ARM_UART0_IMSC) & ~INT_TX);
			break;
		}
	}

	PeripheralExit ();

	m_SpinLock.Release ();

	if (bMagicReceived)
	{
		(*m_pMagicReceivedHandler) ();
	}
}

void  CSerialDevice_irq::TX_Flush(void){
	// out_led3.Invert();

	if (m_pInterruptSystem != 0)
	{
		m_SpinLock.Acquire ();

		if (usartBufferOut.getCount() >0)
		{
			PeripheralEntry ();

			while (usartBufferOut.getCount() >0)
			{
				if (!(read32 (ARM_UART0_FR) & FR_TXFF_MASK))			// FiFo Full?
				{
					write32 (ARM_UART0_DR,  usartBufferOut.remove());	// Fill FiFo
				}
				else
				{
					write32 (ARM_UART0_IMSC, read32 (ARM_UART0_IMSC) | INT_TX);	//Enable Interruppts
					break;
				}
			}

			PeripheralExit ();
		}

		m_SpinLock.Release ();
	}
}


void CSerialDevice_irq::InterruptStub (void *pParam)
{
	CSerialDevice_irq *pThis = (CSerialDevice_irq *) pParam;
//    out_led1.Invert();
//    m_LVGScreen.Rotate();

	assert (pThis != 0);

	pThis->InterruptHandler ();
//    out_led1.Invert();
}

#else	// #ifndef USE_RPI_STUB_AT

boolean CSerialDevice_irq::Initialize (unsigned nBaudrate)
{
	CDeviceNameService::Get ()->AddDevice ("ttyS1", this, FALSE);

	return TRUE;
}

int CSerialDevice_irq::Write (const void *pBuffer, unsigned nCount)
{
	asm volatile
	(
		"push {r0-r2}\n"
		"mov r0, %0\n"
		"mov r1, %1\n"
		"bkpt #0x7FFB\n"	// send message to GDB client
		"pop {r0-r2}\n"

		: : "r" (pBuffer), "r" (nCount)
	);

	return nCount;
}

#endif
