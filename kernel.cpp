//
// kernel.cpp
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
/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier <dot> hosxe (at) g m a i l <dot> com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
 * RasPreenFM2 
 * Ugly Hacked Port of PreenFM2 for baremetal Raspberry Pi 3B & Hifiberry DAC+ Audio card
 * Copyright 2019 styro2000
 *
 * Author: styro2000:  (uglyplugins (at) defekt (dot) ch
 * 
 * out_led1 out_led2 out_led3 are for testing purposes.
 * the 4 encoders are at ports 24-31 of the MAX6957
 * the buttons are on port 12-19, in the File hardware/Encoders.cpp
 * is the wiring described... 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * THIS IS ALPHA-SOFTWARE DONT USE IT FOR LIVE PURPOSES!
 * I AM NOT RESPONSIBLE FOR ANY TRAGEDIES AT GIGS :-)
 */


#include "kernel.h"
#include <circle/machineinfo.h>
// #include "screenmanager.h"
#include "global_obj.h"
// #include "PreenFM.h"
#include "Encoders.h"
#include "FMDisplay.h"
#include "Synth.h"
#include "MidiDecoder.h"
#include "Storage.h"
#include "Hexter.h"
#include "LiquidCrystal.h"

#define SPI_CLOCK_SPEED		2000000		// Hz
#define SPI_CPOL		0
#define SPI_CPHA		0
#define SPI_CHIP_SELECT		0		// 0 or 1, or 2 (for SPI1)
#define TEST_DATA_LENGTH	20		// number of data bytes transfered

SynthState         synthState;
Synth              synth;

LiquidCrystal      lcd ;
FMDisplay          fmDisplay ;
MidiDecoder        midiDecoder;
Encoders           encoders ;
Storage            usbKey ;
Hexter             hexter;

static const char FromKernel[] = "kernel";

const char* ProgVersion = "RasPreenFM2 0.05b from 2.11 " __DATE__ " " __TIME__;
const char* line2 = "By styro2000";
const char* line3 = "Based on PreenFM2";
const char* line4 = "By Xavier Hosxe";
const char* line5 = "and the Circle Framework";
const char* line6 = "By R. Stange";



CKernel::CKernel (void)
:	m_Screen (LV_HOR_RES_MAX, LV_VER_RES_MAX),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_USBHCI (&m_Interrupt, &m_Timer),
//	m_SerialMIDI (&m_Soundmanager,&m_Interrupt,FALSE),
	m_SerialMIDI (&m_Soundmanager,&m_Interrupt,TRUE),
//	m_DWHCI (&m_Interrupt, &m_Timer),
	m_I2CMaster (CMachineInfo::Get ()->GetDevice (DeviceI2CMaster)),
	m_SPIMaster (SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA),
	m_Soundmanager (&m_Interrupt,&m_SerialMIDI, &synth)
//	m_GUI (&m_Screen, &m_Interrupt)
{
	m_ActLED.Blink (5);	// show we are alive 
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
		bOK = m_USBHCI.Initialize ();
	}

	// if (bOK)
	// {
	// 	bOK = m_DWHCI.Initialize ();
	// }
	
	if (bOK)
	{
		bOK = m_I2CMaster.Initialize ();
	}

	if (bOK)
	{
		bOK = m_SPIMaster.Initialize ();
		InitializeMAX6957();
	}


	if (bOK)
	{
		lcd.SetGUI(&m_Screen);
	    lcd.begin(20, 4);

	}

	TScreenStatusX Statusx;
	Statusx = m_Screen.GetStatus();

	TScreenColor BGColor = 0x0088;
