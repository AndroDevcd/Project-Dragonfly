# How Project Dragonfly is put together

## 1. The one idea

**Every program is an object that is stepped.**

```
app := new controller_app(devices);        computer := new flight_computer(devices);
for(;;) { app.step(); }                    for(;;) { computer.step(); }
```

A step does what is due *now* and returns: read the frames that arrived, send what has to go
out, read the buttons, draw a frame. Nothing waits and nothing runs in a thread of its own.
The old code had a loop per job - one waiting for a button, one polling the radio, one per
screen updating a picture - which needed four threads, locks between them, and could not be
tested without the hardware.

Because a step only talks to its devices through small interfaces, the same program runs on
a Raspberry Pi and inside a test:

```
              radio      clock      gpio      display      distance_sensor
on the Pi     nRF24L01   system     wiringPi  SSD1306      HC-SR04           (C++ behind them)
in a test     fake_radio fake_clock fake_gpio fake_display fake_distance_sensor
```

A test moves the clock itself, presses buttons by setting pins, loses radio frames on
purpose, and asks the fake display what was drawn where.

## 2. The three projects

```
            +------------------+          +------------------+
            |    controller    |          |      drone       |
            |  app, screens,   |          | flight_computer, |
            |  ui, buttons     |          | altimeter        |
            +---------+--------+          +---------+--------+
                      |                             |
                      +-------------+---------------+
                                    |
                          +---------+---------+
                          |       link        |
                          | radio_link,       |
                          | frames, messages, |
                          | radio/gpio/clock  |
                          +-------------------+
```

`link` is a library (a folder of Sharp files each project names in its `settings.json`).
`controller` and `drone` never see each other's code: all they share is the link and the
messages in `link/src/messages`.

Each project keeps its code in `src/` and its `main()` in `main/`. That split is what lets
the tests and the simulator use a project's code without its `main`.

## 3. The link

### Frames

The radio carries 32 bytes. A message of any length is cut into pieces of 25 bytes:

```
byte 0      the kind of message
byte 1      flags: RELIABLE, LAST, ACK
byte 2-3    the message's number
byte 4-5    which piece of it (0, 1, 2 ...)
byte 6      how many bytes follow (0 - 25)
byte 7-31   the bytes
```

65,536 pieces of 25 bytes: over 1.6 million bytes a message. (`link/src/protocol/frame.sharp`)

### Reliable and not

| | sent | lost frame | used for |
|---|---|---|---|
| `send(m)` | one piece at a time; the next after the last was acknowledged | sent again after 40 ms, up to 8 times; then the message has *failed* and the sender is told | commands: mode, jam, notices |
| `publish(m)` | all pieces at once, never again | the receiver throws the rest of that message away | telemetry, sticks, heartbeats |

A reliable message is delivered **once** and **in order**: a piece that arrives twice
(because its acknowledgement was lost) is acknowledged again and not kept twice.

### The connection

```
DISCONNECTED --hello acknowledged / hello heard--> CONNECTED
CONNECTED    --nothing heard for 1.5 s----------> LOST
LOST         --anything heard-------------------> CONNECTED
```

A side with nothing to say sends a heartbeat every 250 ms, so silence means "out of reach".
(`link/src/link.sharp` - the whole protocol is that one file, about 300 lines.)

### Messages

Each message is a small class with `to_message()` and `from_message(m)`
(`link/src/messages/`). To add one: a number in `message_types`, a class, a test.

| message | direction | reliable | bytes |
|---|---|---|---|
| `set_mode` | controller -> drone | yes | mode |
| `steer` | controller -> drone | no | throttle, yaw, pitch, roll |
| `jam` | controller -> drone | yes | channel |
| `set_channel` | controller -> drone | yes | channel |
| `telemetry` | drone -> controller | no | altitude, speed, battery, mode |
| `notice` | drone -> controller | yes | level, text of any length |

### Channels

Both sides start on the home channel, 121. The settings' **Channel** line opens a scan of all
126 channels (`link/src/scanner.sharp`): a bar per channel for how often something strong was
heard there, with the numbers 0, 25, 50, 75, 100, 125 under them and Wi-Fi 1, 6 and 11 marked.
Both buttons together move to the quietest channel:

1. the controller sends `set_channel` on the current channel;
2. the drone's link acknowledges it, and then the drone moves;
3. the controller moves only when that acknowledgement has arrived.

If the acknowledgement is lost, the drone has moved and the controller has not. Neither hears the
other, and each side goes back to the home channel by itself, 3 s after the link called the other
side lost (`link/src/channels.sharp`). Nothing about a move is saved, so after a restart both are
home again.

## 4. The drone

`flight_computer.step()`: link -> messages -> altimeter -> failsafe -> motors -> telemetry.

**The failsafe.** The drone obeys the sticks only while it hears the controller. Link lost
(or never made): sticks centred, throttle at a gentle descent. Connected but no stick message
for half a second: sticks centred. When the link returns the pilot gets a notice, and the
old stick position does not come back by itself.

