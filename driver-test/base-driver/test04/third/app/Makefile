
dst=test
src=$(wildcard *.c)
obj=$(patsubst %.c,%.o,${src})

${dst}:${obj}
	gcc $^ -g -o $@
clean:
	rm -rf *.o ${obj} ${dst}