//	TScreenColor Color = 0x4282;
	TScreenColor Color = 0x4F82;

	Statusx.Color      = Color;
	Statusx.BGColor    = BGColor;
	m_Screen.SetStatus(Statusx);
	lcd.clear();
	lcd.home();

	uint16_t testonBootloader=0;

	testonBootloader=ReadInputMAX6957();
	testonBootloader=ReadInputMAX6957();
	if ((testonBootloader & 0xFF) != 0xFF){					// Bootloader
		m_Logger.Write (FromKernel, LogNotice, "Bootloader");
		m_Timer.SimpleMsDelay(2000);
	}




    synth.setSynthState(&synthState);
    fmDisplay.setSynthState(&synthState);
	fmDisplay.setSynth(&synth);
    midiDecoder.setSynthState(&synthState);
    midiDecoder.setVisualInfo(&fmDisplay);
    midiDecoder.setSynth(&synth);
    midiDecoder.setStorage(&usbKey);
	midiDecoder.setSerialMIDI(&m_SerialMIDI);

    // ---------------------------------------
    // Register listener

    // synthstate is updated by encoder change
    encoders.insertListener(&synthState);

    // fmDisplay and synth needs to be aware of synthState changes
    // synth must be first one, can modify param new value
    /// order of param listener is important... synth must be called first so it's inserted last.
    synthState.insertParamListener(&fmDisplay);
    synthState.insertParamListener(&midiDecoder);
    synthState.insertParamListener(&synth);
	
    // Synth must be second to ba called first (to update global tuning before it's displayed)
    synthState.insertMenuListener(&fmDisplay);
    synthState.insertMenuListener(&synth);
    // Synth can check and modify param new value
    synthState.insertParamChecker(&synth);

    synthState.setStorage(&usbKey);
    synthState.setHexter(&hexter);

    usbKey.init(synth.getTimbre(0)->getParamRaw(), synth.getTimbre(1)->getParamRaw(), synth.getTimbre(2)->getParamRaw(), synth.getTimbre(3)->getParamRaw());
    usbKey.getPatchBank()->setSysexSender(&midiDecoder);
    // usbKey and hexter needs to know if arpeggiator must be loaded and saved
    usbKey.getPatchBank()->setArpeggiatorPartOfThePreset(&synthState.fullState.midiConfigValue[MIDICONFIG_ARPEGGIATOR_IN_PRESET]);
    hexter.setArpeggiatorPartOfThePreset(&synthState.fullState.midiConfigValue[MIDICONFIG_ARPEGGIATOR_IN_PRESET]);
    usbKey.getConfigurationFile()->loadConfig(synthState.fullState.midiConfigValue);
	
    // initialize global tuning
    synth.updateGlobalTuningFromConfig();
		
    usbKey.getConfigurationFile()->loadScalaConfig(&synthState.fullState.scalaScaleConfig);

    // Load scala scales if enabled
    if (synthState.fullState.scalaScaleConfig.scalaEnabled) {
    	usbKey.getScalaFile()->loadScalaScale(&synthState.fullState.scalaScaleConfig);
    }

    synth.buildNewSampleBlock();
    synth.buildNewSampleBlock();


    float v1 = ((OneSynthParams*)synth.getTimbre(0)->getParamRaw())->env1b.releaseTime;
    float v2 = ((OneSynthParams*)synth.getTimbre(0)->getParamRaw())->env4b.releaseTime;
    ((OneSynthParams*)synth.getTimbre(0)->getParamRaw())->env1b.releaseTime = 1.1f;
    ((OneSynthParams*)synth.getTimbre(0)->getParamRaw())->env4b.releaseTime = 0.8f;

	m_Logger.Write (FromKernel, LogNotice, "ProgVersion: %s", ProgVersion);
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
//	m_Logger.Write (FromKernel, LogNotice, "sizeof int %d: ", sizeof(int));


	lcd.setCursor(1,1);
	lcd.print(ProgVersion);
	lcd.setCursor(1,2);
	lcd.print(line2);
	lcd.setCursor(1,3);
	lcd.print(line3);
	lcd.setCursor(1,4);
	lcd.print(line4);
	lcd.setCursor(1,5);
	lcd.print(line5);
	lcd.setCursor(1,6);
	lcd.print(line6);

	m_Timer.SimpleMsDelay(2000);

	lcd.noCursor();
	m_Screen.SetBigMode(1);
	lcd.clear();
	lcd.home();


    // Load default combo if any
    usbKey.getComboBank()->loadDefaultCombo();
    // Load User waveforms if any
    usbKey.getUserWaveform()->loadUserWaveforms();
    // In any case init tables
    synthState.propagateAfterNewComboLoad();



    fmDisplay.init(&lcd, &usbKey);

    int bootOption = synthState.fullState.midiConfigValue[MIDICONFIG_BOOT_START];

    if (bootOption == 0) {
        fmDisplay.displayPreset();
        fmDisplay.setRefreshStatus(12);
    } else {
        // Menu
        synthState.buttonPressed(BUTTON_MENUSELECT);
        // Load
        synthState.buttonPressed(BUTTON_MENUSELECT);
        if (bootOption == 1) {
        	// Bank
            synthState.buttonPressed(BUTTON_MENUSELECT);
        } else if (bootOption == 2) {
        	// Combo
            synthState.encoderTurned(0, 1);
            synthState.buttonPressed(BUTTON_MENUSELECT);
        } else if (bootOption == 3) {
        	// DX7
            synthState.encoderTurned(0, 1);
            synthState.encoderTurned(0, 1);
            synthState.buttonPressed(BUTTON_MENUSELECT);
        }
        // First preset...
        synthState.buttonPressed(BUTTON_MENUSELECT);
    }


	encoders.setButtonRegister(&ButtonEncoderRegister);
	InitializeHiFiBerry();

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	unsigned int nCount = 0;

	out_led1.Set(0);
	out_led2.Set(0);
