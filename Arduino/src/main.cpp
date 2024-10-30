#include "main.h"


void setup()
{
#if DEBUG
  Serial.begin(DEBUG_BAUDRATE);
  Serial.println("Init Ethernet");
#endif
  Ethernet.init(10);  // Most Arduino shields use digital Pin 10
  Ethernet.begin(mac, ip);  //Init ethernet-shield
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
#if DEBUG
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
#endif
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  //Check if there is com to router
  while (Ethernet.linkStatus() == LinkOFF)
  {
#if DEBUG
    Serial.println("Ethernet cable is not connected.");
#endif
    delay(500);
  }


#if DEBUG
  Serial.println("Init LCD");
#endif
  lcd.init();       //Init LCD
  lcd.backlight();  //Backlight on
  lcd.clear();      //Clear old content

#if DEBUG
  Serial.println("Init MQTT");
#endif
   mqttClient.setServer(MQTT_BROKER_IP,MQTT_BROKER_PORT);
   mqttClient.setCallback(callback);
#if DEBUG
  Serial.println("Finished setup()");
#endif
}

void loop()
{
  static int iOldMappedValue = 0;
  if(!mqttClient.connected())
    reconnectMQTT();
  
  mqttClient.loop();
  int iMappedValue = map(analogRead(ANALOG_PIN), 0, 1023, 0, 100);
  if(iOldMappedValue != iMappedValue && mqttClient.connected())
  {
#if DEBUG
  Serial.println("New mapped value: " + String(iMappedValue));
#endif    
    snprintf(msg, MSG_BUFFER_SIZE, String(iMappedValue).c_str());    //Convert message to char
    mqttClient.publish(MQTT_ANALOGIN_TOPIC, msg, true);             //Send to broker
    iOldMappedValue = iMappedValue;
  }
  if(updateLCD)
  {
#if DEBUG
  Serial.println("Update display");
#endif
    UpdateDisplay();
  }
}

bool reconnectMQTT()
{
  unsigned long ulCancel = millis() + int(MQTT_TIMEOUT);
  unsigned long ulLastTime = millis();


  while (!mqttClient.connected()) 
  {
    if(millis() >= ulCancel)
    {
      break;
    }
    if(((millis() - ulLastTime) > 2000))
    {
#if DEBUG
      Serial.println("MQTT-User: " + String(MQTT_USER));
      Serial.println("MQTT-Pass: " + String(MQTT_PASS));
      Serial.print("MQTT needs User and Pass: ");
      Serial.println(MQTT_SECURE);
      Serial.println("Try to connect to MQTT .....");
#endif     
      if(MQTT_SECURE)
      {
#if DEBUG
        Serial.println("Try with User and pass");
#endif
        mqttClient.connect(MQTT_DEVICE_ID, MQTT_USER, MQTT_PASS);
      }
      else
      {
#if DEBUG
        Serial.println("Try without User and pass");
#endif
        mqttClient.connect(MQTT_DEVICE_ID);
      }

      if(!mqttClient.connected())
      {
#if DEBUG
        Serial.print("failed, rc=");
        Serial.println(mqttClient.state());
#endif
        delay(500);
      }
      ulLastTime = millis();
    }
  }

#if DEBUG
  Serial.print("Status MQTT: ");
#endif
  if(mqttClient.connected())
  {
#if DEBUG
    Serial.println("Connected");
#endif
    mqttClient.subscribe(MQTT_TEMP_TOPIC,1);
    mqttClient.subscribe(MQTT_HUMI_TOPIC,1);
    mqttClient.subscribe(MQTT_BUTTON_TOPIC,1);
    mqttClient.subscribe(MQTT_ANALOGIN_TOPIC,1);
  }
#if DEBUG
  else
  {
    Serial.println("NOT Connected");
  }
#endif
  return mqttClient.connected();
}

void callback(char* topic, byte* payload, unsigned int length)
{
  String stMessage = "";
  for (unsigned int i = 0; i < length; i++)
    stMessage += String((char)payload[i]);

  //Check if temp one has changed
  if(String(topic) == String(MQTT_TEMP_TOPIC))
  {
    if(lastTemp != stMessage.toInt())
    {
#if DEBUG
  Serial.println("Msg from " + String(MQTT_TEMP_TOPIC) + " changed to " + stMessage);
#endif
      lastTemp = stMessage.toInt();
      updateLCD = true;
    }
  }
  //Check if temp two has changed
  if(String(topic) == String(MQTT_HUMI_TOPIC))
  {
    if(lastHumi != stMessage.toInt())
    {
#if DEBUG
  Serial.println("Msg from " + String(MQTT_HUMI_TOPIC) + " changed to " + stMessage);
#endif
      lastHumi = stMessage.toInt();
      updateLCD = true;
    }
  }
  //Check if pressure one has changed
  if(String(topic) == String(MQTT_ANALOGIN_TOPIC))
  {
    if(lastAnalog != stMessage.toInt())
    {
#if DEBUG
  Serial.println("Msg from " + String(MQTT_ANALOGIN_TOPIC) + " changed to " + stMessage);
#endif
      lastAnalog = stMessage.toInt();
      updateLCD = true;
    }
  }
  //Check is pressure two has changed
  if(String(topic) == String(MQTT_BUTTON_TOPIC))
  {
    if(lastButton != stMessage.toInt())
    {
#if DEBUG
  Serial.println("Msg from " + String(MQTT_BUTTON_TOPIC) + " changed to " + stMessage);
#endif
      lastButton = stMessage.toInt();
      updateLCD = true;
    }
  }
}

void UpdateDisplay()
{
  lcd.clear();
  lcd.home();
  lcd.print("Temp: " + String(lastTemp));
  lcd.setCursor(0,1);
  lcd.print("Humi: " + String(lastHumi));
  lcd.setCursor(0,2);
  lcd.print("Analog: " + String(lastAnalog));
  lcd.setCursor(0,3);
  lcd.print("Button: " + String(lastButton));
  updateLCD = false;
}