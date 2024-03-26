CC = gcc
CFLAGS = -Wall

# 定义 server 目标文件
SERVER_OBJ = server.o RequestHandle.o FileHandle.o StateHandle.o SocketHandle.o StructofServer.o

# 定义 client 目标文件
CLIENT_OBJ = client.o

# 编译生成 server 可执行文件
server: $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o server $(SERVER_OBJ)

# 编译生成 client 可执行文件
client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o client $(CLIENT_OBJ)

# 编译 server.c 生成 server.o 目标文件
server.o: server.c RequestHandle.h FileHandle.h StateHandle.h SocketHandle.h StructofServer.h
	$(CC) $(CFLAGS) -c server.c

# 编译 RequestHandle.c 生成 RequestHandle.o 目标文件
RequestHandle.o: RequestHandle.c RequestHandle.h FileHandle.h StateHandle.h SocketHandle.h StructofServer.h
	$(CC) $(CFLAGS) -c RequestHandle.c

# 编译 FileHandle.c 生成 FileHandle.o 目标文件
FileHandle.o: FileHandle.c FileHandle.h StructofServer.h StateHandle.h
	$(CC) $(CFLAGS) -c FileHandle.c

# 编译 StateHandle.c 生成 StateHandle.o 目标文件
StateHandle.o: StateHandle.c StateHandle.h StructofServer.h
	$(CC) $(CFLAGS) -c StateHandle.c

# 编译 SocketHandle.c 生成 SocketHandle.o 目标文件
SocketHandle.o: SocketHandle.c SocketHandle.h StructofServer.h
	$(CC) $(CFLAGS) -c SocketHandle.c

# 编译 StructofServer.c 生成 StructofServer.o 目标文件
StructofServer.o: StructofServer.c StructofServer.h
	$(CC) $(CFLAGS) -c StructofServer.c

# 编译 client.c 生成 client.o 目标文件
client.o: client.c
	$(CC) $(CFLAGS) -c client.c

# 清理编译生成的文件
clean:
	rm -f server client *.o
