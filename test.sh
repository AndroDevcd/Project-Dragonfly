#!/bin/bash
# Builds and runs every test of Project Dragonfly on this machine - no hardware needed.
#
#     bash test.sh
#
# Needs Sharp installed (`sharpc` and `sharp` on the PATH; 0.4.5 or newer).
# Each test program prints its sections and a last line `RESULT PASS (n checks)` or `RESULT FAIL`;
# this script prints that line for each and ends with 0 only when all passed.
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE" || exit 1
command -v sharpc > /dev/null || { echo "sharpc is not on the PATH: install Sharp first (see README.md)"; exit 1; }
failed=0
for project in tests/link tests/drone tests/controller tests/flight; do
    if ! sharpc -P "$project" > "/tmp/dragonfly_build.txt" 2>&1; then
        failed=$((failed+1)); echo "$project: DID NOT BUILD"; head -20 /tmp/dragonfly_build.txt; continue
    fi
    name=$(grep -o '"output": *"[^"]*"' "$project/settings.json" | cut -d'"' -f4)
    sharp "$project/build/outputs/$name" > /tmp/dragonfly_run.txt 2>&1 || failed=$((failed+1))
    echo "$project: $(grep '^RESULT' /tmp/dragonfly_run.txt || echo 'no result - it stopped early')"
    grep "FAIL  " /tmp/dragonfly_run.txt
done
echo
[ $failed = 0 ] && echo "all tests passed" || echo "$failed test program(s) failed"
[ $failed = 0 ]
