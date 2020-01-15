
#include "LiquidCrystal.h"
#include "global_obj.h"
#include <circle/util.h>
#include <circle/string.h>

inline void delay_ms(unsigned int ms) {
	// PreenFM2_uDelay(ms * 1000);
}

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//

// #define SET(pin) GPIO_SetBits( pin.pinNumber)
// #define RESET(pin) GPIO_ResetBits( pin.pinNumber)

// This library has been modified to be compatible with STM32F4 library
// and specialized for the PreenFM mk2
// By Xavier [ dot ] Hosxe (at) g m a i l [ dot] com

LiquidCrystal::LiquidCrystal() {
	/* GPIOA clock enable */
	// RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	struct pin pins[6]  = {
		// { GPIOA, GPIO_Pin_0 },
		// { GPIOA, GPIO_Pin_1 },
		// { GPIOA, GPIO_Pin_2 },
		// { GPIOA, GPIO_Pin_3 },
		// { GPIOA, GPIO_Pin_4 },
		// { GPIOA, GPIO_Pin_5 }
	};

	realTimeDisplay = true;
	// For bootloader and firmware
	delayAfterCommand = 50;

	_rs_pin = pins[0];
	_enable_pin = pins[1];

	_data_pins[0] = pins[2];
	_data_pins[1] = pins[3];
	_data_pins[2] = pins[4];
	_data_pins[3] = pins[5];

	/* GPIOB clock enable */


	// GPIO_InitTypeDef GPIO_InitStructure;
	// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	// GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	// GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// // We assume all pins are same GPIO....
	// GPIO_InitStructure.GPIO_Pin = pins[0].pinNumber | pins[1].pinNumber | pins[2].pinNumber | pins[3].pinNumber | pins[4].pinNumber | pins[5].pinNumber;
	// GPIO_Init(pins[0].gpio, &GPIO_InitStructure);

}

void LiquidCrystal::SetGUI(CScreenDeviceX *GUI){
	xGUI = GUI;
//	xGUI->setupLCDArea();
}
 

void LiquidCrystal::begin(unsigned char cols, unsigned char lines) {
	_numlines = lines;
	_currline = 0;

	// // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!  according to
	// // datasheet, we need at least 40ms after power rises above 2.7V
	// // before sending commands. Arduino can turn on way befer 4.5V so
	// // we'll wait 50
	// delay_ms(50);

	// // Init 4 bits communication

	// // Now we pull both RS and R/W low to begin commands
	// RESET(_rs_pin);
	// RESET(_enable_pin);


	// // this is according to the hitachi HD44780 datasheet
	// // page 45 figure 23


	// // Send function set command sequence
	// sendInitCommand(LCD_FUNCTIONSET | LCD_8BITMODE);
	// delay_ms(5); // wait more than 4.1ms

	// // second try
	// sendInitCommand(LCD_FUNCTIONSET | LCD_8BITMODE);
	// delay_ms(1);

	// // third go
	// sendInitCommand(LCD_FUNCTIONSET | LCD_8BITMODE);
	// delay_ms(1);

	// // And set the 4 bits mode
	// sendInitCommand(LCD_FUNCTIONSET | LCD_4BITMODE);
	// delay_ms(1);


	// if (lines > 1) {
		// _displayfunction = LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
	// } else {
		// _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	// }

	// // finally, set # lines, font size, etc.
	// command(LCD_FUNCTIONSET | _displayfunction );

	// command(LCD_DISPLAYCONTROL);

	// // clear
    // command(LCD_CLEARDISPLAY); // clear display, set cursor position to zero
    // delay_ms(2);

	// // Initialize to default text direction (for romance languages)
	// _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// // set the entry mode
	// command(LCD_ENTRYMODESET | _displaymode);

	// // turn the display on with no cursor or blinking default
	// _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	 // display();

	char zzz[20];
	zzz[0] = '\x1b';
	zzz[1] = '[';
	zzz[2] = 'H';
	xGUI->Write(zzz,3);
	zzz[0] = '\x1b';
	zzz[1] = '[';
	zzz[2] = 'J';
	xGUI->Write(zzz,3);


#ifndef BOOTLOADER
	delayAfterCommand = 38;
#endif

}



/********** high level commands, for the user! */

void LiquidCrystal::setRealTimeAction(bool realTime) {
	 this->realTimeDisplay = realTime; 
}


