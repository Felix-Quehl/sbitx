#include <stdio.h>
#include <linux/types.h>
#include <stdint.h>
#include <string.h>
#include <wiringPi.h>
#include "i2cbb.h"
#include "oled.h"

/* 
The oled framebuffer is divided into 8 pages, top to bottom.
Each page is 8 rows wide. Each vertical row is represented by a byte
*/

#define SDA 23 
#define SCL 22

static uint8_t oled_bmp[10000];
static uint8_t oled_pages = 8; 

// 6x8 technoblogy font
const static uint8_t font[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 
   0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 
   0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00, 
   0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00, 
   0x23, 0x13, 0x08, 0x64, 0x62, 0x00, 
   0x36, 0x49, 0x56, 0x20, 0x50, 0x00, 
   0x00, 0x08, 0x07, 0x03, 0x00, 0x00, 
   0x00, 0x1C, 0x22, 0x41, 0x00, 0x00, 
   0x00, 0x41, 0x22, 0x1C, 0x00, 0x00, 
   0x2A, 0x1C, 0x7F, 0x1C, 0x2A, 0x00, 
   0x08, 0x08, 0x3E, 0x08, 0x08, 0x00, 
   0x00, 0x80, 0x70, 0x30, 0x00, 0x00, 
   0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 
   0x00, 0x00, 0x60, 0x60, 0x00, 0x00, 
   0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 
   0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00, 
   0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 
   0x72, 0x49, 0x49, 0x49, 0x46, 0x00, 
   0x21, 0x41, 0x49, 0x4D, 0x33, 0x00, 
   0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 
   0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 
   0x3C, 0x4A, 0x49, 0x49, 0x31, 0x00, 
   0x41, 0x21, 0x11, 0x09, 0x07, 0x00, 
   0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 
   0x46, 0x49, 0x49, 0x29, 0x1E, 0x00, 
   0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 
   0x00, 0x40, 0x34, 0x00, 0x00, 0x00, 
   0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 
   0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 
   0x00, 0x41, 0x22, 0x14, 0x08, 0x00, 
   0x02, 0x01, 0x59, 0x09, 0x06, 0x00, 
   0x3E, 0x41, 0x5D, 0x59, 0x4E, 0x00, 
   0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00, 
   0x7F, 0x49, 0x49, 0x49, 0x36, 0x00, 
   0x3E, 0x41, 0x41, 0x41, 0x22, 0x00, 
   0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00, 
   0x7F, 0x49, 0x49, 0x49, 0x41, 0x00, 
   0x7F, 0x09, 0x09, 0x09, 0x01, 0x00, 
   0x3E, 0x41, 0x41, 0x51, 0x73, 0x00, 
   0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00, 
   0x00, 0x41, 0x7F, 0x41, 0x00, 0x00, 
   0x20, 0x40, 0x41, 0x3F, 0x01, 0x00, 
   0x7F, 0x08, 0x14, 0x22, 0x41, 0x00, 
   0x7F, 0x40, 0x40, 0x40, 0x40, 0x00, 
   0x7F, 0x02, 0x1C, 0x02, 0x7F, 0x00, 
   0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00, 
   0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00, 
   0x7F, 0x09, 0x09, 0x09, 0x06, 0x00, 
   0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00, 
   0x7F, 0x09, 0x19, 0x29, 0x46, 0x00, 
   0x26, 0x49, 0x49, 0x49, 0x32, 0x00, 
   0x03, 0x01, 0x7F, 0x01, 0x03, 0x00, 
   0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00, 
   0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00, 
   0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00, 
   0x63, 0x14, 0x08, 0x14, 0x63, 0x00, 
   0x03, 0x04, 0x78, 0x04, 0x03, 0x00, 
   0x61, 0x59, 0x49, 0x4D, 0x43, 0x00, 
   0x00, 0x7F, 0x41, 0x41, 0x41, 0x00, 
   0x02, 0x04, 0x08, 0x10, 0x20, 0x00, 
   0x00, 0x41, 0x41, 0x41, 0x7F, 0x00, 
   0x04, 0x02, 0x01, 0x02, 0x04, 0x00, 
   0x40, 0x40, 0x40, 0x40, 0x40, 0x00, 
   0x00, 0x03, 0x07, 0x08, 0x00, 0x00, 
   0x20, 0x54, 0x54, 0x78, 0x40, 0x00, 
   0x7F, 0x28, 0x44, 0x44, 0x38, 0x00, 
   0x38, 0x44, 0x44, 0x44, 0x28, 0x00, 
   0x38, 0x44, 0x44, 0x28, 0x7F, 0x00, 
   0x38, 0x54, 0x54, 0x54, 0x18, 0x00, 
   0x00, 0x08, 0x7E, 0x09, 0x02, 0x00, 
   0x18, 0xA4, 0xA4, 0x9C, 0x78, 0x00, 
   0x7F, 0x08, 0x04, 0x04, 0x78, 0x00, 
   0x00, 0x44, 0x7D, 0x40, 0x00, 0x00, 
   0x20, 0x40, 0x40, 0x3D, 0x00, 0x00, 
   0x7F, 0x10, 0x28, 0x44, 0x00, 0x00, 
   0x00, 0x41, 0x7F, 0x40, 0x00, 0x00, 
   0x7C, 0x04, 0x78, 0x04, 0x78, 0x00, 
   0x7C, 0x08, 0x04, 0x04, 0x78, 0x00, 
   0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 
   0xFC, 0x18, 0x24, 0x24, 0x18, 0x00, 
   0x18, 0x24, 0x24, 0x18, 0xFC, 0x00, 
   0x7C, 0x08, 0x04, 0x04, 0x08, 0x00, 
   0x48, 0x54, 0x54, 0x54, 0x24, 0x00, 
   0x04, 0x04, 0x3F, 0x44, 0x24, 0x00, 
   0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00, 
   0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00, 
   0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00, 
   0x44, 0x28, 0x10, 0x28, 0x44, 0x00, 
   0x4C, 0x90, 0x90, 0x90, 0x7C, 0x00, 
   0x44, 0x64, 0x54, 0x4C, 0x44, 0x00, 
   0x00, 0x08, 0x36, 0x41, 0x00, 0x00, 
   0x00, 0x00, 0x77, 0x00, 0x00, 0x00, 
   0x00, 0x41, 0x36, 0x08, 0x00, 0x00, 
   0x02, 0x01, 0x02, 0x04, 0x02, 0x00, 
   0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };

