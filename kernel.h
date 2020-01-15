//
// kernel.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2019  R. Stange <rsta2@o2online.de>
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
#ifndef _kernel_h
#define _kernel_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
// #include <circle/screen.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/input/touchscreen.h>
#include <littlevgl/littlevgl.h>
#include <circle/types.h>
#include <circle/usb/dwhcidevice.h>
#include <circle/i2cmaster.h>
#include <circle/spimaster.h>
#include <fatfs/ff.h>
#include "serial_irq.h"
// #include "screenmanager.h"
#include "serialmidi.h"
#include "soundmanager.h"
#include "RingBuffer.h"
#include "screenx.h"

#define TEST_DATA_LENGTH	11		// number of data bytes transfered



extern RingBuffer<uint8_t, 200> usartBufferIn;
extern RingBuffer<uint8_t, 100> usartBufferOut;
extern unsigned int preenTimer;

void fillSoundBuffer(void);

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	boolean Initialize (void);

	TShutdownMode Run (void);

	uint16_t ButtonEncoderRegister;

private:
	void InitializeHiFiBerry(void);
	void InitializeMAX6957(void);
	uint16_t ReadInputMAX6957(void);
	void writeLedMAX6957(uint8_t LED, uint8_t data);

private:
	// do not change this order
	CMemorySystem		m_Memory;
	CActLED				m_ActLED;
	CKernelOptions		m_Options;
	CDeviceNameService	m_DeviceNameService;
	CCPUThrottle		m_CPUThrottle;
	CScreenDeviceX		m_Screen;
//	CSerialDevice		m_Serial;
	CExceptionHandler	m_ExceptionHandler;
	CInterruptSystem	m_Interrupt;
	CTimer				m_Timer;
	CLogger				m_Logger;
	CUSBHCIDevice		m_USBHCI;
//	CEMMCDevice			m_EMMC;

	CTouchScreenDevice	m_TouchScreen;
	CSerialMIDI			m_SerialMIDI;
//	CDWHCIDevice		m_DWHCI;
	CI2CMaster			m_I2CMaster;
	CSPIMaster			m_SPIMaster;
	CSoundManager		m_Soundmanager;	
//	CLittlevGL			m_GUI;

//	CGPIOPin 			InPins[8];
//	CGPIOPin InPin;

//	FATFS				m_FileSystem;
	u8 SPITxData[TEST_DATA_LENGTH];
	u8 SPIRxBuffer[TEST_DATA_LENGTH];


};

#endif
