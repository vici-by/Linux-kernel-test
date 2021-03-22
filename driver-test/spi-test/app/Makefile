
dst=testspi
src=$(wildcard *.c)
obj=$(patsubst %.c,%.o,${src})

${dst}:spidev.c
	aarch64-linux-gnu-gcc $@ -o $^
	
clean:
	rm -rf *.o ${obj} ${dst}
