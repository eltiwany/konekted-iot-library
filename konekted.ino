#include "konekted.h"
Konekted konekted(true);

// Setup konekted services
char* konekted_api = "http://konekted-api.nafuutronics.com/api/";
char* konekted_token = "fHgsTBKFY3T1GOwj";

// Setup WiFi credentials
char* wifi_name = "samsung-a30s"; 
char* wifi_password = "usiniulize"; 

void setup() {
  konekted.begin_serial(115200);
  konekted.begin(konekted_api, konekted_token, wifi_name, wifi_password);
  
  while(!konekted.connected()) {
    Serial.println("Connection to konekted API services failed, retry in 5 seconds ...");
    delay(5000);
  }
}

void loop() {
  konekted.run();
}