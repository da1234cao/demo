#include "llhttp.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
  llhttp_t parser;
  llhttp_settings_t settings;

  /* Initialize user callbacks and settings */
  llhttp_settings_init(&settings);

  /* Set user callback */
  // settings.on_message_complete = handle_on_message_complete;

  /* Initialize the parser in HTTP_BOTH mode, meaning that it will select between
  * HTTP_REQUEST and HTTP_RESPONSE parsing automatically while reading the first
  * input.
  */
  llhttp_init(&parser, HTTP_BOTH, &settings);

  /* Parse request! */
  const char* request = "GET / HTTP/1.1\r\n"
                        "Host: 180.101.49.13\r\n"
                        "Connection: keep-alive\r\n"
                        "Cache-Control: max-age=0\r\n"
                        "Upgrade-Insecure-Requests: 1\r\n"
                        "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\r\n"
                        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
                        "Accept-Encoding: gzip, deflate\r\n"
                        "Accept-Language: zh-CN,zh;q=0.9\r\n"
                        "Cookie: BD_HOME=1; BD_UPN=12314753; BD_CK_SAM=1; channel=180.101.49.13; baikeVisitId=b4df012b-69e5-400f-9431-21eba73c79a5\r\n"
                        "\r\n";
  int request_len = strlen(request);

  enum llhttp_errno err = llhttp_execute(&parser, request, request_len);
  if (err == HPE_OK) {
    printf("Successfully parsed.\n");
    printf("method: %s\n",llhttp_method_name(llhttp_get_method(&parser)));
    printf("method: %s\n",parser.data);
  } else {
    fprintf(stderr, "Parse error: %s %s\n", llhttp_errno_name(err),
            parser.reason);
  }
}