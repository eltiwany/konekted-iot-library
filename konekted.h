/*
  Konekted.h - Library for interfacing with Konekted IoT Services.
  Created by Ali A. Saleh, 25/05/2022.
  Released into the public domain.
*/
#ifndef KONEKTED_H
#define KONEKTED_H

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>

class Konekted
{
public:
  // Variables
  String sensors;
  String actuators;
  // --- for non bloking delay
  int actuator_period = 3500;
  unsigned long actuator_time_now = 0;
  
  int sensor_period = 4000;
  unsigned long sensor_time_now = 0;

  // Contructor
  Konekted(bool debug = false);

  // Methods
  void begin_serial(int baudrate = 9600);
  void begin(char *konekted_api, char *konekted_token, char *wifi_name, char *wifi_password);
  void run();
  bool connected();
  void establish_hardware_connections();
  void set_board_status(bool online);
  void automate_actuators();
  void automate_sensors();

private:
  // Variables
  bool _debug;
  // -- for wifi and server
  char *_konekted_api;
  char *_konekted_token;
  char *_wifi_name;
  char *_wifi_password;
  WiFiClient _wifi_client;

  // Sensor variables
  long _duration;
  int _distance;

  // Methods
  String _get_data(String url);
  String _post_data(String url, String payload);
  String _send_error(String message);
  uint8_t _get_analog_pin(int number);
  long _get_data_from_sensor(String _column, String _connections);
  void _connect_actuator(String _device);
  void _switch_actuator(String _device);
  void _send_sensor_data(String _device);
  void _connect_sensor(String device_connections);
  bool _in_sensor_output_array(String searchedValue);
};

#endif
