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

int process_new(char *buf, struct sockaddr_in *client)
{
        session s;
        size_t len;
	
        assert(SESSION_NAME_LEN < BUF_LEN);

        len = strnlen(buf, SESSION_NAME_LEN);
        if (len == SESSION_NAME_LEN) {
		fprintf(stderr, "process_new: bad data");
		return -1;
        }

        memset(&s, 0, sizeof(session));
        strcpy(s.name, buf);
        memcpy(&s.sa, &client, sizeof(struct sockaddr));
        s.state = STATE_ACK;

        session_add(&s);
        session_print(stdout);
	return 0;
}

int fpunch_process(int serverfd)
{
        char buf[BUF_LEN];
        struct sockaddr_in client;
        socklen_t client_len;
        ssize_t n;

        client_len = sizeof(client);
        n = recvfrom(serverfd, buf, BUF_LEN, 0,
                     (struct sockaddr *)&client,
                     &client_len);
        if (n == -1) {
                perror("recvfrom");
                return -1;
        }
        return 0;
}

