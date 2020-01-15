//
// miniorgan.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2017  R. Stange <rsta2@o2online.de>
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
#ifndef _soundmanager_h
#define _soundmanager_h

class CSoundManager;


#define USE_I2S

#ifdef USE_I2S
	#include <circle/i2ssoundbasedevice.h>
	#define SOUND_CLASS	CI2SSoundBaseDevice
//	#define SAMPLE_RATE	96000
	#define SAMPLE_RATE	48000
//	#define CHUNKSIZE	8192
//	#define CHUNKSIZE	128
//	#define CHUNKSIZE	64
	#define CHUNKSIZE	32
#else
	#include <circle/pwmsoundbasedevice.h>
	#define SOUND_CLASS	CPWMSoundBaseDevice
	#define SAMPLE_RATE	48000
#endif

#include <circle/interrupt.h>
#include <circle/types.h>
#include <circle/gpiopin.h>
#include "serialmidi.h"
#include "synth/Synth.h"

class CSerialMIDI;

struct TNoteInfo
{
	char	Key;
	u8	KeyNumber;	// MIDI number
};

class CSoundManager : public SOUND_CLASS
{
public:
	CSoundManager (CInterruptSystem *pInterrupt, CSerialMIDI *pm_SerialMIDI, Synth *psynth);
//	CSoundManager (CInterruptSystem *pInterrupt, CSerialMIDI *pm_SerialMIDI);
	~CSoundManager (void);

	boolean Initialize (void);

	void Process (void);
	// void NoteOn(u8 ucChannel, u8 ucKeyNumber, u8 ucVelocity);
	// void NoteOff(u8 ucChannel, u8 ucKeyNumber, u8 ucVelocity);
	unsigned GetChunk (u32 *pBuffer, unsigned nChunkSize);

	int m_clipping=0;


private:
	// static void MIDIPacketHandler (unsigned nCable, u8 *pPacket, unsigned nLength);


private:
	CSerialMIDI *pm_SerialMIDI;

	u8 		m_MIDIMessage[20];

	int      m_nLowLevel;
	int      m_nNullLevel;
	int      m_nHighLevel;
	int      m_nCurrentLevel;
	unsigned m_nSampleCount;
	unsigned m_nFrequency;		// 0 if no key pressed
	unsigned m_nPrevFrequency;
	int 	m_left;
    int 	m_right;
	u8	alternate=1;

	Synth *m_psynth;

	u8 m_ucKeyNumber;

	static const float s_KeyFrequency[];
	static const TNoteInfo s_Keys[];

	static CSoundManager *s_pThis;
	static float s_SineTable[];

	float m_nEnvCount;





};

#endif
