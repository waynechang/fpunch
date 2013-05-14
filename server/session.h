#ifndef _SESSION_H
#define _SESSION_H

#include <netinet/in.h>

#define SESSION_NAME_LEN 64

#define STATE_SYN	0
#define STATE_ACK	1
#define STATE_LISTEN	2
#define STATE_TRANSFER	3
#define STATE_FIN	4
#define STATE_SEND	5
#define STATE_ERROR	6

typedef struct session {
	char name[SESSION_NAME_LEN];
	struct sockaddr_in sa;
	struct timeval checkin;
	char state;

	struct session *next;
} session;


int session_add(const session *s);
int session_remove(const session *s);
int session_get(session *s);
int session_update(const session *s);

#endif

