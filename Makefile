CFLAGS=-Wall -Wextra -Wswitch-enum -Wmissing-prototypes -std=c11 -pedantic
CC=cc
LIBS=

.PHONY: all
all: basm bme debasm

basm: ./src/basm.c ./src/bm.h
	$(CC) $(CFLAGS) -o basm ./src/basm.c $(LIBS)

bme: ./src/bme.c ./src/bm.h
	$(CC) $(CFLAGS) -o bme ./src/bme.c $(LIBS)

debasm: ./src/debasm.c ./src/bm.h
	$(CC) $(CFLAGS) -o debasm ./src/debasm.c $(LIBS)


.PHONY: examples
examples: ./examples/fib.bm ./examples/123.bm

./examples/fib.bm: ./examples/fib.basm
	./basm ./examples/fib.basm ./examples/fib.bm

./examples/123.bm: 
	./basm ./examples/123.basm ./examples/123.bm
clear:
	rm basm bme ./examples/*bm debasm

test:
	# produce binary file
	./basm ./examples/fib.basm  ./examples/fib.bm
	
	# look up binary file
	./debasm ./examples/fib.bm
	
	# execute binary file
	./bme -i ./examples/fib.bm  -l 69 