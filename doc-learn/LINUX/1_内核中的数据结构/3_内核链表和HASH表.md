# 3_内核链表和HASH表

## 相关链接

- [Linux内核源码git](https://github.com/torvalds/linux)
- [驱动链表测试代码](https://github.com/vici-by/Linux-kernel-test/tree/main/driver-test/base-struct/list)
- [用户态链表及HASH表移植](https://github.com/vici-by/Linux-kernel-test/tree/main/app-test/usual_lib)



```
(base) baiy@inno-MS-7B89:list$ tree -L 1
.
├── test01  # 测试Linux list代码
└── test02  # 测试Linux hlist代码
└── test02  # 测试Linux jhash得效率及分布
```





## Linux内核的链表

Linux内核链表位于 *include/linux/list.h* 代码中 ，实现只要理解了 container_of 和  C链表，其实比较简单，这里只简单整理下常用API和 一些特殊点

注：可参考<font color=green>《Linux内核设计与实现 第三版》 6.1章节</font>，也可参考 [内核数据结构 —— 内核链](https://blog.csdn.net/zhoutaopower/article/details/86480124)。





### 内核链表原理

内核将数据域与链表域分开，这样可以

### 内核链表基础操作

```
#include <linux/list.h>

struct list_test_info {
    struct list_head head;  // 链表头部结构体
};

struct list_test_node {
    unsigned int num;       // 数据域
    struct list_head node;  // 链表域
};
// 初始化链表头
INIT_LIST_HEAD(&head1.head);
INIT_LIST_HEAD(&head2.head);

// 添加和删除
//
for(i=0; i<5; ++i){
    struct list_test_node *n1 = kzalloc(sizeof(struct list_test_node), GFP_KERNEL);
    struct list_test_node *n2 = kzalloc(sizeof(struct list_test_node), GFP_KERNEL);
    INIT_LIST_HEAD(&n1->node);
    INIT_LIST_HEAD(&n2->node);
    n1->num = i;
    n2->num = i + 5;
    list_add_tail(&n1->node,&head1.head); // list_add 把链表添加在链表首部, list_add_tail把数据添加到链表尾部
    list_add_tail(&n2->node,&head2.head); // 注：因为是双向链表，所以不用考虑遍历到尾部的性能问题，只需要在prev添加即可
}

// 遍历
{
    struct list_head *p;
    struct list_test_node *n1, *n2 * tmpn2;
    pr_info("Dump List n1 used list_for_each\n");
    // 方式一： 链表遍历原理
    list_for_each(p, &head1.head){                          // 遍历所有链表
        n1 = list_entry(p, struct list_test_node, node);    // 根据链表域取出结构体首地址
        pr_info("n1 is %d\n",n1->num);
    }

    // 方式二： list_for_each_entry 常用链表遍历
    pr_info("Dump List n2 used list_for_each_entry\n");     // list_for_each + list_entry 二合一
    list_for_each_entry(n2, &head2.head, node){             // list_for_each_entry_reverse 反向遍历
        pr_info("nn is %d\n",n2->num);
    }
    
    // 方式三 : list_for_each_entry_safe 一般用在删除链表
    pr_info("Delete List n2 used list_for_each_entry_safe\n");
    list_for_each_entry_safe(n2, tmpn2, &head2.head, node){
        list_del(&n2->node);
        kfree(n2);
    }
}


// 链表的添加
// list_add / list_add_tail 添加到链表头部或尾部

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


// 链表的合并 splice 将前一个链表删除 前加或追加到后一个链表上
pr_info("splice list\n"); // splice： 黏接
{
    struct list_test_node *n1, *n2;

    list_splice_init(&head1.head,&head2.head);  // list_splice_tail_init 追加方式
    pr_info("Dump List n1 used list_for_each_entry\n");
    list_for_each_entry(n1, &head1.head, node){
        pr_info("n1 is %d\n",n1->num);
    }

    pr_info("Dump List n2 used list_for_each_entry\n");
    list_for_each_entry(n2, &head2.head, node){ // list_for_each_entry_reverse 反向遍历
        pr_info("nn is %d\n",n2->num);
    }
}

// 链表的左移
{
    struct list_test_node *n1, *n2;
    list_rotate_left(&head2.head);      // 左移一个元素：函数用于将链表第一节点移动到链表的末尾。
    pr_info("Dump List n2 used list_for_each_entry\n");
    list_for_each_entry(n2, &head2.head, node){ // list_for_each_entry_reverse 反向遍历
        pr_info("nn is %d\n",n2->num);
    }
}
```





### READ_ONCE和WRITE_ONCE

参考：[Linux内核中的READ_ONCE和WRITE_ONCE宏](https://zhuanlan.zhihu.com/p/344256943)

其实就是利用 barrier() 来防止编译器优化代码顺序



```
static __always_inline void __read_once_size(const volatile void *p, void *res, int size)
{
    switch (size) {
    case 1: *(__u8_alias_t  *) res = *(volatile __u8_alias_t  *) p; break;
    case 2: *(__u16_alias_t *) res = *(volatile __u16_alias_t *) p; break;
    case 4: *(__u32_alias_t *) res = *(volatile __u32_alias_t *) p; break;
    case 8: *(__u64_alias_t *) res = *(volatile __u64_alias_t *) p; break;
    default:
        barrier();                                              // 最终会调用barrier来防止编译器优化代码顺序
        __builtin_memcpy((void *)res, (const void *)p, size);
        barrier();
    }
}

// check == 1 
#define __READ_ONCE(x, check)                       \
({                                  \
    union { typeof(x) __val; char __c[1]; } __u;            \
    if (check)                          \
        __read_once_size(&(x), __u.__c, sizeof(x));     \
    else                                \
        __read_once_size_nocheck(&(x), __u.__c, sizeof(x)); \
    __u.__val;                          \
})
#define READ_ONCE(x) __READ_ONCE(x, 1)
```





### list_empty_careful  和 list_empty的区别

在SMP系统中检测链表是否为空

```
list_empty_careful
    return (next == head) && (next == head->prev);
```

list_empty_careful() 函数判断一个链表是否为空链表，并确定其他 CPU 并未修改过链表 表头的 prev 和 next 成员。





### list_for_each_entry() 和 list_for_each_entry_safe()

```
#define list_for_each_entry(pos, head, member)              \
    for (pos = list_first_entry(head, typeof(*pos), member);    \
         &pos->member != (head);                    \
         pos = list_next_entry(pos, member))

#define list_for_each_entry_safe(pos, n, head, member)          \
    for (pos = list_first_entry(head, typeof(*pos), member),    \
        n = list_next_entry(pos, member);           \
         &pos->member != (head);                    \
         pos = n, n = list_next_entry(n, member))
```

很明显：

- 在list_for_each_entry 取出链表中某一个元素时， 不能删除，否则下次执行 list_next_entry就会崩溃。
- 在list_for_each_entry_safe取出链表中某一个元素时，用<font color=red>另一个指针保存了下一个节点位置</font>，所以删除节点后 仍然可以取下一个节点。









### list_move 和 list_splice 

list_move 只是将一个链表的某一个node删除，并添加到另一个链表。

list_splice则是将某一个链表全部挂到另一个链表上去。 





### 实战使用

```
pci_register_host_bridge 中

LIST_HEAD(resources);
list_splice_init(&bridge->windows, &resources);



// 将resources链表删除 并添加到 bridge->windows中
#define resource_list_for_each_entry(entry, list)   \
    list_for_each_entry((entry), (list), node)
resource_list_for_each_entry_safe(window, n, &resources) {
    list_move_tail(&window->node, &bridge->windows);
}    
```



## Linux内核的hash表

### HASH表得原理

参考：[图文并茂详解数据结构之哈希表](https://zhuanlan.zhihu.com/p/144296454)

文章已经描述得比较清楚了，这里不在详细说明原理。 



### 内核Hlist结构

注：**hlist只是实现了 hash表中得 链表法 得链表实现** ， 具体得hash函数并没有实现，需要用户实际使用时实现

![image.png](https://cdn.nlark.com/yuque/0/2021/png/2819254/1617847742412-ef6ff4ac-ba89-475a-bdac-26450957c229.png)

这里网上描述比较多的是hlist得结构问题

```
//hash桶的头结点
struct hlist_head {
    struct hlist_node *first;//指向每一个hash桶的第一个结点的指针
};
//hash桶的普通结点
struct hlist_node {
    struct hlist_node *next;//指向下一个结点的指针
    struct hlist_node **pprev;//指向上一个结点的next指针的地址 注意，这里是二级指针
};
```

![image.png](https://cdn.nlark.com/yuque/0/2021/png/2819254/1617848349270-67746c84-606d-444c-8d37-89341da7f98f.png)



**重点：**[**[实例\]内核中hash list(hlist)学习**](https://www.openwrtdl.com/wordpress/内核中hash-list-hlist) **中提出得几个问题**：

1. hlist_head为什么实现不用list？而使用单向链表

  因为hash表是一个hlist_head得大数组， 这里用一个指针达到了节省空间得目的

1. 为什么要用pprev，而不用prev指针？

   因为hlist_head和hlist_node结构不一致， prev指向得不一定是&(hlist_node * next)，也有可能是&（hlist_head * first)



```
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
    struct hlist_node *first = h->first;
    n->next = first;
    if (first)                      // 当前hash链表上已经存在一个list_node，则前插一个节点
        first->pprev = &n->next;    // 这里就是指向了next
    WRITE_ONCE(h->first, n);
    n->pprev = &h->first;           // 这里前插时， pprev指向得就是 &h->firsh, 而不是next
}
```



实现实例

```
// linux-git\drivers\tty\serial\8250\8250_core.c ： serial_link_irq_chain

#define NR_IRQ_HASH     32  /* Can be adjusted later */
static struct hlist_head irq_lists[NR_IRQ_HASH];  // hash表数组

struct hlist_head *h;
struct hlist_node *n;
h = &irq_lists[up->port.irq % NR_IRQ_HASH];      // hash算法：根据port为hash表数组索引
hlist_for_each(n, h) {                           // 遍历当前hash链，来查找是否已经存在
    i = hlist_entry(n, struct irq_info, node);
    if (i->irq == up->port.irq)
        break;
}

if (n == NULL) {
    hlist_add_head(&i->node, h);                // 如不存在，添加一个
}
```





### 内核HASH表基础操作

注：其实Hash表脱离了 hash函数，就是简单得链表操作，逻辑比较简单，看代码即可

```
// 初始化Hlist链表头
for (i=0; i<NR_TEST_HASH; ++i) {
    INIT_HLIST_HEAD(&test_lists[i]);
}

// 申请并添加哈希表
pr_info("create hlist\n");
for (i=0; i<30; ++i){
    // 生成NR_TEST_HASH以内得随机数，然后添加到自己得链表中
    struct  test_hlist *l = NULL;
    unsigned int index = get_random_int() & (NR_TEST_HASH - 1);  // 这里用 随机数&数组大小得mask 来假设一个hash函数
    l = kzalloc(sizeof(struct test_hlist), GFP_KERNEL);
    INIT_HLIST_NODE(&l->node);
    l->nr = i;
    hlist_add_head(&l->node, &test_lists[index]);
    pr_info("l->nr is %d insert hlist %d", i, index);
}


// 哈希表的遍历
pr_info("traversal hlist\n");
{
    for (i=0; i<NR_TEST_HASH; ++i){
        struct  test_hlist *l = NULL;
        pr_info("traversal hlist %d", i);
        hlist_for_each_entry(l, &test_lists[i], node ) {
            pr_info("l->nr is %d\n", l->nr);
        }
    }
}


// hlist的添加
// hlist_add_head   添加到链表头部或尾部

// hlist的删除
// hlist_del / hlist_del_init  删除并对其初始化

// hlist的判空
// hlist_empty 直接判断 READ_ONCE(head->first) == NULL; 即可


// 释放链表
pr_info("delete hlist\n");
{
    for (i=0; i<NR_TEST_HASH; ++i){
        struct  test_hlist *l = NULL;
        struct hlist_node  *n = NULL;
        pr_info("delete hlist %d", i);
        hlist_for_each_entry_safe(l, n, &test_lists[i], node ) {
            // hlist_del(&l->node);
            // kfree(l);
        }
    }
}
```



### jhash算法

参考：[Linux内核中的hash与bucket](http://www.nowamagic.net/academy/detail/3008086)（hash bucket)



hash的中文意思是“散列”，可解释为：分散排列。**一个好的hash函数应该做到对所有元素平均分散排列，尽量避免或者降低他们之间的冲突（Collision）**。有必要再次提醒大家的是，**hash函数的选择必须慎重，如果不幸所有的元素之间都产生了冲突，那么hash表将退化为链表，其性能会大打折扣，时间复杂度迅速降为O(n)**，绝对不要存在任何侥幸心理，因为那是相当危险的



一般情况下都是  **自己根据数据特性来考虑使用的 hash 算法，不是千篇一律咬死一个不放**。

比如 **存放 IP 地址的 hash table，用一个 65536 的桶就很好，把 IP 的后 16bit 作为 key**。这种方法绝对比 hash_long、jhash 等函数的碰撞率低。



**内核常用得就是jhash，是一种比较通用得hash手段（我这里不追求算法原理，会用就行）**

参考：[hash爬坑1：jhash使用](https://zhuanlan.zhihu.com/p/62275777)

**通过jhash将一块数据（不管是一个字符串，还是结构体）进行hash操作，得到一个单向hash值**。如果是结构体的话，注意不要用空隙哦，也就是做好字节对齐~



```
// initval可以理解为hash算法得种子，一般可以为 #define VOICE_HASH_GOLDEN_INTERER   (0x9e370001)
static inline u32 jhash_1word(u32 a, u32 initval);
static inline u32 jhash_2words(u32 a, u32 b, u32 initval);
static inline u32 jhash_3words(u32 a, u32 b, u32 c, u32 initval);
static inline u32 jhash2(const u32 *k, u32 length, u32 initval);
static inline u32 jhash(const void *key, u32 length, u32 initval);
```











## 用户态移植List和Hash表

参考：[用户态常用库](https://github.com/vici-by/Linux-kernel-test/tree/main/app-test/usual_lib)