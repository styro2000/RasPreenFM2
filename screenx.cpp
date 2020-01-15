//
// screen.cpp
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
#include "screenx.h"
#include <circle/devicenameservice.h>
#include <circle/synchronize.h>
#include <circle/util.h>

#define ROTORS		4

enum TScreenState
{
	ScreenStateStart,
	ScreenStateEscape,
	ScreenStateBracket,
	ScreenStateNumber1,
	ScreenStateQuestionMark,
	ScreenStateSemicolon,
	ScreenStateNumber2,
	ScreenStateNumber3
};

CScreenDeviceX::CScreenDeviceX (unsigned nWidth, unsigned nHeight, boolean bVirtual)
:	m_nInitWidth (nWidth),
	m_nInitHeight (nHeight),
	m_bVirtual (bVirtual),
	m_pFrameBuffer (0),
	m_pBuffer (0),
	m_nState (ScreenStateStart),
	m_nScrollStart (0),
	m_nCursorX (0),
	m_nCursorY (0),
	m_bCursorOn (TRUE),
	m_Color (NORMAL_COLOR),
	m_bInsertOn (FALSE),
	m_bUpdated (FALSE)
#ifdef SCREEN_DMA_BURST_LENGTH
	, m_DMAChannel (DMA_CHANNEL_NORMAL)
#endif
#ifdef REALTIME
	, m_SpinLock (TASK_LEVEL)
#endif
{
}

CScreenDeviceX::~CScreenDeviceX (void)
{
	if (m_bVirtual)
	{
		delete [] m_pBuffer;
	}
	m_pBuffer = 0;
	
	delete m_pFrameBuffer;
	m_pFrameBuffer = 0;
}

boolean CScreenDeviceX::Initialize (void)
{
	if (!m_bVirtual)
	{
		m_pFrameBuffer = new CBcmFrameBuffer (m_nInitWidth, m_nInitHeight, DEPTH);
#if DEPTH == 8
		m_pFrameBuffer->SetPalette (NORMAL_COLOR, NORMAL_COLOR16);
		m_pFrameBuffer->SetPalette (HIGH_COLOR,   HIGH_COLOR16);
		m_pFrameBuffer->SetPalette (HALF_COLOR,   HALF_COLOR16);
#endif
		if (!m_pFrameBuffer->Initialize ())
		{
			return FALSE;
		}

		if (m_pFrameBuffer->GetDepth () != DEPTH)
		{
			return FALSE;
		}

		m_pBuffer = (TScreenColor *) (uintptr) m_pFrameBuffer->GetBuffer ();
		m_nSize   = m_pFrameBuffer->GetSize ();
		m_nPitch  = m_pFrameBuffer->GetPitch ();
		m_nWidth  = m_pFrameBuffer->GetWidth ();
		m_nHeight = m_pFrameBuffer->GetHeight ();

		// Ensure that each row is word-aligned so that we can safely use memcpyblk()
		if (m_nPitch % sizeof (u32) != 0)
		{
			return FALSE;
		}
		m_nPitch /= sizeof (TScreenColor);
	}
	else
	{
		m_nWidth = m_nInitWidth;
		m_nHeight = m_nInitHeight;
		m_nSize = m_nWidth * m_nHeight * sizeof (TScreenColor);
		m_nPitch = m_nWidth;

		m_pBuffer = new TScreenColor[m_nWidth * m_nHeight];
	}

	m_nUsedHeight = m_nHeight / m_CharGen.GetCharHeight () * m_CharGen.GetCharHeight ();
	m_nScrollEnd = m_nUsedHeight;

	CursorHome ();
	ClearDisplayEnd ();
	InvertCursor ();

	CDeviceNameService::Get ()->AddDevice ("tty1", this, FALSE);

	return TRUE;
}

unsigned CScreenDeviceX::GetWidth (void) const
{
	return m_nWidth;
}

unsigned CScreenDeviceX::GetHeight (void) const
{
	return m_nHeight;
}

unsigned CScreenDeviceX::GetColumns (void) const
{
	return m_nWidth / m_CharGen.GetCharWidth ();
}

unsigned CScreenDeviceX::GetRows (void) const
{
	return m_nUsedHeight / m_CharGen.GetCharHeight ();
}

