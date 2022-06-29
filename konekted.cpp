#include "konekted.h"

Konekted::Konekted(bool debug)
{
  _debug = debug;
}

void Konekted::begin_serial(int baudrate)
{
  Serial.begin(baudrate);
}

void Konekted::begin(char *konekted_api, char *konekted_token, char *wifi_name, char *wifi_password)
{
  // Make these variables global to the class
  _konekted_api = konekted_api;
  _konekted_token = konekted_token;
  _wifi_name = wifi_name;
  _wifi_password = wifi_password;

  WiFi.begin(_wifi_name, _wifi_password);
  if (_debug)
  {
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
  }
}

/**
 * @brief Called in a loop to perform 
 * necessary readings and automations
 * 
 */
void Konekted::run() {
  // Automate actuators
  automate_actuators();

  // Read and send sensor data to API
  automate_sensors();
  
}

/**
 * @brief Connect to API and returns connection status
 *
 * @return true
 * @return false
 */
bool Konekted::connected()
{
  String url = String(_konekted_api) + "get-board-omc/?token=" + String(_konekted_token);
  JSONVar data = JSON.parse(_get_data(url));
  if ((bool)data["success"])
  {
    set_board_status(true);
    delay(500);
    establish_hardware_connections();
    delay(500);
    return true;
  }
  return false;
}

/**
 * @brief Set board online or offline
 * To allow application to see when board is connected
 *
 * @param online
 */
void Konekted::set_board_status(bool online)
{
  String data = "token=" + String(_konekted_token) + "&status=" + (online ? "1" : "0");
  String url = String(_konekted_api) + "set-board-omc";

  String responseStr = _post_data(url, data);
  JSONVar response = JSON.parse(responseStr);

  if (_debug)
  {
    Serial.print(response["message"]);
  }
}

/**
 * @brief Fetch connections and dynamically allocate them to the board
 *
 */
void Konekted::establish_hardware_connections()
{
  String url = String(_konekted_api) + "get-connections-omc/?token=" + String(_konekted_token);
  JSONVar response = JSON.parse(_get_data(url));

  // Saving connections to local variables
  sensors = JSON.stringify(response["data"]["sensors"]);
  actuators = JSON.stringify(response["data"]["actuators"]);

  // Connect sensors
  if (JSON.typeof(JSON.parse(sensors)) != "undefined")
  {

    if (_debug)
    {
      Serial.print("Sensors: ");
      Serial.println(JSON.parse(sensors).length());
    }

    for (int i = 0; i < JSON.parse(sensors).length(); i++)
    {
      String sensor = JSON.stringify(JSON.parse(sensors)[i]);
      _connect_sensor(sensor);
    }
  }

  // Connect actuators
  if (JSON.typeof(JSON.parse(actuators)) != "undefined")
  {
    if (_debug)
    {
      Serial.print("Actuators: ");
      Serial.println(JSON.parse(actuators).length());
    }

    for (int i = 0; i < JSON.parse(actuators).length(); i++)
    {
      String actuator = JSON.stringify(JSON.parse(actuators)[i]); 
      _connect_actuator(actuator);
    }
  }
}

/**
 * @brief Assign a pin to input or output
 *
 * @param device
 */
void Konekted::_connect_actuator(String _device)
{
  JSONVar device = JSON.parse(_device);
  // Fetch actuator informations
  JSONVar actuator = device["actuator"];
  const int id = actuator["id"];
  const char *name = actuator["name"];

  // fetch actuator connections as array
  String connections_str = JSON.stringify(device["connections"]);
  JSONVar connections = JSON.parse(connections_str);

  if (_debug)
  {
    Serial.print("Connections: ");
    Serial.print(connections.length());
    Serial.print(" -> ");
    Serial.println(connections_str);
  }

  for (int i = 0; i < connections.length(); i++)
  {
    String _board_pin_type = JSON.stringify(connections[i]["board_pin_type"]);
    _board_pin_type.replace("\"", "");
    uint8_t _pin = (_board_pin_type == "A") ? _get_analog_pin((int)connections[i]["board_pin_number"]) : (int)connections[i]["board_pin_number"];

    if (_debug)
    {
      Serial.println();
      Serial.print("Setting ");
      Serial.print(connections[i]["actuator_pin_type"]);
      Serial.print(" -> ");
      Serial.print(_pin);
    }
    
    if (_debug)
      Serial.println(" as output.");
    pinMode(_pin, OUTPUT);
  }
}

