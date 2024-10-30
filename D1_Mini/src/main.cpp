#include <main.h>

void setup()
{
#if DEBUG
  Serial.begin(DEBUG_BAUDRATE);
#endif
  
  Wire.begin(I2C_SDA, I2C_SCL); //Init i2c, see plattformio.ini
  bool bStatus;
  //Init first sensor
  bStatus = bmeOne.begin(0x76);
  if (!bStatus)
  {
#if DEBUG
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
#endif
    while (1);
  }
#if DEBUG
  else
    Serial.println("Valid BME280 - 1 sensor!");

  Serial.println("Init WiFi");
#endif
  if(!initWiFi())
  {
    while (1);
  }
  
#if DEBUG
  Serial.println("WiFi connected.");
  Serial.print("IP-Adress is: ");
  Serial.println(WiFi.localIP());
#endif
  mqttClient.setServer(MQTT_BROKER_IP,MQTT_BROKER_PORT);
}

void loop()
{
  static bool bFirstRun = true;
  static unsigned long ulLastTime = millis();
  if(WiFi.status() != WL_CONNECTED)
  {
    if(!initWiFi())
      ESP.restart();
  }
  
  if(!mqttClient.connected())
  {
    reconnectMQTT();
  }

  mqttClient.loop();
  if((millis() - ulLastTime > UPDATERATE || bFirstRun)
      && WiFi.status() == WL_CONNECTED && mqttClient.connected())
  {
    static float fTemp,fPress,fHum;
    fTemp = bmeOne.readTemperature(); //Get temp one
    fPress = bmeOne.readPressure() / 100.0F;  //Get press one
    fHum = bmeOne.readHumidity();
    //Now convert and send to broker
    snprintf(msg, MSG_BUFFER_SIZE, String(fTemp,1).c_str());    //Convert message to char
    mqttClient.publish(MQTT_TEMP_TOPIC, msg, true);             //Send to broker
    snprintf(msg, MSG_BUFFER_SIZE, String(fPress,1).c_str());   //Convert message to char
    mqttClient.publish(MQTT_PRESS_TOPIC, msg, true);            //Send to broker
    snprintf(msg,MSG_BUFFER_SIZE, String(fHum,1).c_str());      //Convert message to char
    mqttClient.publish(MQTT_HUMI_TOPIC, msg, true);             //Send to broker

    ulLastTime = millis();
    bFirstRun = false;
  }

}

bool initWiFi()
{
  unsigned long ulMillis = millis();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#if DEBUG
  Serial.print("Connection to WiFi .");
#endif
  while(WiFi.status() != WL_CONNECTED)
  {
    ESP.wdtFeed();  //If not call, watchdog will reset controller
#if DEBUG
  Serial.print(" .");
#endif
    if (WiFi.status() == WL_CONNECT_FAILED)
    {
#if DEBUG
      Serial.print(" FAILED");
#endif
      return false;
    }
    //After a defined time, cancel connection
    if(millis() - ulMillis  > WIFI_TIMEOUT)
    {
#if DEBUG
      Serial.println(" FAILED");
      Serial.println("WiFi timeout.");
#endif
      return false;
    }
    delay(500);
    ESP.wdtFeed();
  }
#if DEBUG
    Serial.println(" DONE");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif
  return true;
}

bool reconnectMQTT()
{
  unsigned long ulCancel = millis() + int(WIFI_TIMEOUT);
  unsigned long ulLastTime = millis();


  while (!mqttClient.connected()) 
  {
    ESP.wdtFeed();  //If not call, watchdog will reset controller
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
  if(mqttClient.connected())
  {
    Serial.println("Connected");
  }
  else
  {
    Serial.println("NOT Connected");
  }
#endif
  return mqttClient.connected();
}