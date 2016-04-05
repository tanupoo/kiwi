#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <err.h>

#include <curl/curl.h>

/*
 * @param data the payload data.
 * @param peer the server name.  "localhost" if it's NULL.
 * @param stdarg a list of headers.  the end must be NULL.
 */
int
http_post(char *data, char *server_uri,
    size_t (*cb)(char *, size_t, size_t, void *), void *cb_arg,
    int n_args, ...)
{
	CURL *curl;
	CURLcode res = 0;
	va_list ap;
	char *h;
	struct curl_slist *headers = NULL;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	curl = curl_easy_init();
	if(!curl) {
		printf("curl_easy_init() failed: %s\n",
		    curl_easy_strerror(res));
		errx(1, "curl_eary_init() failed.");
	}

	curl_easy_setopt(curl, CURLOPT_USERAGENT, "kiwi/0.1");
	curl_easy_setopt(curl, CURLOPT_URL, server_uri);
#if 0
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
#else
	curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, data);
#endif
	if (cb != NULL) {
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb); 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, cb_arg); 
	}

	/* set headers */
	va_start(ap, n_args);
	while (n_args-- > 0) {
		if ((h = va_arg(ap, char *)) == NULL)
			break;
		headers = curl_slist_append(headers, h);
	}
	va_end(ap);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

	res = curl_easy_perform(curl);
	if(res != CURLE_OK) {
		printf("curl_easy_perform() failed: %s\n",
		    curl_easy_strerror(res));
	}

	if (headers != NULL)
		curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return 0;
}
