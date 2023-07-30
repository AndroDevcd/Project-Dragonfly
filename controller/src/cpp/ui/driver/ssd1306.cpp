//
// Created by BNunnally on 8/4/2020.
//
#include "../../../../../stdimports.h"
#include "ssd1306_i2c.h"
#include <wiringPi.h>
#include "../../../../generated/snb_api.h"

using namespace snb_api;

scope_begin(ui_driver)

void setup() {
    ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
    ssd1306_clearDisplay();
}

void clear_display() {
    ssd1306_clearDisplay();
}

void display() {
    ssd1306_display();
}

void dim(var yes) {
    ssd1306_dim((int)yes);
}

void move_cursor(var x, var y) {
    ssd1306_moveCursor((int)x, (int)y);
}

void draw_word(SharpObject str, var len, var text_size) {
    if(len > 0) {
        ssd1306_setTextSize((int)text_size);
        long double *rawData = internal::get_raw_number_data(str);
        
        string data = "";
        for(long i = 0; i < len; i++) {
            data += (char)rawData[i];
        }

        ssd1306_drawString(data.c_str());
    }
}


void draw_img(SharpObject bytes, var width, var height, var x, var y, var skip_count) {
   int xStart = x;
   y--;
   
   long imgCursor = 0;
   for(long i = 0; i < height; i++) {
       y++;
       x = xStart;
       ssd1306_moveCursor(xStart, (int)y);
       long double *rawData = internal::get_raw_number_data(bytes);
       
       if(y >= HEIGHT)
          break;
          
       for(long j = 0; j < width; j++) {
           ssd1306_moveCursor(x, y);
           
           if(x >= WIDTH) {
              imgCursor += width - x;
           }
           
           if(rawData[imgCursor] > 0) {
               ssd1306_drawPixel(x, y, rawData[imgCursor] - 1);
           }
           
           imgCursor++;
           x++;
       }
       
       if(skip_count > 0)
          imgCursor += skip_count - 1;
   }
}


scope_end()
