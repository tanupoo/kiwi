#include <string.h>

#include "http_curl.h"

int
main()
{
	char *peer = "http://localhost:9999/";
	char *data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\"><soapenv:Body><ns2:dataRQ xmlns:ns2=\"http://soap.fiap.org/\"><transport xmlns=\"http://gutp.jp/fiap/2009/11/\"><body><point id=\"http://fiap.tanu.org/test/sin0001\"><value time=\"2014-11-09T10:53:35+0900\">0.573323</value></point><point id=\"http://fiap.tanu.org/test/cos0001\"><value time=\"2014-11-09T10:53:35+0900\">0.819330</value></point></body></transport></ns2:dataRQ></soapenv:Body></soapenv:Envelope>";

	http_xml_post(data, strlen(data), peer);
}
