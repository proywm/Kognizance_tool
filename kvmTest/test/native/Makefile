EXES= test1 test2
CFLAGS= -g -Wall -O3 -fopenmp
CC=gcc

all: $(EXES)

test%: test%.c
	${CC} ${CFLAGS} -std=gnu99 $^ -o $@ -pthread

check: $(EXES)
	./test

clean: 
	rm -f $(EXES)