void LiquidCrystal::clear() {
    if (this->realTimeDisplay) {
		xGUI->ClearDisplay();

		// u16 h = xGUI->GetHeight();
		// u16 w = xGUI->GetWidth();

		// for (u16 x=0;x < w;x++){
		// 	for (u16 y=0;y<h;y++){
		// 		xGUI->SetPixel(x,y,BLACK_COLOR);
		// 	}
		// }


    } else {
        struct LCDAction action;
        action.value = 0;
        action.mode = 0;
        action.clear = 1;
        lcdActions.insert(action);
    }
}

void LiquidCrystal::realTimeAction(LCDAction *action, void (*callback)()) {
    if (action->clear > 0) {
        // for (int i=0; i<4; i++) {
    	callback();
        //     setCursor(0, i);
        //     for (int k=0; k<20; k++) {
        //         callback();
        //         print(' ');
        //     }
        // }
		clear();
    } else {
        callback();
        send(action->value, action->mode);
    }
}



void LiquidCrystal::realTimeAction(LCDAction *action) {
    if (action->clear > 0) {
		clear();
    } else {
        send(action->value, action->mode);
    }
}


void LiquidCrystal::home() {

	char zzz[20];
	memset (zzz,0, sizeof(zzz));
	zzz[0] = '\x1b';
	zzz[1] = '[';
	zzz[2] = 'H';
	xGUI->Write(zzz,3);

//	xGUI->setCursorLCDArea(0,0);
	// command(LCD_RETURNHOME); // set cursor position to zero
	// delay_ms(2); // Maple mod
	//delayMicroseconds(2000);  // this command takes a long time!
}

void LiquidCrystal::setCursor(unsigned char col, unsigned char row) {
	// int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

	// if (row > _numlines) {
	// 	row = _numlines - 1; // we count rows starting w/0
	// }

	// if (col > 20){
	// 	col = 0;
	// } 

	char zzz[20];
	char ccol[10];
	char crow[10];

	memset (ccol,0, sizeof(ccol));
	memset (crow,0, sizeof(crow));
	memset (zzz,0, sizeof(zzz));

	itoa(col+3,ccol,10);			// colums & Rows werden von 1 an gezählt 
	itoa(row+2,crow,10);


	zzz[0] = '\x1b';
	zzz[1] = '[';
	strcpy(&zzz[2],crow);
	zzz[2+strlen(crow)] = ';';
	strcpy(&zzz[3+strlen(crow)],ccol);
	zzz[3+strlen(crow)+strlen(ccol)] = 'H';
	xGUI->Write(zzz,strlen (zzz));

// 	if (row > _numlines) {
// //		xGUI->setCursorLCDArea(col,row);
// 		// row = _numlines - 1; // we count rows starting w/0
// 	}

	// command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	// command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::display() {
	_displaycontrol |= LCD_DISPLAYON;
	// command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal::noCursor() {
	char zzz[20];
	memset (zzz,0, sizeof(zzz));

	_displaycontrol &= ~LCD_CURSORON;
	zzz[0] = '\x1b';
	zzz[1] = '[';
	zzz[2] = '?';
	zzz[3] = '2';
	zzz[4] = '5';
	zzz[5] = 'l';
	xGUI->Write(zzz,strlen (zzz));
	// command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::cursor() {
	char zzz[20];
	memset (zzz,0, sizeof(zzz));

	_displaycontrol |= LCD_CURSORON;
	zzz[0] = '\x1b';
	zzz[1] = '[';
	zzz[2] = '?';
	zzz[3] = '2';
	zzz[4] = '5';
	zzz[5] = 'h';
	xGUI->Write(zzz,strlen (zzz));
	// command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	// command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal::blink() {
	_displaycontrol |= LCD_BLINKON;
	// command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal::scrollDisplayLeft(void) {
	// command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidCrystal::scrollDisplayRight(void) {
	// command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	// command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	// command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	// command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	// command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal::createChar(unsigned char location,
		unsigned char charmap[]) {
		xGUI->SetCustomChar(location,charmap);
}

/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystal::command(unsigned char value) {
	 send(value, false);
}

inline void LiquidCrystal::write(unsigned char value) {
	if (value == '/'){
		value = 8;
//		value +=1;
	}else if( value == 2){
		value = 0;
	}
	send(value, true);
}




