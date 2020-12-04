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
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>



/* 定义设备类型 */
#define IOC_MAGIC  'p'
#define PCIE_DEMO_READ_CONFIG		_IO(IOC_MAGIC, 0)
#define PCIE_DEMO_DUMP_RESOURCE		_IO(IOC_MAGIC, 1)
#define PCIE_DEMO_READ_BAR			_IO(IOC_MAGIC, 2)
#define PCIE_DEMO_WRITE_BAR			_IO(IOC_MAGIC, 3)

#define DEV_NAME "/dev/PCIE_DEMO"

struct pci_demo_opt {
	unsigned long barnum;
	unsigned long len;
	unsigned long addr;
	unsigned long offset;
};

void dump_data(void * d, size_t len)
{
    int i = 0;
    printf("%.8x:\t",i);
    for(i=0; i<len; ++i){
        printf("%.2x ", ((unsigned char *)d)[i]);
        if((i+1) %16 == 0) {
            printf("\n%.8x:\t",i+1);
        }
    }
    if((i+1) %16 != 0) {
        printf("\n");
    }

}
int htoi(char * s)
{
    int i;
    int n = 0;

    if (s[0] == '0' && (s[1]=='x' || s[1]=='X')) {
        i = 2;
    } else {
        return atoi(s);
    }
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >='A' && s[i] <= 'Z');++i)
    {
        if (tolower(s[i]) > '9') {
            n = 16 * n + (10 + tolower(s[i]) - 'a');
        } else {
            n = 16 * n + (tolower(s[i]) - '0');
        }
    }
    return n;
}




int main(int argc, char * argv[])
{
	int ret = 0;
	int i   = 0;
	int fd = open(DEV_NAME, O_RDWR);
	if(fd < 0){
		perror("open failed");
		return -1;
	}
	struct pci_demo_opt opt;
    struct pci_demo_opt topt;
	memset(&opt, 0, sizeof(struct pci_demo_opt));
    srandom((long int)time(NULL));


	switch(argv[1][0]){
	case 'r':
		{
			opt.barnum = htoi(argv[2]);
			opt.len = htoi(argv[3]);
			opt.offset = htoi(argv[4]);
			opt.addr = (unsigned long)malloc(opt.len);
            memset((void *)(opt.addr), 0, opt.len );
			ret = ioctl(fd, PCIE_DEMO_READ_BAR, &opt);
			if(ret < 0){
				perror("read bar failed\n");
				free((void*)(opt.addr));
				break;
			}
			dump_data((void*)(opt.addr), opt.len);
			free((void*)(opt.addr));
		}
		break;
	case 'w':
        {
			opt.barnum = htoi(argv[2]);
			opt.len = htoi(argv[3]);
			opt.offset = htoi(argv[5]);
			opt.addr = (unsigned long)malloc(opt.len);
            memset((void *)(opt.addr), htoi(argv[4]), opt.len );
			ret = ioctl(fd, PCIE_DEMO_WRITE_BAR, &opt);
			if(ret < 0){
				perror("write bar failed\n");
				free((void*)(opt.addr));
				break;
			}
			free((void*)(opt.addr));
        }
		break;
    case 'd':
        {
			ret = ioctl(fd, PCIE_DEMO_DUMP_RESOURCE, &opt);
			if(ret < 0){
				perror("dump resource failed\n");
				break;
			}
        }
		break;
    case 'c':
        {
			opt.barnum = 0;
			opt.len = 0x40;
			opt.addr = (unsigned long)malloc(opt.len);
            memset((void *)(opt.addr), 0, opt.len );
			ret = ioctl(fd, PCIE_DEMO_READ_CONFIG, &opt);
			if(ret < 0){
				perror("read config failed\n");
				free((void*)(opt.addr));
				break;
			}
			dump_data((void*)(opt.addr), opt.len);
			free((void*)(opt.addr));
        }
		break;
    case 's':
        {
            opt.barnum = htoi(argv[2]);
            opt.len = htoi(argv[3]);
            opt.addr = (unsigned long)malloc(opt.len);
            topt.barnum = htoi(argv[2]);
            topt.len = htoi(argv[3]);
			opt.offset = htoi(argv[4]);
            topt.addr = (unsigned long)malloc(opt.len);

            for(i=0;i<1000;++i){
                memset((void *)(topt.addr),0x00, opt.len);
                memset((void *)(opt.addr),random() % 255, opt.len);
                ret = ioctl(fd, PCIE_DEMO_WRITE_BAR, &opt);
                if(ret < 0){
                    perror("write bar failed");
                    break;
                }
                ret = ioctl(fd, PCIE_DEMO_READ_BAR, &topt);
                if(ret < 0){
                    perror("write bar failed");
                    break;
                }
                if(memcmp(opt.addr, topt.addr, opt.len) != 0) {
                    printf("memcmp failed\n");
                } else {
                    printf("memcmp OK\n");
                }
            }
            free((void *)(opt.addr));
            free((void *)(topt.addr));
        }
        break;
	}


	close(fd);
	return 0;
}
