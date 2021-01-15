/*************************************************************************
    > File Name: s_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-11-17:48:56
    > Func: 
 ************************************************************************/

#include "s_common.h"
#include "s_bitmap.h"

// #define TEST_ALIGNMENT
// #define TEST_STOI
// #define TEST_BUILD_BUG_ON_ZERO // 测试编译器进行类型判断
#define BITMAP_TEST



#ifdef TEST_BUILD_BUG_ON_ZERO
static void test_compile_check_type()
{
    // 测试指针类型与数据类型 用编译器 __builtin_types_compatible_p 进行类型判断
    char *ch = malloc(3);
    char ar[3];
    unsigned int a,b;
    // ERROR:  printf("ch is a arr? %d,same type is %d\n",__must_be_array(ch),__same_type(ch, &(ch)[0]));
    printf("ch same type is %d\n",__same_type(ch, &(ch)[0]));
    printf("ar is a arr? %d,same type is %d\n",__must_be_array(ar),__same_type(ar, &(ar)[0]));
    printf("int a & b same type is %d\n",__same_type(a, b));
}

#endif


#ifdef BITMAP_TEST
static void test_bitmap()
{
    int i = 0;

#if  1
    printf("Dump Last word:\n");
	printf("Bitmap(0):  %#lx, %#lx\n", 0L, BITMAP_LAST_WORD_MASK(0));
	printf("Bitmap(1):  %#lx, %#lx\n", -1L,BITMAP_LAST_WORD_MASK(1));
	printf("Bitmap(2):  %#lx, %#lx\n", -2L,BITMAP_LAST_WORD_MASK(2));
	printf("Bitmap(3):  %#lx, %#lx\n", -3L,BITMAP_LAST_WORD_MASK(3));
	printf("Bitmap(4):  %#lx, %#lx\n", -4L,BITMAP_LAST_WORD_MASK(4));
	printf("Bitmap(5):  %#lx, %#lx\n", -5L,BITMAP_LAST_WORD_MASK(5));
	printf("Bitmap(6):  %#lx, %#lx\n", -6L,BITMAP_LAST_WORD_MASK(6));
	printf("Bitmap(7):  %#lx, %#lx\n", -7L,BITMAP_LAST_WORD_MASK(7));
	printf("Bitmap(8):  %#lx, %#lx\n", -8L,BITMAP_LAST_WORD_MASK(8));
    printf("Bitmap(9):  %#lx, %#lx\n", -9L,BITMAP_LAST_WORD_MASK(9));
	printf("Bitmap(10): %#lx, %#lx\n", -10L,BITMAP_LAST_WORD_MASK(10));
	printf("Bitmap(12): %#lx, %#lx\n", -12L,BITMAP_LAST_WORD_MASK(12));
	printf("Bitmap(16): %#lx, %#lx\n", -16L,BITMAP_LAST_WORD_MASK(16));
	printf("Bitmap(18): %#lx, %#lx\n", -18L,BITMAP_LAST_WORD_MASK(18));
	printf("Bitmap(20): %#lx, %#lx\n", -20L,BITMAP_LAST_WORD_MASK(20));
	printf("Bitmap(22):  %#lx\n", BITMAP_LAST_WORD_MASK(22));
	printf("Bitmap(24):  %#lx\n", BITMAP_LAST_WORD_MASK(24));
	printf("Bitmap(26):  %#lx\n", BITMAP_LAST_WORD_MASK(26));
	printf("Bitmap(27):  %#lx\n", BITMAP_LAST_WORD_MASK(27));
	printf("Bitmap(28):  %#lx\n", BITMAP_LAST_WORD_MASK(28));
	printf("Bitmap(29):  %#lx\n", BITMAP_LAST_WORD_MASK(29));
	printf("Bitmap(30):  %#lx\n", BITMAP_LAST_WORD_MASK(30));
	printf("Bitmap(31):  %#lx\n", BITMAP_LAST_WORD_MASK(31));
	printf("Bitmap(31):  %#lx\n", BITMAP_LAST_WORD_MASK(32));
#endif
    // 其实一般会用全局变量，然后直接清0，或者使用 bitmap_alloc
    DECLARE_BITMAP(bitmap, 100); //  定义一个100bit的位图,变量名bitmap
    //__bitmap_clear(bitmap, 0, BITS_TO_LONGS(100) * BITS_PER_LONG );

    unsigned arr_sz=ARRAY_SIZE(bitmap);
    for(i=0;i<arr_sz;++i)
        printf("[%d]=%#lx\n",i,bitmap[i]);
    printf("bitmap size is %d-%lu\n",arr_sz, BITS_TO_LONGS(100));

    // 批量设置位操作
    printf("Start Batch setup operation\n");
    __bitmap_set(bitmap,3,80);
    for(i=0;i<arr_sz;++i)
        printf("[%d]=%#lx\n",i,bitmap[i]);

    // 批量清除操作
    printf("Start Clear setup operation\n");
    __bitmap_clear(bitmap,3,80);
    for(i=0;i<arr_sz;++i)
        printf("[%d]=%#lx\n",i,bitmap[i]);




}

#endif

int main(int argc, char * argv[])
{
    printf("sizeof long is %ld\n",sizeof(long));
#ifdef TEST_ALIGNMENT
    if(argc < 2) {
        printf("TEST_ALIGNMENT:Input %s [val] [size]\n",argv[0]);
        return -1;
    }
	int val  = atoi(argv[1]);
	int size = atoi(argv[2]);
	printf("TEST_ALIGNMENT:[val-%d,size-%d]alig down is %d, up is %d\n",val,size, alignment_down(val,size), alignment_up(val,size));
#endif
#ifdef TEST_STOI
    if(argc < 2){
        printf("TEST_STOI:Input %s [val]\n",argv[0]);
        return -1;
    }
    printf("TEST_STOI:input %s to [%lu]\n",argv[1],stoi(argv[1]));
#endif

#ifdef TEST_BUILD_BUG_ON_ZERO
    test_compile_check_type();
#endif

#ifdef BITMAP_TEST
    test_bitmap();
#endif


	return 0;
}
