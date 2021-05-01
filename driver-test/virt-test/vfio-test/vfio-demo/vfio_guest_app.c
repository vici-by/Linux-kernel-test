#if 0
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>


struct termios otermios, ntermios;

/*
struct termios
{
    tcflag_t c_iflag;      // input modes
    tcflag_t c_oflag;      // output modes
    tcflag_t c_cflag;      // control modes
    tcflag_t c_lflag;      // local modes
    cc_t     c_cc[NCCS];   // special characters
};
*/

static void dump_tc(struct termios * mios)
{


}

static int init_uart(int fd, int baud)
{
    tcgetattr(fd, &otermios);       // 用来恢复串口属性
    printf("Dump Default tc:\n");
    dump_tc(&otermios);
    tcgetattr(fd, &ntermios);

}


static int

int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("Input %s /dev/ttyn\n", argv[0]);
        return 0;
    }
    printf("INIT UART");

    int fd = open(argv[1]);
    assert(fd > 0);
    init_uart(fd, );

}
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>


typedef unsigned short __u16;
typedef unsigned int   __u32;
typedef unsigned long  __u64;
#define ___constant_swab16(x) ((__u16)(				\
	(((__u16)(x) & (__u16)0x00ffU) << 8) |			\
	(((__u16)(x) & (__u16)0xff00U) >> 8)))

#define ___constant_swab32(x) ((__u32)(				\
	(((__u32)(x) & (__u32)0x000000ffUL) << 24) |		\
	(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) |		\
	(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) |		\
	(((__u32)(x) & (__u32)0xff000000UL) >> 24)))

#define ___constant_swab64(x) ((__u64)(				\
	(((__u64)(x) & (__u64)0x00000000000000ffULL) << 56) |	\
	(((__u64)(x) & (__u64)0x000000000000ff00ULL) << 40) |	\
	(((__u64)(x) & (__u64)0x0000000000ff0000ULL) << 24) |	\
	(((__u64)(x) & (__u64)0x00000000ff000000ULL) <<  8) |	\
	(((__u64)(x) & (__u64)0x000000ff00000000ULL) >>  8) |	\
	(((__u64)(x) & (__u64)0x0000ff0000000000ULL) >> 24) |	\
	(((__u64)(x) & (__u64)0x00ff000000000000ULL) >> 40) |	\
	(((__u64)(x) & (__u64)0xff00000000000000ULL) >> 56)))


typedef struct VFIO_MSG{
    unsigned int magic;     // 必须是 0x5A5AA5A5，否则无效帧;
    unsigned int cmd;       // 命令帧, bit31==1代表命令ack报文
    unsigned int rvd;
    unsigned int ack;       // 如果是ack，那么返回值。如果cmd需要带参数
}VFIO_MSG;


#define MAX_BUF_SIZE     2048
char buf[MAX_BUF_SIZE+2];

#define MY_END_CHAR      0x13


int setup_port(int fd, int baud, int databits, int parity, int stopbits);
int reset_port(int fd);
int read_data(int fd, void *buf, int len);
int write_data(int fd, void *buf, int len);
void print_usage(char *program_name);


int main(int argc, char *argv[]) //./a.out /dev/ttyS3 38400 8 0 1 0
{
    int fd;
    int baud;
    int len;
    int count;
    int i;
    int databits;
    int stopbits;
    int parity;
    int read_or_write;


    if (argc != 7) {
        print_usage(argv[0]);
        return 1;
    }

    baud = atoi(argv[2]);
    if ((baud < 0) || (baud > 921600)) {
        fprintf(stderr, "Invalid baudrate!\n");
        return 1;
    }

    databits = atoi(argv[3]);
    if ((databits < 5) || (databits > 8)) {
        fprintf(stderr, "Invalid databits!\n");
        return 1;
    }

    parity = atoi(argv[4]);
    if ((parity < 0) || (parity > 2)) {
        fprintf(stderr, "Invalid parity!\n");
        return 1;
    }

    stopbits = atoi(argv[5]);
    if ((stopbits < 1) || (stopbits > 2)) {
        fprintf(stderr, "Invalid stopbits!\n");
        return 1;
    }

    read_or_write = atoi(argv[6]);

    fd = open(argv[1], O_RDWR, 0);
    if (fd < 0) {
        fprintf(stderr, "open <%s> error %s\n", argv[1], strerror(errno));
        return 1;
    }

    if (setup_port(fd, baud, databits, parity, stopbits)) {
        fprintf(stderr, "setup_port error %s\n", strerror(errno));
        close(fd);
        return 1;
    }

    count = 0;

    if (read_or_write == 1) {
        fprintf(stderr, "Begin to send:\n");

        while ( (len = read(0, buf, MAX_BUF_SIZE)) > 0 ) {
            if (len == 1) {
                buf[0] = MY_END_CHAR;
                buf[1] = 0;
                write_data(fd, buf, len);
                break;
            }


            i = write_data(fd, buf, len);
            if (i == 0) {
                fprintf(stderr, "Send data error!\n");
                break;
            }

            //count += len;
            //fprintf(stderr, "Send %d bytes\n", len);
        }

    } else if (read_or_write == 2){

        fprintf(stderr, "Begin to send:\n");

        while(1){
            printf("Start transmit\n");
            memset(buf,'\0',MAX_BUF_SIZE);
            VFIO_MSG * p = (VFIO_MSG *)buf;
            p->magic = ___constant_swab32(0x5A5AA5A5);
            p->cmd   = ___constant_swab32(0x3);
            printf("Start Send:\n");
            write_data(fd, buf, sizeof(VFIO_MSG));

            sleep(5);
        }
#if 0
        while ( (len = read(0, buf, MAX_BUF_SIZE)) > 0 ) {
            if (len == 1) {
                buf[0] = MY_END_CHAR;
                buf[1] = 0;
                write_data(fd, buf, len);
                break;
            }


            i = write_data(fd, buf, len);
            if (i == 0) {
                fprintf(stderr, "Send data error!\n");
                break;
            }

            //count += len;
            //fprintf(stderr, "Send %d bytes\n", len);
        }

#endif

    }else {

        fprintf(stderr, "Begin to recv:\n");
	int x = 0;
        len = MAX_BUF_SIZE;

        while (1) {

            i = read_data(fd, buf, len);
            if (i > 0) {
                //count += i;
                //fprintf(stderr, "Recv %d byte\n", i);
		for(x=0;x<i;x++)
                fprintf(stderr,"%02X", buf[x]);
                if (buf[i-1] == MY_END_CHAR) {
                    break;
                }
            }
        }
    }

    reset_port(fd);
    close(fd);

    return 0;
}


