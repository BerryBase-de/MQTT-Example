#include <main.h>

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
#if DEBUG
  Serial.begin(DEBUG_BAUDRATE);
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
  mqttClient.setCallback(callback);
#if DEBUG
  Serial.println("Activate watchdog.");
#endif

  esp_task_wdt_init(10,true);
  esp_task_wdt_add(NULL);
}

void loop()
{
  static bool bFirstRun = true;
  static int iLastButtonState = 0;
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

  int iNewButtonState = digitalRead(BUTTON_PIN);
  if(iLastButtonState != iNewButtonState)
  {
    snprintf(msg, MSG_BUFFER_SIZE, String(iNewButtonState).c_str());    //Convert message to char
    mqttClient.publish(MQTT_BUTTON_TOPIC, msg, true);             //Send to broker
    iLastButtonState = iNewButtonState;
  }
  if(updateLED)
  {
    if(lastButton == HIGH)
    {
      int iPWM = map(lastAnalog, 0, 100, 0, 255);
#if DEBUG
  Serial.println("Set LED on to PWM = " + String(iPWM));
#endif
      analogWrite(LED_PIN, iPWM);
    }
    else
    {
#if DEBUG
  Serial.println("Set LED off");
#endif
      analogWrite(LED_PIN, 0);
    }

    updateLED = false;
  }

  esp_task_wdt_reset();
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
    esp_task_wdt_reset();
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
    if(millis() - ulMillis  > WLANTIMEOUT)
    {
#if DEBUG
      Serial.println(" FAILED");
      Serial.println("WiFi timeout.");
#endif
      return false;
    }
    delay(500);
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
  unsigned long ulCancel = millis() + int(WLANTIMEOUT);
  unsigned long ulLastTime = millis();


  while (!mqttClient.connected()) 
  {
    esp_task_wdt_reset();
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

  //Check if pressure one has changed
  if(String(topic) == String(MQTT_ANALOGIN_TOPIC))
  {
    if(lastAnalog != stMessage.toInt())
    {
#if DEBUG
  Serial.println("Msg from " + String(MQTT_ANALOGIN_TOPIC) + " changed to " + stMessage);
#endif
      lastAnalog = stMessage.toInt();
      updateLED = true;
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
      updateLED = true;
    }
  }
}