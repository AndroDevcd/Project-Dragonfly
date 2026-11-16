#!/bin/bash
# Compiles the HARDWARE half of every C++ driver on a PC - without linking and without running it.
#
# On a PC the drivers build their "no hardware" half (link/native/hardware.h), so a mistake in the
# real half would only show up on the Raspberry Pi. This compiles that half against stand-in headers
# for RF24 and wiringPi (tests/native_check/stubs), with the prototypes `sharpc` generated for each
# project, so a wrong type or a misspelt call is found here.
#
#     sharpc -P controller && sharpc -P drone      (writes build/outputs/sni in both)
#     bash tests/check_native.sh
HERE="$(cd "$(dirname "$0")/.." && pwd)"
CXX=${CXX:-g++}
failed=0
check() {       # check <project> <file>
    if "$CXX" -fsyntax-only -std=c++17 -Wall -Wno-unused-parameter -Wno-unused-variable -D__arm__ \
        -I "$HERE/tests/native_check/stubs" -I "$HERE/$1/build/outputs/sni" -I "$HERE/link/native" -I "$HERE/$1/native" "$2" 2> /tmp/check_native.txt; then
        echo "ok    $1: ${2#$HERE/}"
    else
        failed=$((failed+1)); echo "FAIL  $1: ${2#$HERE/}"; head -12 /tmp/check_native.txt
    fi
}
for project in controller drone; do
    [ -f "$HERE/$project/build/outputs/sni/dragonfly_sni.h" ] || { echo "build $project first: sharpc -P $project"; exit 1; }
    for file in "$HERE"/link/native/*.cpp "$HERE/$project"/native/*.cpp "$HERE/$project"/native/vendor/*.cpp "$HERE/$project/build/outputs/sni/dragonfly_sni.cpp"; do
        [ -f "$file" ] && check $project "$file"
    done
done
echo "native check: $failed file(s) failed"
[ $failed = 0 ]
