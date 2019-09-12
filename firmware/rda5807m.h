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

#ifndef RDA5807M_H_
#define RDA5807M_H_

#define RDA5807_SEEK_TUNE_COMPLETE	0x4000
#define RDA5807_HAS_RDS_INFO		0x8000
#define RDA5807_CHANNEL_INFO		0x03FF
#define RDA5807_STEREO				0x0400

#define RDA5807_START_TUNE			0x01
#define RDA5807_TUNE_UP				0x02

#define RDS_GROUP			0xF800
#define RDS_GROUP_A0		0x0000
#define RDS_GROUP_B0		0x0800

#define MAX_STATION_NAME_SIZE	10

typedef enum
{
	SKMODE_DOWN = 0,
	SKMODE_UP
} SEEK_MODE;

void updateReceiverConfig(unsigned char updateLength);													
void initReceiver();
void readConfiguration(unsigned short *rxData);
void readConfigurationFast(unsigned short *rxData);

void updateChannel(unsigned short channel);
void startSeek(SEEK_MODE seekMode);

void extractFrequency(unsigned short freqBlock, char *freqStr);
BOOL extractRDSData(unsigned short *rxData, char *stationTempBuffer, char *StationName);
void clearRDSData(char *stationTempBuffer, char *StationName, unsigned char bufferSize);

#endif /* RDA5807M_H_ */