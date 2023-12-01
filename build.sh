#!/bin/bash

set -xe

CC=${CC:=/nix/store/dq0xwmsk1g0i2ayg6pb7y87na2knzylh-gcc-wrapper-11.3.0/bin/cc}
CFLAGS="-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -O3  -pedantic -std=c11"
LIBS=

$CC $CFLAGS -o basm ./src/basm.c $LIBS
$CC $CFLAGS -o bme ./src/bme.c $LIBS
$CC $CFLAGS -o debasm ./src/debasm.c $LIBS

for example in `find examples/ -name \*.basm | sed "s/\.basm//"`; do
  cpp -P "$example.basm" > "$example.basm.pp"
  ./basm "$example.basm.pp" "$example.bm"
  rm -r "$example.basm.pp"
done