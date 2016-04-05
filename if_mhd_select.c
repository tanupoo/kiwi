/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <microhttpd.h>
#include <err.h>

#include "kiwi.h"

char *kiwi_server_name = KIWI_SERVER_NAME "(microhttpd/0.9.48)";

#define POST_REQ_MAX_BUFLEN	1280
#define POST_RES_MAX_BUFLEN	1280
#define GET_RES_MAX_BUFLEN	1280
#define MAX_PATHLEN             256

#define KIWI_HS_URL_MAXLEN	1024
#define KIWI_HS_FILE_MAXLEN	2048 * 1024	/* 2MB */

char s_header_host[128];

/* default server addr&port. */
#define DEFAULT_SERVER_ADDR "127.0.0.1"
#define DEFAULT_SERVER_PORT "18886"

const char *s_post_header = "<html><body>";
const char *s_post_footer = "</body></html>";

const char *s_query_header = "{\"kiwi\":{\"version\":\"20141225\",\"point\":{";
const char *s_query_footer = "}}}";

struct re_patt {
	char *patt;
	regex_t re;
};
static struct re_patt re_doc = { ".*\\.(html|css|js|jpg|png)" };
//static struct re_patt re_query = { "k=[0-9a-z]{41}(&k=[0-9a-z]{41}){0,20}" };
//static struct re_patt re_query2 = { ".*/\?k=[0-9a-zA-Z/:_\\-]{0,128}(&k=[0-9a-zA-Z/:_\\-]{0,128}){0,20}" };

#define INTERNAL_ERROR_PAGE "<html><head><title>Internal error</title></head><body>Internal error</body></html>"

static struct MHD_Response *internal_error_response;

struct hs_request_t {
	struct MHD_PostProcessor *pp;
	struct kiwi_ctx *kiwi;
	struct kiwi_xbuf *xbuf;
};

struct hs_query_t {
	int count;
	struct kiwi_ctx *kiwi;
	struct kiwi_xbuf *xbuf;
};

/*
 * set document root.
 */
static int
hs_set_root(char *root_dir)
{
	char path[MAX_PATHLEN];

	if (root_dir == NULL) {
		if (getcwd(path, MAX_PATHLEN) == NULL)
			err(1, "ERROR: %s: getcwd()", __FUNCTION__);
		root_dir = path;
	}

	warnx("ERROR: %s: trying change root to %s", __FUNCTION__, root_dir);
	if (chroot(root_dir) == -1)
		err(1, "ERROR: %s: chroot()", __FUNCTION__);
	warnx("ERROR: %s: change root to %s", __FUNCTION__, root_dir);

	return 0;
}

/*
 * functions related to regex(3).
 */

static int
hs_re_init()
{
	int ret;
	char errbuf[48];

	if ((ret = regcomp(&re_doc.re, re_doc.patt, REG_EXTENDED)) != 0) {
		regerror(ret, &re_doc.re, errbuf, sizeof(errbuf));
		errx(1, "ERROR: %s: regcomp failed for [%s] %s\n",
		    __FUNCTION__, re_doc.patt, errbuf);
	}

	return 0;
}

static int
hs_re_match(struct kiwi_ctx *kiwi, struct re_patt *patt, const char *target)
{
	int ret;
	size_t maxgroup = 10;
	regmatch_t groups[maxgroup];
	char errbuf[48];

	if ((ret = regexec(&patt->re, target, maxgroup, groups, 0)) != 0) {
		regerror(ret, &patt->re, errbuf, sizeof(errbuf));
		if (kiwi->debug > 3) {
			warnx("DEBUG: %s: regexec: %d %s. [%s] !~ [%s]\n",
			    __FUNCTION__, ret, errbuf, patt->patt, target);
		}
		return 0;
	}

	return 1;
}

static void
hs_re_free()
{
	regfree(&re_doc.re);
}

/*
 * functions to handle HTTP.
 */

