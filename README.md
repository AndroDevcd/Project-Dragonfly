## Project-Dragonfly

A complete system for flying a bi-copter drone: the drone's flight computer and the
hand-held controller, both written in [Sharp](https://github.com/AndroDevcd/Sharp) with a
thin layer of C++ for the hardware, both running on a Raspberry Pi.

Named after the dragonfly - how it flies, and how it is built.

### What is in here

```
link/           the radio link both programs share, and the hardware they have in common
  src/            Sharp: frames, messages, the link itself, radio / pins / clock (real and fake)
  native/         C++: the nRF24L01 radio, the pins
controller/     the hand-held controller: display, two buttons, settings
  src/            the app, the screens, the UI toolkit, its hardware
  main/           main(): the real hardware put together
  native/         C++: the SSD1306 display
drone/          the flight computer: sensors, failsafe, telemetry
  src/  main/  native/
simulator/      both programs running together on a PC, with no hardware
tests/          everything above, tested on a PC
docs/           how it is put together (ARCHITECTURE.md)
```

`controller` and `drone` are two separate Sharp projects. Each builds on its own, and each
uses `link` as a library.

### Try it on a PC (no hardware)

Install Sharp (0.4.5 or newer), then:

```
bash test.sh                                        every test: about 200 checks
sharpc -P simulator
sharp simulator/build/outputs/simulator             14 seconds of flight, printed once a second
```

The simulator runs the real controller and the real drone program in one process, on a radio
that lives in memory, and prints what is on the controller's screen:

```
---- 3 s ------------------------------------------
  (0, 0) 30x8  "03:18"
  (112, 0) 16x9  [battery_4]
  (96, 0) 13x9  [signal_4]
  (16, 22) 12x16  "0"
  (31, 30) 24x8  "cm/s"
  (100, 18) 18x8  "ALT"
  (97, 28) 24x8  "1.50"
  drone: mode normal, connected, throttle 0, heard 1, telemetry 15, 150 cm up
```

### Build it for the Raspberry Pi

`pi_system_setup.bash` installs what a fresh Pi needs: g++, wiringPi, the RF24 library, and
Sharp (built on the Pi). Then, in a new terminal:

```
bash build.sh
sharp controller/build/outputs/controller           on the controller's Pi
sharp drone/build/outputs/drone                     on the drone's Pi
```

To stop either program type `q` and Enter: the controller blanks its screen and turns its light off,
the drone tells its motors to stop. Ctrl+C stops it at once, without the tidying.

`sharpc -P controller` builds the Sharp code and the C++ library in one go
(`controller/build/outputs/controller` and `dragonfly.so` beside it). The same command works on a
PC: there the C++ is built without hardware, so it is still compiled and checked, and the
program says "the pins cannot be used" and ends.

### The hardware

| part | what | where the code is |
|---|---|---|
| nRF24L01 | the radio, on both sides (CE on GPIO 22, SPI bus 0) | `link/native/radio_nrf24.cpp` |
| SSD1306 | the controller's 128 x 64 OLED display (I2C) | `controller/native/display_ssd1306.cpp` |
| two buttons, one light | the controller's (wiringPi pins 28, 29, 27) | `controller/src/input/buttons.sharp` |
| HC-SR04 | the drone's ultrasound height sensor (pins 6, 5) | `drone/native/distance_hc_sr04.cpp` |
| motors, battery gauge | **not built yet**: the drone accepts every command and does nothing with it | `drone/src/hardware/pi_devices.sharp` |

### How the two talk

The radio carries 32 bytes at a time and loses some of them. On top of it the link gives
both programs messages of any length - cut into frames, numbered, acknowledged, sent again
when lost - and tells them when the other side has gone quiet. Commands (a mode change) are
sent reliably; telemetry (height, speed, battery, five times a second) is sent once and
replaced by the next. When the drone stops hearing the controller it stops obeying the
sticks and descends gently. `docs/ARCHITECTURE.md` has the details.