CBcmFrameBuffer *CScreenDeviceX::GetFrameBuffer (void)
{
	return m_pFrameBuffer;
}

TScreenStatusX CScreenDeviceX::GetStatus (void)
{
	TScreenStatusX Status;

	Status.pContent   = m_pBuffer;
	Status.nSize	  = m_nSize;
	Status.nState     = m_nState;
	Status.nScrollStart = m_nScrollStart;
	Status.nScrollEnd   = m_nScrollEnd;
	Status.nCursorX   = m_nCursorX;
	Status.nCursorY   = m_nCursorY;
	Status.bCursorOn  = m_bCursorOn;
	Status.Color      = m_Color;
	Status.BGColor    = m_BGColor;
	Status.bInsertOn  = m_bInsertOn;
	Status.nParam1    = m_nParam1;
	Status.nParam2    = m_nParam2;
	Status.bUpdated   = m_bUpdated;

	return Status;
}

boolean CScreenDeviceX::SetStatus (const TScreenStatusX &Status)
{
	if (   m_nSize  != Status.nSize
	    || m_nPitch != m_nWidth)
	{
		return FALSE;
	}

	m_SpinLock.Acquire ();

	if (   m_bUpdated
	    || Status.bUpdated)
	{
		m_SpinLock.Release ();

		return FALSE;
	}

	memcpyblk (m_pBuffer, Status.pContent, m_nSize);

	m_nState     = Status.nState;
	m_nScrollStart = Status.nScrollStart;
	m_nScrollEnd   = Status.nScrollEnd;
	m_nCursorX   = Status.nCursorX;
	m_nCursorY   = Status.nCursorY;
	m_bCursorOn  = Status.bCursorOn;
	m_Color      = Status.Color;
	m_BGColor    = Status.BGColor;
	m_bInsertOn  = Status.bInsertOn;
	m_nParam1    = Status.nParam1;
	m_nParam2    = Status.nParam2;

	m_SpinLock.Release ();

	DataMemBarrier ();

	return TRUE;
}

int CScreenDeviceX::Write (const void *pBuffer, size_t nCount)
{
	m_SpinLock.Acquire ();

	m_bUpdated = TRUE;
	
	InvertCursor ();
	
	const char *pChar = (const char *) pBuffer;
	int nResult = 0;

	while (nCount--)
	{
		Write (*pChar++);
		
		nResult++;
	}

	InvertCursor ();
	
	m_bUpdated = FALSE;

	m_SpinLock.Release ();

	DataMemBarrier ();

	return nResult;
}

