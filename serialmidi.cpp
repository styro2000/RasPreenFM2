
#include "serialmidi.h"
#include "global_obj.h"

RingBuffer<uint8_t, 200> usartBufferIn;
RingBuffer<uint8_t, 100> usartBufferOut;



CSerialDevice_irq_wrapper::CSerialDevice_irq_wrapper(CSoundManager *pm_SoundManager, CInterruptSystem *pInterruptSystem , boolean bUseFIQ)
//CSerialDevice_irq_wrapper::CSerialDevice_irq_wrapper(CInterruptSystem *pInterruptSystem , boolean bUseFIQ)
:CSerialDevice_irq(pInterruptSystem ,bUseFIQ) //,
// pm_SoundManager (pm_SoundManager)
{
  
}

CSerialDevice_irq_wrapper::~CSerialDevice_irq_wrapper(void){

}

void CSerialDevice_irq_wrapper::InterruptHandler_ext(RingBuffer<u8,200> *pRXFifo){

	u8 cmd = 0;
	u8 note = 0;
	u8 vel = 0;
	u16 data16=0;

// 	data16 = pRXFifo->remove();


// 	if (data16 & 8000){
// 		return;
// 	}

// 	u8 data = (u8) data16;
// 	if ((data > 0xBF) && (data < 0xF8))
// 	{
// 		m_MIDIRunningStatus = 0;
// 		m_MIDIState = 0;
// 	}

// 	if (data & 0x80)
// 	{
// 		m_MIDIRunningStatus = data;
// 		m_MIDIState = 1;
// 		cmd = data; 
// 	}
// 	if (data < 0x80)
// 	{
// 		if (m_MIDIState == 1)
// 		{
// 			note = data;
// 			m_MIDIState++;
// 		}
// 	}
// 	if (m_MIDIState == 2)
// 	{
// 		vel = data;
// 		m_MIDIState = 1;
// 		cmd = m_MIDIRunningStatus;
// 		if (m_MIDIRunningStatus == 0x90){					// Note On
// 			if (vel != 0){			
// 				pm_SoundManager->NoteOn(cmd,note,vel);
// //				m_LVGScreen.setText("Note On ");
// 				SMsg.cmd = NOTE_ON;
// 				m_LVGScreen.PutScreen(SMsg);
// 			}else{
// 				pm_SoundManager->NoteOff(cmd,note,0);
// 				SMsg.cmd = NOTE_OFF;
// 				m_LVGScreen.PutScreen(SMsg);
// //				m_LVGScreen.setText("Note Off ");
// 			}
// 		}else if (m_MIDIRunningStatus == 0x80){				// Note Off
// 				pm_SoundManager->NoteOff(cmd,note,0);	
// 				SMsg.cmd = NOTE_OFF;
// 				m_LVGScreen.PutScreen(SMsg);
// //				m_LVGScreen.setText("Note Off ");
// 		}
// 	}



//	pm_SoundManager->Process();
//	out_led3.Invert();
}


CSerialMIDI::CSerialMIDI (CSoundManager *pm_SoundManager, CInterruptSystem *pInterruptSystem , boolean bUseFIQ)
: pm_SoundManager (pm_SoundManager),
m_Serial(pm_SoundManager,pInterruptSystem,bUseFIQ)
{
//    OutPin.Write(0);
}

CSerialMIDI::~CSerialMIDI (void)
{
}

boolean CSerialMIDI::Initialize (void)
{
	if (m_Serial.Initialize(31250))
	{
		m_bUseSerial = TRUE;
		return TRUE;
	}
	return TRUE;
}

boolean CSerialMIDI::checkMIDI(void)
{
    u8 Buffer[20];
	int nResult = m_Serial.Read(Buffer, sizeof Buffer);
	if (nResult <= 0)
	{
		return FALSE;
	}
//    out_led1.Invert();

	return TRUE;
}

void CSerialMIDI::FlushMIDIOut(void)
{
	m_Serial.TX_Flush();
}

