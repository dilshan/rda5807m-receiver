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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "main.h"
#include "rda5807m.h"
#include "lcd.h"
#include "m62429.h"
#include "storage.h"

int main()
{
	SYS_STATE lastState;
	unsigned short memChannel;
	
	cli();
	
	initSystem();	
	setConfigDefault(&_currentConfig);
	
	// Setting up default values for the variables.
	_currentState = TUNER_IDLE;
	lastState = _currentState;
	_lastButtonState = PINB;
	_clearRDSinfo = FALSE;
	_needUpdateUI = FALSE;
	_updateSystemConfig = FALSE;
	_idleLimit = IDLE_LIMIT_VOLUME;
	_lastEncorderButtonVal = 0xFF;
	_memoryStoredFlag = 0;
	_memAction = MA_NONE;
	_defaultMemChannel = 0;

	memset(_lcdRow1, 0, LCD_MAX_ROW_LENGTH + 1);
	memset(_lcdRow2, 0, LCD_MAX_ROW_LENGTH + 1);
	
	memset(_memoryHoldCounter, 0, MEM_GROUP_SIZE);
	
	_delay_ms(1000);
	clearRDSData(_stationTempBuffer, _stationName, MAX_STATION_NAME_SIZE);
	
	initReceiver();
	_delay_ms(10);
	
	// Load and restore last system configuration.
	loadConfig(&_currentConfig);
	updateChannel(_currentConfig.currentChannel);
	_delay_ms(20);
	updateVolumeControl(_currentConfig.volume);
	_delay_ms(10);
	
	clearLCD();
	
	// Setup timer1 to generate interrupts with 100ms intervals.
	initButtonScanner();
	sei();
	
	// Enable watchdog timer with 2.1 seconds time configuration.
	wdt_enable(WDTO_1S);
	
	// Turn on LCD back-light when system is ready.
	PORTD |= 0x02;
	
	// Start main service loop.
	while(1)
	{
		// Reset watchdog timer.
		wdt_reset();
		
		if(_currentState == TUNER_IDLE)
		{
			// Handle tuner information processing and RDS routines.
			
			// Handle screen transitions.
			if(lastState != TUNER_IDLE)
			{
				clearLCD();
				lastState = TUNER_IDLE;
			}
			
			// Perform memory recall and save operations based on user actions.
			if(_memAction == MA_LOAD)
			{
				memChannel = loadMemorySlot(_defaultMemChannel);
				if(memChannel != _currentConfig.currentChannel)
				{
					updateChannel(memChannel);
					_clearRDSinfo = TRUE;
					_updateSystemConfig = TRUE;
				}
				
				_memAction = MA_NONE;
			}
			else if(_memAction == MA_SAVE)
			{
				if(saveMemoryStation(_defaultMemChannel, _currentConfig.currentChannel) == TRUE)
				{
					_memoryStoredFlag = 1;
				}
				
				_memAction = MA_NONE;
			}
			
			// Read status bits from receiver and extract tuner information.
			readConfiguration(_receiverInfo);
			extractFrequency(_receiverInfo[0], _lcdRow1);
			
			setCursor(1,1);
			printStr(_lcdRow1);
			
			// Handle RDS clear flag raised in ISR.
			if(_clearRDSinfo == TRUE)
			{
				// Clear current RDS data.
				clearRDSData(_stationTempBuffer, _stationName, MAX_STATION_NAME_SIZE);
				clearRow(2);
				
				_clearRDSinfo = FALSE;
				_delay_ms(25);
				continue;
			}
			
			// Update stereo indicator.
			PORTD &= 0xFE;
			if((_receiverInfo[0] & RDA5807_STEREO) != 0x00)
			{
				PORTD |= 0x01;
			}
			
			// Check current status of the tunning process.
			if((_receiverInfo[0] & RDA5807_SEEK_TUNE_COMPLETE) != 0)
			{
				// Extract RDS data if station is available.
				if((extractRDSData(_receiverInfo, _stationTempBuffer, _stationName) == TRUE) && (_memoryStoredFlag == 0))
				{
					setCursor(2, 1);
					memset(_lcdRow2, ' ', LCD_MAX_ROW_LENGTH);
					strcpy(_lcdRow2, _stationName);
					printStr(_lcdRow2);
				}
				
				// Update current channel information in EEPROM.
				if(_updateSystemConfig == TRUE)
				{
					_currentConfig.currentChannel = _receiverInfo[0] & RDA5807_CHANNEL_INFO;
					saveReceiverChannel(&_currentConfig);
				}
			}
			
			// Display "STORED" message during the channel preset.
			if(_memoryStoredFlag > 0)
			{
				setCursor(2, 1);
				printStr("STORED        ");
			}
			
			// Check for rotary encoder rotations.
			if((PINC & 0x30) != 0x30)
			{
				enterVolumeControl();
			}
						
			_delay_ms(20);
		}
		else if(_currentState == VOLUME_CONTROL)
		{
			// Handle rotary encoder related events
			_memAction = MA_NONE;
			
			// Handle screen transitions.
			if(lastState != VOLUME_CONTROL)
			{
				clearLCD();
				printStr("VOLUME: ");
				lastState = VOLUME_CONTROL;
			}
			
			// Check for rotary encoder rotations.
			if((PINC & 0x30) != 0x30)
			{
				if((!(PINC & 0x10)) && (_lastEncoderVal))
				{
					if(PINC & 0x20)
					{
						if(_rotaryEncoderPos < 100)
						{
							_rotaryEncoderPos++;
							_needUpdateUI = TRUE;
						}
					}
					else
					{
						if(_rotaryEncoderPos > 0)
						{
							_rotaryEncoderPos--;
							_needUpdateUI = TRUE;
						}
					}
				}
			}
			
			// Update volume information in both UI and controller.
			if(_needUpdateUI == TRUE)
			{
				// Draw new values into the screen
				sprintf(_lcdRow1, "%d  ", _rotaryEncoderPos);
				setCursor(1, 9);
				printStr(_lcdRow1);
				drawLevel(2, ((_rotaryEncoderPos * (LCD_MAX_ROW_LENGTH - 1)) / 100), FALSE);
				
				// Send volume information to the controller and EEPROM.
				updateVolumeControl(_rotaryEncoderPos);
				_currentConfig.volume = _rotaryEncoderPos;
				saveVolume(&_currentConfig);
				
				_idleCounter = 0;
				_needUpdateUI = FALSE;
			}
			
			_lastEncoderVal = (PINC & 0x10);
			_delay_us(750);
		}
		else if(_currentState == TUNER_CONTROL)
		{
			// Handle manual tunning.
			if(_indicatorCounter == 0)
			{
				readConfigurationFast(_receiverInfo);
							
				// Update stereo indicator.
				PORTD &= 0xFE;
				if((_receiverInfo[0] & RDA5807_STEREO) != 0x00)
				{
					PORTD |= 0x01;
				}	
			}
			
			_indicatorCounter++;
			
			// Handle screen transitions.
			if(lastState != TUNER_CONTROL)
			{
				clearLCD();
				lastState = TUNER_CONTROL;
			}
			
			// Check for rotary encoder rotations.
			if((PINC & 0x30) != 0x30)
			{
				if((!(PINC & 0x10)) && (_lastEncoderVal))
				{
					if(PINC & 0x20)
					{
						if(_rotaryEncoderPos < 210)
						{
							updateChannel(++_rotaryEncoderPos);
							_needUpdateUI = TRUE;
						}
					}
					else
					{
						if(_rotaryEncoderPos > 0)
						{
							updateChannel(--_rotaryEncoderPos);
							_needUpdateUI = TRUE;
						}
					}
				}
			}
			
		
			// Update current tuner information in the screen.
			if(_needUpdateUI == TRUE)
			{
				extractFrequency(_rotaryEncoderPos, _lcdRow1);
				
				setCursor(1,1);
				printStr(_lcdRow1);
				drawLevel(2, ((_rotaryEncoderPos * (LCD_MAX_ROW_LENGTH - 1)) / 210), TRUE);

				_idleCounter = 0;
				_needUpdateUI = FALSE;
			}

			_lastEncoderVal = (PINC & 0x10);
			_delay_us(100);
		}
	}
	
	return 0;
}

