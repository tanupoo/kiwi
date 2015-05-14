#ifdef __linux__
#define _XOPEN_SOURCE
#define _BSD_SOURCE
#endif

#include <sys/types.h>
#include <time.h>
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

#define KIWI_DATETIME_FORMAT "%FT%T"

/* supported time format */
static char *format[] =
{
	"%FT%T%z",
	"%FT%T",
	"%Y-%m-%dT%H:%M:%S%z",
	"%Y-%m-%dT%H:%M:%S",
	"%Y%m%dT%H%M%S%z",
	"%Y%m%dT%H%M%S",
	NULL,
};

/*
 * e.g. 2014-10-17T07:55:00+0900
 */
char *
kiwi_get_time(char *buf, int buflen, time_t specific_time)
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
	if (ret == 0)
		errx(1, "the buffer for strftime() is too short.");

	return buf;
}

char *
kiwi_time_canon(char *src, char *dst, int dstlen)
{
	char **fp;
	char *p;
	struct tm tm;
	char buf[32];
	for (fp = format; *fp != NULL; fp++) {
		memset(&tm, 0, sizeof(tm));
		p = strptime(src, *fp, &tm);
		if (p == NULL)
			continue;
		strftime(buf, sizeof(buf), KIWI_DATETIME_FORMAT, &tm);
		/* XXX do check the length */
	}
	if (p == NULL) {
		warnx("unsupported datetime format [%s]", src);
		return NULL;
	}

	strlcpy(dst, buf, dstlen);

	return dst;
}

/*
 * key : actual key string
 * hash: hashed string
 */

void
kiwi_chunk_dump_value(struct kiwi_chunk_value *vhead)
{
	struct kiwi_chunk_value *q;

	if (vhead == NULL) {
		printf("no key in the chunk_value.\n");
		return;
	}

	printf(" %-24s value\n", "timestamp");
	for (q = vhead; q != NULL; q = q->next) {
		printf("%25s %s\n", q->time, q->value);
	}
}

void
kiwi_chunk_dump_key_list(struct kiwi_chunk_key *khead)
{
	struct kiwi_chunk_key *p;

	if (khead == NULL) {
		printf("no key in the chunk_key.\n");
		return;
	}

	for (p = khead; p != NULL; p = p->next) {
		printf("key = [%s]\n", p->key);
	}
}

void
kiwi_chunk_dump(struct kiwi_chunk_key *khead)
{
	struct kiwi_chunk_key *p;

	if (khead == NULL) {
		printf("no key in the chunk_key.\n");
		return;
	}

	for (p = khead; p != NULL; p = p->next) {
		printf("key = [%s]\n", p->key);
		kiwi_chunk_dump_value(p->value);
	}
}

void
kiwi_chunk_free_value(struct kiwi_chunk_value *vhead)
{
	struct kiwi_chunk_value *q, *next_value;

	if (vhead == NULL)
		errx(1, "chunk_value is null.");

	for (q = vhead; q != NULL; q = next_value) {
		next_value = q->next;
		free(q->time);
		free(q->value);
		free(q);
	}
}

void
kiwi_chunk_free(struct kiwi_chunk_key *khead)
{
	struct kiwi_chunk_key *p, *next_key;

	if (khead == NULL)
		errx(1, "chunk_key is null.");

	for (p = khead; p != NULL; p = next_key) {
		next_key = p->next;
		if (p->value != NULL)
			kiwi_chunk_free_value(p->value);
		free(p->key);
		free(p);
	}
}

/*
 * allocate a chunk_value and add it into the end of chain.
 *
 * @param time time string. if null, it's taken by kiwi_get_time().
 * @return the pointer of the chunk_value allocated.
 */
struct kiwi_chunk_value *
kiwi_chunk_add_value(struct kiwi_chunk_value **kvead, char *value, char *time)
{
	struct kiwi_chunk_value *new, *p;
	char s_time[80];

	if ((new = calloc(1, sizeof(struct kiwi_chunk_value))) == NULL)
		err(1, "calloc()");
	if ((new->value = strdup(value)) == NULL)
		err(1, "strdup()");
	if (time == NULL)
		time = kiwi_get_time(s_time, sizeof(s_time), 0);
	if ((new->time = strdup(time)) == NULL)
		err(1, "strdup()");

	if (*kvead == NULL)
		*kvead = new;
	else {
		/* skip it */
		for (p = *kvead; p->next != NULL; p = p->next)
			;
		p->next = new;
	}

	return new;
}

/*
 * allocate a chunk_value and add it into the end of chain in the chunk_key.
 *
 * @return the pointer of the chunk_value allocated.
 */
struct kiwi_chunk_value *
kiwi_chunk_key_add_value(struct kiwi_chunk_key *key, char *value, char *time)
{
	return kiwi_chunk_add_value(&(key->value), value, time);
}

struct kiwi_chunk_key *
kiwi_chunk_find_key(struct kiwi_chunk_key *khead, char *key)
{
	struct kiwi_chunk_key *p;

	if (khead == NULL)
		return NULL;

	for (p = khead; p != NULL; p = p->next) {
		if (strcmp(p->key, key) == 0)
			return p;
	}

	return NULL;
}

/*
 * find the key in the chain.  return the pointer to it if found.
 * if not, allocate a chunk_key and add it into the end of chain.
 *
 * @return the pointer of the chunk_key allocated.
 */
struct kiwi_chunk_key *
kiwi_chunk_add_key(struct kiwi_chunk_key **khead, char *key)
{
	struct kiwi_chunk_key *new, *p;

	if ((p = kiwi_chunk_find_key(*khead, key)) != NULL)
		return p;

	if ((new = calloc(1, sizeof(struct kiwi_chunk_key))) == NULL)
		err(1, "calloc()");
	if ((new->key = strdup(key)) == NULL)
		err(1, "strdup()");

	if (*khead == NULL)
		*khead = new;
	else {
		/* skip it */
		for (p = *khead; p->next != NULL; p = p->next)
			;
		p->next = new;
	}

	return new;
}

/*
 * find the key in the chain.  return the pointer to it if found.
 * if not, allocate a chunk_key and add it into the end of chain.
 * and add the value into the end of the key chain.
 *
 * @return the pointer of the chunk_key allocated.
 */
struct kiwi_chunk_key *
kiwi_chunk_add(struct kiwi_chunk_key **head, char *key, char *value, char *time)
{
	struct kiwi_chunk_key *p_key;

	p_key = kiwi_chunk_add_key(head, key);
	kiwi_chunk_key_add_value(p_key, value, time);

	return p_key;
}
