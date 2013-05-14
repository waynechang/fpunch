#ifndef _SESSION_H
#define _SESSION_H

#include <netinet/in.h>

#define SESSION_NAME_LEN 64
#define SESSION_MAX      32

#define STATE_ACK	 0
#define STATE_LISTEN	 1
#define STATE_TRANSFER	 2

typedef struct session {
	char name[SESSION_NAME_LEN];
	struct sockaddr_in sa;
	struct timeval checkin;
	int state;

	struct session *next;
} session;


int session_add(const session *s);
int session_remove(const session *s);
int session_get(session *s);
int session_update(const session *s);

#endif

