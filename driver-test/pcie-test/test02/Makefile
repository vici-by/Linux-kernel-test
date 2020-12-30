#Makefile:
# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
    #这部分第二次执行		
    obj-m := driver_pcie_test.o
    #obj-m := xpcie_pf1.o
    # Otherwise we were called directly from the command
    # line; invoke the kernel build system.			
else				
#这部分第一次执行
    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
    #KERNELDIR ?= /home/baiy/workspace/linux-learn/arm64/linux-5.4.51
    #KERNELDIR ?= /home/baiy/workspace/linux-git
    PWD := $(shell pwd)			
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	#install: //加不加伪目标都可以。这里可以加入mod_install信息
	#$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install INSTALL_MOD_PATH=/opt/	// 创建模块依赖关系		
endif			
clean:
	rm -rf *.ko .*.ko.cmd  *.mod.c  *.mod.o .*.mod.o.cmd *.o .*.o.cmd modules.order  Module.symvers .tmp_versions *.mod .driver* .cache*
