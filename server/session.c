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

#include "session.h"

static session *sessions_head = NULL;

int session_add(const session *s)
{
	session *ns, *parse;
	ns = malloc(sizeof(session));
	if (ns == NULL) {
		perror("malloc");
		return -1;
	}
	memcpy(ns, s, sizeof(session));
	ns->next = NULL;

	if (sessions_head == NULL) {
		sessions_head = ns;
	} else {
		for (parse = sessions_head;
		     parse->next != NULL;
		     parse = parse->next)
			;
		parse->next = ns;
	}
	return 0;
}

int session_remove(const session *s)
{
	session *parse, *prev = NULL;
	for (parse = sessions_head;
	     parse != NULL;
	     parse = parse->next) {
		if (strcmp(s->name, parse->name) == 0) {
			if (prev == NULL) {
				sessions_head = parse->next;
			} else {
				prev->next = parse->next;
			}
			free(parse);
			return 0;
		}

		prev = parse;
	}

	fprintf(stderr, "session_remove: session %s not found\n", s->name);
	return -1;
}

int session_get(session *s)
{
	session *p;
	for (p = sessions_head; p != NULL; p = p->next) {
		if (strcmp(s->name, p->name) == 0) {
			memcpy(s, p, sizeof(session));
			return 0;
		}
	}

	fprintf(stderr, "session_get: session %s not found\n", s->name);
	return -1;
}

int session_update(const session *s)
{
	session *p;
	for (p = sessions_head; p != NULL; p = p->next) {
		if (strcmp(p->name, s->name) == 0) {
			memcpy(p, s, sizeof(session));
			return 0;
		}
	}

	fprintf(stderr, "session_update: session %s not found\n", s->name);
	return -1;
}

int session_print(FILE *fp)
{
	session *p;
	int i;
	for (i = 0, p = sessions_head; p != NULL; p = p->next, i++) {
		fprintf(fp, "%02d | %32s | %15s:%08d | %d\n",
			i,
			p->name,
			inet_ntoa(p->sa.sin_addr),
			ntohs(p->sa.sin_port),
			p->state);
	}

	return 0;
}

