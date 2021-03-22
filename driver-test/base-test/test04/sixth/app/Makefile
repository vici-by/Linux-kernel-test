
dst=test
src=$(wildcard *.c)
obj=$(patsubst %.c,%.o,${src})

all:test_read test_write

test_read:test_read.c
	gcc $^ -g -o $@
test_write:test_write.c
	gcc $^ -g -o $@
clean:
	rm -rf *.o test_read test_write
