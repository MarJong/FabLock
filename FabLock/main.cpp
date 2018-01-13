/*
 * FabLock.cpp
 *
 * Created: 07.01.2018 13:36:57
 * Author : Marcel Jongmanns
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <inttypes.h>

#include "CLED.h"
#include "CAudio.h"

#define F_CPU 8000000
#include <util/delay.h>

#define nop		asm volatile("nop")

#define MOTOR_ENABLE		PORTB &= ~_BV(PORTB2)
#define MOTOR_DISABLE		PORTB |= _BV(PORTB2)
#define MOTOR_STEP			if(motorStepNeeded) { PORTB ^= _BV(PORTB1); motorStepNeeded = 0; }
//#define MOTOR_STEP			PORTB ^= _BV(PORTB1); nop; nop; nop; nop; nop; nop; nop; nop;
#define MOTOR_DIR_OPEN		PORTB &= ~_BV(PORTB0)
#define MOTOR_DIR_CLOSE		PORTB |= _BV(PORTB0)


#define OPEN_REQUEST	(!(PIND & _BV(PIND2)))
#define SET_BUSY		PORTD |= _BV(PORTD4)
#define CLR_BUSY		PORTD &= ~_BV(PORTD4)


#define KEY_MISSING			(!(PINC & _BV(PINC2)))
#define WINDOW_UNLOCKED		((PINC & _BV(PINC5)))
#define WINDOW_LOCKED		((PINC & _BV(PINC4)))
#define WINDOW_CLOSED		((PINC & _BV(PINC3)))

volatile uint32_t SysTick = 0;			// system timer 1ms base
volatile uint8_t audioStepNeeded = 0;	// flag for call of audio routine
volatile uint8_t motorStepNeeded = 0;	// flag for call of motor step
volatile uint32_t windowTimeout = 0;	// if = 0 then window needs to be closed /ms

CAudio audio;

CLED ledOpen(&PORTD, PORTD5);
CLED ledClose(&PORTD, PORTD6);
CLED ledPull(&PORTD, PORTD7);

enum WINDOW_STATE {
	STATE_CLOSED = 0, STATE_OPENING, STATE_OPEN, STATE_CLOSING, STATE_ERROR
} state;

int main(void)
{
	// init ports
	DDRB = _BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2);
	DDRC = 0;
    DDRD = _BV(PORTD3) | _BV(PORTD4) | _BV(PORTD5) | _BV(PORTD6) | _BV(PORTD7);
	PORTD |= _BV(PORTD2);
	
	MOTOR_DISABLE;
	SET_BUSY;
	
	// timer0 -> 1kHz SysTick
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS00) | _BV(CS01);
	OCR0A = 124;
	TIMSK0 = _BV(OCIE0A);

	// timer 1 + 2 -> Audio
	audio.init();
	
	state = STATE_ERROR;
	sei();
	
    while (1) 
    {	
		switch (state)
		{
		case STATE_CLOSED:
			CLR_BUSY;
			MOTOR_DISABLE;
			ledOpen.off();
			ledClose.on();
			ledPull.off();
			if(KEY_MISSING)	
			{
				audio.playSong1();
			}
			else
			{
				audio.stopSong();
			}
			
			if(OPEN_REQUEST)
			{
				SET_BUSY;
				MOTOR_ENABLE;
				MOTOR_DIR_OPEN;
				
				audio.stop();
				audio.playSong2();
				state = STATE_OPENING;
				ledOpen.setDelay(300);
				ledClose.off();
			}
			
			break;
			
		case STATE_OPENING:
			MOTOR_DIR_OPEN;
			if(WINDOW_UNLOCKED)
			{
				state = STATE_OPEN;
				MOTOR_DISABLE;
				windowTimeout = 10000;
			}
			else
			{
				MOTOR_STEP;
			}
			break;
			
		case STATE_OPEN:
			ledOpen.on();
			if(!windowTimeout)
			{
				state = STATE_CLOSING;
				ledOpen.off();
				ledClose.setDelay(300);
				
				MOTOR_ENABLE;
				MOTOR_DIR_CLOSE;	
			}
			
			break;
			
		case STATE_CLOSING:
			MOTOR_DIR_CLOSE;
			if(WINDOW_CLOSED)
			{
				ledPull.off();
				if(WINDOW_LOCKED)
				{
					state = STATE_CLOSED;
					audio.stopSong();
				}
				else
				{
					MOTOR_STEP;
				}
			}
			else
			{
				ledPull.on();
			}
			break;
			
		default:
			ledPull.setDelay(50);
			MOTOR_DIR_CLOSE;
			MOTOR_ENABLE;
			if(WINDOW_CLOSED)
			{
				if(WINDOW_LOCKED)
				{
					state = STATE_CLOSED;
				}
				else
				{
					MOTOR_STEP;
				}
			}
			break;
		}
		
		ledPull.step((uint32_t)SysTick);
		ledOpen.step((uint32_t)SysTick);
		ledClose.step((uint32_t)SysTick);
		
		if(audioStepNeeded)
		{
			audio.step();
			audioStepNeeded = 0;
		}
    }
}

ISR(TIMER0_COMPA_vect)
{
	static uint8_t audioTick = 31;
	static uint8_t motorTick = 3;
	
	SysTick++;
	
	audioTick--;
	if(!audioTick)
	{
		audioStepNeeded = 1;
		audioTick = 27;
	}
	
	motorTick--;
	if(!motorTick)
	{
		motorStepNeeded = 1;
		motorTick = 1;
	}
	
	if(windowTimeout)
	{
		windowTimeout--;
	}
}

// configured in audio system
ISR(TIMER1_COMPA_vect)
{
	audio.onTimer();
}