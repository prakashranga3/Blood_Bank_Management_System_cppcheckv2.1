#!/bin/sh
set -eu

SOURCES=$(find . -maxdepth 1 -name '*.c' \
    ! -name 'test_*.c' \
    ! -name 'test_runner.c')

gcc -g -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow \
    -pthread $SOURCES -o bmsv21

echo "Built: ./bmsv21"
