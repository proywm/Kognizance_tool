EXES= test1
all: $(EXES)

test%: test%.c
	gcc -g -O0 -std=gnu99 $^ -o $@ -pthread

check: $(EXES)
	./test

clean: 
	rm -f $(EXES)
