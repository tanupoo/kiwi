/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#include <sys/types.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "kiwi.h"

/*
 * submit data into the peer.
 */
int
kiwi_submit_peer(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head)
{
	struct kiwi_xbuf *xbuf;

	if (!kiwi->encode) {
		warnx("ERROR: %s: encode is not specified.", __FUNCTION__);
		return -1;
	}

	if (!kiwi->transmit) {
		warnx("ERROR: %s: transmit is not specified.", __FUNCTION__);
		return -1;
	}

	if (!kiwi->encode_buffer_size) {
		warnx("ERROR: %s: buffer_size is zero.", __FUNCTION__);
		return -1;
	}
	
	xbuf = kiwi_xbuf_new(kiwi->encode_buffer_size);
	if (xbuf == 0)
		return -1;

	kiwi->encode(kiwi, head, xbuf);
	kiwi->transmit(kiwi, xbuf);

	return 0;
}

/**
 * submit data into the local database.
 */
int
kiwi_submit_ldb(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head)
{
	/* post the data into the local database if db_ctx is defined. */
	if (!kiwi->db_ctx) {
		warnx("WARNING: %s: db is not specified.", __FUNCTION__);
		return -1;
	}

	if (kiwi->db_ctx->db_type == KIWI_DBTYPE_SQLITE3 &&
		kiwi->keymap == NULL) {
		warnx("WARNING: %s: keymap is not specified for sqlite3.",
			__FUNCTION__);
	}
	kiwi_db_insert(kiwi, head);

	return 0;
}
