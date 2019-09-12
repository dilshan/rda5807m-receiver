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

#include <avr/io.h>
#include <util/delay.h>

#include "m62429.h"

void updateVolumeControl(unsigned char level)
{
	unsigned char  dbLevel = (MAX_VOLUME * level) / 100;
	unsigned char dataPos;
	unsigned short outputData = 0x00;
	
	outputData = 0;
	outputData |= (dbLevel & 0x7C);
	outputData |= (dbLevel & 0x03) << 7;
	outputData |=  0x600; 
	
	// Submit data packet into M62429 controller.
	for(dataPos = 0; dataPos < 11; dataPos++)
	{
		PORTC = PORTC & 0x3F;
		_delay_us(5);
		PORTC = PORTC | ((outputData  >> dataPos) & 0x01) << 6;
		_delay_us(5);
		PORTC = PORTC | 0x80;
		_delay_us(5);
	}
	
	// Send latch signal.
	PORTC = PORTC | 0x40;
	_delay_us(5);
	PORTC = PORTC & 0x7F;
}
