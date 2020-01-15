#ifndef _serialmidi_h
#define _serialmidi_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/types.h>
#include "serial_irq.h"
#include "soundmanager.h"
// #include "fifo.h"
#include "utils/RingBuffer.h"
// #include "screenmanager.h"





class CSerialDevice_irq_wrapper: public CSerialDevice_irq{
public:
	CSerialDevice_irq_wrapper (CSoundManager *pm_SoundManager, CInterruptSystem *pInterruptSystem = 0, boolean bUseFIQ = FALSE);
//	CSerialDevice_irq_wrapper (CInterruptSystem *pInterruptSystem = 0, boolean bUseFIQ = FALSE);
	~CSerialDevice_irq_wrapper (void);

	void InterruptHandler_ext(RingBuffer<u8,200> *datx);

 public:
	CSoundManager *pm_SoundManager;	

	u8 m_lastMIDIStatus;
	u8 m_MIDIState=0;
	u8 m_MIDIRunningStatus=0;
	u8 m_trash=0;

//	ScreenMsg SMsg={HELLO_WORLD,0,0.0,NULL};	
};





class CSerialMIDI
{
public:
	CSerialMIDI (CSoundManager *pm_Soundmanager, CInterruptSystem *pInterruptSystem = 0, boolean bUseFIQ = FALSE);
	~CSerialMIDI (void);

	boolean Initialize (void);
	boolean checkMIDI (void);
    void FlushMIDIOut(void);

 
private:
	CSoundManager *pm_SoundManager;	
	CSerialDevice_irq_wrapper m_Serial;
//	CSerialDevice_irq m_Serial;
	boolean m_bUseSerial;
	unsigned m_nSerialState;
	u8 m_SerialMessage[3];
	
};


#endif
