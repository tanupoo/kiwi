/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#ifdef __linux__
#define _XOPEN_SOURCE
#define _BSD_SOURCE
#endif

#include <sys/types.h>
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
		errx(1, "ERROR: %s: no key in the chunk_key.", __FUNCTION__);
		return;
	}

	for (p = khead; p != NULL; p = p->next) {
		kiwi_chunk_dump_value(p->value);
		errx(1, "ERROR: %s: key = [%s]\n", __FUNCTION__, p->key);
	}
}

void
kiwi_chunk_free_value(struct kiwi_chunk_value *vhead)
{
	struct kiwi_chunk_value *q, *next_value;

	if (vhead == NULL)
		errx(1, "ERROR: %s: chunk_value is null.", __FUNCTION__);

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
		errx(1, "ERROR: %s: chunk_key is null.", __FUNCTION__);

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
 * @param time time string. if null, it's taken by calling kiwi_get_strtime().
 * @return the pointer of the chunk_value allocated.
 */
struct kiwi_chunk_value *
kiwi_chunk_add_value(struct kiwi_chunk_value **vhead, char *value, char *time)
{
	struct kiwi_chunk_value *new, *p;
	char s_time[KIWI_TIME_MAXLEN];

	if ((new = calloc(1, sizeof(struct kiwi_chunk_value))) == NULL)
		err(1, "ERROR: %s: calloc()", __FUNCTION__);
	if (value == NULL)
		errx(1, "ERROR: %s: value must be specified.", __FUNCTION__);
	if ((new->value = strdup(value)) == NULL)
		err(1, "ERROR: %s: strdup()", __FUNCTION__);
	if (time == NULL)
		time = kiwi_get_strtime(s_time, sizeof(s_time), 0);
	if ((new->time = strdup(time)) == NULL)
		err(1, "ERROR: %s: strdup()", __FUNCTION__);

	if (*vhead == NULL)
		*vhead = new;
	else {
		/* skip it */
		for (p = *vhead; p->next != NULL; p = p->next)
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
		err(1, "ERROR: %s: calloc()", __FUNCTION__);
	if (key == NULL)
		errx(1, "ERROR: %s: key must be specified.", __FUNCTION__);
	if ((new->key = strdup(key)) == NULL)
		err(1, "ERROR: %s: strdup()", __FUNCTION__);

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
