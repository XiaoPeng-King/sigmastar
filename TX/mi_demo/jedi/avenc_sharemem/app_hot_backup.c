#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <pthread.h>
#include <netinet/tcp.h>

#include <sys/time.h>
#include <errno.h>

#include <stdbool.h>

#include "app_hot_backup.h"
#include "sharemem.h"
#include "linux_common.h"

//#define DEBUG
extern unsigned char g_Exit;

static int fd_config;

static int read_file(FILE_INFO *pFileInfo)
{
    //printf("\n-- read file --\n");
    FILE *pFile = NULL;
    //printf("%s\n", pFileInfo->FileName);
    pFile = OpenFile(pFileInfo->FileName, "r+");
    
    //printf("pFile: 0x%x \n", pFile);
    fseek(pFile, 0L, SEEK_END);
    pFileInfo->uiFileLen = ftell(pFile);
    //printf("pFile: 0x%x \n", pFile);
    
    //printf("uiFileLen = %d \n", pFileInfo->uiFileLen);
    rewind(pFile);
    pFileInfo->pFile = malloc(sizeof(char)*(pFileInfo->uiFileLen));
    ReadFile(pFileInfo->pFile, 1, pFileInfo->uiFileLen, pFile);
    fclose(pFile);

    return 0;
}

static int write_file(FILE_INFO *pFileInfo)
{
    #ifdef DEBUG
    printf("\n------write file start------\n");
    #endif
    FILE *pFile;
    int ret = 0;
    pFile = fopen(pFileInfo->FileName, "w+");
    if (pFile == NULL)
	{
		printf("file no exist \n");
		return -1;
	}
    ret = fwrite(pFileInfo->pFile, 1, pFileInfo->uiFileLen, pFile);
    if (ret != pFileInfo->uiFileLen)
    {
        printf("file write error");
        return -1;
    }
    close(pFile);

    return 0;
}

static STATE send_message(STATE CurrentState, int socket)
{
    #ifdef DEBUG
    printf("\n-----send message------\n");
    #endif
    int ret;
    MESSAGE message;
    message.state = CurrentState;
    message.uiLen = fileLen;
    ret = send(socket, &message, sizeof(MESSAGE), 0);
    if (ret <= 0)
    {
        return ERROR;
    }

    return SUCCEED;
}

static STATE recv_message(int socket)
{
    #ifdef DEBUG
    printf("\n-----recv message----\n");
    #endif
    int ret;
    MESSAGE message;

    ret = recv(socket, &message, sizeof(MESSAGE), 0);
    if (ret <= 0)
    {
        return ERROR;
    }
    
    fileLen = message.uiLen;

    return message.state;
}

static STATE send_file(int socket)
{
    #ifdef DEBUG
    printf("\n---- send file start ----\n");
    #endif
    FILE_INFO *pFileInfo = malloc(sizeof(FILE_INFO));
    STATE state;
    int ret = -1, i;
    
    for (i=0; i<12; i++)
    {

        strcpy(pFileInfo->FileName, FILE_PATH_NAME[i]);
        //printf("%s \n", pFileInfo->FileName);
        ret = read_file(pFileInfo);
        if (ret < 0)
        {
            printf("read file failed \n");
            state = ERROR;
            break;
        }
        ret = send(socket, pFileInfo, sizeof(FILE_INFO), 0);
        if (ret <= 0)
        {
            printf("send failed \n");
            perror("send");
            state = ERROR;
            break;
        }
        ret = send(socket, pFileInfo->pFile, pFileInfo->uiFileLen, 0);
        if (ret <= 0)
        {
            perror("send");
            state = ERROR;
            break;
        }
        else
        {
            state = SUCCEED;
        }
        if (NULL != pFileInfo->pFile)
        {
            free(pFileInfo->pFile);
            pFileInfo->pFile = NULL;
        }
    }
    #ifdef DEBUG
    printf("\n---- send file finish ----\n");
    #endif
    free(pFileInfo);
    
    return state;
}

