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

#include <string.h>
#include <util/delay.h>
#include <stddef.h>
#include <stdio.h>

#include "iic.h"
#include "rda5807m.h"

#define RDA5807M_ADDRESS	0x20

#define RECEIVER_CONFIG_LEN 12

#define INIT_RX_REG_0	0xD0	// DHIZ | DMUTE | MONO | BASS | RCLK_MODE | RCLK | SEEKUP | SEEK
#define INIT_RX_REG_1	0x0F	// SKMODE | CLK_MODE | CLK_MODE | CLK_MODE | RDS_EN | NEW_METHOD | SOFT_RESET | ENABLE
#define INIT_RX_REG_2	0x00	// CHAN
#define INIT_RX_REG_3	0x00	// CHAN | CHAN | DIRECT_MODE | TUNE | BAND | BAND | SPACE | SPACE
#define INIT_RX_REG_4	0x0A	// RSVD | RSVD | RSVD | RSVD | DE | RSVD | SOFTMUTE_EN | AFCD
#define INIT_RX_REG_5	0x00	// RSVD
#define INIT_RX_REG_6	0x88	// INT _MODE | RSVD | RSVD | RSVD | SEEKTH | SEEKTH | SEEKTH | SEEKTH
#define INIT_RX_REG_7	0x0F	// RSVD | RSVD | RSVD | RSVD | VOLUME | VOLUME | VOLUME | VOLUME
#define INIT_RX_REG_8	0x00	// RSVD | OPEN_MODE | OPEN_MODE | RSVD | RSVD | RSVD | RSVD | RSVD
#define INIT_RX_REG_9	0x00	// RSVD
#define INIT_RX_REG_10	0x42	// RSVD | TH_SOFRBLEND | TH_SOFRBLEND | TH_SOFRBLEND | TH_SOFRBLEND | TH_SOFRBLEND | 65M_50M MODE | RSVD
#define INIT_RX_REG_11	0x02	// SEEK_TH_OLD | SEEK_TH_OLD | SEEK_TH_OLD | SEEK_TH_OLD | SEEK_TH_OLD | SEEK_TH_OLD | SOFTBLEND_EN | FREQ_MODE


unsigned char receiverConfig[RECEIVER_CONFIG_LEN] = {INIT_RX_REG_0, INIT_RX_REG_1, INIT_RX_REG_2, INIT_RX_REG_3, INIT_RX_REG_4, INIT_RX_REG_5,
													INIT_RX_REG_6, INIT_RX_REG_7, INIT_RX_REG_8, INIT_RX_REG_9, INIT_RX_REG_10, INIT_RX_REG_11};
													
void updateReceiverConfig(unsigned char updateLength)
{
	if(updateLength > RECEIVER_CONFIG_LEN)
	{
		updateLength = RECEIVER_CONFIG_LEN;
	}
	
	twiWriteReg(RDA5807M_ADDRESS, receiverConfig, updateLength);
}

void initReceiver()
{
	// Initialize receiver chip with default configuration.
	updateReceiverConfig(RECEIVER_CONFIG_LEN);
	_delay_ms(10);
	
	// Release reset flag in configuration.
	receiverConfig[1] = receiverConfig[1] & 0xFD;
	updateReceiverConfig(2);
}

void updateChannel(unsigned short channel)
{
	receiverConfig[2] = channel >> 2;
	receiverConfig[3] = (receiverConfig[3] & 0x0F) | ((channel & 0x03) << 6 ) | 0x10;
	updateReceiverConfig(4);
}

void startSeek(SEEK_MODE seekMode)
{
	unsigned char skMode = (seekMode == SKMODE_UP) ? RDA5807_TUNE_UP : 0x00;
	receiverConfig[0] = receiverConfig[0] & 0xFC;
	receiverConfig[0] = receiverConfig[0] | skMode | RDA5807_START_TUNE;
	updateReceiverConfig(3);
}

void readConfiguration(unsigned short *rxData)
{
	unsigned char regData[12];
	unsigned char regPos;

	if(rxData != NULL)
	{
		twiReadReg(RDA5807M_ADDRESS, regData, 12);
		
		// Convert received 8-bit data into 16-bit data chunks.
		for(regPos = 0; regPos < 6; regPos++)
		{
			rxData[regPos] = ((regData[regPos * 2] << 8) | regData [(regPos * 2) + 1] );
		}
	}
}

void readConfigurationFast(unsigned short *rxData)
{
	unsigned char regData[2];
	
	if(rxData != NULL)
	{
		twiReadReg(RDA5807M_ADDRESS, regData, 2);	
		rxData[0] = ((regData[0] << 8) | regData [1] );	
	}
}

void extractFrequency(unsigned short freqBlock, char *freqStr)
{
	float retData = (((freqBlock & 0x03FF) * 100) + 87000);
	sprintf(freqStr, "%.2f MHz  ", (retData/1000));
}

void clearRDSData(char *stationTempBuffer, char *StationName, unsigned char bufferSize)
{
	memset(stationTempBuffer, 0, bufferSize);
	memset(StationName, 0, bufferSize);
}

BOOL extractRDSData(unsigned short *rxData, char *stationTempBuffer, char *StationName)
{
	unsigned short rdsB, rdsD;
	unsigned char offset, char1, char2;
	BOOL hasStationInfo = FALSE;
	
	if((rxData[0] & RDA5807_HAS_RDS_INFO))
	{
		// Extract RDS data blocks which need to display station name.
		rdsB = rxData[3];
		rdsD = rxData[5];
		
		switch(rdsB & RDS_GROUP)
		{
			case RDS_GROUP_A0:
			case RDS_GROUP_B0:
				// Station information extraction routine.
				
				hasStationInfo = TRUE;
				
				offset = (rdsB & 0x03) << 1;
				char1 = (char)(rdsD >> 8);
				char2 = (char)(rdsD & 0xFF);
				
				// Check for valid array limits.
				if(offset >= MAX_STATION_NAME_SIZE)
				{
					return FALSE;	
				}
				
				// Verify the 1st bit with last received character.
				if (stationTempBuffer[offset] == char1) 
				{
					StationName[offset] = char1;
				} 
				else if((char1 >= 0x20) && (char1 <= 0x7D))
				{
					stationTempBuffer[offset] = char1;
				}

				// Verify the 2nd bit with last received character.
				if (stationTempBuffer[offset + 1] == char2) 
				{
					StationName[offset + 1] = char2;
				} 
				else if((char2 >= 0x20) && (char2 <= 0x7D))
				{
					stationTempBuffer[offset + 1] = char2;
				}
				
				break;
		}
	}

	return hasStationInfo;
}