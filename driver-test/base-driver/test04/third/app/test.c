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
#include <sys/ioctl.h>


#define DEVICE_NAME "/dev/globalmem"
#define GLOBALMEM_SIZE	0x1000

#define MAGIC_NUM 'G'
#define MEM_CLEAR _IO(MAGIC_NUM,1)



void get_input(int * opt, int * val)
{
	int c;
	printf("\ninput info:\n");
	printf("\tinput 'r' to read\n");
	printf("\tinput 'w' to write\n");
	printf("\tinput 'c' to clear\n");
	printf("\tinput 's' to set pos\n");
	printf("\tinput 'p' to get pos\n");
	printf("please input your chooise:");
	c = getchar();
	if(c != 'p' && c != 'w' && c != 'c'){
		printf("please input value:");
		scanf("%d", val);
	}
	while(getchar() != '\n');
	*opt = c;
	
	return;
}

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
	if(argc < 2){
		printf("usage %s [filename]\n",argv[0]);
		return 0;
	}
	char * buf = malloc(GLOBALMEM_SIZE);
	assert(buf != NULL);
	int fd = open(DEVICE_NAME,O_RDWR);
	if(fd < 0){
		perror("open failed:");
		return -1;
	}

	while(1){
		get_input(&opt, &val);
		switch(opt){
			case 'r':
				memset(buf,0,GLOBALMEM_SIZE);
				ret = read(fd, buf,val);
				printf("Read ret %d\n", ret);
				if(ret > 0)
					dump_data(buf,val);
				break;
			case 'w':
				memset(buf,0,GLOBALMEM_SIZE);
				printf("input your message:");
				fgets(buf,GLOBALMEM_SIZE,stdin);
				val = strlen(buf);
				ret = write(fd,buf,val);
				printf("Write ret %d\n", ret);
				break;
			case 'c':
				ret = ioctl(fd,MEM_CLEAR);
				printf("Ioctl ret %d\n", ret);
				break;
			case 's':
				ret = lseek(fd,val,SEEK_SET);
				printf("Set pos is %d\n",ret);
				break;
			case 'p':
				ret = lseek(fd,0,SEEK_CUR);
				printf("Current pos is %d\n",ret);
				break;
			default:
				continue;
		}

	
	}


	close(fd);

	return 0;
}
