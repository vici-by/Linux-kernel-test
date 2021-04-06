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
#include<getopt.h>



/* 定义设备类型 */
#define IOC_MAGIC  'p'
#define PCIE_DEMO_READ_CONFIG       _IO(IOC_MAGIC, 0)
#define PCIE_DEMO_DUMP_RESOURCE     _IO(IOC_MAGIC, 1)
#define PCIE_DEMO_READ_BAR          _IO(IOC_MAGIC, 2)
#define PCIE_DEMO_WRITE_BAR         _IO(IOC_MAGIC, 3)
#define PCIE_BAR_STABLE_TEST        _IO(IOC_MAGIC, 4)

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
    for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') \
            || (s[i] >='A' && s[i] <= 'Z');++i)
    {
        if (tolower(s[i]) > '9') {
            n = 16 * n + (10 + tolower(s[i]) - 'a');
        } else {
            n = 16 * n + (tolower(s[i]) - '0');
        }
    }
    return n;
}



static void app_pcie_test_help(const char * s)
{
    printf("Usage %s:\n",s);
    printf("\t-r read"
         "\n\t-w write"
         "\n\t-s stability\n");
    printf("\t-b barnum,default 0\n");
    printf("\t-l len,default 0x1024\n");
    printf("\t-o offset,default 0\n");
    printf("\t-v write value,default 0x5A\n");
    printf("\t-c stability counts,default 100\n");
}
int main(int argc, char * argv[])
{
    int c;

    int digit_optind = 0;
    int option_index = 0;
    char *string = "rwsb:l:o:v:c:h";

    char cmd = 'r';
    char barno = 0;
    char value = 0x5A;
    size_t len = 0x1024;
    size_t count = 100;
    size_t offset = 0;

    printf("[%s %d]: version %s\n", __func__,__LINE__,"0.0.0.0");

    static struct option long_options[] = {
        // 操作类型   
        {"read",            no_argument,NULL, 'r'},
        {"write",           no_argument,NULL, 'w'},
        {"stability",       no_argument,NULL, 's'},

        // 操作属性
        {"barno",           required_argument,NULL, 'b'},
        {"len",             required_argument,NULL, 'l'},
        {"offset",          required_argument,NULL, 'o'},
        {"value",           required_argument,NULL, 'v'},
        {"count",           required_argument,NULL, 'c'},
        {"help",           required_argument,NULL, 'h'},
        {NULL,     0,                      NULL, 0},
    }; 
    while((c =getopt_long_only(argc,argv,string,long_options,&option_index))!= -1) {
#if 0
        printf("opt = %c\t\t", c);
        printf("optarg = %s\t\t",optarg);
        printf("optind = %d\t\t",optind);
        printf("argv[optind] =%s\t\t", argv[optind]);
        printf("option_index = %d\n",option_index);
#endif
        switch(c){
            case 'r':            
            case 'w':           
            case 's':
                cmd  = c;
            break;
            case 'b':
                barno = htoi(optarg);
            break;
            case 'l':
                len   = strtoul(optarg,NULL,0);
            break;
            case 'o':
                offset = strtoul(optarg,NULL,0);
            break;
            case 'v':
                value  = htoi(optarg);
            break;
            case 'c':
                count  = strtoul(optarg, NULL,0);
            break;
            case 'h':
                app_pcie_test_help(argv[0]);
            return 0;
            default:
                app_pcie_test_help(argv[0]);
                return 0;
            break;
        }
    }
    printf("cmd  is %c, barnum is %d, len %#lx,offset is %#lx, value is %#.2x, count is %#lx\n",
        cmd , barno, len,offset,value,count);


    int ret = 0;
    int i   = 0;
    long j = 0;
    int fd = open(DEV_NAME, O_RDWR);
    if(fd < 0){
        perror("open failed");
        return -1;
    }
    struct pci_demo_opt opt;
    struct pci_demo_opt topt;
    memset(&opt, 0, sizeof(struct pci_demo_opt));
    srandom((long int)time(NULL));


    switch(cmd ){
    case 'r':
        {
            opt.barnum = barno;
            opt.len = len;
            opt.offset = offset;
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
            opt.barnum = barno;
            opt.len = len;
            opt.offset = offset;
            opt.addr = (unsigned long)malloc(opt.len);
            memset((void *)(opt.addr), value, opt.len );
            ret = ioctl(fd, PCIE_DEMO_WRITE_BAR, &opt);
            if(ret < 0){
                perror("write bar failed\n");
                free((void*)(opt.addr));
                break;
            }
            free((void*)(opt.addr));
        }
        break;
    case 's':
        {
#if 0            
            opt.barnum = barno;
            opt.len = len;
            opt.offset = offset;
            opt.addr = (unsigned long)malloc(opt.len);
            topt.barnum = barno;
            topt.len = len;
            topt.offset = offset;
            topt.addr = (unsigned long)malloc(opt.len);

            for(i=0;i<count;++i){
                memset((void *)(topt.addr),0x00, opt.len);
                for(j=0; j<opt.len; ++j)
                    ((char *)(opt.addr))[j] = random() % 255;
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
                if(memcmp((char *)(opt.addr), (char *)(topt.addr), opt.len) != 0) {
                    printf("memcmp failed\n");
                } else {
                    printf("memcmp OK\n");
                }
            }
            free((void *)(opt.addr));
            free((void *)(topt.addr));
#else
            printf("bar no is %d, len is %ld, offset is %ld, count is %ld\n",barno,len,offset,count);
            opt.barnum = barno;
            opt.len = len;
            opt.offset = offset;
            opt.addr = count;
            ret = ioctl(fd, PCIE_BAR_STABLE_TEST, &opt);
            printf("PCIE_BAR_STABLE_TEST test %d\n",ret);
#endif
        }
        break;
    }

    return 0;
}
