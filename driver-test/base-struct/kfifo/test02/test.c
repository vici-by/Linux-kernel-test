/*************************************************************************
    > File Name: test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2021-04-14-16:31:36
    > Func: 
 ************************************************************************/

#include <stdio.h>

struct __kfifo {
	unsigned int	in;      // 入列的时候增加的位置
	unsigned int	out;     // 出列的时候增加的位置
	unsigned int	mask;    // 巧妙的 mask 设计，同时包含了数据的个数信息
	unsigned int	esize;   // 元素的大小
	void            *data;   // 数据
};
struct kfifo {
	union {
		struct __kfifo	kfifo;
		unsigned char	*type;
		const unsigned char	*const_type;
		char		(*rectype)[0];
		void		*ptr;
		void const	*ptr_const;
	};
	unsigned char		buf[0];
};

static inline int __kfifo_int_must_check_helper(int val)
{
        return val;
}

#define __is_kfifo_ptr(fifo)    (sizeof(*fifo) == sizeof(struct __kfifo))

int __kfifo_alloc(struct __kfifo *fifo, unsigned int size,
		size_t esize, int unused)
{
	printf("kfifo %p size %d esize %ld\n", fifo, size, esize);
}

#define kfifo_alloc(fifo, size, gfp_mask) \
__kfifo_int_must_check_helper( \
({ \
	typeof((fifo) + 1) __tmp = (fifo); \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
	__is_kfifo_ptr(__tmp) ? \
	__kfifo_alloc(__kfifo, size, sizeof(*__tmp->type), 0) : \
	-1; \
}) \
)

#define kfifo_alloc2(fifo, size, gfp_mask) \
__kfifo_int_must_check_helper( \
({ \
	typeof((fifo)) __tmp = (fifo); \
	struct __kfifo *__kfifo = &__tmp->kfifo; \
	__is_kfifo_ptr(__tmp) ? \
	__kfifo_alloc(__kfifo, size, sizeof(*__tmp->type), 0) : \
	-1; \
}) \
)


#define kfifo_len(fifo) \
({ \
	typeof((fifo) + 1) __tmpl = (fifo); \
	__tmpl->kfifo.in - __tmpl->kfifo.out; \
})

#define	kfifo_is_full(fifo) \
({ \
	typeof((fifo) + 1) __tmpq = (fifo); \
	kfifo_len(__tmpq) > __tmpq->kfifo.mask; \
})




int main(int argc, char * argv[])
{

	struct kfifo kfifo;
	struct kfifo * fifo = &kfifo;
	typeof((fifo) + 1) tmp1 = (fifo);
	typeof((fifo)) tmp2 = (fifo);
	printf("kfifo is %p, tmp1 is %p, tmp2 is %p\n", fifo,tmp1, tmp2);
	kfifo_alloc(&kfifo, 16, 0);
	kfifo_alloc2(&kfifo, 16, 0);

    printf("kfifo in is %p\n", &fifo->kfifo.in);
    printf("rectype is %ld, point %p\n", sizeof(*tmp1->rectype), &tmp1->rectype);
    fifo->kfifo.in = ~0;
    printf("rectype is %ld, point %p\n", sizeof(*tmp1->rectype), &tmp1->rectype);

	fifo->kfifo.mask = 15;
	fifo->kfifo.in = 10;
	fifo->kfifo.out = 15;

    int len = kfifo_len(fifo);

	printf("kfifo len is %d,is full %d\n", kfifo_len(fifo), kfifo_is_full(fifo));
	fifo->kfifo.in = 15;
	fifo->kfifo.out = 10;
	printf("kfifo len is %d,is full %d\n", kfifo_len(fifo), kfifo_is_full(fifo));
	fifo->kfifo.in = 16;
	fifo->kfifo.out = 0;
	printf("kfifo len is %d,is full %d\n", kfifo_len(fifo), kfifo_is_full(fifo));
	return 0;
}
