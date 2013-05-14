#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../session.h"

int main()
{
	session s;
	char *name = s.name;
	memset(&s, 0, sizeof(session));

	/* add */
	strcpy(name, "cat");
	session_add(&s);
	assert(session_get(&s) == 0);
	fprintf(stderr, "added cat\n");

	strcpy(name, "dog");
	session_add(&s);
	assert(session_get(&s) == 0);
	fprintf(stderr, "added dog\n");

	strcpy(name, "cheetah");
	session_add(&s);
	assert(session_get(&s) == 0);
	fprintf(stderr, "added cheetah\n");

	strcpy(name, "frog");
	session_add(&s);
	assert(session_get(&s) == 0);
	fprintf(stderr, "added frog\n");

	session_print(stdout);

	/* remove */
	strcpy(name, "cat");
	session_remove(&s);
	fprintf(stderr, "removed cat\n");
	assert(session_get(&s) == -1);

	strcpy(name, "dog");
	fprintf(stderr, "dog is still here\n");
	assert(session_get(&s) == 0);

	session_print(stdout);

	strcpy(name, "frog");
	session_remove(&s);
	fprintf(stderr, "removed frog\n");
	assert(session_get(&s) == -1);

	session_print(stdout);

	return 0;
}

