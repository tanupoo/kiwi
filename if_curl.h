/*
 * Copyright (C) 2010 Shoichi Sakane <sakane@tanu.org>, All rights reserved.
 * See the file LICENSE in the top level directory for more details.
 */

#ifndef IF_CURL_H_
#define IF_CURL_H_

#include <stdarg.h>

int http_post(char *, char *,
    size_t (*)(char *, size_t, size_t, void *), void *, int, ...);

#endif /* IF_CURL_H_ */
