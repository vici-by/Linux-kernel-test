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
#define IOC_MAGIC  'm'
#define MTTY_DEMO_WRITE      _IO(IOC_MAGIC, 0)


#define DEV_NAME "/dev/tmtty"


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
    printf("\t-w write");
}
int main(int argc, char * argv[])
{
    int c;

    int digit_optind = 0;
    int option_index = 0;
    char *string = "w";

    char cmd = 'r';


    printf("[%s %d]: version %s\n", __func__,__LINE__,"0.0.0.0");

    static struct option long_options[] = {
        // 操作类型   
        {"write",           no_argument,NULL, 'w'},
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
            case 'h':
                app_pcie_test_help(argv[0]);
            return 0;
            default:
                app_pcie_test_help(argv[0]);
                return 0;
            break;
        }
    }
    printf("cmd  is %c\n",cmd );


    int ret = 0;
    int i   = 0;
    long j = 0;
    int fd = open(DEV_NAME, O_RDWR);
    if(fd < 0){
        perror("open failed");
        return -1;
    }

    srandom((long int)time(NULL));


    switch( cmd ){
    case 'w':
        {
            ret = ioctl(fd, MTTY_DEMO_WRITE, NULL);
            if(ret < 0){
                perror("MTTY_DEMO_WRITE failed\n");
                break;
            }
        }
        break;
    default:
        printf("invaild cmd");
        break;
    }
    close(fd);
    return 0;
}