/**
 * @brief Automate actuators every 3.5 seconds
 *
 * @param device
 */
void Konekted::automate_actuators()
{
  while(millis() >= actuator_time_now + actuator_period) {
    // Non-blocking delay
    actuator_time_now += actuator_period;

    // Actual automation
    String url = String(_konekted_api) + "get-actuators-omc/?token=" + String(_konekted_token);
    JSONVar response = JSON.parse(_get_data(url));

    // Saving connections to local variables
    actuators = JSON.stringify(response["data"]["actuators"]);
    int len = JSON.parse(actuators).length();

    if (_debug)
    {
      Serial.print("Automating ");
      Serial.print(len);
      Serial.println(" devices");
    }

    if (JSON.typeof(JSON.parse(actuators)) != "undefined")
    {
      for (int i = 0; i < len; i++)
      {
        String actuator = JSON.stringify(JSON.parse(actuators)[i]); 
        _switch_actuator(actuator);
      }
    }   
  }
}

/**
 * @brief Automate actuators every 10 seconds
 *
 * @param device
 */
void Konekted::automate_sensors()
{
  while(millis() >= sensor_time_now + sensor_period) {
    // Non-blocking delay
    sensor_time_now += sensor_period;

    // Actual automation
    String url = String(_konekted_api) + "get-sensors-omc/?token=" + String(_konekted_token);
    JSONVar response = JSON.parse(_get_data(url));

    // Saving connections to local variables
    sensors = JSON.stringify(response["data"]["sensors"]);
    int len = JSON.parse(sensors).length();

    if (_debug)
    {
      Serial.print("Preparing to send data of ");
      Serial.print(len);
      Serial.println(" sensors");
    }

    if (JSON.typeof(JSON.parse(sensors)) != "undefined")
    {
      for (int i = 0; i < len; i++)
      {
        String actuator = JSON.stringify(JSON.parse(sensors)[i]); 
        _send_sensor_data(actuator);
      }
    }   
  }
}

/**
 * @brief Switch actuator on/off
 *
 * @param device
 */
void Konekted::_send_sensor_data(String _device)
{
  JSONVar device = JSON.parse(_device);
  if (JSON.typeof(device) != "undefined") {
    // fetch actuator connections as array
    String connections_str = JSON.stringify(device["connections"]);
    JSONVar connections = JSON.parse(connections_str);
    
    // fetch sensor columns as array
    String columns_str = JSON.stringify(device["columns"]);
    JSONVar columns = JSON.parse(columns_str);

    int user_sensor_id = device["sensor"]["id"];

    for (int i = 0; i < columns.length(); i++)
    {
      String column = JSON.stringify(columns[i]["column"]);
      column.replace("\"", "");
      long data = _get_data_from_sensor(column, connections_str);
      String url = String(_konekted_api) + "set-sensor-data-omc/?token=" + String(_konekted_token) + "&user_sensor_id=" + String(user_sensor_id) + "&column=" + column + "&value=" + String(data);
      JSONVar response = JSON.parse(_get_data(url));

      if (_debug) {
        Serial.print("Data from sensor: ");
        Serial.println(data);
        Serial.print("Column: ");
        Serial.println(column);

        Serial.print("Sending data to ");
        Serial.print(url);
        if ((bool)data["success"])
          Serial.println("OK");
        else
          Serial.println("Failed");
      }
    }
  }
}

