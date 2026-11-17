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
#include <thread>

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

// IS THE CHIP STILL THERE? asked before anything that talks to it.
//
// If the radio is unplugged (or its power or a wire fails) while the program runs, the RF24
// library's write() waits in a loop for a "finished" bit from a chip that will never answer, and
// the whole controller freezes - the screen, the buttons, everything, because it is all one loop.
// isChipConnected() reads one register and checks it makes sense; it cannot hang. Every call to
// the radio below asks it first, so a radio that has gone away just means "no frame" / "did not
// send", and the controller keeps running.
static bool chip_there() {
    return started && radio.isChipConnected();
}

// 1 when the chip answered and is listening; 0 when it did not (wiring, power)
int32_t dragonfly_radio_begin(sni_env* env, int32_t is_controller, int32_t channel, int32_t power, int32_t rate) {
    if(!radio.begin()) return 0;
    if(!radio.isChipConnected()) return 0;

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
    if(!chip_there()) return 0;
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
    if(!chip_there() || !radio.available()) return 0;
    sni_array bytes = env->array(into);
    if(bytes.data == nullptr || bytes.count < FRAME_SIZE) return 0;
    radio.read(bytes.data, FRAME_SIZE);
    return FRAME_SIZE;
}

void dragonfly_radio_set_power(sni_env* env, int32_t level) {
    if(!chip_there()) return;
    power_level = (uint8_t) (level < 0 ? 0 : level > 3 ? 3 : level);
    radio.setPALevel(power_of(power_level));
}

// Disturb ONE channel for `millis` milliseconds (at most one second a call), then put everything
// back. Stops early when the VM asks the thread to stop.
//
// It sends a CONSTANT CARRIER: the chip holds an unmodulated signal on the channel the whole time.
// The first version hopped across 25 channels writing packets, so each channel got a few
// microseconds out of every pass - far too little to drown out anything, which is why a set
// channel's traffic did not drop. Sitting on the one channel with a carrier at full power is the
// most a radio like this can do, and it actually blocks it.
void dragonfly_radio_jam(sni_env* env, int32_t channel, int32_t millis) {
    if(!chip_there()) return;
    if(channel < 0 || channel > 125) return;
    if(millis > 1000) millis = 1000;
    uint8_t home = radio.getChannel();

    radio.stopListening();
    radio.setAutoAck(false);
    // a steady carrier on this one channel, at the most power the chip has
    radio.startConstCarrier(RF24_PA_MAX, (uint8_t) channel);

    auto until = std::chrono::steady_clock::now() + std::chrono::milliseconds(millis);
    while(std::chrono::steady_clock::now() < until && !env->should_stop()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    radio.stopConstCarrier();
    radio.setChannel(home);
    radio.setPALevel(power_of(power_level));
    radio.setAutoAck(true);
    radio.startListening();
}

void dragonfly_radio_print_details(sni_env* env) {
    if(chip_there()) radio.printDetails();
}

void dragonfly_radio_set_channel(sni_env* env, int32_t channel) {
    if(!chip_there() || channel < 0 || channel > 125) return;
    radio.setChannel((uint8_t) channel);
}

// IS ANYTHING STRONG ON THESE CHANNELS? - the chip's "received power detector" (RPD): after
// listening on a channel for at least 130 microseconds it says whether a signal of about -64 dBm
// or more was there. That is all the chip can tell: yes or no. (The RF24 library's own "scanner"
// example does exactly this.)
//
// `into` gets 1 or 0 for channel first, first + 1 ...; the radio is back on its own channel and
// listening when this returns. Returns how many channels were asked. At most 16 a call, about
// 3 ms: the program's loop is not held up for longer.
int32_t dragonfly_radio_sense(sni_env* env, int32_t first, int32_t count, sni_ref into) {
    if(!chip_there()) return 0;
    sni_array answers = env->array(into);
    if(answers.data == nullptr) return 0;
    if(count > 16) count = 16;
    if(count > answers.count) count = (int32_t) answers.count;
    uint8_t* out = (uint8_t*) answers.data;
    uint8_t home = radio.getChannel();

    int32_t asked = 0;
    radio.stopListening();
    for(int32_t i = 0; i < count; i++) {
        int32_t channel = first + i;
        if(channel < 0) continue;
        if(channel > 125) break;
        radio.setChannel((uint8_t) channel);
        radio.startListening();
        std::this_thread::sleep_for(std::chrono::microseconds(140));
        bool heard = radio.testRPD();
        radio.stopListening();
        heard = heard || radio.testRPD();       // (what the library's example does too: the bit may set late)
        out[i] = heard ? 1 : 0;
        asked++;
    }
    radio.setChannel(home);
    radio.startListening();
    return asked;
}

#else   // ---- no hardware under this build: every function says so

int32_t dragonfly_radio_begin(sni_env* env, int32_t is_controller, int32_t channel, int32_t power, int32_t rate) { return 0; }
int32_t dragonfly_radio_send(sni_env* env, sni_ref frame) { return 0; }
int32_t dragonfly_radio_receive(sni_env* env, sni_ref into) { return 0; }
void dragonfly_radio_set_power(sni_env* env, int32_t level) { }
void dragonfly_radio_jam(sni_env* env, int32_t channel, int32_t millis) { }
void dragonfly_radio_print_details(sni_env* env) { }
void dragonfly_radio_set_channel(sni_env* env, int32_t channel) { }
int32_t dragonfly_radio_sense(sni_env* env, int32_t first, int32_t count, sni_ref into) { return 0; }

#endif
