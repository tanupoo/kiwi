#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>	// isascii()
#include <err.h>

#include "kiwi.h"

static char *fiap_soap_b1 =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\">"
"<soapenv:Body>"
"<ns2:dataRQ xmlns:ns2=\"http://soap.fiap.org/\">"
"<transport xmlns=\"http://gutp.jp/fiap/2009/11/\">"
"<body>"
;

static char *fiap_soap_b2 =
"</body>"
"</transport>"
"</ns2:dataRQ>"
"</soapenv:Body>"
"</soapenv:Envelope>"
;

static char *fiap_json_b1 = "{\"fiap\":{\"ver\":\"20140401\",\"dataRQ\":{";
static char *fiap_json_b2 = "}}}";

int
if_ieee1888_init(struct kiwi_ctx *kiwi)
{
	return 0;
}

/*
 * fiap_json_b1
 * "<key>": [
 *   { "t": "<time>", "v": "<val>" }, ...
 * ], ...
 * fiap_json_b2
 */
int
if_ieee1888_json_encode(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head, struct kiwi_xbuf *xbuf)
{
	struct kiwi_chunk_key *p;
	struct kiwi_chunk_value *q;
	int len;

	if (head == NULL)
		errx(1, "no data in the kiwi_chunk_key.");

	kiwi_xbuf_strcat(xbuf, fiap_json_b1);

	/* add data and values */
	for (p = head; p != NULL; p = p->next) {
		if (p != head)
			kiwi_xbuf_strcat(xbuf, ",");
		/* add key */
		kiwi_xbuf_vstrcat(xbuf, "\"%s\":[", p->key);
		/* add values */
		for (q = p->value; q != NULL; q = q->next) {
			if (q != p->value)
				kiwi_xbuf_strcat(xbuf, ",");
			kiwi_xbuf_vstrcat(xbuf, 
			    "{\"t\":\"%s\",\"v\":\"%s\"}", q->time, q->value);
		}
		/* close the point */
		kiwi_xbuf_strcat(xbuf, "]");
	}

	len = kiwi_xbuf_strcat(xbuf, fiap_json_b2);
	if (len == xbuf->buflen)
		warnx("buffer is too small.");

	return 0;
}

int
if_ieee1888_soap_encode(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head, struct kiwi_xbuf *xbuf)
{
	struct kiwi_chunk_key *p;
	struct kiwi_chunk_value *q;
	int len;

	if (head == NULL)
		errx(1, "no data in the kiwi_chunk_key.");

	kiwi_xbuf_strcat(xbuf, fiap_soap_b1);

	/* add data and values */
	for (p = head; p != NULL; p = p->next) {
		kiwi_xbuf_vstrcat(xbuf, "<point id=\"%s\">", p->key);
		/* add values */
		for (q = p->value; q != NULL; q = q->next) {
			kiwi_xbuf_vstrcat(xbuf, 
			    "<value time=\"%s\">%s</value>", q->time, q->value);
		}
		/* close the point */
		kiwi_xbuf_strcat(xbuf, "</point>");
	}

	len = kiwi_xbuf_strcat(xbuf, fiap_soap_b2);
	if (len == xbuf->buflen)
		warnx("buffer is too small.");

	return 0;
}

static size_t
if_ieee1888_soap_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t len = size * nmemb;
	int i;

	printf("===\ncallback\n");
	printf("size=%d\n", (int)size);
	printf("nmemb=%d\n", (int)nmemb);

	for (i = 0; i < len; i++) {
		if (isascii(ptr[i]))
			printf("%c", ptr[i]);
		else
			printf("0x%02x", ptr[i]);
	}
	printf("\n");

	return size * nmemb;
}

int
if_ieee1888_http_transmit(struct kiwi_ctx *kiwi, struct kiwi_xbuf *xbuf)
{
	http_post(xbuf->buf, kiwi->server_url, if_ieee1888_soap_cb, kiwi,
	    1, "Content-Type: text/xml");

	return 0;
}
