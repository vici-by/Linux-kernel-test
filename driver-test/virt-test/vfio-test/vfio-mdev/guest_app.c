#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <termios.h>



int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if  ( tcgetattr( fd,&oldtio)  !=  0) { 
        perror("SetupSerial 1");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL | CREAD;
    newtio.c_cflag &= ~CSIZE;

    switch( nBits )
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch( nEvent )
    {
    case 'O':
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E': 
        newtio.c_iflag |= (INPCK | ISTRIP);
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= ~PARODD;
        break;
    case 'N':  
        newtio.c_cflag &= ~PARENB;
        break;
    }

    switch( nSpeed )
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
    }
    if( nStop == 1 )
        newtio.c_cflag &=  ~CSTOPB;
    else if ( nStop == 2 )
        newtio.c_cflag |=  CSTOPB;
        newtio.c_cc[VTIME]  = 0;
        newtio.c_cc[VMIN] = 0;
        tcflush(fd,TCIFLUSH);
    if((tcsetattr(fd,TCSANOW,&newtio))!=0)
    {
        perror("com set error");
        return -1;
    }
    
    //  printf("set done!\n\r");
    return 0;
}

int main(int argc,char *argv[])
{
    char *uart_path="/dev/ttyS4";
    int fd,len;
    char read_buf[512];
	char write_buf[64];
    int r;
	
    memset(read_buf,0,sizeof(read_buf));
	memset(write_buf,0,sizeof(write_buf));
    
    if((fd = open(uart_path,O_RDWR|O_NOCTTY|O_NDELAY))<0)
    {

        printf("open uart %s err \n",uart_path);
        exit(1);
    }
    printf("open uart %s success\n",uart_path);
    set_opt(fd,115200, 8, 'N', 1);
  
    while(1)
    {
    	printf("[%s:%d]please input:\n",__func__,__LINE__);
        gets(write_buf);
	    write(fd,write_buf,strlen(write_buf));
		printf("[%s:%d]please press enter to wait for host write\n",__func__,__LINE__);
		getchar();
        len = read(fd, read_buf, sizeof(read_buf));
		if (len < 0) {
			printf("[%s:%d]read fail\n",__func__,__LINE__);
		}
		printf("host write:%s\n",read_buf);
		memset(read_buf,0,sizeof(read_buf));
		memset(write_buf,0,sizeof(write_buf));
    } 
}