long Konekted::_get_data_from_sensor(String _column, String _connections) {
  JSONVar connections = JSON.parse(_connections);
  // Light with LRD
  if (_column == "LIGHT") {
    for (int i = 0; i < connections.length(); i++) {
      String _sensor_pin_type = JSON.stringify(connections[i]["sensor_pin_type"]);
      _sensor_pin_type.replace("\"", "");
      if (_sensor_pin_type == "A")
        return analogRead(_get_analog_pin((int) connections[i]["board_pin_number"]));
      else
        return digitalRead((long) connections[i]["board_pin_number"]);
    }
  }

  // Distance with ultrasonic sensor
  if (_column == "DIST") {
    uint8_t trig_pin = 100; 
    uint8_t echo_pin = 100;
    for (int i = 0; i < connections.length(); i++) {
      String _sensor_pin_type = JSON.stringify(connections[i]["sensor_pin_type"]);
      _sensor_pin_type.replace("\"", "");
      if (_sensor_pin_type == "TRIG")
        trig_pin = (int) connections[i]["board_pin_number"];
      if (_sensor_pin_type == "ECHO")
        echo_pin = (int) connections[i]["board_pin_number"];
    }

    if (_debug) {
      Serial.print("Echo: ");
      Serial.println(echo_pin);
      Serial.print("Trig: ");
      Serial.println(trig_pin);
    }

    if (trig_pin != 100 && echo_pin != 100) {
      // Ensure pinModes are set correctly
      pinMode(trig_pin, OUTPUT);
      pinMode(echo_pin, INPUT);
      // Clears the trigPin condition
      digitalWrite(trig_pin, LOW);
      delayMicroseconds(2);
      // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
      digitalWrite(trig_pin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trig_pin, LOW);
      // Reads the echoPin, returns the sound wave travel time in microseconds
      _duration = pulseIn(echo_pin, HIGH);
      // Calculating the distance i.e. speed of sound wave divided by 2 (go and back)
      _distance = _duration * 0.034 / 2;
      
      if (_debug) {
        Serial.print("Distance: ");
        Serial.println(_distance);
      }

      return _distance;
    }
  }

  return 0;
}

/**
 * @brief Switch actuator on/off
 *
 * @param device
 */
void Konekted::_switch_actuator(String _device)
{
  JSONVar device = JSON.parse(_device);
  if (JSON.typeof(device) != "undefined") {
    // fetch actuator connections as array
    String connections_str = JSON.stringify(device["connections"]);
    JSONVar connections = JSON.parse(connections_str);

    for (int i = 0; i < connections.length(); i++)
    {
      String _board_pin_type = JSON.stringify(connections[i]["board_pin_type"]);
      _board_pin_type.replace("\"", "");
      uint8_t _pin = (_board_pin_type == "A") ? _get_analog_pin((int)connections[i]["board_pin_number"]) : (int)connections[i]["board_pin_number"];
      
      int _operating_value = JSON.typeof(device["actuator"]["operating_value"]) == "undefined" ? 0 : (int)device["actuator"]["operating_value"];
      int _is_switched_on = JSON.typeof(device["actuator"]["is_switched_on"]) == "undefined" ? 0 : (int)device["actuator"]["is_switched_on"];

      if (_debug)
      {
        Serial.println();
        Serial.print("Switching ");
        Serial.print(device["actuator"]["name"]);
        Serial.print(" -> ");
        Serial.println(_is_switched_on ? " on!" : " off!");
      }
      
      if (_is_switched_on && _board_pin_type == "A") {
        // analogWrite(_pin, _operating_value);
        digitalWrite(_pin, HIGH);
      }
      else if (_is_switched_on && _board_pin_type != "A")
        digitalWrite(_pin, HIGH);
      else
        digitalWrite(_pin, LOW);
    }
  }
}

/**
 * @brief Assign a pin to input or output
 *
 * @param device
 */
