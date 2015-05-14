#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <err.h>

#include "kiwi.h"

#ifdef __linux__
#ifndef strlcpy
#define strlcpy(a, b, c) snprintf(a, c, "%s", b)
#endif
#endif

/* xbuf->buf is always terminated by '\0'. */

void
kiwi_xbuf_free(struct kiwi_xbuf *xbuf)
{
	free(xbuf->buf);
	free(xbuf);
}

struct kiwi_xbuf *
kiwi_xbuf_new(int buflen)
{
	struct kiwi_xbuf *new;

	if ((new = calloc(1, sizeof(*new))) == NULL)
		err(1, "calloc(xbuf)");

	new->buflen = buflen;
	if ((new->buf = calloc(1, new->buflen)) == NULL)
		err(1, "calloc(xbuf->buflen)");
	new->offset = 0;

	return new;
}

size_t
kiwi_xbuf_strcat(struct kiwi_xbuf *xbuf, const char *str)
{
	size_t len;

	xbuf->buf[xbuf->offset] = '\0';

	len = strlcpy(xbuf->buf + xbuf->offset, str,
	    xbuf->buflen - xbuf->offset);

	xbuf->offset += len;

	return len;
}

/*
 * copy data as a string into the xbuf->buf pointed by xbuf->offset.
 */
size_t
kiwi_xbuf_vstrcat(struct kiwi_xbuf *xbuf, const char *fmt, ...)
{
	va_list ap;
	int len;

	if (xbuf == NULL) {
		len = 0;
		va_start(ap, fmt);
		len += vsnprintf(NULL, 0, fmt, ap);
		va_end(ap);
		return len;
	}

	xbuf->buf[xbuf->offset] = '\0';

	len = 0;
	va_start(ap, fmt);
	len += vsnprintf(xbuf->buf + xbuf->offset,
	    xbuf->buflen - xbuf->offset, fmt, ap);
	va_end(ap);

	xbuf->offset += len;

	return len;
}

/*
 * copy data into the xbuf->buf pointed by "offset", not xbuf->offset.
 */
size_t
kiwi_xbuf_memcpy(struct kiwi_xbuf *xbuf, size_t offset, const char *data, size_t size)
{
	if (size + offset >= xbuf->buflen)
		size = xbuf->buflen - offset - 1;

	memcpy(&xbuf->buf[offset], data, size);
	xbuf->buf[offset + size] = '\0';

	xbuf->offset = offset + size;

	return size;
}
