#ifndef __APP_HOT_BACKUP_H__
#define __APP_HOT_BACKUP_H_


#define FILE_PORT   9901
#define SLAVE_IP "192.168.36.224"
#define LENGTH_OF_LISTEN_QUEUE     20
#define RECV_SIZE   1024*256
#define BUFF_SIZE   1024*128
char g_backup_flag;
static unsigned int fileLen;

static char FILE_PATH_NAME[12][30]= {
    "/tmp/configs/config.conf",
    "/tmp/configs/config0.conf",
    "/tmp/configs/config1.conf",
    "/tmp/configs/config2.conf",
    "/tmp/configs/config3.conf",
    "/tmp/configs/config4.conf",
    "/tmp/configs/config5.conf",
    "/tmp/configs/config6.conf",
    "/tmp/configs/config7.conf",
    "/tmp/configs/config8.conf",
    "/tmp/configs/config9.conf",
    "/tmp/configs/config10.conf"
};


typedef enum{
    HEART,
    FILE_SAME,
    FILE_DIFF,
    SLAVE_UPDATE,
    HOST_RELOAD,
    ERROR,
    SUCCEED,
    FAILED,

} STATE;
STATE  currentState;


typedef struct {
    char * pFile;
    char FileName[30];
    unsigned int uiFileLen;
} FILE_INFO;


typedef struct {
    STATE state;
    unsigned int uiLen;
} MESSAGE;


int backup_host();
int backup_slave();

#endif