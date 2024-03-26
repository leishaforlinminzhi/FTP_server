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
#include"FileHandle.h"
#include"SocketHandle.h"

char file_path[MAXBUF];
char file_name[500];
int len;
char line[MAXBUF];
FILE* file;

struct file_trans{
    int fd;
    char file_to_trans[MAXBUF];
};


void get_file_path(const char*msg){
    char* format = "%*s %s";
    sscanf(msg, format, file_path);
    len=strlen(file_path);
    while (len > 0 && (file_path[len - 1] == '\r' || file_path[len - 1] == '\n'))
        file_path[--len] = 0;  
}

void get_file_name(){
    if (strncmp(file_path, "", 0) == 0) {
        strcpy(file_name,NULL);
        return;
    }

    char* lastSlash = strrchr(file_path, '/');
    if (lastSlash != NULL) {
        strcpy(file_name,lastSlash + 1);  // 返回最后一个斜杠之后的字符串
    } else {
        strcpy(file_name,file_path);  // 如果没有斜杠，则整个字符串都是文件名
    }
}

int handle_retr(const char* msg, struct Serverinfo* info) {
    printf("here1");
    info->history_files++;
    memset(info->buf, 0, sizeof(info->buf));
    int bytes_read;
    if (info->state_flag == 2) {
        if(chdir(info->WorkingPlace) != 0){
            strcpy(info->buf, "550 Current path not found.\r\n");
            return 1;
        }
        if(!info->ConnectType){
            strcpy(info->buf, "425 No TCP connection was established.\r\n");
            return 1;
        }

        get_file_path(msg);
        file = fopen(file_path, "rb");

        if (file == NULL) {
            strcpy(info->buf, "551 File not found.\r\n");
            return 1;
        }

        memset(info->buf, 0, sizeof(info->buf));

        pid_t pid = fork();
        if(pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            // 子进程负责传输文件
            if(info->ConnectType == 2){
                // PASV
                int conn;
                if ((conn = accept(info->data_fd, NULL, NULL)) < 0) {
                    exit(EXIT_FAILURE);
                }

                strcpy(info->buf, "150 Start Transporting\r\n");
                int buf_len=strlen(info->buf);
                send(info->listen_fd,info->buf,buf_len,0);
                memset(line,0,sizeof(line));
                while ((bytes_read = fread(line, 1, sizeof(line), file)) > 0) {
                    printf("{%s}\n",line);
                    buf_len=strlen(line);
                    send(conn,line,bytes_read,0);
                    memset(line,0,sizeof(line));
                    info->history_bytes_succ+=bytes_read;
                }
                close(conn);
            }else{
                // PORT
                if ((info->data_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                    printf("Error socket(): %s(%d)\n", strerror(errno), errno);
                    exit(EXIT_FAILURE);
                }
                info->remote_data.sin_family = AF_INET;
                info->remote_data.sin_port = htons(info->data_port);
                if (inet_pton(AF_INET, info->data_ip_str, &info->remote_data.sin_addr) < 0){
                    printf("fail to get Client's IP\n");
                    exit(EXIT_FAILURE);
                }
                if(connect(info->data_fd, (struct sockaddr *)(&info->remote_data), sizeof(info->remote_data)) < 0){
                    exit(EXIT_FAILURE);
                }
                printf("here");
                strcpy(info->buf, "150 Start Transporting\r\n");
                int buf_len=strlen(info->buf);
                send(info->listen_fd,info->buf,buf_len,0);
                
                memset(line,0,sizeof(line));
                while ((bytes_read = fread(line, 1, sizeof(line), file)) > 0) {
                    printf("{%s}\n",line);
                    buf_len=strlen(line);
                    send(info->data_fd,line,bytes_read,0);
                    memset(line,0,sizeof(line));
                    info->history_bytes_succ+=bytes_read;
                }
                close(info->data_fd);
            }
            
            fclose(file);
            exit(EXIT_SUCCESS);
        }else{
            // 父进程等待子进程结束
            int child_status;
            wait(&child_status);
            if (WIFEXITED(child_status)){
                // 子进程正常退出
                info->history_files_succ++;
                strcpy(info->buf, "226 Transfer complete.\r\n");
            } else {
                // 子进程异常退出
                strcpy(info->buf, "426 Connection broken.\r\n");
            }
        }
    }
    else {
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
    }
    return 0;
}

int handle_stor(const char* msg, struct Serverinfo* info){
    info->history_files++;
    memset(info->buf, 0, sizeof(info->buf));
    int len;
    printf("%s\n",msg);
    if (info->state_flag == 2) {
        if(!info->ConnectType){
            strcpy(info->buf, "425 No TCP connection was established.\r\n");
            return 1;
        }
        get_file_path(msg);
        printf("%s\n",file_path);
        if(chdir(info->WorkingPlace) != 0){
            strcpy(info->buf, "550 Current path not found.\r\n");
            return 1;
        }
        
        file = fopen(file_path, "w+r");

        if (file == NULL) {
            sprintf(info->buf, "452 Fail to create the file %s %s.\r\n",file_path,strerror(errno));
            return 1;
        }

        pid_t pid = fork();
        if(pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            // 子进程负责传输文件
            if(info->ConnectType == 2){
                //PASV
                int conn;
                if ((conn = accept(info->data_fd, NULL, NULL)) < 0) {
                    exit(EXIT_FAILURE);
                }

                strcpy(info->buf, "150 Start Transporting\r\n");
                int buf_len=strlen(info->buf);
                send(info->listen_fd,info->buf,buf_len,0);
                memset(line,0,sizeof(line));
                while((len = recv(conn, line, MAXBUF, 0)) > 0){
                    printf("{%.*s}\n", len, line);
                    if(fwrite(line, sizeof(char), len, file)< len || len <= 0){
                        break;
                    }
                    info->history_files++;
                }
                close(conn);
            }else{
                // PORT
                if ((info->data_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                    printf("Error socket(): %s(%d)\n", strerror(errno), errno);
                    exit(EXIT_FAILURE);
                }
                info->remote_data.sin_family = AF_INET;
                info->remote_data.sin_port = htons(info->data_port);
                if (inet_pton(AF_INET, info->data_ip_str, &info->remote_data.sin_addr) < 0){
                    printf("fail to get Client's IP\n");
                    exit(EXIT_FAILURE);
                }
                if(connect(info->data_fd, (const struct sockaddr*)&info->remote_data, sizeof(info->remote_data)) < 0){
                    exit(EXIT_FAILURE);
                }

                strcpy(info->buf, "150 Start Transporting\r\n");
                int buf_len=strlen(info->buf);
                send(info->listen_fd,info->buf,buf_len,0);

                memset(line,0,sizeof(line));
                while((len = recv(info->data_fd, line, MAXBUF, 0)) > 0){
                    printf("{%.*s}\n", len, line);
                    if(fwrite(line, sizeof(char), len, file)< len || len <= 0){
                        break;
                    }
                    info->history_bytes_succ+=len;
                }
                close(info->data_fd);
            }
            
            fclose(file);
            exit(EXIT_SUCCESS);
        }else{
            // 父进程等待子进程结束
            int child_status;
            wait(&child_status);
            if (WIFEXITED(child_status)){
                // 子进程正常退出
                info->history_files_succ++;
                strcpy(info->buf, "226 Transfer complete.\r\n");
            } else {
                // 子进程异常退出
                strcpy(info->buf, "426 Connection broken.\r\n");
            }
        }
    }
    else {
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
    }
    return 1;
}

int handle_mkd(const char* msg, struct Serverinfo* info){
    memset(info->buf, 0, sizeof(info->buf));
    if (info->state_flag == 2) {
        if (strlen(msg) < 4) {
            strcpy(info->buf, "501 Please enter the path.\r\n");
            return 1;
        }
        get_file_path(msg);
        if(chdir(info->WorkingPlace) != 0){
            strcpy(info->buf, "550 Current path not found.\r\n");
            return 1;
        }
        if(mkdir(file_path, S_IRWXU | S_IRWXG | S_IRWXO)!=0){
            chmod(file_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |S_IWOTH);
            strcpy(info->buf, "550 Fail to create the file.\r\n");
            return 1;
        }
        strcpy(info->buf, "250 Successfully created the file.\r\n");
    }
    else {
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
        return 1;
    }
    return 0;
}

int handle_cwd(const char* msg, struct Serverinfo* info){
    memset(info->buf, 0, sizeof(info->buf));
    if (info->state_flag == 2) {
        if (strlen(msg) < 4) {
            strcpy(info->buf, "501 Please enter the path.\r\n");
            return 1;
        }
        get_file_path(msg);
        if(chdir(info->WorkingPlace) != 0){
            strcpy(info->buf, "550 Current path not found.\r\n");
            return 1;
        }
        if(chdir(file_path) != 0){
            sprintf(info->buf, "550 %s: No such file or directory.\r\n", file_path);
            return 1;
        }else{
            getcwd(info->WorkingPlace, sizeof(info->WorkingPlace));
            strcpy(info->buf, "250 Okay.\r\n");
        }
    }
    else {
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
        return 1;
    }
    return 0;
}

int handle_pwd(const char* msg, struct Serverinfo* info) {
    memset(info->buf, 0, sizeof(info->buf));
    if (info->state_flag == 2) {
        if (strlen(msg) > 4) {
            strcpy(info->buf, "501 Invalid input parameters\r\n");
            return 1;
        }
        if (strncmp(info->WorkingPlace, "", 0) == 0) 
            if(!strncmp(info->WorkingPlace,"./",2)){
                strcpy(info->buf, "257 \"/\" \r\n");
            }else if(!strncmp(info->WorkingPlace,"/",1)){
                if(!strncmp(info->WorkingPlace,info->RootPlace,strlen(info->RootPlace))){
                    strcpy(info->buf,info->WorkingPlace + strlen(info->RootPlace));
                }else
                    sprintf(info->buf, "257 \"%s\" \r\n", info->WorkingPlace);
            }else{
                sprintf(info->buf, "257 \"%s\" \r\n", info->WorkingPlace);
            }
        else {
            strcpy(info->buf, "550 Failed to get current directory.\r\n");
            return 1;
        }
    }
    else {
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
        return 1;
    }
    return 0;
}

int handle_list(const char* msg, struct Serverinfo* info){
    memset(info->buf, 0, sizeof(info->buf));
    int Result = 0;
    char Command[300] = "ls -l ";
    char* Buffer = NULL;
    FILE *Pointer;

    if(info->state_flag == 2){
        if(!info->ConnectType){
            strcpy(info->buf, "425 No TCP connection was established.\r\n");
            return 1;
        }

        chdir(info-> WorkingPlace);
        if(strlen(msg) > 5){
            get_file_path(msg);
            strcat(Command,file_path);  
        }
        
        if( (Pointer = popen(Command, "r")) == NULL )
        {
            printf("popen() error!\n");
            return 1;
        }

        if(Result != 0)
        {
            pclose(Pointer);
            return 1;
        }
    
        Buffer = (char*)malloc(sizeof(char) * MAXBUF);
        memset(Buffer, 0, MAXBUF);
        fread(Buffer, sizeof(char), MAXBUF, Pointer);
        pclose(Pointer);


        pid_t pid = fork();
        if(pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if(pid == 0){
            // 子进程负责传输文件
            if(info->ConnectType == 2){
                // PASV
                int conn;
                if ((conn = accept(info->data_fd, NULL, NULL)) < 0) {
                    exit(EXIT_FAILURE);
                }

                strcpy(info->buf, "150 Opening data channel, for directory list.\r\n'\r\n");
                int buf_len=strlen(info->buf);
                send(info->listen_fd,info->buf,buf_len,0);
                send(conn,Buffer,strlen(Buffer),0);
                close(conn);
            }else{
                // PORT
                if ((info->data_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
                    printf("Error socket(): %s(%d)\n", strerror(errno), errno);
                    exit(EXIT_FAILURE);
                }
                info->remote_data.sin_family = AF_INET;
                info->remote_data.sin_port = htons(info->data_port);
                if (inet_pton(AF_INET, info->data_ip_str, &info->remote_data.sin_addr) < 0){
                    printf("fail to get Client's IP\n");
                    exit(EXIT_FAILURE);
                }
                if(connect(info->data_fd, (struct sockaddr *)(&info->remote_data), sizeof(info->remote_data)) < 0){
                    exit(EXIT_FAILURE);
                }

                strcpy(info->buf, "150 Opening data channel, for directory list.\r\n'\r\n");
                int buf_len=strlen(info->buf);
                send(info->listen_fd,info->buf,buf_len,0);
                
                memset(line,0,sizeof(line));
                send(info->data_fd,Buffer,strlen(Buffer),0);
                close(info->data_fd);
            }
            
            fclose(file);
            exit(EXIT_SUCCESS);
        }else{
            // 父进程等待子进程结束
            int child_status;
            wait(&child_status);
            if (WIFEXITED(child_status)){
                // 子进程正常退出
                info->history_files_succ++;
                strcpy(info->buf, "226 Transfer complete.\r\n");
            } else {
                // 子进程异常退出
                strcpy(info->buf, "426 Connection broken.\r\n");
            }
        }

        strcpy(info->buf, Buffer);

        strcpy(info->buf,"226 The entire directory was successfully transmitted.\r\n");
    }else{
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
        return 1; 
    }
    return 0;
}

int handle_rmd(const char* msg, struct Serverinfo* info){
    memset(info->buf, 0, sizeof(info->buf));

    if (info->state_flag == 2) {
        if(chdir(info->WorkingPlace) != 0){
            strcpy(info->buf, "451 Current path not found.\r\n");
            return 1;
        }
        get_file_path(msg);

        if(rmdir(file_path) != 0){
            sprintf(info->buf, "451 Fail to remove the directory. %s\r\n",strerror(errno));
            return 1;
        }
        strcat(info->buf, "226 The directory was successfully removed.\r\n");
        
    }
    else {
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
        return 1;
    }
    return 0;
}

int handle_rnfr(const char* msg, struct Serverinfo* info){
    memset(info->buf, 0, sizeof(info->buf));
    if (info->state_flag == 2) {
        get_file_path(msg);
        if(access(file_path, F_OK) < 0){
            strcpy(info->buf, "550 file not found.\r\n");
            return 1;
        }else{
            info->rename_flag = 1;
            strcpy(info->rename_buf,file_path);
            strcpy(info->buf, "350 file is found.\r\n");
        }
    }else{
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
        return 1;
    }
    return 0;
}

int handle_rnto(const char* msg, struct Serverinfo* info){
    memset(info->buf, 0, sizeof(info->buf));
    if (info->state_flag == 2) {
        if(!info->rename_flag){
            strcpy(info->buf, "503 please do RNFR first.\r\n");
            return 1;
        }else{
            get_file_path(msg);
            get_file_name();
            if(rename(info->rename_buf, file_name) == 0) {
                strcpy(info->buf, "250 file was renamed successfully.\r\n");
                info->rename_flag = 0;
            } else {
                strcpy(info->buf, "550 fail to rename the file.\r\n");
                info->rename_flag = 0;
                return 1;
            }
        }
    }else{
        strcpy(info->buf, "530 Please input your username and password first.\r\n");
        return 1;
    }
    return 0;
}
