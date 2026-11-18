#!/bin/bash
# Builds and runs every test of Project Dragonfly on this machine - no hardware needed.
#
#     bash test.sh
#
# Needs Sharp installed (`sharpc` and `sharp` on the PATH; 0.7.0 or newer: the tests use std.testing,
# and mock<...>() for stand-in devices).
# Each test program prints one line per test and a last line - `all n tests passed`, or
# `k failed, m passed (n tests)` - and exits with 0 only when everything passed. This script prints
# that last line for each program (and every failing test), and ends with 0 only when all passed.
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
    echo "$project: $(grep -E '^all [0-9]+ tests passed|failed, .* passed' /tmp/dragonfly_run.txt | tail -1 || echo 'no result - it stopped early')"
    grep -A4 "^  FAIL  " /tmp/dragonfly_run.txt
done
echo
[ $failed = 0 ] && echo "all tests passed" || echo "$failed test program(s) failed"
[ $failed = 0 ]