#if 0
x HTTP/1.1 200 OK
x Date: Thu, 27 Nov 2014 01:07:30 GMT
+ Server: Apache/2.2.14
Content-Location: index.html.ja
Vary: negotiate,accept-language,Accept-Encoding
TCN: choice
Last-Modified: Tue, 20 May 2014 03:55:47 GMT
ETag: "1081bed-4585-4f9ccdad92ab2"
Accept-Ranges: bytes
Content-Length: 17797
Connection: close
+ Content-Type: text/html
Content-Language: ja
#endif

static void
cb_terminated(struct kiwi_ctx *kiwi, struct MHD_Connection *conn, void **con_cls,
    enum MHD_RequestTerminationCode toe)
{
	struct hs_request_t *request = *con_cls;

	if (kiwi->debug >= 4)
		warnx("%s: called", __FUNCTION__);

	if (NULL == request)
		return;
	if (NULL != request->pp)
		MHD_destroy_post_processor(request->pp);
	if (NULL != request->xbuf)
		kiwi_xbuf_free(request->xbuf);

	free(request);
}

static int
add_common_headers(struct kiwi_ctx *kiwi, struct MHD_Response *response)
{
	struct hmd_headers {
		char *key;
		char *value;
	} hmd_headers[] = {
	    { MHD_HTTP_HEADER_SERVER, kiwi_server_name },
	    { "Access-Control-Allow-Origin", "*" },
	    { "Access-Control-Allow-Headers", "Content-Type" },
	    { "Access-Control-Allow-Methods", "POST, GET, OPTIONS, HEAD" },
	    { "Access-Control-Expose-Headers", "Content-Type, Content-Length" },
	    { "Access-Control-Max-Age", "1728000" },
	    { "Age", "0" },
	    { "Cache-Control", "max-age=0, no-cache, no-store" },
	};
	int i;

	for (i = 0; i < sizeof(hmd_headers)/sizeof(hmd_headers[0]); i++) {
		if (MHD_add_response_header(response,
		    hmd_headers[i].key, hmd_headers[i].value) == MHD_NO) {
			warnx("ERROR: %s: MHD_add_response_header() failed",
			__FUNCTION__);
			return -1;
		}
	}

	return 0;
}

#if 0
static int
add_common_headers(struct kiwi_ctx *kiwi, struct MHD_Response *response)
{
	if (MHD_add_response_header(response, MHD_HTTP_HEADER_SERVER,
	    kiwi_server_name) == MHD_NO ||
	    MHD_add_response_header(response, "Access-Control-Allow-Origin",
	    "*") == MHD_NO ||
	    MHD_add_response_header(response, "Access-Control-Allow-Headers",
	    "Content-Type") == MHD_NO ||
	    MHD_add_response_header(response, "Access-Control-Allow-Methods",
	    "POST, GET, OPTIONS, HEAD") == MHD_NO ||
	    MHD_add_response_header(response, "Access-Control-Expose-Headers",
	    "Content-Type, Content-Length") == MHD_NO ||
	    MHD_add_response_header(response, "Access-Control-Max-Age",
	    "1728000") == MHD_NO ||
	    MHD_add_response_header(response, "Age", "0") == MHD_NO ||
	    MHD_add_response_header(response, "Cache-Control",
	    "max-age=0, no-cache, no-store") == MHD_NO) {
		warnx("ERROR: %s: MHD_add_response_header() failed",
		    __FUNCTION__);
		return -1;
	}

	return 0;
}
#endif

