/*********************************************************************************
 * Copyright (c) 2019 Dilshan R Jayakody.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *********************************************************************************/

#ifndef RDA5807_RADIO_MAIN_
#define RDA5807_RADIO_MAIN_

#include "global.h"
#include "rda5807m.h"
#include "lcd.h"

#define BUTTON_TUNE_UP		0x01
#define BUTTON_TUNE_DOWN	0x02

#define BUTTON_MEM_1		0x04
#define BUTTON_MEM_2		0x08
#define BUTTON_MEM_3		0x10
#define BUTTON_MEM_4		0x20
#define BUTTON_MEM_5		0x40
#define BUTTON_MEM_6		0x80

#define ROTARY_BUTTON		0x08

#define MEM_GROUP_SIZE		6

typedef enum
{
	TUNER_IDLE,
	VOLUME_CONTROL,
	TUNER_CONTROL
} SYS_STATE;

const unsigned char _memoryButtonGroup[] = {BUTTON_MEM_1, BUTTON_MEM_2, BUTTON_MEM_3, BUTTON_MEM_4, BUTTON_MEM_5, BUTTON_MEM_6};
unsigned char _memoryHoldCounter[MEM_GROUP_SIZE];

unsigned short _receiverInfo[6];

unsigned char _idleLimit;
unsigned char _idleCounter;
unsigned short _rotaryEncoderPos;
unsigned char _lastEncoderVal;
unsigned char _lastEncorderButtonVal;
unsigned char _lastButtonState;
unsigned char _memoryStoredFlag;
unsigned char _defaultMemChannel;
unsigned char _indicatorCounter;

BOOL _updateSystemConfig;
BOOL _needUpdateUI;
BOOL _clearRDSinfo;

char _lcdRow1 [LCD_MAX_ROW_LENGTH + 1];
char _lcdRow2 [LCD_MAX_ROW_LENGTH + 1];

char _stationName[MAX_STATION_NAME_SIZE];
char _stationTempBuffer[MAX_STATION_NAME_SIZE];

SYS_STATE _currentState;
SYS_CONFIG _currentConfig;
MEM_ACTION _memAction;

void initSystem();
void setConfigDefault(SYS_CONFIG *config);
void initButtonScanner();

void enterTunerControl();
void enterVolumeControl();
void exitSubSystem();

#endif /* RDA5807_RADIO_MAIN_ */