/*************************************************************************
    > File Name: test01.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-12-20-15:06:27
    > Func: 
 ************************************************************************/

#include <stdio.h>
#include <linux/vfio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>


char buf[1024] = {0};
/*
struct vfio_group_status {
    __u32   argsz;
    __u32   flags;
#define VFIO_GROUP_FLAGS_VIABLE     (1 << 0)
#define VFIO_GROUP_FLAGS_CONTAINER_SET  (1 << 1)
};
*/
struct vfio_group_status group_status;
struct vfio_device_info  device_info;

int main(int argc, char * argv[])
{
    int err = 0;
    int cfd; // contaienr fd
    int gfd; // group fd
    int dfd; // device fd

    //  容器操作
    cfd = open("/dev/vfio/vfio", O_RDWR);
    if(cfd < 0){
        perror("open failed\n");
        return -1;
    }
    if(ioctl(cfd, VFIO_GET_API_VERSION, buf) == 0){
        printf("get version ret %d, buf is %s\n",err,buf);
        if(ioctl(cfd, VFIO_CHECK_EXTENSION, VFIO_TYPE1_IOMMU) == 0){
            ioctl(cfd, VFIO_SET_IOMMU, VFIO_TYPE1_IOMMU);
        }
    }

    // group操作
    gfd = open("/dev/vfio/26", O_RDWR);
    if(gfd < 0){
        perror("open gfd failed\n");
        goto err1;   
    }
    // 判断是否是个属于vfio的有效group，并绑定到容器中
    err = ioctl(gfd, VFIO_GROUP_GET_STATUS, &group_status);
    if( (err == -1) || ((group_status.flags & VFIO_GROUP_FLAGS_VIABLE) == 0) ){
        printf("invaild group for vfio, flags is %d\n", group_status.flags);
        goto err2;
    }
    if( group_status.flags & VFIO_GROUP_FLAGS_CONTAINER_SET) {
        printf("group has set\n");
    } else {
        ioctl(gfd, VFIO_GROUP_SET_CONTAINER, cfd);
    }
    err = ioctl(gfd, VFIO_GROUP_GET_STATUS, &group_status);


    // 设备操作接口
    dfd = ioctl(gfd, VFIO_GROUP_GET_DEVICE_FD, "0000:26:00.0");
    if( (dfd == -1)){
        perror("get device failed\n");
        goto err2;
    }
    printf("get device OK\n");

    ioctl(dfd,VFIO_DEVICE_GET_INFO,&device_info);
    ioctl(dfd,VFIO_GET_IRQ_INFO,&irq);
    ioctl(dfd,VFIO_DEVICE_RESET);

err3:
    close(dfd);
        
err2:
    close(gfd);
err1:
    close(cfd);
    return 0;
}
