//
// Created by BNunnally on 8/4/2020.
//
#include "../../../stdimports.h"
#include "ssd1306_i2c.h"
#include <wiringPi.h>

scope_begin(ui_driver)

void setup() {
    ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);

    ssd1306_display(); //Adafruit logo is visible
    delay(1000);
    ssd1306_clearDisplay();
}

void clear_display() {
    ssd1306_clearDisplay();
}

void display() {
    ssd1306_display();
}

void dim(var &yes) {
    ssd1306_dim((int)yes.value());
}

void move_cursor(var &x, var &y) {
    ssd1306_moveCursor((int)x.value(), (int)y.value());
}

void draw_word(_int8_array &str, var &len) {
    if(len.value() > 0) {
        string data = "";
        for(long i = 0; i < len.value(); i++) {
            data += (char)str[i];
        }

        ssd1306_drawString(data.c_str());
    }
}


scope_end()
