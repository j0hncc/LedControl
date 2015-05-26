#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <eagle_soc.h>
#include <c_types.h>

#define LED_PIN 5 // GPIO0

//////////
// BEGIN EspNeoPixel functions.
//

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

// Holds the pin mask for EspNeoPixel
uint16_t enpPinMask;

uint32_t enpIntrMask;

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
void enpStart(uint8_t pin) {
	enpPinMask = 1 << pin;

	pinMode(pin, OUTPUT);    // set pin as output
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
void ICACHE_FLASH_ATTR enpSendByte(uint8_t data) {
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
void ICACHE_FLASH_ATTR enpSendPixel(uint8_t red, uint8_t grn, uint8_t blu) {
	enpSendByte(grn);
	enpSendByte(red);
	enpSendByte(blu);
}

//
// EspNeoPixel stop sending data.
//
// This just waits a small amount of time to enable the pixels.
//
void enpStop() {
	RESTORE_INTERRUPTS(enpIntrMask);
	//delayMicroseconds(50);
}

//
// END EspNeoPixel functions.
//////////

Timer ledT;
bool which = false;
//char buffer1[18] = "\x00\x40\x00\x40\x00\x00\x00\x00\x40\x00\x40\x00\x40\x00\x00\x00\x00\x40";
//char buffer2[18] = "\x00\x40\x00\x40\x00\x00\x00\x00\x40\x00\x40\x00\x40\x00\x00\x00\x00\x40";

char buffer1[270] = { 0x40, 0, 0, 0, 0x40, 0, 0, 0, 0x40, 0x40, 0, 0, 0, 0x40,
		0, 0, 0, 0x40 };
char buffer2[90] = { 0x40, 0, 0, 0, 0x40, 0, 0, 0, 0x40, 0x40, 0, 0, 0, 0x40, 0,
		0, 0, 0x40 };

void toggle() {
	char * buffer;
	digitalWrite(4, which);

	if (which)
		buffer = buffer1;
	else
		buffer = buffer2;

	//ws2812_writergb(LED_PIN, buffer, sizeof(buffer1));

	enpStart(LED_PIN);
	for (int i = 0; i < sizeof(buffer1); i += 3)
		enpSendPixel(buffer[i], buffer[i + 1], buffer[i + 2]);
	enpStop();

	which = !which;

}

int count = 0;

void walk() {
	digitalWrite(4, which);
	which = !which;
	int i = 0;
	char saved[3];
	saved[i] = buffer1[i];
	i++;
	saved[i] = buffer1[i];
	i++;
	saved[i] = buffer1[i];
	i++;

	i = 0;
	for (int maxx = sizeof(buffer1) - 3; i < maxx;) {
		buffer1[i] = buffer1[i + 3];
		i++;
		buffer1[i] = buffer1[i + 3];
		i++;
		buffer1[i] = buffer1[i + 3];
		i++;
	}

	buffer1[i] = saved[0];
	i++;
	buffer1[i] = saved[1];
	i++;
	buffer1[i] = saved[2];
	i++;

	//ws2812_writergb(LED_PIN, buffer1, sizeof(buffer1));

	char * buffer = buffer1;
	enpStart(LED_PIN);
	for (int ix = 0; ix < sizeof(buffer1); ix += 3)
		enpSendPixel(buffer[ix], buffer[ix + 1], buffer[ix + 2]);
	enpStop();

	count++;
	/*
	 if ( count >= 100)
	 {
	 Serial.println(millis());
	 count=0;
	 }
	 */
	ledT.startOnce();
}

Timer prT;
void prCount() {
	Serial.println(count);
}

#define PR(x) Serial.print(x);
#define PL(x) Serial.println(x);

struct bmph_t {
	union {
	struct {
		uint16_t BM;   // caution 32 bit word alignment? skip this
		uint32_t size;
		uint32_t reserved;
		uint32_t startpix;
		uint32_t hdrsize;   // only 0x28 supported
		uint32_t w, h ;
		uint16_t planes, bpp;  // only 1, 0x18 supported
	};
	uint8_t raw[30];
	};
} bmph;

void checkFile()
{
	char * p;
	//struct { uint16_t bm; uint32_t size, resrvd, startpix, hdrsiz, w, h; } my;

	FileStream f("flag.bmp");
	f.getDataPointer( &p);
	/*
	os_memcpy( &bmph, p, sizeof(bmph));
	for (int i=0; i< sizeof(bmph.raw) ; i++)
		Serial.println( int (bmph.raw[i]) , HEX);
	PR( "BM: "); PL( bmph.BM);
	PR( "size: "); PL( bmph.size);
	PR( "startpix: "); PL( bmph.startpix);
	PR( "width: "); PL( bmph.w);
	*/
	f.seek(0x00); f.getDataPointer( &p); os_memcpy( &bmph.BM, p, 2);
	f.seek(0x02); f.getDataPointer( &p); os_memcpy( &bmph.size, p, 28);
	PR( "BM: "); PL( bmph.BM);
	PR( "size: "); PL( bmph.size);
	PR( "startpix: "); PL( bmph.startpix);
	PR( "hdrsize: " ); PL( bmph.hdrsize );
	PR( "width: " ); PL( bmph.w);
	PR( "height: " ); PL( bmph.h);
	PR( "planes: " ); PL( bmph.planes);
	PR( "bpp: " ); PL( bmph.bpp);

}

void init() {
	pinMode(LED_PIN, OUTPUT);
	pinMode(4, OUTPUT);
	Serial.begin(115200);
	Serial.println("Begin");
	checkFile();
	Serial.print("Buff1: ");
	Serial.println(sizeof(buffer1));
	Serial.print("Heap: ");
	Serial.println(system_get_free_heap_size());

	prT.initializeMs(1000, prCount).start();
	ledT.initializeUs(500, walk).startOnce();

}