static int
queue_response(struct kiwi_ctx *kiwi, struct MHD_Connection *conn, struct kiwi_xbuf *xbuf)
{
	struct MHD_Response *response;
	int ret;

	if (kiwi->debug >= 3)
		warnx("%s: response len=%d", __FUNCTION__, (int)strlen(xbuf->buf));
	if (kiwi->debug >= 4)
		warnx("%s: response [%s]", __FUNCTION__, xbuf->buf);

#if MHD_VERSION >= 0x00094700
	response = MHD_create_response_from_buffer(strlen(xbuf->buf),
	    (void *)xbuf->buf, MHD_RESPMEM_MUST_COPY);
#else
	response = MHD_create_response_from_data(strlen(xbuf->buf),
	    (void *)xbuf->buf, 0, 1);
#endif
	if (response == NULL) {
		warnx("%s: MHD_create_response_from_buffer: error",
		    __FUNCTION__);
		return MHD_NO;
	}

	if (add_common_headers(kiwi, response) < 0)
		return MHD_NO;

	MHD_add_response_header(response, "Content-Type", "text/json");

	if ((ret = MHD_queue_response(conn, MHD_HTTP_OK, response)) != MHD_YES)
		warnx("%s: failed to sent or queued.", __FUNCTION__);

	MHD_destroy_response(response);

	return ret;
}

static int
do_options(struct kiwi_ctx *kiwi, struct MHD_Connection *conn,
    const char *url, const char *method, const char *version,
    const char *xdata, size_t *xdata_size, void **ptr)
{
	static int aptr;
	struct kiwi_xbuf *xbuf;
	int ret;

	/* do never respond on first call */
	if (&aptr != *ptr) {
		*ptr = &aptr;
		return MHD_YES;
	}
	*ptr = NULL;

	xbuf = kiwi_xbuf_new(1);	/* dummy */

	ret = queue_response(kiwi, conn, xbuf);

	kiwi_xbuf_free(xbuf);

	return ret;
}

static int
do_head(struct kiwi_ctx *kiwi, struct MHD_Connection *conn,
    const char *url, const char *method, const char *version,
    const char *xdata, size_t *xdata_size, void **ptr)
{
	static int aptr;
	struct kiwi_xbuf *xbuf;
	int ret;

	/* do never respond on first call */
	if (&aptr != *ptr) {
		*ptr = &aptr;
		return MHD_YES;
	}
	*ptr = NULL;

	xbuf = kiwi_xbuf_new(1);	/* dummy */

	ret = queue_response(kiwi, conn, xbuf);

	kiwi_xbuf_free(xbuf);

	return ret;
}

#if MHD_VERSION >= 0x00094700
static ssize_t
hs_cb_file_reader(void *cls, uint64_t pos, char *buf, size_t max)
#else
static int
hs_cb_file_reader(void *cls, uint64_t pos, char *buf, int max)
#endif
{
	FILE *fp = cls;

	(void)fseek(fp, pos, SEEK_SET);
	return fread(buf, 1, max, fp);
}

static void
hs_cb_file_reader_free(void *cls)
{
	FILE *fp = cls;
	fclose(fp);
}

