//SSID = ESP32-DNSEP-Benoit
//password = petitcoeur

#ifndef ESP_CURL_H
#define ESP_CURL_H

//include at start
void esp_init(); //setup curl and the way to handle request to ESP32

//include each time you want to update state
int esp_request_response(); //perform a request and return the answer as an integer

//include when exiting
void esp_cleanup();  //clean allocated memory


#endif