//	out_led3.Set(0);


  	out_led2.Set(1);
	m_SerialMIDI.Initialize();

    int cpt = 0;
    if (synth.getSampleCount() < 192) {
        while (synth.getSampleCount() < 128 && cpt++<20)
            synth.buildNewSampleBlock();
    }
    cpt = 0;
    if (synth.getSampleCount() < 192) {
        while (synth.getSampleCount() < 128 && cpt++<20)
            synth.buildNewSampleBlock();
    }


	m_Soundmanager.Start();

	static char xxx[40];
	u8 testchar =33;
	u32 counter=0;
	u8 testlen =0;
//	m_Timer.SimpleMsDelay(2000);

  	out_led2.Set(0);
	
	unsigned int encoderTimer = 0;
	unsigned int tempoTimer = 0;
	unsigned int newPreenTimer = 0;
	u32 MAXRead = 0;

	while (1)
	{
	    newPreenTimer = preenTimer;
//		out_led2.Invert();

		if (counter%2000000 == 0){
//			m_GUI.Update();
		}
		if (counter%100000 == 0){
		}
		while (usartBufferIn.getCount() > 0) {
			out_led1.Invert();

			midiDecoder.newByte(usartBufferIn.remove());
		}
		if ((newPreenTimer - encoderTimer) > 60) {
			ButtonEncoderRegister = (uint16_t)ReadInputMAX6957();
//		  	out_led2.Set(1);
			encoders.checkStatus(synthState.fullState.midiConfigValue[MIDICONFIG_ENCODER]);
// 			out_led2.Set(0);
			encoderTimer = newPreenTimer;
		} else if (fmDisplay.needRefresh()) {
			fmDisplay.refreshAllScreenByStep();
		}




		if ((newPreenTimer - tempoTimer) > 10000) {
			synthState.tempoClick();
			fmDisplay.tempoClick();
			tempoTimer = newPreenTimer;
		}

		if (m_Soundmanager.m_clipping != 0){
			fmDisplay.audioClipping();
			m_Soundmanager.m_clipping=0;
		}


		counter++;
		while (lcd.hasActions()) {
			if (usartBufferIn.getCount() > 20) {
				while (usartBufferIn.getCount() > 0) {
					midiDecoder.newByte(usartBufferIn.remove());
				}
			}
			LCDAction action = lcd.nextAction();
			lcd.realTimeAction(&action);
		}
	}

	return ShutdownHalt;
}

void fillSoundBuffer() {
    // int cpt = 0;
    // if (synth.getSampleCount() < 192) {
    //     while (synth.getSampleCount() < 128 && cpt++<20)
    //         synth.buildNewSampleBlock();
    // }
}



void CKernel::InitializeHiFiBerry(void){
	u8 i2c_buffer[20];
	// Reset PCM5121 
	i2c_buffer[0] = 1;
	i2c_buffer[1] = 0x11;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);

	i2c_buffer[0] = 1;
	i2c_buffer[1] = 0x00;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);

	// set Stand-by
	i2c_buffer[0] = 0x02;
	i2c_buffer[1] = 0x10;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);

	// set Powerdown
	i2c_buffer[0] = 0x02;
	i2c_buffer[1] = 0x11;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);
	
	// Stand-by & Powerdown Release
	i2c_buffer[0] = 0x02;
	i2c_buffer[1] = 0x00;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);
	
	// init GPIO 
	i2c_buffer[0] = 8;
	i2c_buffer[1] = 0x1C;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);

	// GPIO4 Output Selection Register GPIO4 output (Page 0 / Register 86, bit 3)
	i2c_buffer[0] = 83;
	i2c_buffer[1] = 0x02;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);

	// GPIO Output Control
	i2c_buffer[0] = 86;
	i2c_buffer[1] = 0x1C;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);

	// Stand-by request
	i2c_buffer[0] = 0x02;
	i2c_buffer[1] = 0x10;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);
	
	// Ignore Clock Halt Detection   0x25
	i2c_buffer[0] = 37;
	i2c_buffer[1] = 0x08;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);

	// Set PLL Clock Source to be BCK instead of SCK  0x0D
	i2c_buffer[0] = 13;
	i2c_buffer[1] = 0x10;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);
	
	// Stand-by release
	i2c_buffer[0] = 0x02;
	i2c_buffer[1] = 0x00;
	m_I2CMaster.Write(0x4D,i2c_buffer,2);
	m_Timer.SimpleusDelay(100);

  	m_Timer.SimpleMsDelay(50);

}


