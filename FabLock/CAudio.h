/* 
* CAudio.h
*
* Created: 07.01.2018 18:17:28
* Author: Marcel Jongmanns
*/


#ifndef __CAUDIO_H__
#define __CAUDIO_H__

#include <inttypes.h>
#include <avr/sfr_defs.h>

typedef struct {
	uint8_t n;
	uint16_t l;
} lied_t;

class CAudio
{
//variables
public:
protected:
	
private:
	uint8_t isPlaying;
	uint16_t phase, pos, len, frequency;
	const lied_t *song;
	
//functions
public:
	CAudio();
	~CAudio();
	
	void init();
	
	void onTimer();
	
	void stop();
	void start(uint8_t freq);
	
	void playSong1();
	void playSong2();
	void stopSong();
	void step();
protected:
private:
	CAudio( const CAudio &c );
	CAudio& operator=( const CAudio &c );

}; //CAudio

#endif //__CAUDIO_H__
