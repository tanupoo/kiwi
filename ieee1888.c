#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <err.h>

#include "kiwi.h"

#ifdef USE_KIWI_CLIENT_CURL
#include "http_curl.h"
#endif

static char *ieee1888_b1 =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
"<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\">"
"<soapenv:Body>"
"<ns2:dataRQ xmlns:ns2=\"http://soap.fiap.org/\">"
"<transport xmlns=\"http://gutp.jp/fiap/2009/11/\">"
"<body>"
;

static char *ieee1888_b2 =
"</body>"
"</transport>"
"</ns2:dataRQ>"
"</soapenv:Body>"
"</soapenv:Envelope>"
;

int
ieee1888_encode(struct kiwi_ctx *kiwi, struct kiwi_chunk_key *head, struct kiwi_xbuf *xbuf)
{
	struct kiwi_chunk_key *p;
	struct kiwi_chunk_value *q;
	int len;

	if (head == NULL)
		errx(1, "no data in the kiwi_chunk_key.");

	kiwi_xbuf_strcat(xbuf, ieee1888_b1);

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

	len = kiwi_xbuf_strcat(xbuf, ieee1888_b2);
	if (len == xbuf->buflen)
		warnx("buffer is too small.");

	return 0;
}

int
ieee1888_transmit(struct kiwi_ctx *kiwi, struct kiwi_xbuf *xbuf)
{
#ifdef USE_KIWI_CLIENT_CURL
	http_xml_post(xbuf->buf, xbuf->offset, kiwi->peer_name);
#endif

	return 0;
}
