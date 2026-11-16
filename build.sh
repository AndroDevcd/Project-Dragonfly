#!/bin/bash
# Builds the two programs of Project Dragonfly - Sharp code and C++ library - and the simulator.
#
#     bash build.sh
#
#     controller/build/outputs/controller   + dragonfly.so (.dll on Windows)
#     drone/build/outputs/drone             + dragonfly.so
#     simulator/build/outputs/simulator
#
# Needs Sharp installed (0.4.5 or newer) and a C++ compiler (g++). On a Raspberry Pi the C++ is
# built against the real hardware libraries (RF24, wiringPi: pi_system_setup.bash installs them);
# anywhere else it is built without hardware, so everything still compiles and is checked.
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE" || exit 1
command -v sharpc > /dev/null || { echo "sharpc is not on the PATH: install Sharp first (see README.md)"; exit 1; }
failed=0
for project in controller drone simulator; do
    echo "== $project"
    sharpc -P "$project" || failed=$((failed+1))
done
[ $failed = 0 ] && echo "built: controller, drone, simulator" || echo "$failed project(s) did not build"
[ $failed = 0 ]
