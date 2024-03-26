#pragma once
#ifndef SOCKETHANDLE_H
#define SOCKETHANDLE_H

#include"StructofServer.h"

extern int handle_port(const char* msg, struct Serverinfo* info);

extern int handle_pasv(const char* msg, struct Serverinfo* info);

#endif // !PLACEHANDLE_H
