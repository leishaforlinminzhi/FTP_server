#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include"StructofServer.h"
#include"SocketHandle.h"

int handle_port(const char* msg, struct Serverinfo* info) {
    memset(info->buf, 0, sizeof(info->buf));
    if(info->state_flag == 2){
        int p[2];
        const char* format = "%*s %d,%d,%d,%d,%d,%d";
        sscanf(msg, format, &info->data_ip[0], &info->data_ip[1], &info->data_ip[2], &info->data_ip[3], &p[0], &p[1]);

        info->data_port = p[0] * 256 + p[1];
        sprintf(info->data_ip_str, "%d.%d.%d.%d",info->data_ip[0], info->data_ip[1], info->data_ip[2], info->data_ip[3]);

        printf("%d,%d,%d,%d,%d,%d\n",info->data_ip[0], info->data_ip[1], info->data_ip[2], info->data_ip[3], p[0], p[1]);

        if(info->data_fd != -1)
            close(info->data_fd);
        
        info->ConnectType = 1;

        strcpy(info->buf, "200 PORT command successful. Data connection established.\r\n");
    }else{
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
    }
    return 0;
}

int handle_pasv(const char* msg, struct Serverinfo* info) {
    memset(info->buf, 0, sizeof(info->buf));
    if(info->state_flag == 2){
        info->data_port = rand() % 45536 + 20000;
        
        if ((info->data_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
            printf("build Error socket: %s(%d)\n", strerror(errno), errno);
            return 1;
        }

        memset(&info->remote_data, 0, sizeof(info->remote_data));
        info->remote_data.sin_family = AF_INET;
        info->remote_data.sin_port = htons(info->data_port);
        info->remote_data.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(info->data_fd, (struct sockaddr*)&info->remote_data, sizeof(info->remote_data)) < 0) {
            printf("fail to bind\n");
            return 1;
        }

        if (listen(info->data_fd, 3) < 0) {
            printf("fail to listen\n");
            return 1;
        }

        info->ConnectType = 2;

        sprintf(info->buf,"227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n", 
                info->data_ip[0],info->data_ip[1],info->data_ip[2],info->data_ip[3],info->data_port/256,info->data_port%256);
    }else{
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
    }
    return 0;
}

