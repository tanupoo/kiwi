/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include <sqlite3.h>

#include "kiwi.h"
#include "if_sqlite3.h"

/*
 * sqlite3: db_purge
 */
static int
kiwi_sqlite3_purge(struct kiwi_ctx *kiwi, char *tabname, int seconds)
{
	static char *base = "DELETE FROM %s WHERE time < ?";
	struct sqlite3 *db = kiwi->db_ctx->db;
	char s_sql[KIWI_SQL_STRLEN];
	sqlite3_stmt *stmt;
	char s_time[KIWI_TIME_MAXLEN];
	int res;
	uint32_t current_time;

	current_time = (time(NULL) - seconds) * 1000;
	kiwi_get_strtime(s_time, sizeof(s_time), current_time);

	res = snprintf(s_sql, sizeof(s_sql), base, tabname);
	if (res >= sizeof(s_sql)) {
		warn("%s: buffer size for sql is too short.", __FUNCTION__);
		return -1;
	}

	if ((res = sqlite3_prepare_v2(db, s_sql, -1,
	    &stmt, NULL)) != SQLITE_OK) {
		warnx("%s: sqlite3_prepare_v2: %s",
		    __FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	if ((res = sqlite3_bind_text(stmt, 1, s_time, -1,
	    SQLITE_STATIC)) != SQLITE_OK) {
		warnx("%s: sqlite3_bind_text: %s",
		    __FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
		warnx("%s: sqlite3_step: %s",
		    __FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}
	if ((res = sqlite3_finalize(stmt)) != SQLITE_OK) {
		warnx("%s: sqlite3_finalize: %s",
		    __FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}

	return 0;
}

static int
kiwi_sqlite3_insert(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head)
{
	static char *base = "INSERT INTO %s (time, value) VALUES (?, ?)";
	struct sqlite3 *db = kiwi->db_ctx->db;
	char *tab_name;
	char s_sql[KIWI_SQL_STRLEN];
	int res;
	sqlite3_stmt *stmt;
	struct kiwi_chunk_key *p;
	struct kiwi_chunk_value *q;

	for (p = head; p != NULL; p = p->next) {
		/* p->key indicates the table name if keymap_len is 0. */
		if (!kiwi->keymap_len)
			tab_name = p->key;
		else {
			tab_name = kiwi_find_keymap_hash(kiwi, p->key);
			if (tab_name == NULL) {
				if (kiwi->debug) {
					warnx("%s: no hash in keymap for %s, "
					    "skip it.", __FUNCTION__, p->key);
				}
				continue;
			}
		}

		res = snprintf(s_sql, sizeof(s_sql), base, tab_name);
		if (res >= sizeof(s_sql)) {
			warn("buffer size for sql is too short.");
			return -1;
		}

		if ((res = sqlite3_prepare_v2(db, s_sql, -1,
		    &stmt, NULL)) != SQLITE_OK) {
			warnx("%s: sqlite3_prepare_v2: %s",
			    __FUNCTION__, sqlite3_errmsg(db));
			return -1;
		}
		for (q = p->value; q != NULL; q = q->next) {
			if ((res = sqlite3_reset(stmt)) != SQLITE_OK) {
				warnx("%s: sqlite3_reset: %s",
				    __FUNCTION__, sqlite3_errmsg(db));
				return -1;
			}
			if ((res = sqlite3_bind_text(stmt, 1, q->time, -1,
			    SQLITE_STATIC)) != SQLITE_OK) {
				warnx("%s: sqlite3_bind_text(1): %s",
				    __FUNCTION__, sqlite3_errmsg(db));
				return -1;
			}
			if ((res = sqlite3_bind_text(stmt, 2, q->value, -1,
			    SQLITE_STATIC)) != SQLITE_OK) {
				warnx("%s: sqlite3_bind_text(1): %s",
				    __FUNCTION__, sqlite3_errmsg(db));
				return -1;
			}
			if ((res = sqlite3_step(stmt)) != SQLITE_DONE) {
				warnx("%s: sqlite3_step: %s",
				    __FUNCTION__, sqlite3_errmsg(db));
				return -1;
			}
		}

		if ((res = sqlite3_finalize(stmt)) != SQLITE_OK) {
			warnx("%s: sqlite3_finalize: %s",
			    __FUNCTION__, sqlite3_errmsg(db));
			return -1;
		}

		/* purge the table if needed */
		if (kiwi->db_ctx->db_max_size != 0) {
			if (kiwi_sqlite3_purge(kiwi, tab_name,
			    kiwi->db_ctx->db_max_size) != 0)
				return -1;
		}
	}

	return 0;
}

/**
 * sqlite3: select the latest value in the tab_name.
 */
static int
kiwi_sqlite3_get_latest(struct kiwi_ctx *kiwi, const char *s_key, struct kiwi_xbuf *x_time, struct kiwi_xbuf *x_value)
{
        static char *base = "SELECT TIME, VALUE FROM %s ORDER BY TIME DESC LIMIT 1";
	struct sqlite3 *db = kiwi->db_ctx->db;
	const char *tab_name;
	char s_sql[KIWI_SQL_STRLEN];
	sqlite3_stmt *stmt = NULL;
	int res;
	int success = 0;

	/* the key indicates the table name if keymap_len is 0. */
	if (!kiwi->keymap_len)
		tab_name = s_key;
	else {
		tab_name = kiwi_find_keymap_hash(kiwi, s_key);
		if (tab_name == NULL) {
			if (kiwi->debug) {
				warnx("%s: no hash in keymap for %s.",
				    __FUNCTION__, s_key);
			}
			return -1;
		}
	}

	res = snprintf(s_sql, sizeof(s_sql), base, tab_name);
	if (res >= sizeof(s_sql)) {
		warn("buffer size for sql is too short.");
		return -1;
	}

	if ((res = sqlite3_prepare_v2(db, s_sql, -1,
	    &stmt, NULL)) != SQLITE_OK) {
		warnx("%s: sqlite3_prepare_v2: %s",
		    __FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}

	do {
		int len;
		char *v;

		/* result */
		res = sqlite3_step(stmt);
		if (res == SQLITE_DONE) {
			warnx("%s: sqlite3_step: no records", __FUNCTION__);
			success = -1;
			break;
		}
		else
		if (res != SQLITE_ROW) {
			warnx("%s: sqlite3_step: %s",
			    __FUNCTION__, sqlite3_errmsg(db));
			success = -1;
			break;
		}

		v = (char *)sqlite3_column_text(stmt, 0);
		len = kiwi_xbuf_memcpy(x_time, 0, v, strlen(v));
		if (len != strlen(v)) {
			warnx("%s: s_time len=%d is too small for the time.",
			    __FUNCTION__, len);
			success = -1;
			break;
		}
		v = (char *)sqlite3_column_text(stmt, 1);
		len = kiwi_xbuf_memcpy(x_value, 0, v, strlen(v));
		if (len != strlen(v)) {
			warnx("%s: s_value len=%d is too small for the value.",
			    __FUNCTION__, len);
			success = -1;
			break;
		}
	} while (0);

	if ((res = sqlite3_finalize(stmt)) != SQLITE_OK) {
		warnx("%s: sqlite3_finalize: %s", __FUNCTION__,
		    sqlite3_errmsg(db));
		return -1;
	}

	return success;
}

/**
 * sqlite3: select the latest value in the tab_name.
 */
static int
kiwi_sqlite3_get_latest(struct kiwi_ctx *kiwi, const char *s_key, const int limit, struct kiwi_chunk_key **head)
{
	return 0;
}

static int
kiwi_sqlite3_close(struct kiwi_ctx *kiwi)
{
	int res;
	struct sqlite3 *db = kiwi->db_ctx->db;

	if ((res = sqlite3_close(db)) != SQLITE_OK) {
		warnx("%s: sqlite3_close(): %s",
		    __FUNCTION__, sqlite3_errmsg(db));
		return -1;
	}

	return 0;
}

static int
kiwi_sqlite3_open(struct kiwi_ctx *kiwi)
{
	int res;
	struct sqlite3 *db;

	if ((res = sqlite3_open(kiwi->db_ctx->db_name, &db)) != SQLITE_OK) {
		warn("%s: sqlite3_open: %s",
		    __FUNCTION__, sqlite3_errmsg(kiwi->db_ctx->db));
		return -1;
	}

	kiwi->db_ctx->db = db;

	return 0;
}

int
kiwi_sqlite3_init(struct kiwi_ctx *kiwi)
{
	kiwi->db_ctx->db_open = kiwi_sqlite3_open;
	kiwi->db_ctx->db_close = kiwi_sqlite3_close;
	kiwi->db_ctx->db_insert = kiwi_sqlite3_insert;
	kiwi->db_ctx->db_purge = kiwi_sqlite3_purge;
	kiwi->db_ctx->db_get_latest = kiwi_sqlite3_get_latest;
	kiwi->db_ctx->db_get_limit = kiwi_sqlite3_get_limit;

	return 0;
}
