
#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC int ls(char* pathName)  // 传入当前目录，发送当前目录下的文件名
{
    MESSAGE msg;
    msg.type = LS;  // ls类型的消息（这个说法怪怪的）

    msg.PATHNAME = (void*)pathName;
    msg.NAME_LEN = strlen(pathName);
    msg.FLAGS = 0;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}