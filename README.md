## Konekted IoT Library
This library allows user to connect to konekted services available at [konekted.nafuutronics.com](http://konekted.nafuutronis.com) to automate devices without code seamlessly and quickly :)

## Requirements Before Installing Konekted Library
1. Install ESP8266 Board to Arduino IDE, follow this tutorial [How To Install ESP8266 Board to Arduino IDE](https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/)
1. This library relies on Arduino_JSON library for decoding data from our API endpoint, to Install 
    - [Arduino JSON Library GitHub Here](https://github.com/arduino-libraries/Arduino_JSON)
    - Open Arduino IDE 
        - Go to Sketch
        - Include Library
        - Manage Libraries
        - Then, search for Arduino_JSON
        - Click Install

## Steps to Configure Konekted Services
1. Go to [konekted.nafuutronics.com](http://konekted.nafuutronis.com) and create account
1. Go to Boards > Link Board then Choose NodeMCU ESP8266 and copy the token generated
1. Now install this library then go to examples > Konekted > Link-NodeMCU-ESP8266
1. Change the following credentials
    - Konekted API Configurations
        - ``char* konekted_api = "api_for_konekted_iot";``
        - ``char* konekted_token = "token generated at step 2";``
    - Wifi Configuration
        - ``char* wifi_name = "wifi_name";`` 
        - ``char* wifi_password = "wifi_password";``
1. Upload the example to your board and now you can use the web service to add devices and control at [konekted.nafuutronics.com](http://konekted.nafuutronis.com)