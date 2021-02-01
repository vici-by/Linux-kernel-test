/*************************************************************************
    > File Name: app_misc_test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2020-10-21-10:29:20
    > Func: 
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <sys/ioctl.h>

/* 定义设备类型 */
#define IOC_MAGIC  'm'
#define MISC_DEMO_TEST   	_IO(IOC_MAGIC, 0)

int main(int argc, char * argv[])
{
	int ret = 0;
	int i   = 0;
	int fd = open("/dev/MISC_DEMO", O_RDWR);
    printf("fd is %d\n",fd);
    assert(fd != -1);
	char * buf = malloc(100);
    assert(buf != NULL);
#if 0
	ret = read(fd, buf, 100);
	printf("read 100 bytes, return %d bytes\n", ret);
	for(i=0;i<100;++i){
		printf("%.2x ", buf[i] & 0xff);
		if((i+1) %16 == 0)
			putchar('\n');
	}
#else
    struct timespec uts;
    ret = clock_gettime(CLOCK_REALTIME, &uts);
    printf("clock get time ret %d,sec %ld, nsec %ld\n",ret, uts.tv_sec, uts.tv_nsec);

    ret = ioctl(fd,MISC_DEMO_TEST,&uts);
    assert(ret != -1);
#endif
    close(fd);
	return 0;
}
