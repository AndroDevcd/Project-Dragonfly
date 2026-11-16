//
// THE DISPLAY: the SSD1306 OLED, 128 x 64, over I2C
// (the Sharp side: controller/src/hardware/pi_display.sharp - class ssd1306_display)
//
// The chip's driver is in vendor/ (ssd1306_i2c.cpp, with its font); this file is the six
// functions the controller's `display` interface needs, and nothing else. The old version
// took every pixel of a picture as a 64-bit decimal and every character one call at a
// time; a picture is now read straight out of the Sharp array, one byte a pixel.
//
#include "hardware.h"
#include "dragonfly_sni.h"
#include <string>

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#ifndef DRAGONFLY_NO_HARDWARE

#include "vendor/ssd1306_i2c.h"

// 1 when the display was started
int32_t dragonfly_display_begin(sni_env* env) {
    ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
    ssd1306_clearDisplay();
    ssd1306_display();
    return 1;
}

void dragonfly_display_clear(sni_env* env) {
    ssd1306_clearDisplay();
}

void dragonfly_display_text(sni_env* env, int32_t x, int32_t y, int32_t size, sni_ref text) {
    std::string value = env->text(text);
    if(value.empty()) return;
    ssd1306_setTextSize(size < 1 ? 1 : size);
    ssd1306_moveCursor(x, y);
    ssd1306_drawString(value.c_str());
}

// `pixels` is width * height bytes, row by row: 0 keep, 1 dark, 2 lit, 3 invert
// (the chip's colours are one less: BLACK 0, WHITE 1, INVERSE 2)
void dragonfly_display_picture(sni_env* env, int32_t x, int32_t y, int32_t width, int32_t height, sni_ref pixels) {
    sni_array bytes = env->array(pixels);
    if(bytes.data == nullptr || bytes.count < (int64_t) width * height) return;
    const uint8_t* data = (const uint8_t*) bytes.data;

    for(int32_t row = 0; row < height; row++) {
        int32_t at_y = y + row;
        if(at_y < 0 || at_y >= DISPLAY_HEIGHT) continue;
        for(int32_t column = 0; column < width; column++) {
            int32_t at_x = x + column;
            if(at_x < 0 || at_x >= DISPLAY_WIDTH) continue;
            uint8_t pixel = data[row * width + column];
            if(pixel > 0) ssd1306_drawPixel(at_x, at_y, (unsigned int) (pixel - 1));
        }
    }
}

void dragonfly_display_show(sni_env* env) {
    ssd1306_display();
}

void dragonfly_display_dim(sni_env* env, int32_t yes) {
    ssd1306_dim(yes ? 1 : 0);
}

#else   // ---- no hardware under this build

int32_t dragonfly_display_begin(sni_env* env) { return 0; }
void dragonfly_display_clear(sni_env* env) { }
void dragonfly_display_text(sni_env* env, int32_t x, int32_t y, int32_t size, sni_ref text) { }
void dragonfly_display_picture(sni_env* env, int32_t x, int32_t y, int32_t width, int32_t height, sni_ref pixels) { }
void dragonfly_display_show(sni_env* env) { }
void dragonfly_display_dim(sni_env* env, int32_t yes) { }

#endif
