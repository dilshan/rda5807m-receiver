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

#ifndef GLOBAL_H_
#define GLOBAL_H_

// FUSE LOW  : 0x3F
// FUSE HIGH : 0xD9

// System clock is set to 4.00MHz.
#define F_CPU	4000000UL

// Default frequency is set to 87.00MHz.
#define DEFAULT_CHANNEL	52

// Default volume level is 0.
#define DEFAULT_VOLUME	0

// Idle limits for volume and tune controls.
#define IDLE_LIMIT_VOLUME	70
#define IDLE_LIMIT_TUNER	150

#define LONG_PRESS_COUNTS	15

typedef enum
{
	FALSE = 0x00,
	TRUE = 0xFF	
} BOOL;

typedef enum
{
	MA_NONE,
	MA_LOAD,
	MA_SAVE
} MEM_ACTION;

typedef struct
{
	unsigned char volume;
	unsigned short currentChannel;
} SYS_CONFIG;

#endif /* GLOBAL_H_ */