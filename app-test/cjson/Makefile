
src := test.c
obj := test.o
dst := test
CFALGS := -O3 -g
LDFLAGS := -L  cJSON -lcjson

$(dst):$(obj)
	gcc $^ -o $@ ${CFLAGS} ${LDFLAGS}
