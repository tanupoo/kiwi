#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#ifdef __linux__
#ifndef strlcpy
#define strlcpy(a, b, c) snprintf(a, c, "%s", b)
#endif
#endif

#include "kiwi.h"

struct x_ringbuf_data {
	char time[KIWI_TIME_MAXLEN];
	char value[KIWI_VALUE_MAXLEN];
};

/*
 * ringbuf: db_purge
 */
static int
kiwi_ringbuf_purge(struct kiwi_ctx *kiwi, char *tabname, int seconds)
{
	return 0;
}

static int
kiwi_ringbuf_insert(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head)
{
	struct ringbuf_holder *holder =
	    (struct ringbuf_holder *)kiwi->db_ctx->db;
	char *key;
	struct kiwi_chunk_key *p;
	struct kiwi_chunk_value *q;
	struct x_ringbuf_data data;

	for (p = head; p != NULL; p = p->next) {
		/* p->key indicates the table name if keymap_len is 0. */
		if (!kiwi->keymap_len)
			key = p->key;
		else {
			key = kiwi_find_keymap_hash(kiwi, p->key);
			if (key == NULL) {
				if (kiwi->debug) {
					warnx("%s: no hash in keymap for %s, "
					    "skip it.", __FUNCTION__, p->key);
				}
				continue;
			}
		}

		for (q = p->value; q != NULL; q = q->next) {
			strlcpy(data.time, q->time, sizeof(data.time));
			strlcpy(data.value, q->value, sizeof(data.value));
			ringbuf_add(holder, key, kiwi->db_ctx->db_max_size,
			    sizeof(struct x_ringbuf_data), &data);
		}
	}

	return 0;
}

static int
set_data_ctx(void *data, void *ctx)
{
	struct x_ringbuf_data *d = (struct x_ringbuf_data *)data;
	struct ringbuf_data_ctx *c = ctx;

	if (c->ptr - c->buf > c->buflen)
		warnx("WARN: c.ptr - c.buf > c.buflen");
	memcpy(c->ptr, d, c->data_size);
	c->ptr += c->data_size;
	c->num_data++;

	return c->data_size;
}

static int
kiwi_ringbuf_get_latest(struct kiwi_ctx *kiwi, const char *s_key, struct kiwi_xbuf *x_time, struct kiwi_xbuf *x_value)
{
	struct ringbuf_holder *holder =
	    (struct ringbuf_holder *)kiwi->db_ctx->db;
	char *key;
	struct x_ringbuf_data data;
	struct ringbuf_data_ctx ctx;
	int len;

	/* the key indicates the table name if keymap_len is 0. */
	if (!kiwi->keymap_len)
		key = (char *)s_key;
	else {
		key = kiwi_find_keymap_hash(kiwi, s_key);
		if (key == NULL) {
			if (kiwi->debug) {
				warnx("%s: no hash in keymap for %s.",
				    __FUNCTION__, s_key);
			}
			return -1;
		}
	}

	ctx.buf = (char *)&data;
	ctx.buflen = sizeof(struct x_ringbuf_data);
	ctx.ptr = (char *)&data;
	ctx.num_data = 0;
	ctx.data_size = sizeof(struct x_ringbuf_data);

	if ((len = ringbuf_get_item(holder, key, 1, set_data_ctx, &ctx)) == -1) {
		kiwi_xbuf_memcpy(x_time, 0, "0", 1);
		kiwi_xbuf_memcpy(x_value, 0, "0", 1);
		return 0;
	}

	len = kiwi_xbuf_memcpy(x_time, 0, data.time, strlen(data.time));
	if (len != strlen(data.time)) {
		warnx("%s: s_time len=%d is too small for the time.",
			__FUNCTION__, len);
		return -1;
	}
	len = kiwi_xbuf_memcpy(x_value, 0, data.value, strlen(data.value));
	if (len != strlen(data.value)) {
		warnx("%s: s_value len=%d is too small for the value.",
			__FUNCTION__, len);
		return -1;
	}

	return 0;
}

static int
kiwi_ringbuf_get_limit(struct kiwi_ctx *kiwi, const char *s_key, const int limit, struct kiwi_chunk_key **head)
{
	struct ringbuf_holder *holder =
	    (struct ringbuf_holder *)kiwi->db_ctx->db;
	char *key;
	struct x_ringbuf_data *data;
	struct ringbuf_data_ctx ctx;
	int len, i;

	/* the key indicates the table name if keymap_len is 0. */
	if (!kiwi->keymap_len)
		key = (char *)s_key;
	else {
		key = kiwi_find_keymap_hash(kiwi, s_key);
		if (key == NULL) {
			if (kiwi->debug) {
				warnx("%s: no hash in keymap for %s.",
				    __FUNCTION__, s_key);
			}
			return -1;
		}
	}

	if ((data = calloc(limit, sizeof(struct x_ringbuf_data))) == NULL) {
		err(1, "ERROR: %s: calloc(x_ringbuf_data)", __FUNCTION__);
		return -1;
	}

	ctx.buf = (char *)data;
	ctx.buflen = limit * sizeof(struct x_ringbuf_data);
	ctx.data_size = sizeof(struct x_ringbuf_data);
	ctx.ptr = (char *)data;
	ctx.num_data = 0;

	if ((len = ringbuf_get_item(holder, key, limit, set_data_ctx, &ctx)) == -1) {
		*head = NULL;
		return 0;
	}

	for (i = 0; i < ctx.num_data; i++) {
		kiwi_chunk_add(head, (char *)s_key, data[i].value, data[i].time);
	}

	return 0;
}

static int
kiwi_ringbuf_close(struct kiwi_ctx *kiwi)
{
	struct ringbuf_holder *holder =
	    (struct ringbuf_holder *)kiwi->db_ctx->db;

	ringbuf_destroy(holder);

	return 0;
}

static int
kiwi_ringbuf_open(struct kiwi_ctx *kiwi)
{
	int preserve = 1;

	kiwi->db_ctx->db = ringbuf_init(preserve, kiwi->debug);

	return 0;
}

int
kiwi_ringbuf_init(struct kiwi_ctx *kiwi)
{
	kiwi->db_ctx->db_open = kiwi_ringbuf_open;
	kiwi->db_ctx->db_close = kiwi_ringbuf_close;
	kiwi->db_ctx->db_insert = kiwi_ringbuf_insert;
	kiwi->db_ctx->db_purge = kiwi_ringbuf_purge;
	kiwi->db_ctx->db_get_latest = kiwi_ringbuf_get_latest;
	kiwi->db_ctx->db_get_limit = kiwi_ringbuf_get_limit;

	return 0;
}
