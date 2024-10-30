#include <Arduino.h>
#include "WiFi.h"
#include <PubSubClient.h>
#include <esp_task_wdt.h>

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

/*
* =================================================================
* Function:     callback   
* Returns:      void
* Description:  Will automatical called, if a subscribed topic
*               has a new message
* topic:        Returns the topic, from where a new msg comes from
* payload:      The message from the topic
* length:       Length of the msg, important to get conntent
* =================================================================
*/
void callback(char* topic, byte* payload, unsigned int length);

/* Variables */
WiFiClient espClient;
PubSubClient mqttClient(espClient);
char msg[MSG_BUFFER_SIZE];
int lastAnalog = 0;
int lastButton = LOW;
bool updateLED = false;