#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include "fpunchd.h"
#include "session.h"

#define BUF_LEN 512

#define reply_str(state, msg, serverfd, client) \
	reply(state, msg, strlen(msg) + 1, serverfd, client)
int reply(char state, const char *msg, size_t len,
	  int serverfd, struct sockaddr_in *client)
{
	char buf[BUF_LEN];
	ssize_t n;

	buf[0] = state;
	assert(len < BUF_LEN - 1);
	memcpy(&buf[1], msg, len);

	n = sendto(serverfd, buf, len, 0,
		   (struct sockaddr *)client, sizeof(struct sockaddr_in));
	if (n == -1) {
		perror("sendto");
		return -1;
	}

	return 0;
}

int process_new(char *buf, int serverfd, struct sockaddr_in *client)
{
        session s;
        size_t len;
	
        assert(SESSION_NAME_LEN < BUF_LEN - 1);

        len = strnlen(&buf[1], SESSION_NAME_LEN);
        if (len == SESSION_NAME_LEN || len == 0) {
		fprintf(stderr, "process_new: bad data");
		return -1;
        }

        memset(&s, 0, sizeof(session));
        strcpy(s.name, &buf[1]);
	if (session_get(&s) == 0) {
		reply_str(STATE_ERROR, "name exists", serverfd, client);
		return 0;
	}

        memcpy(&s.sa, client, sizeof(struct sockaddr));
        s.state = STATE_LISTEN;

        session_add(&s);
        session_print(stdout);

	reply_str(STATE_ACK, s.name, serverfd, client);

	return 0;
}

int process_send(char *buf, int serverfd, struct sockaddr_in *client)
{
	session s;
	size_t len;
	char send_buf[6];

        len = strnlen(&buf[1], SESSION_NAME_LEN);
        if (len == SESSION_NAME_LEN || len == 0) {
		fprintf(stderr, "process_send: bad data");
		return -1;
        }
        memset(&s, 0, sizeof(session));
        strcpy(s.name, &buf[1]);

	if (session_get(&s) == -1) {
		reply_str(STATE_ERROR, "name does not exist", serverfd, client);
		return 0;
	}

	memcpy(&send_buf[0], &s.sa.sin_addr.s_addr, 4);
	memcpy(&send_buf[4], &s.sa.sin_port, 2);

	reply(STATE_ACK, send_buf, 6, serverfd, client);

	return 0;
}

int fpunchd_process(int serverfd)
{
        char buf[BUF_LEN];
        struct sockaddr_in client;
        socklen_t client_len;
        ssize_t n;
	char state;
	int rv;

        client_len = sizeof(client);
        n = recvfrom(serverfd, buf, BUF_LEN, 0,
                     (struct sockaddr *)&client,
                     &client_len);
        if (n == -1) {
                perror("recvfrom");
                return -1;
        } else if (n == 0) {
		fprintf(stderr, "fpunch_process: no type byte");
		return -1;
	}

	state = buf[0];
	switch (state) {
	case STATE_SYN:
		rv = process_new(buf, serverfd, &client);
		if (rv == -1) {
			fprintf(stderr, "process_new() failed\n");
			return -1;
		}
		break;

	case STATE_SEND:
		rv = process_send(buf, serverfd, &client);
		if (rv == -1) {
			fprintf(stderr, "process_send() failed\n");
			return -1;
		}
		break;
	case STATE_ACK:
	case STATE_LISTEN:
	case STATE_TRANSFER:
	case STATE_FIN:
	case STATE_ERROR:
	default:
		fprintf(stderr, "fpunch_process: unknown state %d received\n",
			state);
		break;
	}

        return 0;

}

