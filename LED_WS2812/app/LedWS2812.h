/*
 * LedWS2812.h
 *
 *  Created on: May 26, 2015
 *      Author: John
 */

#ifndef APP_LEDWS2812_H_
#define APP_LEDWS2812_H_
#include <SmingCore/SmingCore.h>


/**
 From esp8266.com Forum post http://www.esp8266.com/viewtopic.php?f=32&t=2723&hilit=ws2812&start=33
   "Anyone get neopixel /ws8212 addressable leds working?"
   Post by Jason Stapels 5/5/2015 9:12 pm.
   This was result of a long thread with contributions I believe by M. Gritch, Makuna, J. Stapels, maybe others.
   Uses CCOUNT cycle count for timing.
   And disables ALL interrupts during refresh to leds.

   Adapted to C++ Class to allow app control of several strips.  J. Clonts 5/26/15
*/

class LedWS2812 {
	uint8_t pin;
	uint16_t enpPinMask;
	uint32_t enpIntrMask;

public:
	LedWS2812(uint8_t pin);
	virtual ~LedWS2812();

	void start();
	void stop(bool delay = false);
	void sendPixel(uint8_t red, uint8_t grn, uint8_t blu);
	void sendByte(uint8_t byt);
};

#endif /* APP_LEDWS2812_H_ */