static STATE recv_file(int socket)
{
    #ifdef DEBUG
    printf("\n-----recv file start-----\n");
    #endif
    FILE_INFO *pFileInfo = malloc(sizeof(FILE_INFO));
    STATE state;
    char recvBuf[RECV_SIZE] = {0};
    int ret = -1, i;
    
    for (i=0; i<12; i++)
    {
        ret = recv(socket, pFileInfo, sizeof(FILE_INFO), 0);
        if (ret <= 0)
        {
            perror("recv");
            state = ERROR;
            break;
        }
        #ifdef DEBUG
        printf("pFileInfo->uiFileLen : %d \n", pFileInfo->uiFileLen);
        #endif
        ret = recv(socket, recvBuf, pFileInfo->uiFileLen, 0);
        if (ret <= 0)
        {
            perror("recv");
            state = ERROR;
            break;
        }
        else
        {
            pFileInfo->pFile = recvBuf;
            
            if (ret == pFileInfo->uiFileLen)
            {
                if ((write_file(pFileInfo)) < 0)
                {
                    perror("write file failed \n");
                    state = ERROR;
                    break;
                }
                else
                {
                    state = SUCCEED;
                }
            }
            else
            {
                state = ERROR;
                break;
            }
        }
        
    }
    #ifdef DEBUG
    printf("\n-----recv file finish-----\n");
    #endif
    free(pFileInfo);

    return state;
}

