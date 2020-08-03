/******
Demo for ssd1306 i2c driver for  Raspberry Pi 
******/

#include <wiringPi.h>
#include "ssd1306_i2c.h"

int main() {

	ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

	ssd1306_display(); //Adafruit logo is visible
	ssd1306_clearDisplay();
	delay(1000);
	ssd1306_dim(1);
	
	ssd1306_setTextSize(1);
	ssd1306_moveCursor(112, 0);
	const char* text = "N";
	ssd1306_drawString(text);
	ssd1306_display();
}
