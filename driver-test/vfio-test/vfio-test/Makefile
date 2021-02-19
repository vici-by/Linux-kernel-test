
src := test01.c
obj := test01.o
dst := test
CFALGS := -O3 -g

$(dst):$(obj)
	gcc $^ -o $@ ${CFLAGS} ${LDFLAGS}
clean:
	rm -rf *.o ${dst}
