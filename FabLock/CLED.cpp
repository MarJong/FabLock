/* 
* CLED.cpp
*
* Created: 07.01.2018 15:12:30
* Author: Marcel Jongmanns
*/


#include "CLED.h"

// default constructor
CLED::CLED(volatile uint8_t *port, uint8_t pin)
{
	this->port = port;
	this->pin = pin;
	this->delay = 0;
	this->last_tick = 0;
} //CLED

// default destructor
CLED::~CLED()
{
} //~CLED

void CLED::on()
{
	*(this->port) |= (1 << this->pin);
	this->delay = 0;
}

void CLED::off()
{
	*(this->port) &= ~(1 << this->pin);
	this->delay = 0;
}

void CLED::toggle()
{
	*(this->port) ^= (1 << this->pin);
	this->delay = 0;
}

void CLED::setDelay(uint32_t newDelay)
{
	this->delay = newDelay;
}

void CLED::step(uint32_t tick)
{
	if(this->delay == 0)
	{
		return;
	}
	
	if((tick - this->last_tick) >= this->delay)
	{
		*(this->port) ^= (1 << this->pin);
		this->last_tick = tick;
	}
}