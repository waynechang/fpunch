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

int fpunch_process(int serverfd)
{
	char buf[512];
	session s;
	struct sockaddr_in client;
	socklen_t client_len;
	ssize_t n;
	size_t len;

	client_len = sizeof(client);
	n = recvfrom(serverfd, buf, BUF_LEN, 0,
		     (struct sockaddr *)&client,
		     &client_len);
	if (n == -1) {
		perror("recvfrom");
		return -1;
	}

	assert(SESSION_NAME_LEN < BUF_LEN);

	len = strnlen(buf, SESSION_NAME_LEN);
	if (len == SESSION_NAME_LEN) {
		buf[len] = '\0';
	}

	memset(&s, 0, sizeof(session));
	strcpy(s.name, buf);
	memcpy(&s.sa, &client, sizeof(struct sockaddr));
	s.state = STATE_LISTEN;

	session_add(&s);
	session_print(stdout);
	return 0;
}

int fpunch_select(int serverfd)
{
	int rv;
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(serverfd, &rfds);

	rv = select(serverfd + 1, &rfds, NULL, NULL, NULL);
	if (rv == -1) {
		perror("select");
		return -1;
	}

	rv = fpunch_process(serverfd);
	if (rv == -1) {
		fprintf(stderr, "fpunch_process() failed\n");
		return -1;
	}

	return 0;
}

int fpunch_listen(uint16_t port)
{
	int serverfd, rv;
	struct sockaddr_in server;
	socklen_t server_len;

	server_len = sizeof(server);
	memset(&server, 0, server_len);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	serverfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (serverfd == -1) {
		perror("socket");
		return -1;
	}

	rv = bind(serverfd, (struct sockaddr *)&server, server_len);
	if (rv == -1) {
		perror("bind");
		return -1;
	}

	for (;;) {
		rv = fpunch_select(serverfd);
		if (rv == -1) {
			fprintf(stderr, "fpunch_select() failed\n");
			return -1;
		}
	}

	rv = close(serverfd);
	if (rv == -1) {
		perror("close");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int rv;
	uint16_t port;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if ((port = atoi(argv[1])) <= 0) {
		fprintf(stderr, "%s: bad port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	rv = fpunch_listen(port);
	if (rv == -1) {
		fprintf(stderr, "fpunch_serve() failed\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}

