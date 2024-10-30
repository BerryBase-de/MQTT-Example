#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
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

/*
* =================================================================
* Function:     UpdateDisplay   
* Returns:      void
* Description:  Display new values on I2C-Display
* =================================================================
*/
void UpdateDisplay();

/* Variables */
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 178, 177);   //Static IP-Adress for arduino
EthernetClient client;
PubSubClient mqttClient(client);
LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_COLUMNS,LCD_ROWS);
bool updateLCD = false;
char msg[MSG_BUFFER_SIZE];

int lastTemp = 0;
int lastHumi = 0;
int lastAnalog = 0;
bool lastButton = 0;