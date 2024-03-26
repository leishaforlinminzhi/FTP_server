#pragma once
#ifndef STATEHANDLE_H
#define STATEHANDLE_H

#include"StructofServer.h"

extern int handle_user(const char* msg, struct Serverinfo* info);

extern int handle_pass(const char* msg, struct Serverinfo* info);

extern int handle_type(const char* msg, struct Serverinfo* info);

extern int handle_syst(const char* msg, struct Serverinfo* info);

extern int handle_quit(const char* msg, struct Serverinfo* info);


#endif // !STATEHANDLE_H