void CScreenDeviceX::Write (char chChar)
{
	switch (m_nState)
	{
	case ScreenStateStart:
		switch (chChar)
		{
		// case '\b':
		// 	CursorLeft ();
		// 	break;

		case '\t':
			Tabulator ();
			break;

		case '\n':
			NewLine ();
			break;

		case '\r':
			CarriageReturn ();
			break;

		case '\x1b':
			m_nState = ScreenStateEscape;
			break;

		default:
			DisplayChar (chChar);
			break;
		}
		break;

	case ScreenStateEscape:
		switch (chChar)
		{
		case 'M':
			ReverseScroll ();
			m_nState = ScreenStateStart;
			break;

		case '[':
			m_nState = ScreenStateBracket;
			break;

		default:
			m_nState = ScreenStateStart;
			break;
		}
		break;

	case ScreenStateBracket:
		switch (chChar)
		{
		case '?':
			m_nState = ScreenStateQuestionMark;
			break;

		case 'A':
			CursorUp ();
			m_nState = ScreenStateStart;
			break;

		case 'B':
			CursorDown ();
			m_nState = ScreenStateStart;
			break;

		case 'C':
			CursorRight ();
			m_nState = ScreenStateStart;
			break;

		case 'D':
			CursorLeft ();
			m_nState = ScreenStateStart;
			break;

		case 'H':
			CursorHome ();
			m_nState = ScreenStateStart;
			break;

		case 'J':
			ClearDisplayEnd ();
			m_nState = ScreenStateStart;
			break;

		case 'K':
			ClearLineEnd ();
			m_nState = ScreenStateStart;
			break;

		case 'L':
			InsertLines (1);
			m_nState = ScreenStateStart;
			break;

		case 'M':
			DeleteLines (1);
			m_nState = ScreenStateStart;
			break;

		case 'P':
			DeleteChars (1);
			m_nState = ScreenStateStart;
			break;

		default:
			if ('0' <= chChar && chChar <= '9')
			{
				m_nParam1 = chChar - '0';
				m_nState = ScreenStateNumber1;
			}
			else
			{
				m_nState = ScreenStateStart;
			}
			break;
		}
		break;

	case ScreenStateNumber1:
		switch (chChar)
		{
		case ';':
			m_nState = ScreenStateSemicolon;
			break;

		case 'L':
			InsertLines (m_nParam1);
			m_nState = ScreenStateStart;
			break;

		case 'M':
			DeleteLines (m_nParam1);
			m_nState = ScreenStateStart;
			break;

		case 'P':
			DeleteChars (m_nParam1);
			m_nState = ScreenStateStart;
			break;

		case 'X':
			EraseChars (m_nParam1);
			m_nState = ScreenStateStart;
			break;

		case 'h':
		case 'l':
			if (m_nParam1 == 4)
			{
				InsertMode (chChar == 'h');
			}
			m_nState = ScreenStateStart;
			break;
			
		case 'm':
			SetStandoutMode (m_nParam1);
			m_nState = ScreenStateStart;
			break;

		default:
			if ('0' <= chChar && chChar <= '9')
			{
				m_nParam1 *= 10;
				m_nParam1 += chChar - '0';

				if (m_nParam1 > 99)
				{
					m_nState = ScreenStateStart;
				}
			}
			else
			{
				m_nState = ScreenStateStart;
			}
			break;
		}
		break;

	case ScreenStateSemicolon:
		if ('0' <= chChar && chChar <= '9')
		{
			m_nParam2 = chChar - '0';
			m_nState = ScreenStateNumber2;
		}
		else
		{
			m_nState = ScreenStateStart;
		}
		break;

	case ScreenStateQuestionMark:
		if ('0' <= chChar && chChar <= '9')
		{
			m_nParam1 = chChar - '0';
			m_nState = ScreenStateNumber3;
		}
		else
		{
			m_nState = ScreenStateStart;
		}
		break;

	case ScreenStateNumber2:
		switch (chChar)
		{
		case 'H':
			CursorMove (m_nParam1, m_nParam2);
			m_nState = ScreenStateStart;
			break;

		case 'r':
			SetScrollRegion (m_nParam1, m_nParam2);
			m_nState = ScreenStateStart;
			break;

		default:
			if ('0' <= chChar && chChar <= '9')
			{
				m_nParam2 *= 10;
				m_nParam2 += chChar - '0';

				if (m_nParam2 > 199)
				{
					m_nState = ScreenStateStart;
				}
			}
			else
			{
				m_nState = ScreenStateStart;
			}
			break;
		}
		break;

	case ScreenStateNumber3:
		switch (chChar)
		{
		case 'h':
		case 'l':
			if (m_nParam1 == 25)
			{
				SetCursorMode (chChar == 'h');
			}
			m_nState = ScreenStateStart;
			break;

		default:
			if ('0' <= chChar && chChar <= '9')
			{
				m_nParam1 *= 10;
				m_nParam1 += chChar - '0';

				if (m_nParam1 > 99)
				{
					m_nState = ScreenStateStart;
				}
			}
			else
			{
				m_nState = ScreenStateStart;
			}
			break;
		}
		break;

	default:
		m_nState = ScreenStateStart;
		break;
	}
}

void CScreenDeviceX::CarriageReturn (void)
{
	m_nCursorX = 0;
}

void CScreenDeviceX::ClearDisplay (void)
{
		// u16 h = m_nHeight;
		// u16 w = m_nWidth;

		// for (u16 x=0;x < w;x++){
		// 	for (u16 y=0;y<h;y++){
		// 		xGUI->SetPixel(x,y,m_BGColor);
		// 	}
		// }
		TScreenColor *pBuffer = m_pBuffer;
		unsigned nSize = m_nSize / sizeof (TScreenColor);
		while (nSize--)
		{
			*pBuffer++ = m_BGColor;
		}
}



