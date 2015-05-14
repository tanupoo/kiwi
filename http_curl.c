#include <stdio.h>
#include <ctype.h>	// write_callback()
#include <err.h>

#include <curl/curl.h>

static size_t
write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
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
http_xml_post(char *data, int datalen, char *peer)
{
	CURL *curl;
	CURLcode res = 0;
	struct curl_slist *headers = NULL;

	curl_global_init(CURL_GLOBAL_DEFAULT);

	headers = curl_slist_append(headers, "Content-Type: text/xml");

	curl = curl_easy_init();
	if(!curl) {
		printf("curl_easy_init() failed: %s\n",
		    curl_easy_strerror(res));
		errx(1, "curl_eary_init() failed.");
	}

	curl_easy_setopt(curl, CURLOPT_URL, peer);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); 

	res = curl_easy_perform(curl);
	if(res != CURLE_OK) {
		printf("curl_easy_perform() failed: %s\n",
		    curl_easy_strerror(res));
	}

	curl_easy_cleanup(curl);
	curl_slist_free_all(headers);
	curl_global_cleanup();

	return 0;
}
