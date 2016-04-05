/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#ifndef IF_KII_H_
#define IF_KII_H_

/* KiiCloud */
struct if_kii_param_t {
	char *app_id;
	char *app_key;
	char *thing_token;
	char *thing_id;
	char *bucket_id;

	/*
	 * internal use
	 */
	char *h_app_id;
	char *h_app_key;
	char *h_thing_token;
	char *h_ctype;
	char *server_uri;
};

int if_kii_init(struct kiwi_ctx *);
int if_kii_encode(struct kiwi_ctx *, struct kiwi_chunk_key *, struct kiwi_xbuf *);
int if_kii_http_transmit(struct kiwi_ctx *, struct kiwi_xbuf *);

#endif /* IF_KII_H_ */