static int
hs_res_doc(struct MHD_Connection *conn, const char *url)
{
	struct MHD_Response *response;
	struct stat st;
	FILE *fp;

	if (stat(url, &st) == -1) {
		warn("%s: stat(%s)", __FUNCTION__, url);
		return MHD_NO;
	}
	if (!(st.st_mode & S_IFREG)) {
		warnx("%s: %s is not a reguler file",
			__FUNCTION__, url);
		return MHD_NO;
	}

	if ((fp = fopen(url, "rb")) == NULL) {
		warnx("%s: fopen(%s)", __FUNCTION__, url);
		return MHD_NO;
	}
	response = MHD_create_response_from_callback(
		st.st_size, KIWI_HS_FILE_MAXLEN,
		&hs_cb_file_reader, fp, &hs_cb_file_reader_free);
	if (response == NULL) {
		fclose(fp);
		return MHD_NO;
	}

	MHD_queue_response(conn, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return MHD_YES;
}

/*
 *        "http://fiap.tanu.org/test/alps01/temp": [
 *          { "time":"2014-12-02T09:17:17+0900", "value":"16.9" }
 *          ]
 */
/* XXX neet to move into json encoding (e.g. json.c) */
static int 
hs_res_cb_mkmsg(void *cls, enum MHD_ValueKind kind, const char *query_key, const char *query_value)
{
	struct hs_query_t *query = cls;
	struct kiwi_ctx *kiwi = query->kiwi;
	struct kiwi_xbuf *xbuf = query->xbuf;
	char *fmt = "\"%s\":[{\"time\":\"%s\",\"value\":\"%s\"}]";
	size_t len;

	if (strcmp(query_key, "k") == 0) {
		struct kiwi_xbuf *x_time = kiwi_xbuf_new(KIWI_TIME_MAXLEN);
		struct kiwi_xbuf *x_value = kiwi_xbuf_new(KIWI_VALUE_MAXLEN);

		if (kiwi_db_get_latest(kiwi, query_value, x_time, x_value) == -1) {
			warnx("%s: getting the latest value is failed.", __FUNCTION__);
			kiwi_xbuf_free(x_time);
			kiwi_xbuf_free(x_value);
			return MHD_NO;
		}

		if (kiwi->debug >= 4) {
			warnx("%s: appending the following message to the response.", __FUNCTION__);
			warnx(fmt, query_value, x_time->buf, x_value->buf);
		}

		/* 1st entry or the successers ? */
		if (query->count)
			kiwi_xbuf_vstrcat(xbuf, ",");

		len = kiwi_xbuf_vstrcat(NULL, fmt, query_value, x_time->buf, x_value->buf);
		if (kiwi_xbuf_vstrcat(xbuf, fmt, query_value, x_time->buf, x_value->buf) != len) {
			warnx("%s: buffer is exeeded. (cb_mkmsg)", __FUNCTION__);
			kiwi_xbuf_free(x_time);
			kiwi_xbuf_free(x_value);
			return MHD_NO;
		}

		query->count++;

		kiwi_xbuf_free(x_time);
		kiwi_xbuf_free(x_value);
	}
	else
	{
		if (kiwi->debug >= 3)
			warnx("%s: invalid key (%s) found", __FUNCTION__, query_key);
		return MHD_YES;		/* just ignore it */
	}

	return MHD_YES;
}

static void
hs_query_free(struct hs_query_t *query)
{
	if (NULL == query)
		return;
	if (NULL != query->xbuf)
		kiwi_xbuf_free(query->xbuf);

	free(query);
}

static struct hs_query_t *
hs_query_new(struct kiwi_ctx *kiwi)
{
	struct hs_query_t *new;

	if ((new = calloc(1, sizeof(struct hs_query_t))) == NULL) {
		if (kiwi->debug >= 4)
			warnx("%s: 1-2", __FUNCTION__);
		warn("%s: calloc(query)", __FUNCTION__);
		return NULL;
	}
	new->kiwi = kiwi;
	if ((new->xbuf = kiwi_xbuf_new(GET_RES_MAX_BUFLEN)) == NULL) {
		if (kiwi->debug >= 4)
			warnx("%s: 1-4", __FUNCTION__);
		warn("%s: calloc(query->xbuf)", __FUNCTION__);
		hs_query_free(new);
		return NULL;
	}

	return new;
}

/*
 *  {
 *    "kiwi": {
 *      "version": "20141225",
 *      "point": {
 *        "http://fiap.tanu.org/test/alps01/temp": [
 *          { "time":"2014-12-02T09:17:17+0900", "value":"16.9" }
 *          ],
 *        "http://fiap.tanu.org/test/alps01/light": [
 *          {"time":"2014-12-02T09:17:17+0900", "value":"-73.0" }
 *          ]
 *      }
 *    }
 *  }
 */
/* XXX need to move into json encoding (e.g. json.c) */
static int
hs_res_query(struct kiwi_ctx *kiwi, struct MHD_Connection *conn, const char *url)
{
	struct hs_query_t *query;
	int num, ret;

	if ((query = hs_query_new(kiwi)) == NULL) {
		warnx("%s: hs_query_t allocation failed.", __FUNCTION__);
		return MHD_NO;
	}

	/* the header of the response of the query */
	if (kiwi_xbuf_strcat(query->xbuf, s_query_header) != strlen(s_query_header)) {
		warnx("%s: GET's response is exeeced (footer).", __FUNCTION__);
		hs_query_free(query);
		return MHD_NO;
	}

	num = MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND, NULL, NULL);
	ret = MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND, &hs_res_cb_mkmsg, query);
	if (num != ret) {
		warnx("%s: too many argument", __FUNCTION__);
		hs_query_free(query);
		return MHD_NO;
	}

	/* the end of the response of the query */
	if (kiwi_xbuf_strcat(query->xbuf, s_query_footer) != strlen(s_query_footer)) {
		warnx("%s: GET's response is exeeced (footer).", __FUNCTION__);
		hs_query_free(query);
		return MHD_NO;
	}

	ret = queue_response(kiwi, conn, query->xbuf);
	hs_query_free(query);

	return ret;
}

