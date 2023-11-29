CFLAGS=-Wall -Wextra -Wswitch-enum -Wmissing-prototypes  -pedantic -std=c11
CC=cc
LIBS=
EXAMPLES=./examples/fib.bm ./examples/123.bm ./examples/123f.bm ./examples/e.bm ./examples/pi.bm ./examples/lerp.bm 


.PHONY: all
all: basm bme debasm

basm: ./src/basm.c ./src/bm.h
	$(CC) $(CFLAGS) -o basm ./src/basm.c $(LIBS)

bme: ./src/bme.c ./src/bm.h
	$(CC) $(CFLAGS) -o bme ./src/bme.c $(LIBS)

debasm: ./src/debasm.c ./src/bm.h
	$(CC) $(CFLAGS) -o debasm ./src/debasm.c $(LIBS)


.PHONY: examples
examples: $(EXAMPLES)
./examples/fib.bm: basm ./examples/fib.bm
	./basm ./examples/fib.basm ./examples/fib.bm

./examples/123.bm: basm ./examples/123.bm
	./basm ./examples/123.basm ./examples/123.bm

./examples/123f.bm: basm ./examples/123f.bm
	./basm ./examples/123f.basm ./examples/123f.bm

./examples/e.bm: basm ./examples/e.bm
	./basm ./examples/e.basm ./examples/e.bm

./examples/pi.bm: basm ./examples/pi.bm
	./basm ./examples/pi.basm ./examples/pi.bm

./examples/lerp.bm: basm ./examples/lerp.bm
	./basm ./examples/lerp.basm ./examples/lerp.bm