static int compare_file()
{
    #ifdef DEBUG
    printf("compare file \n");
    #endif
    char buf1[BUFF_SIZE] = {0};
    char buf2[BUFF_SIZE] = {0};
    FILE *pFile1 = NULL;
    FILE *pFile2 = NULL;
    int ret = 0;

    pFile1 = OpenFile(FILE_PATH_NAME[0], "r+");
    pFile2 = OpenFile(FILE_PATH_NAME[1], "r+");

    ReadFile(buf1, 1, sizeof(char)*BUFF_SIZE, pFile1);
    ReadFile(buf2, 1, sizeof(char)*BUFF_SIZE, pFile2);

    fclose(pFile1);
    fclose(pFile2);

    ret = strcmp(buf1, buf2);
    if (0 == ret)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int backup_host()
{
    printf("\n-------------------host----------------\n");
    //FILE_INFO *pFileInfo = NULL;
    //MESSAGE message;
    STATE CurrentState;
    int i, ret;
    int sock_cli;
    
    struct sockaddr_in  server_addr;
    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(FILE_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SLAVE_IP);
    
ReSocket:
	sock_cli = CreateTCPSocket();
    printf("backup socket succeed \n");
Reconnect:
    if (!g_Exit)
    {
        goto EXIT;
    }
    printf("server_addr : %d \n", ntohs(server_addr.sin_port));
    if (ConnectToServer(sock_cli, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        sleep(1);
        goto Reconnect;
    }
    printf("backup connect succeed \n");

    SetSendTimeOut(sock_cli, 2, 0);
    SetRecvTimeOut(sock_cli, 2, 0);
    //compare file 
    ret = compare_file();
    
    //judge file whether laster
    if (ret)
    {
        #ifdef DEBUG
        printf("FILE_SAME");
        #endif
        CurrentState = FILE_SAME;
    }
    else
    {
        #ifdef DEBUG
        printf("FILE_DIFF");
        #endif
        CurrentState = FILE_DIFF;
    }
    //message.state = CurrentState;
    //send message
    send_message(CurrentState, sock_cli);
    while (g_Exit)
    {
        CurrentState = recv_message(sock_cli);
        #ifdef DEBUG
        printf("state: %d \n", CurrentState);
        #endif
        switch (CurrentState)
        {
            case HOST_RELOAD:
                printf("HOST_RELOAD \n");
                CurrentState = recv_file(sock_cli);
                if (CurrentState == SUCCEED)
                {
                    ret = AppInitCfgInfoFromFile();
                    if (ret < 0)
                    {
                        printf("get config file failed on backup \n");
                    }
                }
                send_message(CurrentState, sock_cli);
                
                break;

            case SLAVE_UPDATE:
                printf("SLAVE_UPDATE \n");
                send_message(SLAVE_UPDATE, sock_cli);
                send_file(sock_cli);//send file to slave
                CurrentState = recv_message(sock_cli); //receive 
                break;

            case HEART:
                printf("HEART \n");
                //printf("backup flag : %d \n", g_backup_flag);
                if (g_backup_flag)
                {
                    send_message(SLAVE_UPDATE, sock_cli);
                    send_file(sock_cli);
                    break;
                }
                send_message(HEART, sock_cli);
                break;

            case SUCCEED:
                printf("SUCCEED \n");
                send_message(HEART, sock_cli);
                g_backup_flag = 0;
                break;

            case FAILED:
                printf("FAILED \n");
                close(sock_cli);
                goto ReSocket;
                break;

            case ERROR:
                printf("ERROR \n");
                close(sock_cli);
                goto ReSocket;
                break;

            default:
                printf("state: %d \n", CurrentState);
                close(sock_cli);
                goto ReSocket;
                break;
        }
        sleep(1);
    }
EXIT:
    close(sock_cli);
    return 0;
}

int backup_slave()
{
    printf("\n-------------------slave----------------\n");
    //FILE_INFO *pFileInfo = NULL;
    //MESSAGE message;
    STATE CurrentState;
    int i, ret;
    int server_socket, new_server_socket;

    struct sockaddr_in  server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(FILE_PORT);

ReSocket:
    sleep(1);
	server_socket = CreateTCPSocket();
    printf("backup socket succeed \n");
	SocketBindToAddr(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("backup bind succeed \n");
    
    // listen
    ListenSocket(server_socket, LENGTH_OF_LISTEN_QUEUE);
    
ReAccept:
    new_server_socket = AcceptConnect(server_socket, NULL,NULL);
    printf("backup accept succeed \n");

    SetRecvTimeOut(new_server_socket, 3, 0);
    SetSendTimeOut(new_server_socket, 3, 0);

    while (g_Exit)
    {
        //recive message
        printf("start recv message \n");
        CurrentState = recv_message(new_server_socket);
        printf("recv message succeed, current state : %d \n", CurrentState);
        switch (CurrentState)
        {
            case FILE_SAME:
                printf("FILE_SAME \n");
                
                //send_message(HEART, new_server_socket);
                send_message(HOST_RELOAD, new_server_socket);
                //send file
                ret = send_file(new_server_socket);
                break;

            case FILE_DIFF:
                printf("FILE_DIFF \n");
                send_message(HEART, new_server_socket);
                //send_message(HOST_RELOAD, new_server_socket);
                //send file
                //ret = send_file(new_server_socket);
                break;

            case SLAVE_UPDATE:
                printf("SLAVE_UPDATE \n");
                CurrentState = recv_file(new_server_socket);
                send_message(CurrentState, new_server_socket);
                break;

            case FAILED:
                printf("FAILED \n");
                close(new_server_socket);
                goto ReAccept;
                break;

            case SUCCEED:
                printf("SUCCEED \n");
                send_message(HEART, new_server_socket);
                break;

            case HEART:
                printf("HEART\n");
                //send message
                send_message(HEART, new_server_socket);
                break;

            case ERROR:
                printf("ERROR \n");
                close(new_server_socket);
                goto ReAccept;
                break;

            default:
                printf("state: %d \n", CurrentState);
                close(new_server_socket);
                goto ReAccept;
                break;
        }
        sleep(1);
    }
    close(new_server_socket);
    close(server_socket);
    return 0;
}