# 目录说明



当前目录为  **驱动测试目录**

| 总目录       | 测试目录  | 测试说明                                                     |
| ------------ | --------- | ------------------------------------------------------------ |
| base-test    |           |                                                              |
| chardev-test |           |                                                              |
| vfio-test    | vfio-mdev | 内核vfio测试代码：samples\vfio-mdev\mtty.c  用vfio-mdev模拟一个tty串口设备，进行测试主从间通信 |
|              | vfio-test | 模拟qemu去访问vfio-mdev设备                                  |
|              | vfio-demo | 测试通过vfio模拟一个pci设备，进行Guest和Host通信，并评估主从间通信时间 |

