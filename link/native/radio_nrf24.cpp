//
// THE RADIO: the nRF24L01, through the RF24 library
// (the Sharp side: link/src/hardware/pi_radio.sharp - class nrf24_radio)
//
// This file does ONE thing: put a 32-byte frame on the air, and hand over a frame that
// arrived. Everything above that - messages of any length, acknowledgements, resends,
// "is the other side still there" - is the link, in Sharp (link/src/link.sharp), where it
// is tested on a PC without a radio.
//
// WHAT THE OLD DRIVER DID THAT THIS ONE DOES NOT
//   - It waited 55 ms after every switch between sending and listening. The chip needs
//     130 microseconds, and the library already waits for them.
//   - It printed a line for every call. At hundreds of frames a second that was most of
//     the time spent.
//   - `listen` and `jam` could loop forever inside C++, where the VM cannot reach them.
//     Nothing here waits for the other side: every function returns at once.
//   - It built its own packet chain (a count in the first packet, a length in the last).
//     The link's frames replace it.
//   - A power level of 3 (the chip's maximum) was silently turned into 0.
//
// WIRING: CE on GPIO 22, the chip on SPI bus 0 / chip select 0, as before.
//
#include "hardware.h"
#include "dragonfly_sni.h"
#include <cstring>

#define FRAME_SIZE 32

#ifndef DRAGONFLY_NO_HARDWARE

#include <RF24/RF24.h>
#include <chrono>

static RF24 radio(22, 0);
static bool started = false;
static uint8_t power_level = 0;

// The two ends listen on each other's sending address: one byte differs.
static uint8_t controller_address[5] = { 0x1A, 0xCE, 0x2B, 0xCE, 0x5F };
static uint8_t drone_address[5]      = { 0x33, 0xCE, 0x2B, 0xCE, 0x5F };

static rf24_pa_dbm_e power_of(int32_t level) {
    switch(level) {
        case 1: return RF24_PA_LOW;
        case 2: return RF24_PA_HIGH;
        case 3: return RF24_PA_MAX;
        default: return RF24_PA_MIN;
    }
}

static rf24_datarate_e rate_of(int32_t rate) {
    switch(rate) {
        case 1: return RF24_1MBPS;
        case 2: return RF24_2MBPS;
        default: return RF24_250KBPS;     // the longest range
    }
}

// 1 when the chip answered and is listening; 0 when it did not (wiring, power)
int32_t dragonfly_radio_begin(sni_env* env, int32_t is_controller, int32_t channel, int32_t power, int32_t rate) {
    if(!radio.begin()) return 0;

    power_level = (uint8_t) (power < 0 ? 0 : power > 3 ? 3 : power);
    radio.setPALevel(power_of(power_level));
    radio.setDataRate(rate_of(rate));
    radio.setChannel((uint8_t) (channel < 0 ? 0 : channel > 125 ? 125 : channel));
    radio.setPayloadSize(FRAME_SIZE);
    // The chip's own acknowledgement and three quick resends: cheap, and it takes most of the
    // losses before the link's (slower) resend has to. The link does not depend on it.
    radio.setAutoAck(true);
    radio.setRetries(2, 3);

    if(is_controller) {
        radio.openWritingPipe(drone_address);
        radio.openReadingPipe(1, controller_address);
    } else {
        radio.openWritingPipe(controller_address);
        radio.openReadingPipe(1, drone_address);
    }
    radio.startListening();
    started = true;
    return 1;
}

// 1 when the frame left the radio. (The chip's "nobody acknowledged" is NOT an error here:
// the link finds out by itself, and a frame without an answer is normal while out of range.)
int32_t dragonfly_radio_send(sni_env* env, sni_ref frame) {
    if(!started) return 0;
    sni_array bytes = env->array(frame);
    if(bytes.data == nullptr || bytes.count <= 0) return 0;

    uint8_t out[FRAME_SIZE];
    memset(out, 0, FRAME_SIZE);
    memcpy(out, bytes.data, (size_t) (bytes.count < FRAME_SIZE ? bytes.count : FRAME_SIZE));

    radio.stopListening();
    radio.write(out, FRAME_SIZE);
    radio.startListening();
    return 1;
}

// copies the next frame into `into` (an array of at least 32 bytes); the number of bytes, 0 when none is waiting
int32_t dragonfly_radio_receive(sni_env* env, sni_ref into) {
    if(!started || !radio.available()) return 0;
    sni_array bytes = env->array(into);
    if(bytes.data == nullptr || bytes.count < FRAME_SIZE) return 0;
    radio.read(bytes.data, FRAME_SIZE);
    return FRAME_SIZE;
}

void dragonfly_radio_set_power(sni_env* env, int32_t level) {
    if(!started) return;
    power_level = (uint8_t) (level < 0 ? 0 : level > 3 ? 3 : level);
    radio.setPALevel(power_of(power_level));
}

// Disturb `channel` and the 24 above it for `millis` milliseconds (at most one second a call),
// at full power, then put everything back. Stops early when the VM asks the thread to stop.
void dragonfly_radio_jam(sni_env* env, int32_t channel, int32_t millis) {
    if(!started) return;
    if(millis > 1000) millis = 1000;
    uint8_t home = radio.getChannel();
    static const char noise[FRAME_SIZE] = "{fmju=j\"!@#$%^&\",hygt3454fr7";

    radio.stopListening();
    radio.setAutoAck(false);
    radio.setPALevel(RF24_PA_MAX);
    auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(millis);
    while(std::chrono::steady_clock::now() < until && !env->should_stop()) {
        for(int i = 0; i < 25; i++) {
            int at = channel + i;
            if(at > 125) break;
            radio.setChannel((uint8_t) at);
            radio.write(noise, FRAME_SIZE);
        }
    }
    radio.setChannel(home);
    radio.setPALevel(power_of(power_level));
    radio.setAutoAck(true);
    radio.startListening();
}

void dragonfly_radio_print_details(sni_env* env) {
    if(started) radio.printDetails();
}

#else   // ---- no hardware under this build: every function says so

int32_t dragonfly_radio_begin(sni_env* env, int32_t is_controller, int32_t channel, int32_t power, int32_t rate) { return 0; }
int32_t dragonfly_radio_send(sni_env* env, sni_ref frame) { return 0; }
int32_t dragonfly_radio_receive(sni_env* env, sni_ref into) { return 0; }
void dragonfly_radio_set_power(sni_env* env, int32_t level) { }
void dragonfly_radio_jam(sni_env* env, int32_t channel, int32_t millis) { }
void dragonfly_radio_print_details(sni_env* env) { }

#endif
