# 目录说明





## 目录列表

### base-struct 基础结构和描述

这个目录主要是学习Linux基础得数据结构和框架

相关配套文档参考：[1_内核中的数据结构](https://github.com/vici-by/Linux-kernel-test/tree/main/doc-learn/LINUX/1_%E5%86%85%E6%A0%B8%E4%B8%AD%E7%9A%84%E6%95%B0%E6%8D%AE%E7%BB%93%E6%9E%84)

相关应用代码移植参考：[usual_lib](https://github.com/vici-by/Linux-kernel-test/tree/main/app-test/usual_lib)

```c
base-struct/
├── bitmap	# 测试Linux bitmap的代码
├── idr		# 测试Linux idr的代码
├── kfifo	# 测试Linux kfifo的代码
├── list	# 测试Linux list/hlist的代码
├── macro	# 测试Linux macro的代码
├── notify	# 测试Linux notify的代码
├── procfs	# 测试Linux procfs的代码
├── rbtree	# 测试Linux rbtree的代码
└── sysfs	# 测试Linux sysfs的代码
```



#### macro目录



#### bitmap目录

```c
(base) baiy@inno-MS-7B89:bitmap$ tree -L 1 
├── test01	# 测试Linux bitops代码
└── test02	# 测试Linux bitmap代码  // TBD
```



#### list目录

```c
(base) baiy@inno-MS-7B89:list$ tree -L 1
├── test01	# 测试Linux list代码
└── test02	# 测试Linux hlist代码
└── test02	# 测试Linux jhash得效率及分布
```



#### kfifo目录

```c
(base) baiy@inno-MS-7B89:kfifo$ tree -L 1 
├── test01	# 内核测试kfifodemo
├── test02	# kfifo 宏测试（用户态代码）
└── test03	# 内核自带得kfifodemo
```





#### IDR目录

```

```



#### rbtree目录

```

```









### base-driver 基础的设备驱动



### module-driver 常用模块驱动

- dma-test DMA模块测试
- pcie-test PCIE模块驱动测试





### virt-test 虚拟化相关测试