void Konekted::_connect_sensor(String _device)
{
  JSONVar device = JSON.parse(_device);
  // Fetch sensor informations
  JSONVar sensor = device["sensor"];
  const int id = sensor["id"];
  const char *name = sensor["name"];
  const int interval = sensor["interval"];

  // fetch sensor connections as array
  String connections_str = JSON.stringify(device["connections"]);
  JSONVar connections = JSON.parse(connections_str);

  if (_debug)
  {
    Serial.print("Connections: ");
    Serial.print(connections.length());
    Serial.print(" -> ");
    Serial.println(connections_str);
  }

  for (int i = 0; i < connections.length(); i++)
  {
    String _board_pin_type = JSON.stringify(connections[i]["board_pin_type"]);
    _board_pin_type.replace("\"", "");
    uint8_t _pin = (_board_pin_type == "A") ? _get_analog_pin((int)connections[i]["board_pin_number"]) : (int)connections[i]["board_pin_number"];

    if (_debug)
    {
      Serial.println();
      Serial.print("Setting ");
      Serial.print(connections[i]["sensor_pin_type"]);
      Serial.print(" -> ");
      Serial.print(_pin);
    }

    // Check if sensor pin is output or input and assign it with pinMode
    if (_in_sensor_output_array(JSON.stringify(connections[i]["sensor_pin_type"])))
    {
      pinMode(_pin, OUTPUT);
      if (_debug)
        Serial.println(" as output.");
    }
    else
    {
      pinMode(_pin, INPUT);
      if (_debug)
        Serial.println(" as input.");
    }
  }

  // fetch sensor columns as array
  JSONVar columns = device["columns"];

  // if (name == "HC-SR04") {

  // }
}

/**
 * @brief Search if pin output in a sensor
 *
 * @param searchedValue
 * @return true
 * @return false
 */
bool Konekted::_in_sensor_output_array(String searchedValue)
{
  String _searchedValue = searchedValue;
  _searchedValue.replace("\"", "");
  if (
      _searchedValue == "TRIG" ||
      _searchedValue == "OUT")
  {
    return true;
  }

  return false;
}

uint8_t Konekted::_get_analog_pin(int number) {
  uint8_t _pin = A0;
  switch (number)
  {
  case 0:
    _pin = A0;
    break;
  // case 1: // -> for arduino uno/nano/mega
  //   _pin = A1;
  //   break;
  // case 2:
  //   _pin = A2;
  //   break;
  // case 3:
  //   _pin = A3;
  //   break;
  // case 4:
  //   _pin = A4;
  //   break;
  // case 5:
  //   _pin = A5;
  //   break;
  // case 6:
  //   _pin = A6;
  //   break;
  
  default:
    break;
  }
  return _pin;
}

/**
 * @brief Get Data from URL (API) and returns results
 *
 * @param url
 * @return String
 */
String Konekted::_get_data(String url)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    // Initialize HTTP Request
    HTTPClient http;

    http.begin(_wifi_client, url.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    // Connected
    JSONVar payload = JSON.parse(http.getString());

    if (_debug)
    {
      Serial.println(url);
      Serial.print("HTTP RESPONSE CODE: ");
      Serial.println(httpResponseCode);
      Serial.println();
      Serial.println("HTTP RESPONSE PAYLOAD: ");
      Serial.println(JSON.stringify(payload));
    }

    http.end();

    return JSON.stringify(payload);
  }
  return _send_error("WiFi Disconnected!");
}

/**
 * @brief Post data to URL provided
 * This method actually uses GET request not POST
 *
 * @param url
 * @param payload
 * @return String
 */
String Konekted::_post_data(String url, String payload)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    // Initialize HTTP Request
    HTTPClient http;
    String urlToProcess = (url + String("/?") + payload);
    urlToProcess.replace("\"", "");
    http.begin(_wifi_client, urlToProcess.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    // Connected
    JSONVar payload = JSON.parse(http.getString());

    if (_debug)
    {
      Serial.println(urlToProcess);
      Serial.println(payload);
      Serial.print("HTTP RESPONSE CODE: ");
      Serial.println(httpResponseCode);
      Serial.println();
      Serial.println("HTTP RESPONSE PAYLOAD: ");
      Serial.println(JSON.stringify(payload));
    }

    http.end();

    return JSON.stringify(payload);
  }
  return _send_error("WiFi Disconnected!");
}

/**
 * @brief Sending error message to Serial output
 * For debugging purposes only
 *
 * @param message
 * @return String
 */
String Konekted::_send_error(String message)
{
  JSONVar error;
  error["status"] = 500;
  error["data"]["message"] = message;
  if (_debug)
  {
    Serial.print(message);
  }
  return JSON.stringify(error);
}
