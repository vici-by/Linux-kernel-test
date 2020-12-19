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



int main(int argc, char * argv[])
{
	int ret = 0;
	int i   = 0;
	int fd = open("/dev/MISC_DRMMM_TEST", O_RDWR);
	char * buf = malloc(100);
	if(!buf){
		perror("buf malloc failed");
		close(fd);
		return -1;
	}
	ret = read(fd, buf, 100);
	printf("read 100 bytes, return %d bytes\n", ret);
	for(i=0;i<100;++i){
		printf("%.2x ", buf[i] & 0xff);
		if((i+1) %16 == 0)
			putchar('\n');
	}

	return 0;
}
