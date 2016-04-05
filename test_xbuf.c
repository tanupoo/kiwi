/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "kiwi.h"

int
test_xbuf_strcat()
{
	struct kiwi_xbuf *x;
	char *s1 = "01234";
	char *s2 = "56789";

	x = kiwi_xbuf_new(10);

	kiwi_xbuf_strcat(x, s1);
	kiwi_xbuf_strcat(x, s2);
	printf("%s\n", x->buf);

	kiwi_xbuf_free(x);

	return 0;
}

int
test_xbuf_vstrcat()
{
	struct kiwi_xbuf *x;
	char *s1 = "01%s";
	char *s2 = "56%s";

	x = kiwi_xbuf_new(10);

	kiwi_xbuf_vstrcat(x, s1, "234");
	kiwi_xbuf_vstrcat(x, s2, "789");
	printf("%s\n", x->buf);

	kiwi_xbuf_free(x);

	return 0;
}

int
test_xbuf_memcpy()
{
	struct kiwi_xbuf *x;
	char *s1 = "01234";
	char *s2 = "56789";

	x = kiwi_xbuf_new(10);

	kiwi_xbuf_memcpy(x, 0, s1, strlen(s1));
	kiwi_xbuf_memcpy(x, 3, s2, strlen(s2));
	printf("%s\n", x->buf);

	kiwi_xbuf_free(x);

	return 0;
}

int
main(int argc, char *argv[])
{
	test_xbuf_strcat();
	test_xbuf_vstrcat();
	test_xbuf_memcpy();

	return 0;
}

