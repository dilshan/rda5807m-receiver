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

#include "iic.h"

#include <util/twi.h>

#define TWI_READ	0x01
#define TWI_WRITE	0x00

BOOL twiStart(unsigned char addr)
{
	unsigned char ackStatus;
	
	// Reset TWI and wait for end of transmission of START signal.
	TWCR = 0;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	
	if((TWSR & 0xF8) != TW_START)
	{
		// Fail to send START signal.
		return FALSE;
	}
	
	// Transmit slave device address.
	TWDR = addr;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	
	// Waiting ACK from slave device.
	ackStatus = TW_STATUS & 0xF8;
	if ((ackStatus != TW_MT_SLA_ACK) && (ackStatus != TW_MR_SLA_ACK))
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL twiWrite(unsigned char data)
{
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1 << TWINT)));
	return ((TWSR & 0xF8) != TW_MT_DATA_ACK) ? FALSE : TRUE;
}

void twiStop()
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

unsigned char twiReadAck()
{
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA); 
	while(!(TWCR & (1 << TWINT)));
	return TWDR;
}

unsigned char twiReadNAck()
{
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1<<TWINT)));
	return TWDR;
}

BOOL twiWriteReg(unsigned char addr, unsigned char *data, unsigned char dataLen)
{
	unsigned char pos;
	
	// Transmit START with slave address.
	if(twiStart(addr | TWI_WRITE) != TRUE)
	{
		return FALSE;
	}
	
	// Submit each byte to the slave device.
	for(pos = 0; pos < dataLen; pos++)
	{
		twiWrite(data[pos]);
	}
	
	// Finish transmission with STOP.
	twiStop();
	
	return TRUE;
}

BOOL twiReadReg(unsigned char addr, unsigned char *data, unsigned char dataLen)
{
	unsigned char pos;
	
	// Transmit START with slave address.
	if(twiStart(addr | TWI_READ) != TRUE)
	{
		return FALSE;
	}
	
	// Capture data received from the device with ACK.
	for (pos = 0; pos < (dataLen - 1); pos++)
	{
		data[pos] = twiReadAck();
	}
	
	// Read last data byte.
	data[(dataLen - 1)] = twiReadNAck();
	
	// Finish transmission with STOP.
	twiStop();
	
	return TRUE;
}