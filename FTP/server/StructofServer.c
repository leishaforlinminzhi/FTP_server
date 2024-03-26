#include"StructofServer.h"

void server_init(struct Serverinfo* info) {
	info->p = 0;

	memset(&info->remote, 0, sizeof(info->remote));

	info->state_flag = 0;
	info->binary_flag = 0;
	info->rename_flag = 0;

	info->listen_fd = -1;
	info->data_fd = -1;

	info->listen_ip[0] = 127;
	info->listen_ip[1] = 0;
	info->listen_ip[2] = 0;
	info->listen_ip[3] = 1;
	info->listen_port = 10000;

	info->data_ip[0] = 127;
	info->data_ip[1] = 0;
	info->data_ip[2] = 0;
	info->data_ip[3] = 1;
	info->data_port = 20001;

	strcpy(info->WorkingPlace, "/tmp");
	strcpy(info->RootPlace, info->WorkingPlace);

	info->history_files = 0;
	info->history_bytes_succ = 0;
	info->history_files_succ = 0;
	return;
}