static int
do_get(struct kiwi_ctx *kiwi, struct MHD_Connection *conn,
    const char *url0, const char *method, const char *version,
    const char *xdata, size_t *xdata_size, void **ptr)
{
	static int aptr;
	int urllen;
	const char *url;
	int ret;

	/* need to skip the 1st slash unless it's done chroot.  */
	if (kiwi->server_root_dir)
		url = url0;
	else
		url = &url0[1];

	/* do never respond on first call */
	if (&aptr != *ptr) {
		*ptr = &aptr;
		return MHD_YES;
	}
	*ptr = NULL;

	/* check the length of the url */
	urllen = strlen(url0);
	if (urllen > KIWI_HS_URL_MAXLEN) {
		warnx("%s: url is too long=%d", __FUNCTION__, urllen);
		return MHD_NO;
	}

	/* document ? */
	if (hs_re_match(kiwi, &re_doc, url))
		return hs_res_doc(conn, url);

#if 1
	/* query for key ? */
	ret = MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND,
	    NULL, NULL);
#else
	/* XXX how can i check the query part in the url. */
	ret = hs_re_match(kiwi, &re_query2, url);
#endif

	if (ret	== 0) {
		warnx("%s: invalid request [%s]", __FUNCTION__, url);
		return MHD_queue_response(conn,
		    MHD_HTTP_INTERNAL_SERVER_ERROR, internal_error_response);
	}

	return hs_res_query(kiwi, conn, url);
}

static int
cb_post_iter(void *cls, enum MHD_ValueKind kind,
    const char *key, const char *filename,
    const char *content_type, const char *transfer_encoding,
    const char *data, uint64_t off, size_t size)
{
	struct hs_request_t *request = cls;
	struct kiwi_ctx *kiwi = request->kiwi;
	struct kiwi_xbuf *xbuf = request->xbuf;

	if (kiwi->debug >= 4) {
		warnx("%s: 0 key=%s off=%d size=%d",
		    __FUNCTION__, key, (int)off, (int)size);
	}

	if (off > sizeof(xbuf->buflen)) {
		warnx("%s: off=%d is bigger than buflen=%d",
		    __FUNCTION__, (int)off, (int)xbuf->buflen);
		return MHD_YES;
	}

	if (kiwi_xbuf_memcpy(xbuf, off, data, size) != size) {
		warnx("%s: xbuf_memcat failed xbuflen=%d off=%d size%d.",
		    __FUNCTION__, xbuf->buflen, (int)off, (int)size);
		return MHD_YES;
	}

	return MHD_YES;
}

