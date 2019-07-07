EXES= test1
all: $(EXES)

test%: test%.c
	gcc -g -O2 -std=gnu99 -I . -L . $^ -o $@ -pthread -lperform

check: $(EXES)
	./test

clean: 
	rm -f $(EXES)
