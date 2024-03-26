#pragma once
#ifndef FILEHANDLE_H
#define FILEHANDLE_H

#include"StructofServer.h"

extern int handle_retr(const char* msg, struct Serverinfo* info);

extern int handle_stor(const char* msg, struct Serverinfo* info);

extern int handle_pwd(const char* msg, struct Serverinfo* info);

extern int handle_mkd(const char* msg, struct Serverinfo* info);

extern int handle_cwd(const char* msg, struct Serverinfo* info);

extern int handle_list(const char* msg, struct Serverinfo* info);

extern int handle_rmd(const char* msg, struct Serverinfo* info);

extern int handle_rnfr(const char* msg, struct Serverinfo* info);

extern int handle_rnto(const char* msg, struct Serverinfo* info);


#endif // !FILEHANDLE_H