void CScreenDeviceX::ClearDisplayEnd (void)
{
	ClearLineEnd ();
	
	unsigned nPosY = m_nCursorY + m_CharGen.GetCharHeight ();
	unsigned nOffset = nPosY * m_nPitch;
	
	TScreenColor *pBuffer = m_pBuffer + nOffset;
	unsigned nSize = m_nSize / sizeof (TScreenColor) - nOffset;
	
	while (nSize--)
	{
		*pBuffer++ = m_BGColor;
	}
}

void CScreenDeviceX::ClearLineEnd (void)
{
	for (unsigned nPosX = m_nCursorX; nPosX < m_nWidth; nPosX += m_CharGen.GetCharWidth ())
	{
		EraseChar (nPosX, m_nCursorY);
	}
}

void CScreenDeviceX::CursorDown (void)
{
	if (m_bBigMode){
		m_nCursorY += m_CharGen.GetCharHeight ()*4;
		if (m_nCursorY >= m_nScrollEnd)
		{
			Scroll ();

			m_nCursorY -= m_CharGen.GetCharHeight ()*4;
		}
	}else{
		m_nCursorY += m_CharGen.GetCharHeight ();
		if (m_nCursorY >= m_nScrollEnd)
		{
			Scroll ();

			m_nCursorY -= m_CharGen.GetCharHeight ();
		}
		
	}
}

void CScreenDeviceX::CursorHome (void)
{
	m_nCursorX = 0;
	m_nCursorY = m_nScrollStart;
}

void CScreenDeviceX::CursorLeft (void)
{
	if (m_nCursorX > 0)
	{
		m_nCursorX -= m_CharGen.GetCharWidth ();
	}
	else
	{
		if (m_nCursorY > m_nScrollStart)
		{
			m_nCursorX = m_nWidth - m_CharGen.GetCharWidth ();
			m_nCursorY -= m_CharGen.GetCharHeight ();
		}
	}
}

void CScreenDeviceX::CursorMove (unsigned nRow, unsigned nColumn)
{
	if (m_bBigMode){
		unsigned nPosX = (nColumn - 1) * m_CharGen.GetCharWidth ()*4;
		unsigned nPosY = (nRow - 1) * m_CharGen.GetCharHeight ()*4;

		if (   nPosX < m_nWidth
			&& nPosY >= m_nScrollStart
			&& nPosY < m_nScrollEnd)
		{
			m_nCursorX = nPosX;
			m_nCursorY = nPosY;
		}
	}else{
		unsigned nPosX = (nColumn - 1) * m_CharGen.GetCharWidth ();
		unsigned nPosY = (nRow - 1) * m_CharGen.GetCharHeight ();

		if (   nPosX < m_nWidth
			&& nPosY >= m_nScrollStart
			&& nPosY < m_nScrollEnd)
		{
			m_nCursorX = nPosX;
			m_nCursorY = nPosY;
		}
	}
}

void CScreenDeviceX::CursorRight (void)
{
	if (m_bBigMode){
		m_nCursorX += m_CharGen.GetCharWidth ()*4;
		if (m_nCursorX >= m_nWidth)
		{
			NewLine ();
		}
	}else{
		m_nCursorX += m_CharGen.GetCharWidth ();
		if (m_nCursorX >= m_nWidth)
		{
			NewLine ();
		}
	}
}

void CScreenDeviceX::CursorUp (void)
{
	if (m_nCursorY > m_nScrollStart)
	{
		m_nCursorY -= m_CharGen.GetCharHeight ();
	}
}

void CScreenDeviceX::DeleteChars (unsigned nCount)	// TODO
{
}

void CScreenDeviceX::DeleteLines (unsigned nCount)	// TODO
{
}

void CScreenDeviceX::DisplayChar (char chChar)
{
	// TODO: Insert mode
	
	// if (' ' <= (unsigned char) chChar)
	// {
		DisplayChar (chChar, m_nCursorX, m_nCursorY, m_Color);

		CursorRight ();
	// }
}

void CScreenDeviceX::EraseChars (unsigned nCount)
{
	if (nCount == 0)
	{
		return;
	}

	unsigned nEndX = m_nCursorX + nCount * m_CharGen.GetCharWidth ();
	if (nEndX > m_nWidth)
	{
		nEndX = m_nWidth;
	}

	for (unsigned nPosX = m_nCursorX; nPosX < nEndX; nPosX += m_CharGen.GetCharWidth ())
	{
		EraseChar (nPosX, m_nCursorY);
	}
}

