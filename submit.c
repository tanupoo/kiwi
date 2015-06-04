#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>	/* printf() */
#include <stdlib.h>	/* exit() */
#include <err.h>

#ifdef USE_KIWI_CODEC_JSON
#include <jansson.h>
#endif

#include "kiwi.h"

#ifdef USE_KIWI_CODEC_JSON
int
kiwi_json_parse_point(struct kiwi_ctx *kiwi, json_t *root, struct kiwi_chunk_key *p_point)
{
	json_t *v_value, *v_time;
	char *s_value, *s_time;

	if ((v_value = json_object_get(root, KIWI_JSON_KEY_VALUE)) == NULL) {
		warnx("%s: no value is defined.", __FUNCTION__);
		return -1;
	}
	if (!json_is_string(v_value)) {
		warnx("%s: string is required for value.", __FUNCTION__);
		return -1;
	}
	s_value = (char *)json_string_value(v_value);
#ifdef DEBUG_CODEC_JSON
	if (kiwi->kiwi_debug >= 3)
		printf("        %s\n", s_value);
#endif

	if ((v_time = json_object_get(root, KIWI_JSON_KEY_TIME)) == NULL) {
		warnx("%s: no time is defined.\n", __FUNCTION__);
		return -1;
	}
	if (!json_is_string(v_time)) {
		warnx("%s: string is required for time.", __FUNCTION__);
		return -1;
	}
	s_time = (char *)json_string_value(v_time);
#ifdef DEBUG_CODEC_JSON
	if (kiwi->kiwi_debug >= 3)
		printf("        %s\n", s_time);
#endif

	kiwi_chunk_key_add_value(p_point, (char *)s_value, (char *)s_time);

	return 0;
}

struct kiwi_chunk_key *
kiwi_json_parse_pointlist(struct kiwi_ctx *kiwi, json_t *root)
{
	json_t *pv, *v;
	void *iter;
	char *s_point;
	int psize, pvsize;
	int c, i;
	struct kiwi_chunk_key *head = NULL, *p_point;

	psize = json_object_size(root);
	if (psize > 10) {
		warnx("%s: too many objects are specified.", __FUNCTION__);
		return NULL;
	}

	c = 0;
	for (iter = json_object_iter(root); iter != NULL; iter = json_object_iter_next(root, iter)) {
		s_point = (char *)json_object_iter_key(iter);
#ifdef DEBUG_CODEC_JSON
		if (kiwi_debug >= 3)
			printf("    Point: %s\n", s_point);
#endif
		
		if ((pv = json_object_iter_value(iter)) == NULL) {
			warnx("%s: no value is defined for %s",
			    __FUNCTION__, s_point);
			return NULL;
		}
		pvsize = json_array_size(pv);
		if (pvsize > 50) {
			warnx("%s: too many values %d are defined.",
			    __FUNCTION__, pvsize);
			return NULL;
		}

		p_point = kiwi_chunk_add_key(&head, s_point);

		for (i = 0; i < pvsize; i++) {
			if ((v = json_array_get(pv, i)) == NULL) {
				warnx("%s: array is null, but still %d\n",
				    __FUNCTION__, i);
				if (head != NULL)
					kiwi_chunk_free(head);
				return NULL;
			}
			if (!json_is_object(v)) {
				warnx("%s: value is not a json object.\n",
				    __FUNCTION__);
				if (head != NULL)
					kiwi_chunk_free(head);
				return NULL;
			}
			if (kiwi_json_parse_point(kiwi, v, p_point) != 0) {
				if (head != NULL)
					kiwi_chunk_free(head);
				return NULL;
			}
		}

		c++;
	}
	if (c < psize) {
		warnx("%s: array size of point is conflict, psize=%d count=%d",
			__FUNCTION__, psize, c);
		return NULL;
	}

	return head;
}

struct kiwi_chunk_key *
kiwi_json_tochunk(struct kiwi_ctx *kiwi, json_t *root)
{
	json_t *k0, *v;
	const char *s;

	if (root == NULL)
		errx(1, "bad json_parse() call");

	if (json_object_size(root) != 1) {
		warnx("%s: invalid object size.\n", __FUNCTION__);
		return NULL;
	}
	if ((k0 = json_object_get(root, KIWI_JSON_KEY_KIWI)) == NULL) {
		warnx("%s: no kiwi object is found.\n", __FUNCTION__);
		return NULL;
	}
	if (json_object_size(k0) != 2) {
		warnx("%s: invalid object size of kiwi.\n", __FUNCTION__);
		return NULL;
	}

	/* check version */
	if ((v = json_object_get(k0, KIWI_JSON_KEY_VERSION)) == NULL) {
		warnx("%s: version is required.\n", __FUNCTION__);
		return NULL;
	}
	if (!json_is_string(v)) {
		warnx("%s: version is not a string.\n", __FUNCTION__);
		return NULL;
	}
	s = json_string_value(v);
	if (strcmp(KIWI_VERSION, s) != 0) {
		warnx("%s: unsupported version %s", __FUNCTION__, s);
		return NULL;
	}

	/* check point */
	if ((v = json_object_get(k0, KIWI_JSON_KEY_POINT)) == NULL) {
		warnx("%s: point is required.\n", __FUNCTION__);
		return NULL;
	}
	if (!json_is_object(v)) {
		warnx("%s: point is not a json object.\n", __FUNCTION__);
		return NULL;
	}

	return kiwi_json_parse_pointlist(kiwi, v);
}

int
kiwi_submit_file(struct kiwi_ctx *kiwi, char *filename)
{
	json_t *json;
	json_error_t error;
	struct kiwi_chunk_key *head;

	if ((json = json_load_file(filename, 0, &error)) == 0) {
		warnx("%s: failed to load %s, on line %d: %s",
		    __FUNCTION__, filename, error.line, error.text);
		return -1;
	}

	if ((head = kiwi_json_tochunk(kiwi, json)) == NULL)
		return -1;
	kiwi_chunk_dump(head);

	/*
	insert db
	submit to server
	*/

	return 0;
}
#endif

/*
 * submit data into the peer.
 * @note head will be free in the end of this function.
 */
int
kiwi_submit_peer(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head)
{
	struct kiwi_xbuf *xbuf;

	xbuf = kiwi_xbuf_new(4906);
	if (kiwi->encode) {
		kiwi->encode(kiwi, head, xbuf);
	}
	if (kiwi->transmit && xbuf != NULL) {
		kiwi->transmit(kiwi, xbuf);
	}
	kiwi_xbuf_free(xbuf);

	kiwi_chunk_free(head);

	return 0;
}

/**
 * submit data into the local database.
 * @note head will be free in the end of this function.
 */
int
kiwi_submit(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head)
{
	/* post the data into the local database if db_ctx is defined. */
	if (kiwi->db_ctx) {
		if (kiwi->db_ctx->db_type == KIWI_DBTYPE_SQLITE3 &&
		    kiwi->keymap == NULL) {
			warnx("%s: sqlite3 requires a keymap.",
			    __FUNCTION__);
		}
		kiwi_db_insert(kiwi, head);
	}

	kiwi_chunk_free(head);

	return 0;
}
