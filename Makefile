CFLAGS=-Wall -Wextra -Wswitch-enum -Wmissing-prototypes  -pedantic -std=c11
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
examples: ./examples/fib.bm ./examples/123.bm ./examples/123f.bm

./examples/fib.bm: 
	./basm ./examples/fib.basm ./examples/fib.bm

./examples/123.bm:
	./basm ./examples/123.basm ./examples/123.bm

./examples/123f.bm:
	./basm ./examples/123f.basm ./examples/123f.bm


clear:
	rm  ./examples/*bm

test1:
	# produce binary file
	./basm ./examples/fib.basm  ./examples/fib.bm
	
	# look up binary file
	./debasm ./examples/fib.bm
	
	# execute binary file
	./bme -i ./examples/fib.bm  -l 69

test2:
	# produce binary file
	./basm ./examples/123f.basm  ./examples/123f.bm
	
	# look up binary file
	./debasm ./examples/123f.bm
	
	# execute binary file
	./bme -i ./examples/123f.bm  -l 69 