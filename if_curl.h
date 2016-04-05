#ifndef IF_CURL_H_
#define IF_CURL_H_

#include <stdarg.h>

int http_post(char *, char *,
    size_t (*)(char *, size_t, size_t, void *), void *, int, ...);

#endif /* IF_CURL_H_ */