static int
do_post(struct kiwi_ctx *kiwi, struct MHD_Connection *conn,
    const char *url, const char *method, const char *version,
    const char *xdata, size_t *xdata_size, void **ptr)
{
	struct hs_request_t *request;
	struct kiwi_xbuf *xbuf;
	int ret;

	request = *ptr;
	if (kiwi->debug >= 4)
		warnx("%s: 0 len=%d", __FUNCTION__, (int)*xdata_size);
	if (request == NULL) {
		if (kiwi->debug >= 4)
			warnx("%s: 1-1", __FUNCTION__);
		if ((request = calloc(1, sizeof(struct hs_request_t))) == NULL) {
			if (kiwi->debug >= 4)
				warnx("%s: 1-2", __FUNCTION__);
			warn("calloc(request)");
			return MHD_NO;
		}
		if ((request->xbuf = kiwi_xbuf_new(POST_REQ_MAX_BUFLEN)) == NULL) {
			if (kiwi->debug >= 4)
				warnx("%s: 1-4", __FUNCTION__);
			warn("calloc(request->xbuf)");
			return MHD_NO;
		}
		*ptr = request;
		if ((request->pp = MHD_create_post_processor(conn,
		    1024, &cb_post_iter, request)) == NULL) {
			if (kiwi->debug >= 4)
				warnx("%s: 1-5", __FUNCTION__);
			warnx("MHD_create_post_processor: failed");
			// Should free request here ?
			// NO, it is taken place incb_terminated().
			return MHD_NO;
		}
		if (kiwi->debug >= 4)
			warnx("%s: 1-6", __FUNCTION__);
		return MHD_YES;
	}

	if (kiwi->debug >= 4)
		warnx("%s: 0-1", __FUNCTION__);

	/* evaluate POST data */
	MHD_post_process(request->pp, xdata, *xdata_size);
	if (*xdata_size != 0) {
		if (kiwi->debug >= 4)
			warnx("%s: 2-1", __FUNCTION__);
		*xdata_size = 0;
		return MHD_YES;
	}
	if (kiwi->debug >= 4)
		warnx("%s: 0-2", __FUNCTION__);

	/* done with POST data, serve response */
	MHD_destroy_post_processor(request->pp);
	request->pp = NULL;

	if (kiwi->debug >= 4)
		warnx("%s: 0-3", __FUNCTION__);

	xbuf = kiwi_xbuf_new(POST_RES_MAX_BUFLEN);

	if (kiwi_xbuf_strcat(xbuf, s_post_header) != strlen(s_post_header)) {
		warnx("POST's response is exeeced. (header)");
		kiwi_xbuf_free(xbuf);
		return MHD_NO;
	}

	if (kiwi_xbuf_strcat(xbuf, request->xbuf->buf) != request->xbuf->offset) {
		warnx("POST's response is exeeced. (xdata)");
		kiwi_xbuf_free(xbuf);
		return MHD_NO;
	}

	if (kiwi_xbuf_strcat(xbuf, s_post_footer) != strlen(s_post_footer)) {
		warnx("POST's response is exeeced (footer).");
		kiwi_xbuf_free(xbuf);
		return MHD_NO;
	}

	if (kiwi->debug >= 4)
		warnx("%s: 0-99", __FUNCTION__);

	ret = queue_response(kiwi, conn, xbuf);
	kiwi_xbuf_free(xbuf);

	return ret;
}

static int
conn_handler(void *cls, struct MHD_Connection *conn,
    const char *url, const char *method, const char *version,
    const char *xdata, size_t *xdata_size, void **ptr)
{
	struct kiwi_ctx *kiwi = cls;

	if (kiwi->debug >= 4) {
#ifdef __x86_64__
		warnx("%s: 0 %s %s len=%lu *ptr=%02x",
		    __FUNCTION__, method, url, *xdata_size, *(char *)ptr);
#else
		warnx("%s: 0 %s %s len=%u *ptr=%02x",
		    __FUNCTION__, method, url, *xdata_size, *(char *)ptr);
#endif
	}

	/* check method */
	if (strcmp(method, MHD_HTTP_METHOD_HEAD) == 0) {
		return do_head(kiwi, conn, url, method, version,
		    xdata, xdata_size, ptr);
	}
	else
	if (strcmp(method, MHD_HTTP_METHOD_OPTIONS) == 0) {
		return do_options(kiwi, conn, url, method, version,
		    xdata, xdata_size, ptr);
	}
	else
	if (strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
		return do_get(kiwi, conn, url, method, version,
		    xdata, xdata_size, ptr);
	}
	else
	if (strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
		return do_post(kiwi, conn, url, method, version,
		    xdata, xdata_size, ptr);
	}

	warnx("unexpected method %s", method);
	return MHD_NO;
}

