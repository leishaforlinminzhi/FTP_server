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

#include"RequestHandle.h"
#include"StructofServer.h"
#include"StateHandle.h"
#include"SocketHandle.h"
#include"FileHandle.h"

enum cmd_type {
    USER, PASS, RETR, STOR, QUIT,
    SYST, TYPE, PORT_, PASV, MKD,
    CWD, PWD, LIST, RMD, RNFR,
    RNTO, UNKNOWN
};

enum cmd_type get_cmd_type(const char* cmd) {
    if (strncmp(cmd, "USER", 4) == 0) {
        return USER;
    }
    else if (strncmp(cmd, "PASS ", 5) == 0) {
        return PASS;
    }
    else if (strncmp(cmd, "RETR ", 5) == 0) {
        return RETR;
    }
    else if (strncmp(cmd, "STOR ", 5) == 0) {
        return STOR;
    }
    else if (strncmp(cmd, "QUIT", 4) == 0) {
        return QUIT;
    }
    else if (strncmp(cmd, "SYST", 4) == 0) {
        return SYST;
    }
    else if (strncmp(cmd, "TYPE ", 5) == 0) {
        return TYPE;
    }
    else if (strncmp(cmd, "PORT ", 5) == 0) {
        return PORT_;
    }
    else if (strncmp(cmd, "PASV", 4) == 0) {
        return PASV;
    }
    else if (strncmp(cmd, "MKD", 3) == 0) {
        return MKD;
    }
    else if (strncmp(cmd, "CWD", 3) == 0) {
        return CWD;
    }
    else if (strncmp(cmd, "PWD", 3) == 0) {
        return PWD;
    }
    else if (strncmp(cmd, "LIST", 4) == 0) {
        return LIST;
    }
    else if (strncmp(cmd, "RMD ", 4) == 0) {
        return RMD;
    }
    else if (strncmp(cmd, "RNFR ", 5) == 0) {
        return RNFR;
    }
    else if (strncmp(cmd, "RNTO ", 5) == 0) {
        return RNTO;
    }
    printf("----here\n");
    return UNKNOWN;
}

int handle_request(const char* msg, struct Serverinfo* info) {
    memset(info->buf, 0, sizeof(info->buf));
    switch (get_cmd_type(msg))
    {
    case USER:
        handle_user(msg, info);
        break;
    case PASS:
        handle_pass(msg, info);
        break;
    case RETR:
        printf("retr");
        handle_retr(msg, info);
        chdir(info->RootPlace);
        break;
    case STOR:
        handle_stor(msg, info);
        chdir(info->RootPlace);
        break;
    case QUIT:
        handle_quit(msg, info);
        break;
    case SYST:
        handle_syst(msg, info);
        break;
    case TYPE:
        handle_type(msg, info);
        break;
    case PORT_:
        handle_port(msg, info);
        break;
    case PASV:
        handle_pasv(msg, info);
        break;
    case MKD:
        handle_mkd(msg, info);
        chdir(info->RootPlace);
        break;
    case CWD:
        handle_cwd(msg, info);
        chdir(info->RootPlace);
        break;
    case PWD:
        handle_pwd(msg, info);
        chdir(info->RootPlace);
        break;
    case LIST:
        handle_list(msg, info);
        chdir(info->RootPlace);
        break;
    case RMD:
        handle_rmd(msg, info);
        chdir(info->RootPlace);
        break;
    case RNFR:
        handle_rnfr(msg, info);
        chdir(info->RootPlace);
        break;
    case RNTO:
        handle_rnto(msg, info);
        chdir(info->RootPlace);
        break;
    default:
        strcpy(info->buf,"500 Syntax error, command unrecognized");
        break;
    }
    int buf_len=strlen(info->buf);
    send(info->listen_fd,info->buf,buf_len,0);
    return 0;
}
