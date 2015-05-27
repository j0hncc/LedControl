#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "LedWS2812.h"
#include "tpm2net.h"

LedWS2812 stripA( 4), stripB( 5) ;  // gpio4, gpio5

Timer ledT;
bool which = false;
//char buffer1[18] = "\x00\x40\x00\x40\x00\x00\x00\x00\x40\x00\x40\x00\x40\x00\x00\x00\x00\x40";
//char buffer2[18] = "\x00\x40\x00\x40\x00\x00\x00\x00\x40\x00\x40\x00\x40\x00\x00\x00\x00\x40";

char buffer1[40*3] = { 0x40, 0, 0,   0, 0x40, 0,   0, 0, 0x40,   0x40, 0, 0,   0, 0x40, 0,   0, 0, 0x40 };
char buffer2[40*3] = { 0x40, 0x40, 0,   0x40, 0x40, 0,   0, 0x40, 0x40,   0, 0x40, 0x40,   0, 0x40, 0,   0, 0, 0x40 };

void writeBuffOnLed( char * buf, int len, LedWS2812 & strip)
{
	strip.start();
	for (int i = 0; i < len; i += 3)
		strip.sendPixel(buf[i], buf[i + 1], buf[i + 2]);
	strip.stop(false);
}


void toggle() {
	char * buffer;

	if (which)
		buffer = buffer1;
	else
		buffer = buffer2;

	writeBuffOnLed( buffer, sizeof(buffer1), stripA);

	which = !which;
}

int framecount = 0;


void rotateBuff( char * buffer, int len) {
	int i = 0;
	char saved[3];
	saved[i] = buffer[i];
	i++;
	saved[i] = buffer[i];
	i++;
	saved[i] = buffer[i];
	i++;

	i = 0;
	for (int maxx = len - 3; i < maxx;) {
		buffer[i] = buffer[i + 3];
		i++;
		buffer[i] = buffer[i + 3];
		i++;
		buffer[i] = buffer[i + 3];
		i++;
	}

	buffer[i] = saved[0];
	i++;
	buffer[i] = saved[1];
	i++;
	buffer[i] = saved[2];
	i++;
}

void walk()
{
	rotateBuff( buffer1, sizeof(buffer1));
	writeBuffOnLed( buffer1, sizeof(buffer1), stripA);
	rotateBuff( buffer2, sizeof(buffer2));
	writeBuffOnLed( buffer2, sizeof(buffer2), stripB);
	framecount++;
	ledT.startOnce();

}

#define PR(x) Serial.print(x);
#define PL(x) Serial.println(x);

Timer prT;
void prCount() {
	static int last=0;
	PR( framecount - last);
	last=framecount;

	PR( " , ");
	PL(framecount);
}


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

	FileStream f("flag.bmp");
	f.getDataPointer( &p);
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

void tpm2Recv( char * buff, int len)
{
	// first 30 go to stripA, 2nd 30 to stripB
	if (len > 90)
	{
		writeBuffOnLed( buff, 90, stripA);
		writeBuffOnLed( buff+90 , len - 90, stripB);
	}
	else
		writeBuffOnLed( buff, len, stripA);

}
void connectOk()
{
	debugf("I'm CONNECTED");
	Serial.println(WifiStation.getIP().toString());
	tpm2net_init( tpm2Recv);
}
void connectFail()
{
	debugf("I'm NOT CONNECTED!");
	WifiStation.waitConnection(connectOk, 10, connectFail); // Repeat and check again
}

void init() {
	Serial.begin(115200);
	Serial.println("Begin");
	WifiAccessPoint.enable( false);
	WifiStation.enable( true);
	WifiStation.config("ssid", "password");

	//checkFile();
	Serial.print("Buff1: ");
	Serial.println(sizeof(buffer1));
	Serial.print("Heap: ");
	Serial.println(system_get_free_heap_size());

	//prT.initializeMs(1000, prCount).start();
	// ledT.initializeUs(600, walk).startOnce();

	WifiStation.waitConnection(connectOk, 30, connectFail); // We recommend 20+ seconds at start

}
