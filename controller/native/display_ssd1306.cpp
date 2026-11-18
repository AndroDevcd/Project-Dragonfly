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
#include <wiringPiI2C.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cstdio>

// The driver's picture of the screen (vendor/ssd1306_i2c.cpp): one number per 8 pixels, a column
// of one PAGE. The screen is 8 pages of 128 columns; a page is a strip 8 pixels high.
extern unsigned int buffer[];
extern int i2cd;                        // the open I2C connection to the chip

#define PAGES (DISPLAY_HEIGHT / 8)

// what the chip was last sent, page by page - so a page that did not change is not sent again
static uint8_t sent[PAGES][DISPLAY_WIDTH];
static bool sent_valid = false;

// PUTTING A FRAME ON THE SCREEN
//
// The driver's own ssd1306_display() sends the 1024 bytes of a frame as 1024 separate I2C
// messages (its comment says so: "Should be optimized"). Each message is an address, a control
// byte and one byte of picture: about a third of a second for a frame, and the program draws 30
// a second. The screen could be watched filling in from one side, and nothing else - the radio,
// the buttons - got a turn while it did.
//
// Here a frame is sent as it should be:
//   - a PAGE that is the same as what the chip already shows is not sent at all. Most frames are
//     the same as the one before, and those now cost nothing;
//   - a page that changed is sent in pieces of CHUNK bytes, one message each.
//
// WHY PIECES, NOT ONE MESSAGE OF 128: on the Raspberry Pi a long I2C message failed (seen on a
// Pi 2: the chip took every short command, then "Input/output error" on the first 129-byte
// message - and afterwards it did not answer at all until it was powered again). 16 bytes a
// message is what Adafruit's own driver sends and works everywhere; it is still 8 messages a page
// instead of 128. The chip's address pointer moves on by itself, so the pieces need nothing
// between them.
//
// And if even a piece fails, that piece is sent the old way, one byte a message - slow, but the
// way that was known to reach this chip.
#define CHUNK 16

// one message of picture bytes (0x40: "picture follows"); false when the chip did not take it
static bool send_bytes(const uint8_t* bytes, int count) {
    uint8_t message[1 + CHUNK];
    message[0] = 0x40;
    memcpy(message + 1, bytes, count);
    if(write(i2cd, message, 1 + count) == (ssize_t) (1 + count)) return true;
    for(int i = 0; i < count; i++) {
        if(wiringPiI2CWriteReg8(i2cd, 0x40, bytes[i]) < 0) return false;
    }
    return true;
}

// Returns false when the chip did not take a message (it is not there any more).
static bool send_changed_pages() {
    bool ok = true;
    for(int page = 0; page < PAGES; page++) {
        uint8_t now[DISPLAY_WIDTH];
        for(int column = 0; column < DISPLAY_WIDTH; column++) {
            now[column] = (uint8_t) buffer[page * DISPLAY_WIDTH + column];
        }
        if(sent_valid && memcmp(now, sent[page], DISPLAY_WIDTH) == 0) continue;

        // where the bytes go: columns 0-127 of this one page - as single commands, the way the
        // driver sends every command (and the way the start-up commands just got through)
        ssd1306_command(SSD1306_COLUMNADDR);
        ssd1306_command(0);
        ssd1306_command(DISPLAY_WIDTH - 1);
        ssd1306_command(SSD1306_PAGEADDR);
        ssd1306_command(page);
        ssd1306_command(page);

        bool page_ok = true;
        for(int at = 0; at < DISPLAY_WIDTH && page_ok; at += CHUNK) {
            page_ok = send_bytes(now + at, CHUNK);
        }
        if(!page_ok) {
            ok = false;                 // not remembered as sent: it is tried again with the next frame
            continue;
        }
        memcpy(sent[page], now, DISPLAY_WIDTH);
    }
    sent_valid = ok;
    return ok;
}

// 1 when the display was started and answers
int32_t dragonfly_display_begin(sni_env* env) {
    // WHICH step fails is printed (to the error output): "does not answer" alone cannot tell a
    // display that is not there from one that drops out when it is switched on.
    ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
    if(i2cd < 0) {
        fprintf(stderr, "display: the I2C bus could not be opened (%s) - is I2C switched on? (sudo raspi-config)\n", strerror(errno));
        return 0;
    }
    // Is a chip there at all? The driver never looks at what its messages come back with, so a
    // display that is not connected used to count as started. One message that does nothing
    // (0xE3) is the question: no chip, no acknowledgement, and the write fails. It is asked a
    // few times: a chip that was just switched on may need a moment.
    uint8_t nothing[] = { 0x00, 0xE3 };
    bool answered = false;
    for(int attempt = 0; attempt < 5 && !answered; attempt++) {
        if(write(i2cd, nothing, sizeof(nothing)) == (ssize_t) sizeof(nothing)) answered = true;
        else usleep(20000);
    }
    if(!answered) {
        fprintf(stderr, "display: nothing at I2C address 0x%02X acknowledged a message after the start-up commands (%s)\n",
                SSD1306_I2C_ADDRESS, strerror(errno));
        return 0;
    }

    ssd1306_clearDisplay();
    sent_valid = false;                 // the first frame is sent whole
    if(!send_changed_pages()) {
        fprintf(stderr, "display: the chip answered, then stopped while the first picture was sent (%s)\n", strerror(errno));
        return 0;
    }
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
    send_changed_pages();
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