**The altimeter** reads the distance sensor more often near the ground (every 100 ms below
1.5 m, 250 ms above 3 m) and works out the climb rate.

Motors and the battery gauge are interfaces with placeholder implementations
(`unwired_motors`, `unwired_battery`) until the hardware exists.

## 5. The controller

`controller_app.step()`: link -> what the drone said -> connection, signal bars, the light ->
settings carried out -> buttons -> a frame.

- **`controller_state`** is the one place that holds what is shown: link state, signal,
  altitude, speed, battery, the drone's mode, its last notice, the clock.
- **`settings`** holds what the pilot chose and saves it as `name=value` lines. The flight
  mode is *sent* to the drone and counts as set only when the drone acknowledged it; until
  then the settings screen marks it with a `*`. It is sent again after a reconnect.
- **Buttons.** Two buttons give three events: UP, DOWN, and SELECT (both together). Holding
  one repeats it every 250 ms. (`controller/src/input/buttons.sharp`)

The settings also hold a **Jam** time (0 / 10 ... 60 s, 0 = off) and a **Jam channel**, and a
**Power off** line that opens a confirm screen (Are you sure... / Yes / No, No marked first); Yes
tidies the hardware and, on the Pi, powers it down. On the channels screen the bottom row chooses
between **Move** (to the quietest channel), **Jam** (the saved channel for the saved time, in
bursts that stop by themselves) and **Back**.

## 6. The UI toolkit

Android's relative layout, written the way Compose and SwiftUI build a screen.

```
class home_screen base screen {
    override def body() : view {
        bar := status_bar(app.state);
        speed := text(app.state.speed_text()).size(2);
        unit := text("cm/s");

        return relative()
            .add(bar.at_top())
            .add(speed.below(bar).at_left().margin_left(16).margin_top(10))
            .add(unit.right_of(speed).align_bottom(speed).margin_left(3));
    }
}
```

- **Views are values in variables.** No view ids, no `find_view_by_id`, no casts.
- **`body()` is asked for every frame** and describes the screen from the state as it is. A
  screen never updates a view; when the altitude changes, the next `body()` says the new
  number. So no screen has a background loop.
- **Relations place a view**: `below` `above` `right_of` `left_of`, `align_left` ...
  `align_bottom`, `at_left` ... `at_bottom`, `center`, margins. Size is the content (default),
  the container (`fill_width`) or a number (`width(30)`).
- **Containers nest**: `status_bar(state)` returns one, and every screen puts it at its top.
- **A screen owns only what is its own**: the settings screen keeps which line is chosen.
- **The navigator** is a pile of screens: `open`, `replace`, `back`.

Two views placed by each other, or against a view that was never added, throw a
`layout_exception` that says so.

| file | what |
|---|---|
| `ui/view.sharp` | a view: size, relations, margins |
| `ui/widgets.sharp` | `text`, `picture`, `spacer` |
| `ui/relative.sharp` | the container and the layout; `draw_frame` |
| `ui/menu.sharp` | a list with a marker, scrolling to the chosen line |
| `ui/screen.sharp` | `screen`, `navigator`, the button events |
| `ui/icons/` | the pictures, one file each |

## 7. The C++

One small file per chip, each with a real half and a "no hardware" half
(`link/native/hardware.h`). A driver does the least possible: the radio driver puts 32 bytes
on the air and hands over 32 bytes that arrived. Everything with a decision in it is Sharp.

The functions are declared in Sharp in a `native "dragonfly" { }` block; `sharpc` writes the
typed C++ prototypes (`build/outputs/sni/dragonfly_sni.h`), so a driver whose function does
not match its declaration does not compile. Each project's C++ becomes one library,
`dragonfly.so`, built by `sharpc -P` from the `"native"` setting in its `settings.json`. The
Pi's own libraries (`rf24`, `wiringPi`) are named for ARM on Linux only
(`"linux": { "processor": { "arm": { "libraries": [...] } } }`), so the same project builds on a PC.

What the rewrite fixed in the old radio driver: a 55 ms wait after every switch between
sending and listening; a printed line for every call; `listen` and `jam` that could loop
forever inside C++; a power level of 3 turned into 0; a hand-made packet chain.

## 8. The tests

| project | what it runs | checks |
|---|---|---|
| `tests/link` | two links over a fake radio: long messages, lost frames, lost acknowledgements, giving up, silence | 74 |
| `tests/drone` | the flight computer against a controller-side link | 38 |
| `tests/controller` | the layout, the menu, the buttons; the app against a drone-side link | 71 |
| `tests/flight` | the real controller app and the real flight computer together | 21 |
| `tests/check_native.sh` | compiles the hardware half of every C++ driver against stand-in headers | - |

`bash test.sh` runs the first four. None of them needs hardware, and none of them waits:
time is a clock the test moves.

**What the tests cannot tell:** whether the real chips behave. The C++ drivers have been
compiled but never run - that happens on the Pi.
