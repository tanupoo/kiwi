/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <err.h>

#include "kiwi.h"

int f_debug = 0;

char *prog_name = NULL;

void
usage()
{
	printf(
"Usage: %s [-dh]\n"
	, prog_name);

	exit(0);
}

int
test_kiwi_get_strtime()
{
	char buf[512];
	char *result;
#ifdef KIWI_TIME_WITH_MS
	char *expected = "2016-03-19T11:54:21.987+0900";
#else
	char *expected = "2016-03-19T11:54:21+0900";
#endif

	result = kiwi_get_strtime(buf, sizeof(buf), 1458356061987);
	if (strcmp(expected, result)) {
		warnx("ERROR: mismatch. result=[%s] expected=[%s]",
		    result, expected);
		return -1;
	}
	return 0;
}

int
test_kiwi_get_strunixtime()
{
	return 0;
}

int
run()
{
	char buf[512];

	printf("current time: [%s]\n", kiwi_get_strtime(buf, sizeof(buf), 0));
	test_kiwi_get_strtime();

	printf("current time: [%s]\n", kiwi_get_strunixtime(buf, sizeof(buf)));
	test_kiwi_get_strunixtime();

	return 0;
}

int
main(int argc, char *argv[])
{
	int ch;

	prog_name = 1 + rindex(argv[0], '/');

	while ((ch = getopt(argc, argv, "dh")) != -1) {
		switch (ch) {
		case 'd':
			f_debug++;
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		usage();

	run();

	return 0;
}

