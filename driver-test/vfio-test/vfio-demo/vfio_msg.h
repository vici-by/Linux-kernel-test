#ifndef __VFIO_MSG_H
#define __VFIO_MSG_H
typedef enum vfio_cmd{
    CMD_CHANNEL_STAT,
    CMD_CHANNEL_START,
    CMD_CHANNEL_STOP,
    CMD_CHANNEL_PULSE,
    CMD_CHANNEL_CONTINUE,
}vfio_cmd;

typedef enum vfio_cmd_ack{
    CMD_ACK_OK,
    CMD_ACK_RETRY,
    CMD_ACK_ABORT,
}vfio_cmd_ack;


typedef struct VFIO_MSG{
    unsigned int magic;     // 必须是 0x5A5AA5A5，否则无效帧;
    unsigned int cmd;       // 命令帧, bit31==1代表命令ack报文
    unsigned int rvd;
    unsigned int ack;       // 如果是ack，那么返回值。如果cmd需要带参数
}VFIO_MSG;
typedef struct VFIO_MSG_LIST{
    struct list_head list;
    VFIO_MSG msg;
}VFIO_MSG_LIST;

#endif