#if 0
static void
cb_timeout(struct kiwi_ctx *kiwi)
{
	int qlen = 0;

	if (qlen)
		warnx("%s: qlen=%d\n", __FUNCTION__, qlen);
}
#endif

/*
 * TODO io_ctx may take multiple IOs.
 */
int
kiwi_server_loop(struct kiwi_ctx *kiwi)
{
	int n_server_port;
	struct MHD_Daemon *daemon;
	unsigned int flags = 0;
	struct timeval *timeout;
	fd_set rfd, wfd, efd;
#if MHD_VERSION >= 0x00094700
	MHD_socket fd_max;
#else
	int fd_max;
#endif
#if 0
	MHD_UNSIGNED_LONG_LONG mhd_timeout;
#endif
	int nfd;

	/* change root anyway */
	hs_set_root(kiwi->server_root_dir);

	if (!kiwi->server_addr) {
		warnx("%s: set %s as the server addr", __FUNCTION__,
		    DEFAULT_SERVER_ADDR);
		kiwi->server_addr = strdup(DEFAULT_SERVER_ADDR);
	}
	if (!kiwi->server_port) {
		warnx("%s: set %s as the server port", __FUNCTION__,
		    DEFAULT_SERVER_PORT);
		kiwi->server_port = strdup(DEFAULT_SERVER_PORT);
	}
	snprintf(s_header_host, sizeof(s_header_host),
	    "%s:%s", kiwi->server_addr, kiwi->server_port);
	n_server_port = atoi(kiwi->server_port);

	hs_re_init();

	/* set MHD flags */
	if (kiwi->debug)
		flags |= MHD_USE_DEBUG;

	/* error page */
#if MHD_VERSION >= 0x00094700
	internal_error_response = MHD_create_response_from_buffer(
	    strlen(INTERNAL_ERROR_PAGE), (void *)INTERNAL_ERROR_PAGE,
	    MHD_RESPMEM_PERSISTENT);
#else
	internal_error_response = MHD_create_response_from_data(
	    strlen(INTERNAL_ERROR_PAGE), (void *)INTERNAL_ERROR_PAGE,
	    0, 1);
#endif
	(void)MHD_add_response_header(internal_error_response,
	    MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");

	/* TODO see benchmark.c and benchmark_https.c demo.c digest_auth.c */
	daemon = MHD_start_daemon(flags,
	    n_server_port, NULL, NULL,
	    &conn_handler, kiwi,
	    MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)15,
	    MHD_OPTION_NOTIFY_COMPLETED, &cb_terminated, kiwi,
	    MHD_OPTION_END);
	if (NULL == daemon)
		return -1;

	timeout = NULL;
	simple_ev_init_fdset(kiwi->ev_ctx, &rfd, &wfd, &efd, &fd_max);
	while (1) {
		fd_max = 0;
		/* set fds */
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		FD_ZERO(&efd);
		if (MHD_get_fdset(daemon, &rfd, &wfd, &efd, &fd_max) != MHD_YES)
			break; /* fatal internal error */
		simple_ev_set_fdset(kiwi->ev_ctx, &fd_max);
		simple_ev_set_timeout(kiwi->ev_ctx, &timeout);

		nfd = select(fd_max + 1, &rfd, &wfd, &efd, timeout);
		if (nfd < 0) {
			int i;
			warn("ERROR: %s: select()", __FUNCTION__);
			/* XXX fd dump */
			warnx("fd_max = %d", fd_max);
			warnx("rfd %p\n", &rfd);
			for (i = 0; i < fd_max; i++) {
				char c;
				int e = write(i, &c, 1);
				warnx("fd=%d e=%d\n", i, e);
			}
			break;
		}
		else if (nfd > 0)
			MHD_run(daemon);

		/* check it anyway regardless of timeout */
		simple_ev_check(kiwi->ev_ctx);
	}

	MHD_stop_daemon(daemon);
	hs_re_free();
	MHD_destroy_response(internal_error_response);

	return 0;
}
