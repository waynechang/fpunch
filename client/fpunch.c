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
#include "fpunch.h"
#include "session.h"

#define BUF_LEN 512


int fpunch_waitread(int sockfd)
{
	fd_set rfds;
	struct timeval tv;
	int rv;

	FD_ZERO(&rfds);
	FD_SET(sockfd, &rfds);

	tv.tv_sec = 5;
	tv.tv_usec = 0;

	rv = select(sockfd + 1, &rfds, NULL, NULL, &tv);
	if (rv == -1) {
		perror("select");
		return -1;
	}

	return rv;
}

int bindr3(int sockfd, struct sockaddr_in *addr)
{
	int rv, tries = 0;
	uint16_t port;

	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = htonl(INADDR_ANY);
	do {
		port = 0xC000 + rand() % 0x3FFF;
		addr->sin_port = htons(port);
		rv = bind(sockfd, (struct sockaddr *) addr,
			  sizeof(struct sockaddr_in));
		if (rv == -1) {
			tries++;
			continue;
		}
		break;
	} while (tries < 3);

	return rv;
}

int load_server(struct sockaddr_in *addr)
{
	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = inet_addr("50.22.61.235");
	addr->sin_port = htons(6699);

	return 0;
}

int load_peer(const char *buf, struct sockaddr_in *addr)
{
	if (buf[0] != STATE_ACK) {
		fprintf(stderr, "load_peer(): bad server data\n");
		return -1;
	}
	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	memcpy(&addr->sin_addr.s_addr, &buf[1], 4);
	memcpy(&addr->sin_port, &buf[5], 2);

	return 0;
}

int fpunch_transfer(const char *key, const char *file)
{
	char buf[BUF_LEN];
	int sockfd, rv;
	struct sockaddr_in client, server, inbound, peer;
	socklen_t inbound_len;
	ssize_t n;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		perror("socket");
		return -1;
	}

	rv = bindr3(sockfd, &client);
	if (rv == -1) {
		fprintf(stderr, "bindr3 failed()\n");
		return -1;
	}

	rv = load_server(&server);
	if (rv == -1) {
		fprintf(stderr, "load_server() failed\n");
		return -1;
	}

	assert(strlen(key) < BUF_LEN - 1);
	for (;;) {
		buf[0] = STATE_SEND;
		strcpy(&buf[1], key);

		n = sendto(sockfd, buf, 1 + strlen(key) + 1, 0,
			   (struct sockaddr *)&server,
			   sizeof(struct sockaddr_in));
		if (n == -1) {
			perror("sendto");
			continue;
		}

		rv = fpunch_waitread(sockfd);
		if (rv == -1) {
			fprintf(stderr, "fpunch_waitread() failed\n");
			continue;
		} else if (rv == 0) {
			fprintf(stderr, "fpunch_waitread() timed out\n");
			continue;
		}

		inbound_len = sizeof(inbound);
		n = recvfrom(sockfd, buf, BUF_LEN, 0,
			     (struct sockaddr *)&inbound, &inbound_len);
		if (n == -1) {
			perror("recvfrom");
			continue;
		}
		fprintf(stderr, "received %d bytes\n", (int) n);
		assert(n >= 7);

		rv = load_peer(buf, &peer);
		if (rv == -1) {
			fprintf(stderr, "load_peer() failed\n");
			continue;
		}
		fprintf(stderr, "loaded address: %s:%d\n",
			inet_ntoa(peer.sin_addr),
			ntohs(peer.sin_port));
		break;
	}

	rv = close(sockfd);
	if (rv == -1) {
		perror("close");
		return -1;
	}

	return 0;
}

void print_usage(const char *prog)
{
	fprintf(stderr, "Usage: %s listen <key>\n"
			"   or: %s transfer <key> <file>\n",
		prog, prog);
}

int main(int argc, char **argv)
{
	int rv;
	uint16_t port;

	fprintf(stderr, "argc = %d\n", argc);
	if (argc < 3) {
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[1], "listen") == 0) {
		if (argc < 3) {
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "Listening!\n");

		/*
		rv = fpunch_listen(argv[2]);
		if (rv == -1) {
			fprintf(stderr, "fpunch_listen() failed\n");
			exit(EXIT_FAILURE);
		}
		*/
	} else if (strcmp(argv[1], "transfer") == 0) {
		if (argc < 4) {
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
		fprintf(stderr, "Transfering!\n");
		
		rv = fpunch_transfer(argv[2], argv[3]);
		if (rv == -1) {
			fprintf(stderr, "fpunch_transfer() failed\n");
			exit(EXIT_FAILURE);
		}
	} else {
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	return 0;
}