void oled_refresh() {
  uint8_t renderingFrame = 0xB0;
	uint8_t oled_sequence[4]; 
	int i = 0;

	for (uint8_t i = 0; i < oled_pages; i++){
		oled_sequence[0] = 0xB0 | i;
		oled_sequence[1] = 0x0;
		oled_sequence[2] = 0x10;

 		int e = i2cbb_write_i2c_block_data (OLED_ADDR, OLED_COMMAND, 4, oled_sequence);
		if (e)
			printf("oled_write: error writing ssd1306 register at %d index\n", e);
	
 		e = i2cbb_write_i2c_block_data (OLED_ADDR, OLED_DATA, 128, oled_bmp + (i * 128));
		if (e)
			printf("oled_write: error writing ssd1306 frame buffer at %d index\n", e);
	}
	
}


int oled_init(){
  i2cbb_init(SDA, SCL); //just in case...
 	int e = i2cbb_write_i2c_block_data (OLED_ADDR, OLED_COMMAND, sizeof(oled_init_sequence), oled_init_sequence);
	if (e){
		printf("oled display not detected\n");
		return -1;
	}
	return 0;
}

const char *oled_write(uint8_t col, uint8_t row, const char *string){
	int offset = (128 * row) + col + 2;
	uint8_t *b = oled_bmp + offset;

	while(*string >= ' '){
		const uint8_t *f = font + ((*string-32) * 6);
		for (int j = 0; j < 6; j++)
			*b++ = *f++;	
		string++;
	}
	return string;
}

#define MAX_LINES 100
#define MAX_COLS 20
static char circular_buffer[MAX_COLS * MAX_LINES];

void oled_console(int style, char *string){
	
}
void oled_clear(){
	memset(oled_bmp, 0, sizeof(oled_bmp));
}
/*
int main(int argc, char **argvc){
	wiringPiSetup();
  i2cbb_init(SDA, SCL);
	delay(10);

	oled_init();
	delay(100);
	memset(oled_bmp, 0, sizeof(oled_bmp));
//	memset(oled_bmp+512, 0xff, 128);
	oled_refresh();
	delay(1000);
	oled_write(3,1, "1234567890123456");
	oled_refresh();
}
*/