void CScreenDeviceX::InsertLines (unsigned nCount)	// TODO
{
}

void CScreenDeviceX::InsertMode (boolean bBegin)
{
	m_bInsertOn = bBegin;
}

void CScreenDeviceX::NewLine (void)
{
	CarriageReturn ();
	CursorDown ();
}

void CScreenDeviceX::ReverseScroll (void)
{
	if (   m_nCursorX == 0
	    && m_nCursorY == 0
	)
	{
		InsertLines (1);
	}
}

void CScreenDeviceX::SetCursorMode (boolean bVisible)
{
	m_bCursorOn = bVisible;
}

void CScreenDeviceX::SetScrollRegion (unsigned nStartRow, unsigned nEndRow)
{
	unsigned nScrollStart = (nStartRow - 1) * m_CharGen.GetCharHeight ();
	unsigned nScrollEnd   = nEndRow * m_CharGen.GetCharHeight ();

	if (   nScrollStart <  m_nUsedHeight
	    && nScrollEnd   >  0
	    && nScrollEnd   <= m_nUsedHeight
	    && nScrollStart <  nScrollEnd)
	{
		m_nScrollStart = nScrollStart;
		m_nScrollEnd   = nScrollEnd;
	}

	CursorHome ();
}

// TODO: standout mode should be useable together with one other mode
void CScreenDeviceX::SetStandoutMode (unsigned nMode)
{
	switch (nMode)
	{
	case 0:
	case 27:
		m_Color = NORMAL_COLOR;
		break;
		
	case 1:
		m_Color = HIGH_COLOR;
		break;

	case 2:
		m_Color = HALF_COLOR;
		break;

	case 7:				// TODO: reverse mode
	default:
		break;
	}
}

void CScreenDeviceX::SetBigMode (bool BigMode){
	m_bBigMode = BigMode;
}

void CScreenDeviceX::Tabulator (void)
{
	unsigned nTabWidth = m_CharGen.GetCharWidth () * 8;
	
	m_nCursorX = ((m_nCursorX + nTabWidth) / nTabWidth) * nTabWidth;
	if (m_nCursorX >= m_nWidth)
	{
		NewLine ();
	}
}

void CScreenDeviceX::Scroll (void)
{
	unsigned nLines = m_CharGen.GetCharHeight ();

	u32 *pTo = (u32 *) (m_pBuffer + m_nScrollStart * m_nPitch);
	u32 *pFrom = (u32 *) (m_pBuffer + (m_nScrollStart + nLines) * m_nPitch);

	unsigned nSize = m_nPitch * (m_nScrollEnd - m_nScrollStart - nLines) * sizeof (TScreenColor);
	if (nSize > 0)
	{
#ifdef SCREEN_DMA_BURST_LENGTH
		m_DMAChannel.SetupMemCopy (pTo, pFrom, nSize, SCREEN_DMA_BURST_LENGTH, FALSE);

		m_DMAChannel.Start ();
		m_DMAChannel.Wait ();
#else
		unsigned nSizeBlk = nSize & ~0xF;
		memcpyblk (pTo, pFrom, nSizeBlk);

		// Handle framebuffers with row lengths not aligned to 16 bytes
		memcpy ((u8 *) pTo + nSizeBlk, (u8 *) pFrom + nSizeBlk, nSize & 0xF);
#endif

		pTo += nSize / sizeof (u32);
	}

	nSize = m_nPitch * nLines * sizeof (TScreenColor) / sizeof (u32);
	while (nSize--)
	{
		*pTo++ = m_BGColor;
	}
}

