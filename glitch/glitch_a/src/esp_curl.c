#include "esp_curl.h"
#include <curl/curl.h>

static CURL *curl;
static const char *esp32_url = "http://192.168.4.1/button";

static char esp_response = '0'; // response buffer: holds only one byte (char)

static size_t write_callback(char *contents, size_t size, size_t nmemb,
                             void *userp) {
  *((char *)userp) = contents[0]; // store first byte of response in the
                                  // variable pointed to by CURLOPT_WRITEDATA
  return size * nmemb;
}

void esp_init() {
  curl = curl_easy_init(); // initialise curl session
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL,
                     esp32_url); // set the target URL for HTTP requests
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     write_callback); // set function to handle response data
    curl_easy_setopt(
        curl, CURLOPT_WRITEDATA,
        &esp_response); // set address where response data will be writte

  } else {
    printf("ERROR : failed to init curl\n");
  }
}

int esp_request_response() {
  if (!curl)
    return -1;

  CURLcode res = curl_easy_perform(
      curl); // perform the HTTP request configured in esp_init()

  if (res != CURLE_OK) {
    printf("Request failed: %s\n", curl_easy_strerror(res));
    return -1;
  }

  return esp_response - '0'; // convert and return response char to int
}

void esp_cleanup() {
  if (curl) {
    curl_easy_cleanup(curl);
    curl = NULL;
  }
}

