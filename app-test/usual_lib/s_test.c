/*************************************************************************
    > File Name: s_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com
    > Created Time: 2020-12-11-17:48:56
    > Func:
 ************************************************************************/

#include "s_util.h"


// #define TEST_ALIGNMENT
// #define TEST_STOI
// #define TEST_BUILD_BUG_ON_ZERO // 测试编译器进行类型判断
// #define BITMAP_TEST
// #define TEST_LIST
#define TEST_HASH



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

#endif // BITMAP_TEST


#ifdef TEST_LIST
struct list_test_info {
	struct list_head head;  // 链表头部结构体
};

struct list_test_node {
	unsigned int num;		// 数据域
	struct list_head node;	// 链表域
};

static void list_test(void)
{
	int i = 0;
	struct list_test_info head1, head2;

	pr_info("Test List [E]\n");

	// 初始化链表头
	INIT_LIST_HEAD(&head1.head);
	INIT_LIST_HEAD(&head2.head);

	// 申请并添加链表
	pr_info("create list\n");
	for(i=0; i<5; ++i){
		struct list_test_node *n1 = malloc(sizeof(struct list_test_node));
		struct list_test_node *n2 = malloc(sizeof(struct list_test_node));
		INIT_LIST_HEAD(&n1->node);
		INIT_LIST_HEAD(&n2->node);
		n1->num = i;
		n2->num = i + 5;
		list_add_tail(&n1->node,&head1.head); // list_add 把链表添加在链表首部, list_add_tail把数据添加到链表尾部
		list_add_tail(&n2->node,&head2.head); // 注：因为是双向链表，所以不用考虑遍历到尾部的性能问题，只需要在prev添加即可
	}


	// 链表的遍历
	pr_info("traversal list\n");
	{
		struct list_head *p;
		struct list_test_node *n1, *n2;
		pr_info("Dump List n1 used list_for_each\n");
		// 方式一
		list_for_each(p, &head1.head){
			n1 = list_entry(p, struct list_test_node, node);
			pr_info("n1 is %d\n",n1->num);
		}

		// 方式二
		pr_info("Dump List n2 used list_for_each_entry\n");
		list_for_each_entry(n2, &head2.head, node){ // list_for_each_entry_reverse 反向遍历
			pr_info("nn is %d\n",n2->num);
		}
	}

	// 链表的添加
	// list_add / list_add_tail	添加到链表头部或尾部

	// 链表的删除
	// list_del / list_del_init  删除并对其初始化

	// 链表的判空
	// list_empty 直接判断 READ_ONCE(head->next) == head; 即可

	// 获取链表首元素 和 尾部元素
	// list_first_entry / list_last_entry


	// 链表的移动 把一个链表节点从当前链表删除并添加到另一个链表中
	// 就是 list_del 和 list_add的组合
	// list_move(&n1, &head1.head);
    // list_move_tail(&n2, &head1.head);


	// 链表的合并
	pr_info("splice list\n"); // splice： 黏接
	{
		struct list_test_node *n1, *n2;

		list_splice_init(&head1.head,&head2.head);
		pr_info("Dump List n1 used list_for_each_entry\n");
		list_for_each_entry(n1, &head1.head, node){
			pr_info("n1 is %d\n",n1->num);
		}

		pr_info("Dump List n2 used list_for_each_entry\n");
		list_for_each_entry(n2, &head2.head, node){ // list_for_each_entry_reverse 反向遍历
			pr_info("n2 is %d\n",n2->num);
		}
	}

	// 链表的左移
	pr_info("rotate_left list\n"); // rotate ：旋转
	{
		struct list_test_node *n2;
		list_rotate_left(&head2.head);		// 左移一个元素：函数用于将链表第一节点移动到链表的末尾。
		pr_info("Dump List n2 used list_for_each_entry\n");
		list_for_each_entry(n2, &head2.head, node){ // list_for_each_entry_reverse 反向遍历
			pr_info("n2 is %d\n",n2->num);
		}
	}

	// 释放链表
	pr_info("delete list\n");
	{
		struct list_test_node *n1, *tmpn1;
		struct list_test_node *n2, *tmpn2;
		pr_info("Delete List n1 used list_for_each_entry_safe\n");
		list_for_each_entry_safe(n1, tmpn1, &head1.head, node){
			list_del(&n1->node);
			free(n1);
		}
		pr_info("Delete List n2 used list_for_each_entry_safe\n");
		list_for_each_entry_safe(n2, tmpn2, &head2.head, node){
			list_del(&n2->node);
			free(n2);
		}
	}
	pr_info("Test List [X]\n\n\n");

}
#endif // TEST_LIST


#ifdef TEST_HASH
// 定义一组hlist（桶个数为8）， 利用jhash对 1千个数据进行jhash，分别计算使用率

//常规算法的黄金分隔
#define VOICE_HASH_GOLDEN_INTERER   (0x9e370001)

//hash桶的大小为2的11次方，即2047+1
#define NR_TEST_HASH                   8

static void jash_test(void)
{
	int i = 0;
	int index = 0;
	// 定义三组hash表，分别计算
	struct test_hlist{
		unsigned int nr;
		struct	hlist_node node;
	};
	static	struct hlist_head test_lists[NR_TEST_HASH];  // hash表数组 jhash


	for (i=0; i<NR_TEST_HASH; ++i) {
		INIT_HLIST_HEAD(&test_lists[i]);
	}

	pr_info("create hlist");
	for (i=0; i<1000; ++i) {
		struct  test_hlist *l = NULL;
		index = jhash_1word(i, VOICE_HASH_GOLDEN_INTERER) & (NR_TEST_HASH-1);
		// index = i % NR_TEST_HASH;
		l = malloc(sizeof(struct test_hlist));
		INIT_HLIST_NODE(&l->node);
		l->nr = i;
		hlist_add_head(&l->node, &test_lists[index]);
	}
	pr_info("\n");

	pr_info("statistics hlist");
	{
		for (i=0; i<NR_TEST_HASH; ++i){
			struct  test_hlist *l = NULL;
			struct hlist_node  *n = NULL;
			unsigned int count = 0;
			hlist_for_each_entry_safe(l, n, &test_lists[i], node ) {
				++count;
			}
			pr_info("list %d has %d counts", i, count);
		}
	}
	pr_info("\n");


	// 释放链表
	pr_info("delete hlist");
	{
		for (i=0; i<NR_TEST_HASH; ++i){
			struct  test_hlist *l = NULL;
			struct hlist_node  *n = NULL;
			pr_info("delete hlist %d", i);
			hlist_for_each_entry_safe(l, n, &test_lists[i], node ) {
				hlist_del(&l->node);
				free(l);
			}
		}
	}
	pr_info("\n");
}
#endif



int main(int argc, char * argv[])
{
#ifdef TEST_ALIGNMENT
    printf("sizeof long is %ld\n",sizeof(long));
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


#ifdef TEST_LIST
	list_test();
#endif

#ifdef TEST_HASH
	jash_test();
#endif
	return 0;
}
