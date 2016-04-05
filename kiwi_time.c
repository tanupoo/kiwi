/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#ifdef __linux__
#define _XOPEN_SOURCE
#define _BSD_SOURCE
#endif

#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <err.h>

#ifdef __linux__
#ifndef strlcpy
#define strlcpy(a, b, c) snprintf(a, c, "%s", b)
#endif
#endif

#include "kiwi.h"

#ifdef KIWI_TIME_WITH_MS
/*
 * make a string formed ISO 8601 style with a milisecond.
 * timezone depends on the system configuration.
 * @param buf buffer into which the result of a time string is set.
 * @param specific_time number in msec. if it's zero, it takes current time.
 */
char *
kiwi_get_strtime(char *buf, int buflen, uint64_t specific_time)
{
	struct timeval tv;
	struct tm tm;
	long msec;
	int offset;
	int ret;

	msec = 0;
	if (specific_time) {
		tv.tv_sec = specific_time / 1000;
		msec = specific_time % 1000;
	} else {
		/* get local time in microseconds */
		gettimeofday(&tv, NULL);
		msec = tv.tv_usec / 1000;
	}
	localtime_r(&tv.tv_sec, &tm);

	offset = 0;
	ret = strftime(buf, buflen, "%Y-%m-%dT%H:%M:%S", &tm);
	if (ret == 0)
		errx(1, "ERROR: %s: buffer t is too short.", __FUNCTION__);

	offset += ret;
	ret = snprintf(buf + offset, buflen - offset, ".%03ld", msec);
	if (ret == 0)
		errx(1, "ERROR: %s: buffer f is too short.", __FUNCTION__);

	offset += ret;
	ret = strftime(buf + offset, buflen - offset, "%z", &tm);
	if (ret == 0)
		errx(1, "ERROR: %s: buffer z is too short.", __FUNCTION__);

	return buf;
}
#else /* ! KIWI_TIME_WITH_MS */
/*
 * e.g. 2014-10-17T07:55:00+0900
 */
char *
kiwi_get_strtime(char *buf, int buflen, uint64_t specific_time)
{
	time_t t;
	struct tm tm;
	int ret;

	buf[0] = '\0';

	t = specific_time;
	if (t == 0) {
		t = time(NULL);
	}

	localtime_r(&t, &tm);

	ret = strftime(buf, buflen, "%Y-%m-%dT%H:%M:%S%z", &tm);
	if (ret == 0) {
		errx(1, "ERROR: %s: the buffer for strftime() is too short.",
		    __FUNCTION__);
	}

	return buf;
}
#endif

/*
 * put a unix time in miliseconds into the buffer specified.
 * the timezone is converted into GMT in according to the system configuration..
 */
char *
kiwi_get_strunixtime(char *buf, int buflen)
{
	struct timeval tp;
	struct timezone tzp;
	uint64_t msec;
	int res;

	if (gettimeofday(&tp, &tzp))
		err(1, "ERROR: %s: ", __FUNCTION__);

	tp.tv_sec += tzp.tz_minuteswest * 60;
	msec = tp.tv_sec * 1000 + tp.tv_usec / 1000;
	res = snprintf(buf, buflen, "%lld", msec);
	if (res == 0)
		errx(1, "ERROR: %s: buf is too short.", __FUNCTION__);

	return buf;
}
