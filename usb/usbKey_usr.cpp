/*
 * Copyright 2013 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier . hosxe (at) gmail . com)
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
 * CallBack user file for  USB Key access from PreenFM 2
 * Copied and adapted from ST firmware library
 */
#include "usbKey_usr.h"
#include <fatfs/diskio.h>

#include "LiquidCrystal.h"
#include "global_obj.h"

volatile u8 usb_filesomething_hack;

extern LiquidCrystal lcd;

// Cannot be in class : used in PreenFM_irq.c and in usbh_msc_fatfs.c
// USB_OTG_CORE_HANDLE          usbOTGHost;
// USBH_HOST                    usbHost;

FATFS fatfs;
FRESULT res;
FILINFO fileinfo;
FIL fileR;
DIR dir;


struct usbCommandParam commandParams;


void displayFileError(const char* msg) {
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(msg);
    while (true);
}

#define DRIVE		"USB:"


int USB_Storage_Command(void) {
    unsigned int numberOfBytes;

    switch (commandParams.commandState) {
    case COMMAND_INIT:
	    if (f_mount (&fatfs, DRIVE, 1) != FR_OK){
            displayFileError("MOUNT ERROR, USB-Stick Not Found\n Please insert and Reboot!");
        }
        commandParams.commandState = COMMAND_NONE;
        commandParams.commandResult = COMMAND_SUCCESS;
        break;

    case COMMAND_LOAD:
//        out_led3.Set(1);
//        usb_filesomething_hack = 5;
        if (f_open(&fileR, commandParams.commandFileName,  FA_READ) != FR_OK) {
            displayFileError("OPEN ERROR");
        }

        f_lseek(&fileR, commandParams.commandSeek);
        f_read (&fileR, commandParams.commandParam1, commandParams.commandParamSize, &numberOfBytes);
        if (numberOfBytes != commandParams.commandParamSize) {
            displayFileError("READ ERROR");
        }
        if (f_close(&fileR) != FR_OK) {
            displayFileError("CLOSE ERROR");
        }
        commandParams.commandResult = COMMAND_SUCCESS;
        commandParams.commandState = COMMAND_NONE;
//        usb_filesomething_hack = 0;
//        out_led3.Set(0);

        break;
    case COMMAND_SAVE:
        if (f_open(&fileR, commandParams.commandFileName,  FA_OPEN_ALWAYS | FA_WRITE) != FR_OK) {
            displayFileError("OPEN ERROR");
        }
        f_lseek(&fileR, commandParams.commandSeek);
        f_write(&fileR, commandParams.commandParam1, commandParams.commandParamSize, &numberOfBytes);
        if (numberOfBytes != commandParams.commandParamSize) {
            displayFileError("WRITE ERROR");
        }
        if (f_close(&fileR) != FR_OK) {
            displayFileError("CLOSE ERROR");
        }
        commandParams.commandResult = COMMAND_SUCCESS;
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_DELETE:
        // Don't look at the result....
        f_unlink(commandParams.commandFileName);
        commandParams.commandResult = COMMAND_SUCCESS;
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_EXISTS:
        if (f_stat(commandParams.commandFileName, &fileinfo) == FR_OK) {
            commandParams.commandResult = fileinfo.fsize;
        } else {
            commandParams.commandResult = -1;
        }
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_OPEN_DIR:
        if (f_opendir ( &dir, commandParams.commandFileName) != FR_OK) {
            commandParams.commandResult = COMMAND_FAILED;
        } else {
            commandParams.commandResult = COMMAND_SUCCESS;
        }
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_NEXT_FILE_NAME:
        if (f_readdir ( &dir, &fileinfo) != FR_OK) {
            commandParams.commandResult = COMMAND_FAILED;
            commandParams.commandState = COMMAND_NONE;
            break;
        }
        if (fileinfo.fname[0] == 0) {
            commandParams.commandResult = COMMAND_FAILED;
            commandParams.commandState = COMMAND_NONE;
            break;
        }
        for (int k=0; k<13; k++) {
            ((char*)commandParams.commandParam1)[k] = fileinfo.fname[k];
        }
        *((unsigned long*)commandParams.commandParam2) = fileinfo.fsize;
        commandParams.commandResult = COMMAND_SUCCESS;
        commandParams.commandState = COMMAND_NONE;
        break;
    case COMMAND_RENAME:
        if (f_rename(commandParams.commandFileName, (const char*)commandParams.commandParam1) != FR_OK) {
            commandParams.commandResult = COMMAND_FAILED;
        } else {
            commandParams.commandResult = COMMAND_SUCCESS;
        }
        commandParams.commandState = COMMAND_NONE;
        break;
// #ifdef BOOTLOADER
    // Low level only accessible by bootloader in mass storage device mode.
    case DISKIO_GETSECTORNUMBER:
//    	disk_initialize(0);
    	if ((numberOfBytes = disk_ioctl(0, GET_SECTOR_COUNT, (unsigned long*)commandParams.commandParam1)) != RES_OK) {
            commandParams.commandResult = COMMAND_FAILED;
            *((int*)commandParams.commandParam1) = 0;
    	} else {
            commandParams.commandResult = COMMAND_SUCCESS;
    	}
        commandParams.commandState = COMMAND_NONE;
    	break;
    case DISKIO_READ:
	  	if (disk_read(0, (BYTE*)commandParams.commandParam1, (DWORD)*((int*)commandParams.commandParam2), (BYTE)commandParams.commandParamSize) != RES_OK) {
	  		commandParams.commandResult = COMMAND_FAILED;
	  	} else {
	  		commandParams.commandResult = COMMAND_SUCCESS;
	  	}
        commandParams.commandState = COMMAND_NONE;
    	break;
    case DISKIO_WRITE:
	  	if (disk_write(0, (BYTE*)commandParams.commandParam1, (DWORD)*((int*)commandParams.commandParam2), (BYTE)commandParams.commandParamSize) != RES_OK) {
	  		commandParams.commandResult = COMMAND_FAILED;
	  	} else {
	  		commandParams.commandResult = COMMAND_SUCCESS;
	  	}
        commandParams.commandState = COMMAND_NONE;
    	break;
// #endif
    case COMMAND_NONE:
        break;
    }
//    usb_filesomething_hack = 0;

	return 0;
}

/**
 * @brief  USBH_USR_DeInit
 *         Deint User state and associated variables
 * @param  None
 * @retval None
 */
// void USBH_USR_DeInit(void) {
// }

/**
 * @brief  This function handles the program fail.
 * @param  None
 * @retval None
 */
void Fail_Handler(const char*msg) {
    lcd.setCursor(0, 0);
	lcd.print(msg);
	while (1) {	}
}

