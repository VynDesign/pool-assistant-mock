#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>

#include "secrets.h"
const char ssid[] = SECRET_WIFI_SSID;
const char pass[] = SECRET_WIFI_PASS;
const char mqttUser[] = SECRET_MQTT_USER;
const char mqttPass[] = SECRET_MQTT_PASS;

#include "config.h"
const char mqttDevice[] = CONFIG_DEVICE_NAME;
IPAddress mqttBroker = CONFIG_MQTT_BROKER;
const char tempTopic[] = CONFIG_MQTT_SENSOR_CHANNEL_WATER_TEMP;
const char orpTopic[] = CONFIG_MQTT_SENSOR_CHANNEL_ORP;
const char phTopic[] = CONFIG_MQTT_SENSOR_CHANNEL_PH;
const long readingInterval = CONFIG_SENSOR_READING_INTERVAL;
unsigned long previousMillis = 0;
int count = 0;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

long ph;
long orp;
long temp;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  connectWifi();
  connectMqtt();
}

void loop() {
  // keep client alive
  mqttClient.poll();

  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= readingInterval) {
    // save the last time a message was sent
    previousMillis = currentMillis;
    // send message, the Print interface can be used to set the message contents
    ph = random(65, 80);
    orp = random(5800, 8600);
    temp = random(665, 735);
    sendMessage(phTopic, ph);
    sendMessage(orpTopic, orp);
    sendMessage(tempTopic, temp);
    count++;
  }
}

void connectWifi() {
    if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }

  Serial.print("You're connected to the network");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void connectMqtt() {
  mqttClient.setId(mqttDevice);
  mqttClient.setUsernamePassword(mqttUser, mqttPass);
  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(mqttBroker);
  if (!mqttClient.connect(mqttBroker)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");
  Serial.println();
}

void sendMessage(char topic[99], long value) {
  float val = (float) value / 10;
  Serial.print("Sending message to topic: ");
  Serial.print(topic);
  Serial.print(val);
  mqttClient.beginMessage(topic);
  mqttClient.print(val);
  mqttClient.endMessage();  
  Serial.println();
}