void CScreenDeviceX::DisplayChar (char chChar, unsigned nPosX, unsigned nPosY, TScreenColor Color)
{
	if (m_bBigMode){

		for (unsigned y = 0; y < m_CharGen.GetCharHeight ()*4; y++)
		{
			for (unsigned x = 0; x < m_CharGen.GetCharWidth ()*4; x++)
			{
				if (chChar >= ' '){ 
					SetPixel (nPosX + x, nPosY + y, m_CharGen.GetPixel (chChar, x/4, y/4) ? Color : m_BGColor);
				}else{
					if (y <56){
						SetPixel (nPosX + x, nPosY + y, GetCustomPixel (chChar, x/4, y/4) ? Color : m_BGColor);
					}else{
						SetPixel (nPosX + x, nPosY + y, m_BGColor);						
					}
				}
			}
		}
	}else{
		for (unsigned y = 0; y < m_CharGen.GetCharHeight (); y++)
		{
			for (unsigned x = 0; x < m_CharGen.GetCharWidth (); x++)
			{
				SetPixel (nPosX + x, nPosY + y,
					m_CharGen.GetPixel (chChar, x, y) ? Color : m_BGColor);
			}
		}
	}
}

void CScreenDeviceX::EraseChar (unsigned nPosX, unsigned nPosY)
{
	for (unsigned y = 0; y < m_CharGen.GetCharHeight (); y++)
	{
		for (unsigned x = 0; x < m_CharGen.GetCharWidth (); x++)
		{
			SetPixel (nPosX + x, nPosY + y, m_BGColor);
		}
	}
}

void CScreenDeviceX::InvertCursor (void)
{
	if (!m_bCursorOn)
	{
		return;
	}
	
	if (m_bBigMode){

		for (unsigned y = m_CharGen.GetUnderline ()*4; y < m_CharGen.GetCharHeight ()*4; y++)
		{
			for (unsigned x = 0; x < m_CharGen.GetCharWidth ()*4; x++)
			{
				if (GetPixel (m_nCursorX + x, m_nCursorY + y) == m_BGColor)
				{
					SetPixel (m_nCursorX + x, m_nCursorY + y, m_Color);
				}
				else
				{
					SetPixel (m_nCursorX + x, m_nCursorY + y, m_BGColor);
				}
			}
		}
	}else{
	
		for (unsigned y = m_CharGen.GetUnderline (); y < m_CharGen.GetCharHeight (); y++)
		{
			for (unsigned x = 0; x < m_CharGen.GetCharWidth (); x++)
			{
				if (GetPixel (m_nCursorX + x, m_nCursorY + y) == m_BGColor)
				{
					SetPixel (m_nCursorX + x, m_nCursorY + y, m_Color);
				}
				else
				{
					SetPixel (m_nCursorX + x, m_nCursorY + y, m_BGColor);
				}
			}
		}
	}
}

void CScreenDeviceX::SetPixel (unsigned nPosX, unsigned nPosY, TScreenColor Color)
{
	if (   nPosX < m_nWidth
	    && nPosY < m_nHeight)
	{
		if (likely(m_bFlipMode==0)){
			m_pBuffer[m_nPitch * nPosY + nPosX] = Color;
		}else{
			m_pBuffer[(m_nPitch * m_nHeight + m_nWidth)-(m_nPitch * nPosY + nPosX)] = Color;
		}
	}
}

TScreenColor CScreenDeviceX::GetPixel (unsigned nPosX, unsigned nPosY)
{
	if (   nPosX < m_nWidth
	    && nPosY < m_nHeight)
	{
		if (likely(m_bFlipMode==0)){
			return m_pBuffer[m_nPitch * nPosY + nPosX];
		}else{
			return m_pBuffer[(m_nPitch * m_nHeight + m_nWidth)-(m_nPitch * nPosY + nPosX)];
		}
			
	}
	
	return m_BGColor;
}

void CScreenDeviceX::Rotor (unsigned nIndex, unsigned nCount)
{
	static const char chChars[] = "-\\|/";

	nIndex %= ROTORS;
	nCount &= 4-1;

	unsigned nPosX = m_nWidth - (nIndex + 1) * m_CharGen.GetCharWidth ();

	DisplayChar (chChars[nCount], nPosX, 0, HIGH_COLOR);
}

boolean CScreenDeviceX::GetCustomPixel (u8 slot, unsigned nPosX, unsigned nPosY)
{

	return CustomChars[slot].CharData[nPosY] & (0x80 >> nPosX) ? TRUE : FALSE;
}

void CScreenDeviceX::SetCustomChar (u8 slot, unsigned char *font_data)
{
	CustomChars[slot].character = 128+slot;
	for (int i=0; i<16;i++){
		CustomChars[slot].CharData[i] = font_data[i/2];
	}
}
