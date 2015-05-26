#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "LedWS2812.h"
//#include <eagle_soc.h>
//#include <c_types.h>

#define LED_PIN 5 // GPIO0

LedWS2812 strip1( LED_PIN);

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

	strip1.start();
	for (int i = 0; i < sizeof(buffer1); i += 3)
		strip1.sendPixel(buffer[i], buffer[i + 1], buffer[i + 2]);
	strip1.stop();

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
	strip1.start();
	for (int ix = 0; ix < sizeof(buffer1); ix += 3)
		strip1.sendPixel(buffer[ix], buffer[ix + 1], buffer[ix + 2]);
	strip1.stop();

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
