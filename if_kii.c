#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <err.h>

#include "kiwi.h"

int
if_kii_init(struct kiwi_ctx *kiwi)
{
	char buf[2048];
	struct if_kii_param_t *param;

	if (!kiwi->client_param) {
		errx(1, "ERROR: %s: client_param is not defined.\n",
		    __FUNCTION__);
	}
	param = (struct if_kii_param_t *)kiwi->client_param;
	snprintf(buf, sizeof(buf), "X-Kii-AppID: %s", param->app_id);
	param->h_app_id = strdup(buf);
	snprintf(buf, sizeof(buf), "X-Kii-AppKey: %s", param->app_key);
	param->h_app_key = strdup(buf);
	snprintf(buf, sizeof(buf), "Authorization: Bearer %s",
	    param->thing_token);
	param->h_thing_token = strdup(buf);
	snprintf(buf, sizeof(buf),
	    "Content-Type: application/vnd.%s.mydata+json", param->app_id);
	param->h_ctype = strdup(buf);

	/* e.g. server_url in KII must be like "https://api-jp.kii.com" */
	snprintf(buf, sizeof(buf),
	    "%s/api/apps/%s/things/%s/buckets/%s/objects",
	    kiwi->server_url, param->app_id, param->thing_id, param->bucket_id);
	param->server_uri = strdup(buf);

	return 0;
}

/*
 * {
 *   "kii": {
 *     "versoin":"20160323",
 *     "produced":<unix time>,
 *     "point":{
 *       "<key>":{
 *         "time":"<ISO8601 time (i.e. yyyy-MM-DDThh:mm:ss.fff+z)>",
 *         "value":"<value>"
 *       }, ...
 *     }
 *   }
 * }
 */
int
if_kii_encode(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head, struct kiwi_xbuf *xbuf)
{
	struct kiwi_chunk_key *p;
	struct kiwi_chunk_value *q;
	char utime[128];
	int len;

	if (head == NULL)
		errx(1, "ERROR: %s: no data in the kiwi_chunk_key.",
		    __FUNCTION__);

	kiwi_get_strunixtime(utime, sizeof(utime));
	kiwi_xbuf_vstrcat(xbuf,
	    "{\"kii\":{\"version\":\"%s\",\"produced\":%s,\"point\":{",
	    "20160323", utime);

	/* add data and values */
	for (p = head; p != NULL; p = p->next) {
		if (p != head)
			kiwi_xbuf_strcat(xbuf, ",");
		kiwi_xbuf_vstrcat(xbuf, "\"%s\":{", p->key);
		/* add values */
		for (q = p->value; q != NULL; q = q->next) {
			/* q->time to long long number string. */
			kiwi_xbuf_vstrcat(xbuf, "\"time\":\"%s\",", q->time);
			kiwi_xbuf_vstrcat(xbuf, "\"value\":\"%s\"", q->value);
		}
		/* close the point */
		kiwi_xbuf_strcat(xbuf, "}");
	}

	len = kiwi_xbuf_strcat(xbuf, "}}}");
	if (len == xbuf->buflen)
		warnx("WARNING: %s: buffer is too small.", __FUNCTION__);

	return 0;
}

#include <ctype.h>

void
http_dump(char *buf, size_t len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (isascii(buf[i]))
			printf("%c", buf[i]);
		else
			printf("0x%02x", buf[i]);
	}
	printf("\n");
}

static size_t
if_kii_http_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	struct kiwi_ctx *kiwi = (struct kiwi_ctx *)userdata;

	if (kiwi->debug >= 2) {
		printf("===\ncallback\n");
		printf("size=%d\n", (int)size);
		printf("nmemb=%d\n", (int)nmemb);
		printf("userdata=%d\n", (int)nmemb);
		http_dump(ptr, size * nmemb);
	}

	return size * nmemb;
}

/*
 * @param headers the payload data.
 * @param data the payload data.
 * @param peer the name of the server. 
 * @note data and peer are assumed to be terminated by '\0'.
 */
int
if_kii_http_transmit(struct kiwi_ctx *kiwi, struct kiwi_xbuf *xbuf)
{
	struct if_kii_param_t *param;

	param = (struct if_kii_param_t *)kiwi->client_param;
	http_post(xbuf->buf, param->server_uri, if_kii_http_cb, kiwi, 4,
	    param->h_ctype, param->h_app_id, param->h_app_key,
	    param->h_thing_token);

	return 0;
}
