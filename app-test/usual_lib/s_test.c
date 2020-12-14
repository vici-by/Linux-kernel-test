/*************************************************************************
    > File Name: s_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-11-17:48:56
    > Func: 
 ************************************************************************/

#include "s_common.h"

#define TEST_ALIGNMENT

int main(int argc, char * argv[])
{
#ifdef TEST_ALIGNMENT
    if(argc < 2) {
        printf("Input %s [val] [size]\n",argv[0]);
        return -1;
    }
	int val  = atoi(argv[1]);
	int size = atoi(argv[2]);
	printf("[val-%d,size-%d]alig down is %d, up is %d\n",val,size, alignment_down(val,size), alignment_up(val,size));
#endif
	return 0;
}
