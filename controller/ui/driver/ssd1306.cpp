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

void draw_word(_int8_array &str, var &len, var &text_size) {
    if(len.value() > 0) {
        ssd1306_setTextSize((int)text_size.value());
        
        string data = "";
        for(long i = 0; i < len.value(); i++) {
            data += (char)str[i];
        }

        ssd1306_drawString(data.c_str());
    }
}


void draw_img(var_array &bytes, var &width, var &height, var &x, var &y, var &skip_count) {
   int xStart = x.value();
   y--;
   
   long imgCursor = 0;
   for(long i = 0; i < height.value(); i++) {
       y++;
       x = xStart;
       ssd1306_moveCursor(xStart, (int)y.value());
       
       if(y.value() >= HEIGHT)
          break;
          
       for(long j = 0; j < width.value(); j++) {
           ssd1306_moveCursor(x.value(), y.value());
           
           if(x.value() >= WIDTH) {
              imgCursor += width.value() - x.value();
           }
           
           if(bytes[imgCursor] > 0) {
               ssd1306_drawPixel(x.value(), y.value(), bytes[imgCursor] - 1);
           }
           
           imgCursor++;
           x++;
       }
       
       if(skip_count.value() > 0)
          imgCursor += skip_count.value() - 1;
   }
}


scope_end()
