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

#include <stddef.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

void writeLCDCmd(unsigned char cmd)
{
	PORTD &= 0x03;
	_delay_us(50);
	PORTD |= cmd << 4;
	
	// Toggle RS and send upper word to LCD.
	PORTD |= 0x08;
	_delay_ms(4);
	PORTD &= 0xF7;
	
	_delay_ms(4);
}

void writeLCDData(unsigned char data)
{
    char lowWord = (data & 0x0F) << 4;
    
	// Send high value of the byte.
    PORTD |= 0x04;
    PORTD = (PORTD & 0x0F) | (data & 0xF0);
    PORTD |= 0x08;
    
	// Send low value of the byte.
    _delay_us(50);
    PORTD &= 0xF7;
    PORTD = (PORTD & 0x0F) | lowWord;
    PORTD |= 0x08;
    
    _delay_us(50);
    PORTD &= 0xF7;
}

void initLCD()
{
	// Try to reset the HD44780 controller.
	_delay_ms(5);
	writeLCDCmd(0x03);
	_delay_ms(5);
	writeLCDCmd(0x03);
	_delay_ms(15);
	writeLCDCmd(0x03);
	
	// Initialize display with default character set font size.
	writeLCDCmd(0x02);
	writeLCDCmd(0x02);
	writeLCDCmd(0x08);
	writeLCDCmd(0x00);
	writeLCDCmd(0x0C);
	writeLCDCmd(0x00);
	writeLCDCmd(0x06);
}

void clearLCD()
{
	writeLCDCmd(0x00);
	writeLCDCmd(0x01);	
}

void setCursor(unsigned char row, unsigned char col)
{
	unsigned char cmd = (row == 1 ? 0x80 : 0xC0) + col - 1;
	writeLCDCmd(cmd >> 4);
	writeLCDCmd(cmd & 0x0F);
}

void clearRow(unsigned char row)
{
	unsigned char charPos;
	setCursor(row, 1);
	
	for(charPos = 0; charPos < LCD_MAX_ROW_LENGTH; charPos++)
	{
		writeLCDData(' ');
	}
	
	setCursor(row, 1);
}

void printStr(char *str)
{
	if(str != NULL)
	{
		unsigned char charPos;
		
		// Clamp specified string to 16 characters to support 16x2 LCD.
		unsigned char maxPos = (strlen(str) > LCD_MAX_ROW_LENGTH) ? LCD_MAX_ROW_LENGTH : strlen(str);
		for(charPos = 0; charPos < maxPos; charPos++)
		{
			writeLCDData(str[charPos]);
		}	
	}
}

void drawLevel(unsigned char row, unsigned char level, BOOL positon)
{
	unsigned char pos;
	unsigned char charData;
	
	setCursor(row, 1);
	for(pos = 0; pos < LCD_MAX_ROW_LENGTH; pos++)
	{
		if(positon == FALSE)
		{
			charData = (pos <= level) ? 0xFF : 0x20;
		}
		else
		{
			charData = (pos == level) ? 0xFF : '-';
		}
		
		writeLCDData(charData);
	}
}