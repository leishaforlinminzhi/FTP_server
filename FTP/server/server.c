#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

#include "RequestHandle.h"
#include "StructofServer.h"

int port;
char place[500];

int handle_argument(int argc, char* argv[]) {

    strcpy(place, "/tmp");
    port = 21;

    switch (argc) {
        case 1:
            return 0;
            break;
        case 3:
            if (!strncmp(argv[1], "-port", 5)) {
                port = atoi(argv[2]);
                if (port < 0 || port > 65535)
                    return 1; // for port invalid
                return 0;
            } else if (!strncmp(argv[1], "-root", 5)) {
                if (argv[2] != NULL) {
                    //if (!access(argv[2], F_OK)) {
                        strcpy(place, argv[2]);
                        return 0;
                    //}
                }
                return 2; // for root invalid
            }
            return 3; // for argument invalid
            break;
        case 5:
            if (!strncmp(argv[1], "-port", 5) && !strncmp(argv[3], "-root", 5)) {
                port = atoi(argv[2]);
                strcpy(place, argv[4]);
            } else if (!strncmp(argv[3], "-port", 5) && !strncmp(argv[1], "-root", 5)) {
                port = atoi(argv[4]);
                strcpy(place, argv[2]);
            } else {
                return 3; // for argument invalid
            }
            if (port < 0 || port > 65535)
                return 1; // for port invalid
            return 0;
            break;
        default:
            break;
    }
    return 3;
}

void* handle_connection(void* arg) {
    struct Serverinfo* info = (struct Serverinfo*) arg;

    strcpy(info->buf, "220 ftp.ssast.org FTP server ready.\r\n");
    send(info->listen_fd, info->buf, strlen(info->buf), 0);
    int lenth;

    while ((lenth = recv(info->listen_fd, info->sentence, 8192, 0)) > 0) {
        info->sentence[lenth] = '\0';
        printf("%s\n",info->sentence);
        handle_request(info->sentence, info);
        if(strncmp(info->sentence,"QUIT",4)==0)
            break;
    }

    close(info->listen_fd);
    free(info);

    return NULL;
}

int main(int argc, char *argv[]) {

    printf("%d\n", handle_argument(argc, argv));
    printf("now:%d %s\n", port, place);

    struct stat st;
    if (stat(place, &st)) {
        mkdir(place, S_IRWXU | S_IRWXG | S_IRWXO);
    }
    chdir(place);

    int listen_fd;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    struct sockaddr_in remote;

    remote.sin_family = AF_INET;
    remote.sin_port = htons(port);
    remote.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr*) &remote, sizeof(remote)) < 0) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    if (listen(listen_fd, 3) < 0) {
        printf("fail to listen\n");
        return 1;
    }

    int conn;
    while (1) {
        if ((conn = accept(listen_fd, NULL, NULL)) < 0) {
            //printf("Error accept\n");
            continue;
        }

        struct Serverinfo* info = (struct Serverinfo*) malloc(sizeof(struct Serverinfo));
        server_init(info);
        info->listen_fd = conn;
        info->listen_port = port;
        strcpy(info->WorkingPlace, "./");
        getcwd(info->RootPlace, MAXBUF);

        pthread_t tid;
        pthread_create(&tid, NULL, handle_connection, info);
        pthread_detach(tid);
    }

    close(listen_fd);

    return 0;
}