void LiquidCrystal::print(char c) {
	char buff[10];

	if (c == 0){				// midiIn
		// char buff[] ={LV_SYMBOL_CHARGE};	
		// print (buff);
//		c ='\\';	
    	write(c);
 	}
	else if (c == 1){		// minusPoint	// backslash
//		c ='\\';	
    	write(c);
//	   	write('!');
	}
	else if (c == 2){			
//		c ='\\';	
    	write(c);
//    	write(c+1);
//		c ='-';		
//		c ='z';		
				
	}
	else if (c == 3){			// stepCursor
		// char buff[] ={LV_SYMBOL_UP};
		// print (buff);
//		c ='\\';	
    	write(c);
	}
	else if (c == 4){				// stepPos
		// char buff[] ={LV_SYMBOL_DOWN};
		// print (buff);
    	write(c);
	}		
	else if (c == 5){			// firstSteps
		// char buff[] ={LV_SYMBOL_PREV};
		// print (buff);
    	write(c);
	}
	else if (c == 6){			// thirdStep
		// char buff[] ={LV_SYMBOL_CHARGE};
		// print (buff);
    	write(c);
	}
	else if (c == 7){			// note
		// char buff[] ={LV_SYMBOL_AUDIO};	
    	write(c);
		// print (buff);
	}else{

    	write(c);		
	}
}

void LiquidCrystal::print(const char str[]) {
    while (*str) {
        write(*str++);
    }
}

void LiquidCrystal::print(int n) {
	unsigned char buf[12];
	unsigned long i = 0;

	if (n == 0) {
	    print('0');
	    return;
	}

	if (n < 0) {
	    print('-');
	    n = -n;
	}

	while (n > 0) {
	    buf[i++] = n % 10;
	    n /= 10;
	}

	for (; i > 0; i--) {
	    print((char)('0' + buf[i - 1]));
	}
}

void LiquidCrystal::print(unsigned int n) {
	unsigned char buf[12];
	unsigned long i = 0;

	if (n == 0) {
	    print('0');
	    return;
	}

	if (n < 0) {
	    print('-');
	    n = -n;
	}

	while (n > 0) {
	    buf[i++] = n % 10;
	    n /= 10;
	}

	for (; i > 0; i--) {
	    print((char)('0' + buf[i - 1]));
	}
}

void LiquidCrystal::print(float f) {
    if (f < 0.0) {
        print((char)2);
//        print('-');
        f = - f;
    } else {
        print(' ');
    }
    if (f < 10.0f) {
        int integer = (int) f;
        print((char)((char)integer+'0'));
        print('.');
        f -= integer;
        int valueTimes100 = (int)(f*100+.0005);
        if (valueTimes100 < 10) {
            print("0");
            print(valueTimes100);
        } else {
            print(valueTimes100);
        }
    } else {
        int integer = (int) f;
        print(integer);
        print('.');
        f -= integer;
        print((int)(f*10));
    }
}

void LiquidCrystal::printWithOneDecimal(float f) {

    if (f < 10.0f) {
        print('0');
    }
    int integer = (int) f;
    print(integer);
    print('.');
    f -= integer;
    int valueTimes10 = (int)(f*10+.0005);
    print(valueTimes10);
}



/************ low level data pushing commands **********/


void LiquidCrystal::sendInitCommand(unsigned char value) {
//	RESET(_rs_pin);

	write4bits(value >> 4);
	pulseEnable(delayAfterCommand);
}


// write either command or data, with automatic 4/8-bit selection
void LiquidCrystal::send(unsigned char value, bool mode) {
	char xxx[3];
	if (this->realTimeDisplay) {
		xxx[0]=value;
		xxx[1]=0;
		xGUI->Write(xxx,1);
//		if (mode) {
//			xGUI->writeLCDArea(value);
//		} else {
			// RESET(_rs_pin);
//		}
	} else {
		if (!lcdActions.isFull()) {
			struct LCDAction action;
			action.value = value;
			action.mode = mode;
			action.clear = 0;
			lcdActions.insert(action);
		}
	}
}

LCDAction LiquidCrystal::nextAction() {
    return lcdActions.remove();
}


void LiquidCrystal::pulseEnable(int delay) {
	// _enable_pin should already be LOW (unless someone else messed
	// with it), so don't sit around waiting for long.
	// RESET(_enable_pin);
	// PreenFM2_uDelay(1);

	// Enable pulse must be > 450 ns.  Value chosen here according to
	// the following threads:
	// http://forums.leaflabs.com/topic.php?id=640
	// http://forums.leaflabs.com/topic.php?id=512
	// SET(_enable_pin);
	// PreenFM2_uDelay(1);
	// RESET(_enable_pin);
	// // Commands needs > 37us to settle.
	// PreenFM2_uDelay(delay);
}

void LiquidCrystal::write4bits(unsigned char value) {
	// for (int i = 0; i < 4;	 i++) {
		// if ((value >> i) & 0x01) {
			// SET(_data_pins[i]);
		// } else {
			// RESET(_data_pins[i]);
		// }
	// }
}

