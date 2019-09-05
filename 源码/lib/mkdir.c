
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

PUBLIC int mkdir(char* path)
{
    MESSAGE msg;
    msg.type = MKDIR;

    msg.PATHNAME = (void*)path;
    msg.NAME_LEN = strlen(path);
    msg.FLAGS = 0;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.RETVAL;
}