static int baudflag_arr[] = {
    B921600, B460800, B230400, B115200, B57600, B38400,
    B19200,  B9600,   B4800,   B2400,   B1800,  B1200,
    B600,    B300,    B150,    B110,    B75,    B50
};
static int speed_arr[] = {
    921600,  460800,  230400,  115200,  57600,  38400,
    19200,   9600,    4800,    2400,    1800,   1200,
    600,     300,     150,     110,     75,     50
};


int speed_to_flag(int speed)
{
    int i;

    for (i = 0;  i < sizeof(speed_arr)/sizeof(int);  i++) {
        if (speed == speed_arr[i]) {
            return baudflag_arr[i];
        }
    }

    fprintf(stderr, "Unsupported baudrate, use 9600 instead!\n");
    return B9600;
}


static struct termio oterm_attr;

int setup_port(int fd, int baud, int databits, int parity, int stopbits)
{
    struct termio term_attr;


    if (ioctl(fd, TCGETA, &term_attr) < 0) {
        return -1;
    }


    memcpy(&oterm_attr, &term_attr, sizeof(struct termio));

    term_attr.c_iflag &= ~(INLCR | IGNCR | ICRNL | ISTRIP);
    term_attr.c_oflag &= ~(OPOST | ONLCR | OCRNL);
    term_attr.c_lflag &= ~(ISIG | ECHO | ICANON | NOFLSH);
    term_attr.c_cflag &= ~CBAUD;
    term_attr.c_cflag |= CREAD | speed_to_flag(baud);


    term_attr.c_cflag &= ~(CSIZE);
    switch (databits) {
        case 5:
            term_attr.c_cflag |= CS5;
            break;

        case 6:
            term_attr.c_cflag |= CS6;
            break;

        case 7:
            term_attr.c_cflag |= CS7;
            break;

        case 8:
        default:
            term_attr.c_cflag |= CS8;
            break;
    }


    switch (parity) {
        case 1:
            term_attr.c_cflag |= (PARENB | PARODD);
            break;

        case 2:
            term_attr.c_cflag |= PARENB;
            term_attr.c_cflag &= ~(PARODD);
            break;

        case 0:
        default:
            term_attr.c_cflag &= ~(PARENB);
            break;
    }



    switch (stopbits) {
        case 2:
            term_attr.c_cflag |= CSTOPB;
            break;

        case 1:
        default:
            term_attr.c_cflag &= ~CSTOPB;
            break;
    }

    term_attr.c_cc[VMIN] = 1;
    term_attr.c_cc[VTIME] = 0;

    if (ioctl(fd, TCSETAW, &term_attr) < 0) {
        return -1;
    }

    if (ioctl(fd, TCFLSH, 2) < 0) {
        return -1;
    }

    return 0;
}


int read_data(int fd, void *buf, int len)
{
    int count;
    int ret;

    ret = 0;
    count = 0;

    //while (len > 0) {

    ret = read(fd, (char*)buf + count, len);
    if (ret < 1) {
        fprintf(stderr, "Read error %s\n", strerror(errno));
        //break;
    }

    count += ret;
    len = len - ret;

    //}

    *((char*)buf + count) = 0;
    return count;
}


int write_data(int fd, void *buf, int len)
{
    int count;
    int ret;

    ret = 0;
    count = 0;

    while (len > 0) {

        ret = write(fd, (char*)buf + count, len);
        if (ret < 1) {
            fprintf(stderr, "Write error %s\n", strerror(errno));
            break;
        }

        count += ret;
        len = len - ret;
    }

    return count;
}


void print_usage(char *program_name)
{
    fprintf(stderr,
            "*************************************\n"
            "  A Simple Serial Port Test Utility\n"
            "*************************************\n\n"
            "Usage:\n  %s <device> <baud> <databits> <parity> <stopbits> <read_or_write>\n"
            "       databits: 5, 6, 7, 8\n"
            "       parity: 0(None), 1(Odd), 2(Even)\n"
            "       stopbits: 1, 2\n"
            "       read_or_write: 0(read), 1(write)\n"
            "Example:\n  %s /dev/ttyS0 9600 8 0 1 0\n\n",
            program_name, program_name
           );
}


int reset_port(int fd)
{
    if (ioctl(fd, TCSETAW, &oterm_attr) < 0) {
        return -1;
    }

    return 0;
}


