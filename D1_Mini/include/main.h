#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
#include <PubSubClient.h>

//Functions
/*
* =================================================================
* Function:     setup
* Description:  Setup display and sensors   
* Returns:      void
* =================================================================
*/
void setup();

/*
* =================================================================
* Function:     loop
* Description:  Main loop to let program work   
* Returns:      void
* =================================================================
*/
void loop();

/*
* =================================================================
* Function:     initWifi()
* Description:  Init or reconnect to WiFi
* Returns:      true for connected in time, otherwise false
* =================================================================
*/
bool initWiFi();

/*
* =================================================================
* Function:     reconnectMQTT()
* Description:  Init or reconnect to MQTT
* Returns:      true for connected in time, otherwise false
* =================================================================
*/
bool reconnectMQTT();

/* Variables */
Adafruit_BME280 bmeOne;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
char msg[MSG_BUFFER_SIZE];