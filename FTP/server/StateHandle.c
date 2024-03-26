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


int handle_user(const char* msg, struct Serverinfo* info) {
    memset(info->buf, 0, sizeof(info->buf));
    switch (info->state_flag)
    {
    case 0:
        if (strncmp(msg, "USER anonymous", strlen("USER anonymous")) == 0) { 
            info->state_flag = 1; 
            strcpy(info->buf, "331 Guest login ok, send your complete e-mail address as password.\r\n");
        }
        else {
            strcpy(info->buf, "530 All users other than anonymous are not supported.\r\n");
        }
        break;
    case 1:
        strcpy(info->buf, "530 USER anonymous is logging in, please input the password.\r\n");
        break;
    case 2:
        strcpy(info->buf, "502 USER anonymous already login.\r\n");
        break;
    default:
        break;
    }
    return 0;
}

int handle_pass(const char* msg, struct Serverinfo* info) {
    memset(info->buf, 0, sizeof(info->buf));
    switch (info->state_flag)
    {
    case 0:
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
        break;
    case 1:
        if (strlen(msg) > 6) { 
            info->state_flag = 2; 
            strcpy(info->buf, "230-\r\n230-Welcome to\r\n230-School of Software\r\n230-FTP Archives at ftp.ssast.org\r\n230-\r\n230-This site is provided as a public service by School of\r\n230-Software. Use in violation of any applicable laws is strictly\r\n230-prohibited. We make no guarantees, explicit or implicit, about the\r\n230-contents of this site. Use at your own risk.\r\n230-\r\n230 Guest login ok, access restrictions apply.\r\n");
        }
        else
            strcpy(info->buf, "500 Please enter your complete e-mail address as password.\r\n");
        break;
    case 2:
        strcpy(info->buf, "502 USER anonymous already login.\r\n");
        break;
    default:
        break;
    }
    return 0;
}

int handle_type(const char* msg, struct Serverinfo* info) {
    memset(info->buf, 0, sizeof(info->buf));
    if (info->state_flag == 2) {
        if (strncmp(msg, "TYPE A", 6) == 0 || strncmp(msg, "TYPE A N", 8) == 0) {
            info->binary_flag = 0;
            strcpy(info->buf, "200 Type set to A.\r\n");
        }else if (strncmp(msg, "TYPE I", 6) == 0 || strncmp(msg, "TYPE L 8", 8) == 0) {
            info->binary_flag = 1;
            strcpy(info->buf, "200 Type set to I.\r\n");
        }else {
            strcpy(info->buf, "501 Invalid input parameters\r\n");
        }
    }
    else {
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
    }
    return 0;
}

int handle_syst(const char* msg, struct Serverinfo* info) {
    memset(info->buf, 0, sizeof(info->buf));
    if (info->state_flag == 2) {
        if (info->binary_flag)
            strcpy(info->buf, "215 UNIX Type: A\r\n");
        else
            strcpy(info->buf, "215 UNIX Type: L8\r\n");
    }
    else {
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
    }
    return 0;
}

int handle_quit(const char* msg, struct Serverinfo* info){
    sprintf(info->buf, "221-You have transferred %d bytes in %d files.\r\n",info->history_bytes_succ,info->history_files_succ);
    strcat(info->buf, "221-Thank you for using the FTP service on ftp.ssast.org.\r\n");
    strcat(info->buf, "221 Goodbye.\r\n.\r\n");
    info->state_flag = 0;
    return 0;
}
