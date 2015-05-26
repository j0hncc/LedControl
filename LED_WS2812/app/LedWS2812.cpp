/*
 * LedWS2812.cpp
 *
 *  Created on: May 26, 2015
 *      Author: John
 */

#include "LedWS2812.h"
#include <SmingCore/SmingCore.h>

LedWS2812::LedWS2812(uint8_t pin) : pin(pin) {
	enpPinMask = 1 << pin;
	pinMode(pin, OUTPUT);    // set pin as output
}

LedWS2812::~LedWS2812() {
	stop();
}

// Clear and return current interrupt level.
#define CLEAR_INTERRUPTS() ({ \
  uint32_t __lvl; \
  __asm__ __volatile__(   "rsil %0, 15 \n" \
                        : "=a" (__lvl) : : "memory" ); \
  __lvl; \
})

// Restore interrupt level.
#define RESTORE_INTERRUPTS(lvl) do { \
  uint32_t __lvl = (lvl); \
  __asm__ __volatile__( "wsr %0, PS \n" \
                        "rsync \n"      \
                        : : "a" (__lvl) : "memory" ); \
} while(0)

// The number of cycles per nanosecond.
#define NS_TO_CYCLES(n) (n / (1000000000L / F_CPU))

// Store the cycle count to a variable.
#define CYCLE_COUNT() ({ \
  uint32_t __cc; \
  __asm__ __volatile__("rsr %0, CCOUNT \n" : "=a" (__cc)); \
  __cc; \
})

// Delay an arbitrary number of cycles past the specified cycle count.
#define DELAY_CYCLES(cs, cycles) do { \
  uint32_t __cc; \
  do { \
    __cc = CYCLE_COUNT(); \
  } while ((__cc - cs) < cycles); \
} while(0)

// Set pins high for a specified number of nanoseconds.
#define DIGITAL_HIGH_NS(mask, ns) do { \
  uint32_t __cs; \
  __cs = CYCLE_COUNT(); \
  GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, mask); \
  DELAY_CYCLES(__cs, NS_TO_CYCLES(ns)); \
} while(0)

// Set pins low for a specified number of nanoseconds.
#define DIGITAL_LOW_NS(mask, ns) do { \
  uint32_t __cs; \
  __cs = CYCLE_COUNT(); \
  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, mask); \
  DELAY_CYCLES(__cs, NS_TO_CYCLES(ns)); \
} while(0)

//
// EspNeoPixel start.
//
// This disable interrupts and enables the pin for output.
//
// Call this before a sequence of pin updates with the pin the WS2812's are hooked up to.
// Note that this could be make to accept a pinMask in case you wanted to control multiple
// strips at the same time (albeit with the same colors).
//
// pin - The pin the WS2812's are hooked up to.
//
void LedWS2812::start() {

	digitalWrite(pin, LOW);  // set it low
	delay(10);
	enpIntrMask = CLEAR_INTERRUPTS();
}

//
// EspNeoPixel send byte.
//
// This sends a byte out to the WS2812. It should be called three times per pixel,
// once for green, once for red, and once for blue.
//
// The timing in this function is absolutely critical, more info can be found here:
// http://wp.josh.com/2014/05/13/ws2812-neopixels-are-not-so-finicky-once-you-get-to-know-them/
//
// val - An unsigned byte encoded with 8 bits of color information.
//
void ICACHE_FLASH_ATTR LedWS2812::sendByte(uint8_t data) {
	uint8_t b = 8;
	while (b > 0) {   // loop 8 times, once for each bit
		b--;
		if ((data & (1 << b))) {             // check if current bit is high/low
			DIGITAL_HIGH_NS(enpPinMask, 700);  // send WS2812 '1'
			DIGITAL_LOW_NS(enpPinMask, 600);
		} else {
			//enpIntrMask = CLEAR_INTERRUPTS();  // according to josh this is the only part that needs protecting!
			DIGITAL_HIGH_NS(enpPinMask, 350);  // send WS2812 '0'
			DIGITAL_LOW_NS(enpPinMask, 800);
			//RESTORE_INTERRUPTS(enpIntrMask);
		}
	}
}

//
// EspNeoPixel send pixel.
//
// This sends out three bytes (in the correct order) to light a single pixel.
//
// red - unsigned byte of red color information
// grn - unsigned byte of green color information
// blu - unsigned byte of blue color information
//
void ICACHE_FLASH_ATTR LedWS2812::sendPixel(uint8_t red, uint8_t grn, uint8_t blu) {
	sendByte(grn);
	sendByte(red);
	sendByte(blu);
}

//
// EspNeoPixel stop sending data.
//
// This just waits a small amount of time to enable the pixels.
//
void LedWS2812::stop( bool delay) {
	RESTORE_INTERRUPTS(enpIntrMask);
	if (delay) delayMicroseconds(50);
}

//
// END EspNeoPixel functions.
//////////
