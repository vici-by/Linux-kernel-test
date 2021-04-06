/*************************************************************************
    > File Name: test.c
    > Author: baiy
    > Mail: baiyang0223@163.com 
    > Created Time: 2019年01月13日 星期日 19时34分17秒
	> Func: 
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>


#define DEVICE_NAME "/dev/globalfifo"
#define GLOBALMEM_SIZE	0x1000

#define MAGIC_NUM 'G'
#define MEM_CLEAR _IO(MAGIC_NUM,1)




void dump_data(const char * data, size_t len)
{
	int loop;
	
	printf("%.8x:\t",0);
	for(loop = 0; loop < len; ++loop){
		printf("%.2x ",data[loop]);
		if((loop+1) % 16 == 0){
			printf("\n%.8x\t",loop+1);
		}
	}
}


int main(int argc, char * argv[])
{
	int opt;
	int val;
	int ret;
    unsigned char loop = 0;
	char * buf = malloc(0x1000);
	assert(buf != NULL);
	int fd = open(DEVICE_NAME,O_RDWR);
	assert(fd > 0);

	while(1){
        memset(buf,loop % 256, 0x1000);
        ret = write(fd,buf, 0x1000);
        printf("write %d ret %d\n",loop % 256, ret);
        if(ret < 0)
            return -1;
        loop++;
        printf("input to continue:\n");
        getchar();
	}


	close(fd);

	return 0;
}
