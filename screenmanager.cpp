#include "screenmanager.h"
#include "global_obj.h"


char BufferTextArea[200];
char BufferTextArea_X[200] = {"012345678911234567892123456789312345678941234567895123456789"};




RaspiScreen::RaspiScreen(void){
    
}

RaspiScreen::~RaspiScreen(void){
}

void RaspiScreen::Updatex (void){
    while (m_ScreenFifo.getCount() > 0){
        DecodeCommand(m_ScreenFifo.remove());
    }   
    pm_GUI->Update();
}

void RaspiScreen::setupLCDArea (void){

    scr = lv_disp_get_scr_act(NULL);     /*Get the current screen*/

    /*Create a Label on the currently active screen*/
    label1 =  lv_label_create(scr, NULL);
    label2 =  lv_label_create(scr, NULL);

    /*Modify the Label's text*/
    lv_label_set_text(label1, "Hello world!");
    lv_label_set_text(label2, "testtest");

    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 200, 150);
    lv_obj_align(label2, NULL, LV_ALIGN_CENTER, 200, 75);


    lv_style_copy(&style1, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
    style1.body.main_color = LV_COLOR_WHITE;
    style1.body.border.color = LV_COLOR_GRAY;
    style1.body.border.width = 2;
    style1.body.padding.left = 5;            /*Horizontal padding, used by the bar indicator below*/
    style1.body.padding.right = 5;
    style1.body.padding.top = 5;            /*Vertical padding, used by the bar indicator below*/
    style1.body.padding.bottom = 5;
    style1.text.color = LV_COLOR_BLACK;
    style1.text.font = &lv_font_unscii_8;
//    style1.text.font = &lv_font_roboto_16;

    lv_style_copy(&RubOutStyle, &lv_style_plain);
    RubOutStyle.body.main_color = LV_COLOR_WHITE;
    RubOutStyle.text.color = LV_COLOR_BLACK;
    RubOutStyle.text.font = &lv_font_unscii_8;

    LCDArea = lv_ta_create(lv_scr_act(), NULL);
//    lv_obj_set_size(LCDArea, 333, 200);
    lv_obj_set_size(LCDArea, 170, 200);
    lv_obj_set_pos(LCDArea,20,20);


//    lv_obj_align(LCDArea, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_ta_set_cursor_type(LCDArea, LV_CURSOR_BLOCK);
    lv_ta_set_style(LCDArea, LV_TA_STYLE_BG, &style1);
    lv_ta_set_text(LCDArea, "012345678911234567892123456789312345678941234567895123456789");    /*Set an initial text*/

    LCDCanvas = lv_canvas_create(scr, NULL);
    lv_canvas_set_buffer(LCDCanvas, LCDCanvasBuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
//    lv_obj_align(LCDCanvas, NULL, LV_ALIGN_CENTER, 200, 0);
    lv_obj_set_pos(LCDCanvas,450,50);
    lv_canvas_fill_bg(LCDCanvas, LV_COLOR_WHITE);

    lv_canvas_set_style(LCDCanvas, LV_CANVAS_STYLE_MAIN, &style1);
    lv_canvas_draw_rect(LCDCanvas, 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, &style1);

//    lv_canvas_draw_text(LCDCanvas, 2, 40, 200, &style1, "XXXXXXXXXXXXXXX", LV_LABEL_ALIGN_LEFT);

//     writeLCDCanvasStr ("JöLKJöLKJöLKJöLK");


//     lv_style_copy(&style1, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
//     style1.body.main_color = LV_COLOR_WHITE;
//     style1.body.border.color = LV_COLOR_GRAY;
//     style1.body.border.width = 2;
//     style1.body.padding.left = 5;            /*Horizontal padding, used by the bar indicator below*/
//     style1.body.padding.right = 5;
//     style1.body.padding.top = 5;            /*Vertical padding, used by the bar indicator below*/
//     style1.body.padding.bottom = 5;
//     style1.text.color = LV_COLOR_BLACK;
//     style1.text.font = &lv_font_unscii_8;

//     lv_style_copy(&RubOutStyle, &lv_style_plain);
//     RubOutStyle.body.main_color = LV_COLOR_WHITE;
//     RubOutStyle.text.color = LV_COLOR_BLACK;
//     RubOutStyle.text.font = &lv_font_unscii_8;

//     LCDArea = lv_ta_create(lv_scr_act(), NULL);
// //    lv_obj_set_size(LCDArea, 333, 200);
//     lv_obj_set_size(LCDArea, 170, 200);
//     lv_obj_set_pos(LCDArea,20,20);


// //    lv_obj_align(LCDArea, NULL, LV_ALIGN_CENTER, 0, 0);
//     lv_ta_set_cursor_type(LCDArea, LV_CURSOR_BLOCK);
//     lv_ta_set_style(LCDArea, LV_TA_STYLE_BG, &style1);
//     lv_ta_set_text(LCDArea, "012345678911234567892123456789312345678941234567895123456789");    /*Set an initial text*/
// //    lv_ta_set_text(LCDArea, "");    /*Set an initial text*/
// //    lv_obj_set_event_cb(ta1, event_handler);

//     LCDAreatest = lv_ta_create(lv_scr_act(), NULL);
// //    lv_obj_set_size(LCDArea, 333, 200);
//     lv_obj_set_size(LCDAreatest, 170, 200);
//     lv_obj_set_pos(LCDAreatest,250,120);


// //    lv_obj_align(LCDArea, NULL, LV_ALIGN_CENTER, 0, 0);
//     lv_ta_set_cursor_type(LCDAreatest, LV_CURSOR_BLOCK);
//     lv_ta_set_style(LCDAreatest, LV_TA_STYLE_BG, &style1);
//     lv_ta_set_text(LCDAreatest, BufferTextArea_X);    /*Set an initial text*/


// //     LCDPage = lv_page_create(lv_scr_act(), NULL);
// //     lv_obj_set_size(LCDPage, 170, 100);
// //     lv_obj_set_pos(LCDPage,500,20);
// // //    lv_obj_align(LCDPage, NULL, LV_ALIGN_CENTER, 0, 0);
// // //    lv_page_set_style(LCDPage, LV_PAGE_STYLE_SB, &style_sb);           /*Set the scrollbar style*/


// //     LCDLabel

// //     lv_obj_t * cont;

// //     cont = lv_cont_create(lv_scr_act(), NULL);
// //     // lv_obj_set_auto_realign(cont, true);                    /*Auto realign when the size changes*/
// //     // lv_obj_align_origo(cont, NULL, LV_ALIGN_CENTER, 0, 0);  /*This parametrs will be sued when realigned*/
// //     // lv_cont_set_fit(cont, LV_FIT_TIGHT);
// //     // lv_cont_set_layout(cont, LV_LAYOUT_COL_M);
// //     lv_obj_set_pos(cont,500,50);
// //     lv_obj_set_size(cont, 170, 100);
// //     lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &style1);

// //     lv_obj_t * label;
// //     label = lv_label_create(cont, NULL);
// //     lv_obj_set_pos(label,2,2);
// //     lv_obj_set_size(label, 165, 96);
// //     lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style1);
// // //    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK );
// //     lv_label_set_text(label, "012345678911234567892123456789312345678941234567895123456789");

//     LCDCanvas = lv_canvas_create(lv_scr_act(), NULL);
//     lv_canvas_set_buffer(LCDCanvas, LCDCanvasBuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
// //    lv_obj_align(LCDCanvas, NULL, LV_ALIGN_CENTER, 200, 0);
//     lv_obj_set_pos(LCDCanvas,420,50);
//     lv_canvas_fill_bg(LCDCanvas, LV_COLOR_WHITE);

//     lv_canvas_set_style(LCDCanvas, LV_CANVAS_STYLE_MAIN, &style1);
//     lv_canvas_draw_rect(LCDCanvas, 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, &style1);

//     lv_canvas_draw_text(LCDCanvas, 2, 40, 200, &style1, "XXXXXXXXXXXXXXX", LV_LABEL_ALIGN_LEFT);
//     // lv_canvas_draw_text(LCDCanvas, 80, 20, 100, &style1, "text", LV_LABEL_ALIGN_LEFT);
//     // lv_canvas_draw_text(LCDCanvas, 100, 20, 100, &style1, "on text canvas", LV_LABEL_ALIGN_LEFT);
//     // lv_canvas_draw_text(LCDCanvas, 110, 20, 100, &style1, "           ", LV_LABEL_ALIGN_LEFT);
//     // lv_canvas_draw_rect(LCDCanvas, 110, 20, sizeof("           ") * 8, 9, &RubOutStyle);
//     // // lv_canvas_draw_text(canvas, 20, 80, 100, &style1, "xxxxxxxx", LV_LABEL_ALIGN_LEFT);
//     // lv_canvas_draw_text(LCDCanvas, 2, 80, 200, &style1, "01234567891123456789", LV_LABEL_ALIGN_LEFT);

//     /* Test the rotation. It requires an other buffer where the orignal image is stored.
//      * So copy the current image to buffer and rotate it to the canvas */
//     // lv_color_t cbuf_tmp[CANVAS_WIDTH * CANVAS_HEIGHT];
//     // memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));
//     // lv_img_dsc_t img;
//     // img.data = (void *)cbuf_tmp;
//     // img.header.cf = LV_IMG_CF_TRUE_COLOR;
//     // img.header.w = CANVAS_WIDTH;
//     // img.header.h = CANVAS_HEIGHT;

//     // lv_canvas_fill_bg(canvas, LV_COLOR_SILVER);
//     // lv_canvas_rotate(canvas, &img, 30, 0, 0, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);


}

//void RaspiScreen::writeLCDCanvas (char c){
void RaspiScreen::writeLCDArea (char c){
    uint16_t x = (LCDCanvas_Cursor % 20)*8;
    uint16_t y = (LCDCanvas_Cursor / 20)*8;
    char text[2];

    text[0]= c;
    text[1]= 0;    
    if (LCDCanvas != NULL){
        lv_canvas_set_buffer(LCDCanvas, LCDCanvasBuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
        lv_canvas_draw_rect(LCDCanvas, LeftMargin + x, TopMargin +y, 8, 8, &RubOutStyle);
        lv_canvas_draw_text(LCDCanvas, LeftMargin + x, TopMargin +y, 9, &style1, text, LV_LABEL_ALIGN_LEFT);
    }
    LCDCanvas_Cursor++;
    if (LCDCanvas_Cursor > 79)
        LCDCanvas_Cursor = 79;
//    out_led2.Invert();

} 
 
void RaspiScreen::writeLCDCanvasStr (char *text){
    uint16_t x = (LCDCanvas_Cursor % 20)*8;
    uint16_t y = (LCDCanvas_Cursor / 20)*8;
    uint16_t len = strlen (text);
    // char text[2];
    // text[0]= c;
    // text[1]= 0;    

    if (LCDCanvas != NULL){
        lv_canvas_set_buffer(LCDCanvas, LCDCanvasBuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
        lv_canvas_draw_rect(LCDCanvas, LeftMargin + x, TopMargin +y, len * 9, 8, &RubOutStyle);
        lv_canvas_draw_text(LCDCanvas, LeftMargin + x, TopMargin +y, len * 9, &style1, text, LV_LABEL_ALIGN_LEFT);
    }else{
//        out_led2.Invert();
    }
//    lv_canvas_draw_text(LCDCanvas, 2, 40, 200, &style1, "XXXXXXXXXXXXXXX", LV_LABEL_ALIGN_LEFT);
    LCDCanvas_Cursor += len;
    if (LCDCanvas_Cursor > 77)
        LCDCanvas_Cursor = 77;

}

// void RaspiScreen::writeLCDArea (char c){
//     uint16_t CurPos =lv_ta_get_cursor_pos(LCDArea);

//     BufferTextArea[0]= c;
//     BufferTextArea[1]= 0;
//     lv_ta_add_char(LCDArea, c);
// //    lv_ta_set_insert_replace(LCDArea, BufferTextArea);
// //    lv_ta_cursor_right(LCDArea);

// }

void RaspiScreen::writeLCDAreatest (char c){
    uint16_t CurPos =lv_ta_get_cursor_pos(LCDAreatest);

    BufferTextArea_X[0]= c;
    BufferTextArea_X[1]= 'X';
    BufferTextArea_X[2]= 'X';
    BufferTextArea_X[3]= 'X';
    BufferTextArea_X[4]= 0;
    lv_ta_set_cursor_pos(LCDAreatest, 20);
//    lv_ta_add_text(LCDAreatest, BufferTextArea_X);
    lv_ta_set_insert_replace(LCDAreatest, BufferTextArea_X);
//    lv_ta_add_char(LCDAreatest, c);
//    lv_ta_set_insert_replace(LCDArea, BufferTextArea);
//    lv_ta_cursor_right(LCDArea);

}

void RaspiScreen::setCursorLCDArea (uint8_t col, uint8_t row){
//void RaspiScreen::setCursorLCDCanvas (uint8_t col, uint8_t row){
    LCDCanvas_Cursor = col + (row*20);
}

void RaspiScreen::clearLCDArea (void){
//void RaspiScreen::clearLCDCanvas (void){
    LCDCanvas_Cursor = 0;
    lv_canvas_set_buffer(LCDCanvas, LCDCanvasBuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_draw_rect(LCDCanvas, 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, &style1);
}

// void RaspiScreen::setCursorLCDArea (uint8_t col, uint8_t row){
//     lv_ta_set_cursor_pos(LCDArea, col + (row*20));
// }

// void RaspiScreen::clearLCDArea (void){
//     lv_ta_set_text(LCDArea, "");
// }


void RaspiScreen::Initialize (CLittlevGL	*pm_GUI){
     this->pm_GUI = pm_GUI;
}

void RaspiScreen::DecodeCommand (ScreenMsg	ScreenCommand){
    if (ScreenCommand.cmd == HELLO_WORLD){

    }else if (ScreenCommand.cmd == ROTATE){
        Rotate(ScreenCommand.uParam0);

    }else if (ScreenCommand.cmd == NOTE_ON){
        setText("!!!NoTE on!!!");

    }else if (ScreenCommand.cmd == NOTE_OFF){
       setText("!!!NoTE off!!!");
    
    }
}

bool RaspiScreen::PutScreen (ScreenMsg SMsg){
    m_ScreenFifo.insert(SMsg);
    return true;
}

bool RaspiScreen::PutScreen (ScreenCMD cmd,s32 uParam0 ,double dParam1 , char *pString){
	m_ScreenMsg.cmd = cmd;
	m_ScreenMsg.uParam0 = uParam0;
 	m_ScreenMsg.dParam1 = dParam1;
	m_ScreenMsg.pString = pString;
    m_ScreenFifo.insert(m_ScreenMsg);
    return true;
}


void RaspiScreen::HelloWorld (void){
    scr = lv_disp_get_scr_act(NULL);     /*Get the current screen*/

    /*Create a Label on the currently active screen*/
    label1 =  lv_label_create(scr, NULL);
    label2 =  lv_label_create(scr, NULL);

    /*Modify the Label's text*/
    lv_label_set_text(label1, "Hello world!");
    lv_label_set_text(label2, "testtest");

    /* Align the Label to the center
     * NULL means align on parent (which is the screen now)
     * 0, 0 at the end means an x, y offset after alignment*/
    lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 200, 150);
    lv_obj_align(label2, NULL, LV_ALIGN_CENTER, 200, 75);


    lv_style_copy(&style1, &lv_style_plain);    /*Copy a built-in style to initialize the new style*/
    style1.body.main_color = LV_COLOR_WHITE;
    style1.body.border.color = LV_COLOR_GRAY;
    style1.body.border.width = 2;
    style1.body.padding.left = 5;            /*Horizontal padding, used by the bar indicator below*/
    style1.body.padding.right = 5;
    style1.body.padding.top = 5;            /*Vertical padding, used by the bar indicator below*/
    style1.body.padding.bottom = 5;
    style1.text.color = LV_COLOR_BLACK;
    style1.text.font = &lv_font_unscii_8;

    lv_style_copy(&RubOutStyle, &lv_style_plain);
    RubOutStyle.body.main_color = LV_COLOR_WHITE;
    RubOutStyle.text.color = LV_COLOR_BLACK;
    RubOutStyle.text.font = &lv_font_unscii_8;

    LCDArea = lv_ta_create(lv_scr_act(), NULL);
//    lv_obj_set_size(LCDArea, 333, 200);
    lv_obj_set_size(LCDArea, 170, 200);
    lv_obj_set_pos(LCDArea,20,20);


//    lv_obj_align(LCDArea, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_ta_set_cursor_type(LCDArea, LV_CURSOR_BLOCK);
    lv_ta_set_style(LCDArea, LV_TA_STYLE_BG, &style1);
    lv_ta_set_text(LCDArea, "012345678911234567892123456789312345678941234567895123456789");    /*Set an initial text*/
//    lv_ta_set_text(LCDArea, "");    /*Set an initial text*/
//    lv_obj_set_event_cb(ta1, event_handler);

    LCDAreatest = lv_ta_create(lv_scr_act(), NULL);
//    lv_obj_set_size(LCDArea, 333, 200);
    lv_obj_set_size(LCDAreatest, 170, 200);
    lv_obj_set_pos(LCDAreatest,250,120);


//    lv_obj_align(LCDArea, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_ta_set_cursor_type(LCDAreatest, LV_CURSOR_BLOCK);
    lv_ta_set_style(LCDAreatest, LV_TA_STYLE_BG, &style1);
    lv_ta_set_text(LCDAreatest, BufferTextArea_X);    /*Set an initial text*/


//     LCDPage = lv_page_create(lv_scr_act(), NULL);
//     lv_obj_set_size(LCDPage, 170, 100);
//     lv_obj_set_pos(LCDPage,500,20);
// //    lv_obj_align(LCDPage, NULL, LV_ALIGN_CENTER, 0, 0);
// //    lv_page_set_style(LCDPage, LV_PAGE_STYLE_SB, &style_sb);           /*Set the scrollbar style*/


//     LCDLabel

//     lv_obj_t * cont;

//     cont = lv_cont_create(lv_scr_act(), NULL);
//     // lv_obj_set_auto_realign(cont, true);                    /*Auto realign when the size changes*/
//     // lv_obj_align_origo(cont, NULL, LV_ALIGN_CENTER, 0, 0);  /*This parametrs will be sued when realigned*/
//     // lv_cont_set_fit(cont, LV_FIT_TIGHT);
//     // lv_cont_set_layout(cont, LV_LAYOUT_COL_M);
//     lv_obj_set_pos(cont,500,50);
//     lv_obj_set_size(cont, 170, 100);
//     lv_cont_set_style(cont, LV_CONT_STYLE_MAIN, &style1);

//     lv_obj_t * label;
//     label = lv_label_create(cont, NULL);
//     lv_obj_set_pos(label,2,2);
//     lv_obj_set_size(label, 165, 96);
//     lv_label_set_style(label, LV_LABEL_STYLE_MAIN, &style1);
// //    lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK );
//     lv_label_set_text(label, "012345678911234567892123456789312345678941234567895123456789");

    LCDCanvas = lv_canvas_create(scr, NULL);
    lv_canvas_set_buffer(LCDCanvas, LCDCanvasBuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
//    lv_obj_align(LCDCanvas, NULL, LV_ALIGN_CENTER, 200, 0);
    lv_obj_set_pos(LCDCanvas,450,50);
    lv_canvas_fill_bg(LCDCanvas, LV_COLOR_WHITE);

    lv_canvas_set_style(LCDCanvas, LV_CANVAS_STYLE_MAIN, &style1);
    lv_canvas_draw_rect(LCDCanvas, 0, 0, CANVAS_WIDTH, CANVAS_HEIGHT, &style1);

    lv_canvas_draw_text(LCDCanvas, 2, 40, 200, &style1, "XXXXXXXXXXXXXXX", LV_LABEL_ALIGN_LEFT);
    // lv_canvas_draw_text(LCDCanvas, 80, 20, 100, &style1, "text", LV_LABEL_ALIGN_LEFT);
    // lv_canvas_draw_text(LCDCanvas, 100, 20, 100, &style1, "on text canvas", LV_LABEL_ALIGN_LEFT);
    // lv_canvas_draw_text(LCDCanvas, 110, 20, 100, &style1, "           ", LV_LABEL_ALIGN_LEFT);
    // lv_canvas_draw_rect(LCDCanvas, 110, 20, sizeof("           ") * 8, 9, &RubOutStyle);
    // // lv_canvas_draw_text(canvas, 20, 80, 100, &style1, "xxxxxxxx", LV_LABEL_ALIGN_LEFT);
    // lv_canvas_draw_text(LCDCanvas, 2, 80, 200, &style1, "01234567891123456789", LV_LABEL_ALIGN_LEFT);

    /* Test the rotation. It requires an other buffer where the orignal image is stored.
     * So copy the current image to buffer and rotate it to the canvas */
    // lv_color_t cbuf_tmp[CANVAS_WIDTH * CANVAS_HEIGHT];
    // memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));
    // lv_img_dsc_t img;
    // img.data = (void *)cbuf_tmp;
    // img.header.cf = LV_IMG_CF_TRUE_COLOR;
    // img.header.w = CANVAS_WIDTH;
    // img.header.h = CANVAS_HEIGHT;

    // lv_canvas_fill_bg(canvas, LV_COLOR_SILVER);
    // lv_canvas_rotate(canvas, &img, 30, 0, 0, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);


// #define CANVAS_WIDTH  200
// #define CANVAS_HEIGHT  150

//     static lv_style_t style;
//     lv_style_copy(&style, &lv_style_plain);
//     style.body.main_color = LV_COLOR_RED;
//     style.body.grad_color = LV_COLOR_MAROON;
//     style.body.radius = 4;
//     style.body.border.width = 2;
//     style.body.border.color = LV_COLOR_WHITE;
//     style.body.shadow.color = LV_COLOR_WHITE;
//     style.body.shadow.width = 4;
//     style.line.width = 2;
//     style.line.color = LV_COLOR_BLACK;
//     style.text.color = LV_COLOR_BLUE;

//     static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];

//     canvas = lv_canvas_create(lv_scr_act(), NULL);
//     lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
//     lv_obj_align(canvas, NULL, LV_ALIGN_CENTER, 0, 0);
//     lv_canvas_fill_bg(canvas, LV_COLOR_SILVER);

//     lv_canvas_draw_rect(canvas, 70, 60, 100, 70, &style);

//     lv_canvas_draw_text(canvas, 40, 20, 100, &style, "Some text on text canvas", LV_LABEL_ALIGN_LEFT);

//     /* Test the rotation. It requires an other buffer where the orignal image is stored.
//      * So copy the current image to buffer and rotate it to the canvas */
//     lv_color_t cbuf_tmp[CANVAS_WIDTH * CANVAS_HEIGHT];
//     memcpy(cbuf_tmp, cbuf, sizeof(cbuf_tmp));
// //    lv_img_dsc_t img;
// //    img.data = (void *)cbuf_tmp;
//     img.data = (const uint8_t*)cbuf_tmp;
//     img.header.cf = LV_IMG_CF_TRUE_COLOR;
//     img.header.w = CANVAS_WIDTH;
//     img.header.h = CANVAS_HEIGHT;

//     lv_canvas_fill_bg(canvas, LV_COLOR_SILVER);
//     lv_canvas_rotate(canvas, &img, 30, 0, 0, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);

}

void RaspiScreen::setText (char* txt){
    lv_label_set_text(label1, txt);
}
void RaspiScreen::setText2 (char* txt){
    lv_label_set_text(label2, txt);
}

void RaspiScreen::Rotate (void){
    m_angle++;
//    lv_canvas_rotate(canvas, &img, m_angle, 0, 0, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);
}

void RaspiScreen::Rotate (u8 angle){
    m_angle = angle;
//    lv_canvas_rotate(canvas, &img, m_angle, 0, 0, CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);
}

RaspiScreen			m_LVGScreen;
