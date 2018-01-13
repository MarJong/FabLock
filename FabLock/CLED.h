/* 
* CLED.h
*
* Created: 07.01.2018 15:12:30
* Author: Marcel Jongmanns
*/


#ifndef __CLED_H__
#define __CLED_H__

#include <inttypes.h>


class CLED
{
//variables
public:
protected:
	volatile uint8_t *port;
	uint8_t pin;
	uint32_t last_tick, delay;
private:

//functions
public:
	CLED(volatile uint8_t *port, uint8_t pin);
	~CLED();
	
	void on();
	void off();
	void toggle();
	
	void setDelay(uint32_t newDelay);
	void step(uint32_t tick);
protected:
private:


}; //CLED

#endif //__CLED_H__