ISR (TIMER1_COMPA_vect)
{
	unsigned char memPos;
	
	// Reset watchdog timer.
	wdt_reset();
	
	// Reset memory stored flag after nearly 1 seconds.
	if(_memoryStoredFlag > 0)
	{
		if((++_memoryStoredFlag) > 11)
		{
			_memoryStoredFlag = 0;
		}
	}
	
	// Handle tuner related user events.
	if(_currentState == TUNER_IDLE)
	{
		// Check for TUNE UP button event.
		if(((_lastButtonState & BUTTON_TUNE_UP) == 0x00) && ((PINB & BUTTON_TUNE_UP) == BUTTON_TUNE_UP))
		{
			startSeek(SKMODE_UP);
			_clearRDSinfo = TRUE;
			_updateSystemConfig = TRUE;
		}
		
		// Check for TUNE DOWN button event.
		if(((_lastButtonState & BUTTON_TUNE_DOWN) == 0x00) && ((PINB & BUTTON_TUNE_DOWN) == BUTTON_TUNE_DOWN))
		{
			startSeek(SKMODE_DOWN);
			_clearRDSinfo = TRUE;
			_updateSystemConfig = TRUE;
		}
		
		// Check for rotary button press event.
		if(((_lastEncorderButtonVal & ROTARY_BUTTON) == 0x00) && ((PINC & ROTARY_BUTTON) == ROTARY_BUTTON))
		{
			enterTunerControl();
			return;
		}
	
		// Check for memory recall action.
		for(memPos = 0; memPos < MEM_GROUP_SIZE; memPos++)
		{
			// Count hold down times of the memory buttons.
			if(((PINB & _memoryButtonGroup[memPos]) == 0x00) && (_memoryHoldCounter[memPos] < 0xFF))
			{
				_memoryHoldCounter[memPos]++;
			}
			
			// Check for hold down timeout to store the current channel.
			if(_memoryHoldCounter[memPos] >= LONG_PRESS_COUNTS)
			{
				_memoryHoldCounter[memPos] = 0;
				_defaultMemChannel = memPos;
				_memAction = MA_SAVE;
				return;
			}
			
			// On normal button action load programmed channel from memory.
			if(((_lastButtonState & _memoryButtonGroup[memPos]) == 0x00) && ((PINB & _memoryButtonGroup[memPos]) == _memoryButtonGroup[memPos]))
			{
				_defaultMemChannel = memPos;
				_memAction = MA_LOAD;
				_memoryHoldCounter[memPos] = 0;
			}
			
			// Reset memory hold down counter on idle state.
			if((PINB & _memoryButtonGroup[memPos])	== _memoryButtonGroup[memPos])
			{
				_memoryHoldCounter[memPos] = 0;	
			}	
		}
				
		_lastButtonState = PINB;
	}
	else if((_currentState == VOLUME_CONTROL) || (_currentState == TUNER_CONTROL))
	{
		unsigned char idleTimeout;
		
		if(_idleCounter < 0xFF)
		{
			_idleCounter++;
		}
		
		idleTimeout = ((_idleCounter >= _idleLimit) && ((PINC & 0x30) == 0x30));

		if(idleTimeout || ((PINB & 0xFF) != 0xFF))
		{
			// Sub system time-out. Switch UI to the default (tuner).
			exitSubSystem();
		}
		
		if(((_lastEncorderButtonVal & ROTARY_BUTTON) == 0x00) && ((PINC & ROTARY_BUTTON) == ROTARY_BUTTON))
		{
			// Check for rotary button press event.
			if(_currentState == VOLUME_CONTROL)
			{
				enterTunerControl();
			}
			else
			{
				enterVolumeControl();
			}
			
			return;
		}
	}
	
	_lastEncorderButtonVal = PINC;
}

