# BM

Birtual Machine for [Elisp](https://github.com/tsoding/elisp)

# Quick Start

```console
$ ./build.sh    # or ./build_asyc.bat if you are on Windows otherwise also you can use `make -B` on Linux if cc path has wrong, please use it or you can change cc path as gcc like CC=gcc to use ./build.sh
$ make examples
$ ./bme -i ./examples/hello.bm
$ ./bme -i ./examples/fib.bm
$ ./bme -i ./examples/e.bm
$ ./bme -i ./examples/pi.bm
```

## Components

### basm

Assembly language for the virtual machine. For examples see [./examples](./examples) folder

## bme
BM emulator: used for programs generatd by [basm](#basm).

## debasm
Disassembler for the binary file generated by [debasm](#debasm).