void CKernel::InitializeMAX6957(void){

	unsigned int nCount = 0;

// ENCODER A P26/27 
// ENCODER B P24/25 
// ENCODER C P22/23 
// ENCODER D P30/31 



	SPITxData[0] = 0x0B;		//	Port P15-P12
	SPITxData[1] = 0xFF;		//	P15-P12 Input Pullup
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}
	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);


	SPITxData[0] = 0x0C;		//	Port P19-P16
	SPITxData[1] = 0xFF;		//  P19-P16 Input Pullup
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 3");
	}
	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);

	SPITxData[0] = 0x0D;		//	Port P23-P20
	SPITxData[1] = 0xF3;		//	P23 P22 Input Pullup P21 LED P20 Input Pullup
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}
	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);

	SPITxData[0] = 0x0E;		//	Port P27-P24
	SPITxData[1] = 0xFF;		//	P27-P24 Input Pullup
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}
	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);

	SPITxData[0] = 0x0F;		//	Port P31-P28
	SPITxData[1] = 0xFF;		//	P31-P28 Input Pullup
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}
	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);


	SPITxData[0] = 0x02;		//	Global Current
	SPITxData[1] = 0xFF;		// MAX
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 1");
	}
	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);



	SPITxData[0] = 0x04;		//	COnfiguration
	SPITxData[1] = 0x01;		//	No Transition Detection Global Current Disable Shutdown
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 2");
	}
	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);

	SPITxData[0] = 0x35;		//	Set Ports 21
	SPITxData[1] = 0x01;
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 5");
	}
	m_Timer.SimpleusDelay(1000);

	SPITxData[0] = 0x35;
	SPITxData[1] = 0x00;		//	REset Ports 21
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 5");
	}

	SPITxData[0] = 0x0D;		//	Port P23-P20
	SPITxData[1] = 0xFF;		//	P23 P22 P21 P20 Input Pullup 
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}
	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);



}

uint16_t CKernel::ReadInputMAX6957(void){

	uint16_t ret = 0x0000;

	memset(SPITxData,0, sizeof(SPITxData));
	memset(SPIRxBuffer,0, sizeof(SPITxData));

	SPITxData[0] = 0xCC;		//	Read Port P12-19 Switches 
	SPITxData[1] = 0x00;		//	
	SPIRxBuffer[1] = 0;	
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}

	m_Timer.SimpleusDelay(100);

	memset(SPITxData,0, sizeof(SPITxData));
	memset(SPIRxBuffer,0, sizeof(SPITxData));

	SPITxData[0] = 0xD8;		//	Read Port P31-P24 Encoders
	SPITxData[1] = 0x00;		//	
	SPIRxBuffer[1] = 0;	
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}

	m_Timer.SimpleusDelay(100);
	ret = SPIRxBuffer[1];

	memset(SPITxData,0, sizeof(SPITxData));
	memset(SPIRxBuffer,0, sizeof(SPITxData));

	SPITxData[0] = 0xD4;		//	Read Port P20 -P23 Encoder Switchs
	SPITxData[1] = 0x00;		//	
	SPIRxBuffer[1] = 0;	
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}

	m_Timer.SimpleusDelay(100);
	ret |= SPIRxBuffer[1]<<8;

	memset(SPITxData,0, sizeof(SPITxData));
	memset(SPIRxBuffer,0, sizeof(SPITxData));

	SPITxData[0] = 0xD4;		//	Read Port P20 -P23 Encoder Switchs
	SPITxData[1] = 0x00;		//
	SPIRxBuffer[1] = 0;	
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}

	m_Timer.SimpleusDelay(100);
	
	if ((SPIRxBuffer[1] & 0x0F)!= 0x0F){
		ret &= ~(1 << 7);
	}

	return (ret);
}

void CKernel::writeLedMAX6957(uint8_t LED, uint8_t data){
	// LED = 0,1,2
	// u8 SPITxData[TEST_DATA_LENGTH];
	// u8 SPIRxBuffer[TEST_DATA_LENGTH];

	if (LED > 2)
		return;
	
	SPITxData[0] = 0x35;		//	Write Port P19-21 LEDs 
	SPITxData[1] = data;		//	
	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, SPITxData, SPIRxBuffer, 2) != 2)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error 4");
	}

	m_Timer.SimpleusDelay(100);
	m_Timer.SimpleusDelay(30);
}

