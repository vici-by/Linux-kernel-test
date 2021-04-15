#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "s_kfifo.h"


/*
 * function: __kfifo_alloc
 * func: 给kfifo data申请数据
 * esize: 为每个元素得大小
 * unused: 写0就好
 *
 */
int __kfifo_alloc(struct __kfifo *fifo, unsigned int size,
		size_t esize, unsigned int unused)
{
	/*
	 * round down to the next power of 2, since our 'let the indices
	 * wrap' technique works only in this case.
	 */
	size = roundup_pow_of_two(size);

	fifo->in = 0;
	fifo->out = 0;
	fifo->esize = esize;

	if (size < 2) {
		fifo->data = NULL;
		fifo->mask = 0;
		return -EINVAL;
	}

	fifo->data = malloc(size * esize);

	if (!fifo->data) {
		fifo->mask = 0;
		return -ENOMEM;
	}
	fifo->mask = size - 1;

	return 0;
}

void __kfifo_free(struct __kfifo *fifo)
{
	free(fifo->data);
	fifo->in = 0;
	fifo->out = 0;
	fifo->esize = 0;
	fifo->data = NULL;
	fifo->mask = 0;
}