void enterVolumeControl()
{
	_rotaryEncoderPos = _currentConfig.volume;
	_idleCounter = 0;
	_idleLimit = IDLE_LIMIT_VOLUME;
	_lastEncoderVal = (PINC & 0x10);
	_lastEncorderButtonVal = 0xFF;
	_needUpdateUI = TRUE;
	_currentState = VOLUME_CONTROL;
}

void enterTunerControl()
{
	PORTD &= 0xFE;
	_rotaryEncoderPos = _currentConfig.currentChannel;
	_idleCounter = 0;
	_indicatorCounter = 0;
	_idleLimit = IDLE_LIMIT_TUNER;
	_lastEncoderVal = (PINC & 0x10);
	_lastEncorderButtonVal = 0xFF;
	_needUpdateUI = TRUE;
	_currentState = TUNER_CONTROL;
}

void exitSubSystem()
{
	if(_currentState == TUNER_CONTROL)
	{
		_clearRDSinfo = (_rotaryEncoderPos != _currentConfig.currentChannel) ? TRUE : FALSE;
		_currentConfig.currentChannel = _rotaryEncoderPos;
		
		// Save channel only on exit from manual tunning.
		saveReceiverChannel(&_currentConfig);
	}
	
	_idleCounter = 0;
	_lastButtonState = 0xFF;
	_needUpdateUI = FALSE;
	_currentState = TUNER_IDLE;
}

void setConfigDefault(SYS_CONFIG *config)
{
	config->volume = DEFAULT_VOLUME;
	config->currentChannel = DEFAULT_CHANNEL;
}

void initButtonScanner()
{
	TCNT1 = 0x00;
	TCCR1A = 0x00;
	TCCR1B = 0x0A;
	OCR1AH = 0xC3;
	OCR1AL = 0x4F;
	TIMSK = (1 << OCIE1A);
}

void initSystem()
{
	// Setup system registers.
	SFIOR = 0x00;
	
	// Disable UART and all it's I/O pins.
	UCSRB = 0x00;
	UCSRA = 0x00;

	// Setting up I/O ports.
	DDRD = 0xFF;
	PORTD = 0x00;
	DDRB = 0x00;
	PORTB = 0xFF;
	DDRC = 0xC0;
	PORTC = 0x3F;
		
	// Initialize libraries and sub-systems.
	updateVolumeControl(0);
	
	initLCD();
	clearLCD();
	
	_delay_ms(20);
}
