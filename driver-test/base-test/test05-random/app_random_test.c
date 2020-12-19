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
#include <getopt.h>



/* 定义设备类型 */
#define DEV_NAME "/dev/RANDOM_DEMO"

#define IOC_MAGIC  'r'
#define PCIE_DEMO_GET_RANDOM       _IO(IOC_MAGIC, 0)

struct random_demo_opt {
    unsigned long len;
    unsigned long addr;
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



static void app_random_test_help(const char * s)
{
    printf("Usage %s:\n",s);
    printf("\t-r read\n");
    printf("\t-l len,default 0x1024\n");
}
int main(int argc, char * argv[])
{
    int c;

    int digit_optind = 0;
    int option_index = 0;
    char *string = "rl:h";

    char cmd = 'r';
    char barno = 0;
    char value = 0x5A;
    size_t len = 0x1024;
    size_t count = 0x1024;
    size_t offset = 0;

    printf("[%s %d]: version %s\n", __func__,__LINE__,"0.0.0.0");

    static struct option long_options[] = {
        // 操作类型   
        {"read",            no_argument,NULL, 'r'},

        // 操作属性
        {"len",             required_argument,NULL, 'l'},
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
                cmd  = c;
            break;
            case 'l':
                len   = htoi(optarg);
            break;
            case 'h':
                app_random_test_help(argv[0]);
            return 0;
            default:
            break;
        }
    }
    printf("cmd  is %c, len %#lx\n",cmd , len);


    int ret = 0;
    int i   = 0;
    long j = 0;
    int fd = open(DEV_NAME, O_RDWR);
    if(fd < 0){
        perror("open failed");
        return -1;
    }
    struct random_demo_opt opt;
    memset(&opt, 0, sizeof(struct random_demo_opt));
    srandom((long int)time(NULL));


    switch(cmd ){
    case 'r':
        {
            opt.len = len;
            opt.addr = (unsigned long)malloc(opt.len);
            memset((void *)(opt.addr), 0, opt.len );
            ret = ioctl(fd, PCIE_DEMO_GET_RANDOM, &opt);
            if(ret < 0){
                perror("read random failed\n");
                free((void*)(opt.addr));
                break;
            }
            dump_data((void*)(opt.addr), opt.len);
            free((void*)(opt.addr));
        }
        break;
    }

    return 0;
}
