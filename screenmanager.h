#ifndef _lvgscreen_h
#define _lvgscreen_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/serial.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/input/touchscreen.h>
#include <littlevgl/littlevgl.h>
#include <circle/types.h>
#include "utils/RingBuffer.h"

#define CANVAS_WIDTH  300
#define CANVAS_HEIGHT 170


enum ScreenCMD {
	HELLO_WORLD =0,
	ROTATE,
	NOTE_ON,
	NOTE_OFF
};

struct ScreenMsg{
	ScreenCMD cmd = (ScreenCMD)0;
	s32 uParam0 = 0;
	double dParam1 = 0;
	char *pString;
};



class RaspiScreen
{
public:
	RaspiScreen (void);
	~RaspiScreen (void);

	void Initialize (m_Screen	*pm_GUI);
	void HelloWorld (void);
	void setText (char*);
	void setText2 (char*);
	void Rotate (void);
	void Rotate (u8 angle);
	void Updatex (void);
	bool PutScreen (ScreenMsg SMsg);
	bool PutScreen (ScreenCMD cmd,s32 uParam0 = 0,double dParam1 = 0.0, char *pString=NULL);
	void DecodeCommand (ScreenMsg	ScreenCommand);
	void setupLCDArea (void);
	void writeLCDArea (char);
	void setCursorLCDArea (uint8_t col, uint8_t row);
	void clearLCDArea (void);
	void writeLCDAreatest (char c);
	void writeLCDCanvas (char c);
	void writeLCDCanvasStr(char*);
	void setCursorLCDCanvas (uint8_t col, uint8_t row);
	void clearLCDCanvas (void);

private:
    lv_obj_t *scr;
    lv_obj_t *label1; 
    lv_obj_t *label2; 
    lv_img_dsc_t img;

    lv_style_t style1;
    lv_style_t RubOutStyle;

	lv_obj_t * LCDCanvas;

	lv_obj_t * LCDArea;
	lv_obj_t * LCDAreatest;

    lv_obj_t * LCDPage; 
    lv_obj_t * LCDLabel; 

	uint16_t LCDCanvas_Cursor=0;
	uint16_t m_angle=0;
	RingBuffer<ScreenMsg,256> m_ScreenFifo;
	CLittlevGL	*pm_GUI;
	ScreenMsg m_ScreenMsg;

	uint16_t TopMargin = 30;
	uint16_t LeftMargin = 10;
	lv_color_t LCDCanvasBuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];

	char testtext[20]={"TESTTEXT"};
	
	// do not change this order
};

extern RaspiScreen m_LVGScreen;



#endif
