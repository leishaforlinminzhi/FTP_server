#pragma once
#ifndef STRUCTOFSERVER_H
#define STRUCTOFSERVER_H

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

#define MAXBUF 8192

struct Serverinfo {
	struct sockaddr_in remote;
	struct sockaddr_in remote_data;
	socklen_t length_data;

	int p;
	char sentence[MAXBUF];

	int state_flag; // login state
	int binary_flag; // binary state
	int rename_flag; // rnfr state
	char rename_buf[MAXBUF];// rnfr buffer

	int ConnectType; // none:0 port:1 pasv:2

	char RootPlace[MAXBUF];	//root
	char WorkingPlace[MAXBUF]; // work

	int data_fd; // data -1 for no connect
	int data_port;
	int data_ip[4];
	char data_ip_str[100];

	int listen_fd; //client 
	char buf[MAXBUF]; //buffer send to client
	int listen_port;
	char listen_ip[100];

	int history_files;
	int history_bytes_succ;
	int history_files_succ;
};


extern void server_init(struct Serverinfo* info);

#endif // !STRUCTOFSERVER_H
