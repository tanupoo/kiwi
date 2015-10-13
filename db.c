#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#ifdef USE_KIWI_DB_SQLITE3
#include "kiwi_sqlite3.h"
#endif
#ifdef USE_KIWI_DB_MONGODB
#include "kiwi_mongodb.h"
#endif

#include "kiwi.h"

/*
 XXX check whether there is ';' in the tab_name.  that causes sql injection.
 */

/*
 * insert data.
 * if db_max_size is defined (!= 0), then it will call db_purse().
 */
int
kiwi_db_insert(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head)
{
	return kiwi->db_ctx->db_insert(kiwi, head);
}

/*
 * delete old data.
 *
 * @param remain data in seconds.
 */
int
kiwi_db_purge(struct kiwi_ctx *kiwi, char *tab_name, int seconds)
{
	return kiwi->db_ctx->db_purge(kiwi, tab_name, seconds);
}

/*
 * get the latest value.
 */
int
kiwi_db_get_latest(struct kiwi_ctx *kiwi, const char *tab_name, struct kiwi_xbuf *x_time, struct kiwi_xbuf *x_value)
{
	return kiwi->db_ctx->db_get_latest(kiwi, tab_name, x_time, x_value);
}

/*
 * get the value of the number of limit.
 */
int
kiwi_db_get_limit(struct kiwi_ctx *kiwi, const char *tab_name, const int limit, struct kiwi_chunk_key **head)
{
	return kiwi->db_ctx->db_get_limit(kiwi, tab_name, limit, head);
}

int
kiwi_db_close(struct kiwi_ctx *kiwi)
{
	return kiwi->db_ctx->db_close(kiwi);
}

int
kiwi_set_db(struct kiwi_ctx *kiwi, int db_type, char *db_name, int max_size)
{
	if ((kiwi->db_ctx = calloc(1, sizeof(*kiwi->db_ctx))) == NULL)
		err(1, "calloc(kiwi_db_ctx)");

	kiwi->db_ctx->db_type = db_type;
	if (db_name == NULL)
		db_name = "none";
	kiwi->db_ctx->db_name = strdup(db_name);
	kiwi->db_ctx->db_max_size = max_size;

	switch (db_type) {
	case KIWI_DBTYPE_RINGBUF:
		kiwi_ringbuf_init(kiwi);
		break;
#ifdef USE_KIWI_DB_SQLITE3
	case KIWI_DBTYPE_SQLITE3:
		kiwi_sqlite3_init(kiwi);
		break;
#endif
#ifdef USE_KIWI_DB_MONGODB
	case KIWI_DBTYPE_MONGODB:
		errx(1, "db_type %d has not been implemented yet", db_type);
#endif
	default:
		errx(1, "unsupported db_type %d", db_type);
	}

	return kiwi->db_ctx->db_open(kiwi);
}
