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

#include "global.h"

#include <avr/eeprom.h>

#include "m62429.h"

uint8_t data[] EEMEM = {0x00, 0x00, 0x00};

void saveVolume(SYS_CONFIG *config)
{
	eeprom_update_byte(0x00, config->volume);
}

void saveReceiverChannel(SYS_CONFIG *config)
{
	eeprom_update_word((uint16_t*)0x01, config->currentChannel);
}

void loadConfig(SYS_CONFIG *config)
{
	config->volume = eeprom_read_byte(0x00);
	config->currentChannel = eeprom_read_word((uint16_t*)0x01);
	
	// Check overflows in received values.
	if(config->volume > MAX_VOLUME)
	{
		config->volume = DEFAULT_VOLUME;
	}
	
	if(config->currentChannel == 0xFFFF)
	{
		config->currentChannel = DEFAULT_CHANNEL;
	}
}

unsigned short loadMemorySlot(unsigned char memChannel)
{
	unsigned short memoryData = eeprom_read_word((uint16_t*)((memChannel * 2) + 0x03));
	return (memoryData == 0xFFFF) ? DEFAULT_CHANNEL : memoryData;
}

BOOL saveMemoryStation(unsigned char memChannel, unsigned short channel)
{
	unsigned short memAddr = (memChannel * 2) + 0x03;
	if(eeprom_read_word((uint16_t*)memAddr) != channel)
	{
		// Save new channel into specified memory location.
		eeprom_update_word((uint16_t*)memAddr, channel);
		return TRUE;
	}
	
	return FALSE;
}