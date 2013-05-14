#ifndef _FPUNCHD_H
#define _FPUNCHD_H

#define CHANNEL_NAME_LEN 64
#define CHANNEL_MAX	 32

typedef struct channel {
	char name[CHANNEL_NAME_LEN];
	struct sockaddr_in sa;
	int state;
} channel;

#endif

