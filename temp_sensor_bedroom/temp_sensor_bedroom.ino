#include "Secrets.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

#define TEMP_HUM_TOPIC "hass/bedroom/sensor"

#define DHTTYPE DHT22
#define DHTPIN 4

int publishDelay = 60; // Seconds of delay
int forceSend = 10; // Force to send each x minutes

long lastMsg = 0;
long lastForceMsg = 0;
float temp = 0.0;
float hum = 0.0;
bool newConnection = true;

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 11);

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (newConnection || now - lastMsg > (publishDelay * 1000)) {
    lastMsg = now;

    Serial.println("Reading temperature...");

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    Serial.print("Temp: ");
    Serial.println((String)newTemp);

    Serial.print("Hum:  ");
    Serial.println((String)newHum);

    if (newConnection || now - lastForceMsg > (forceSend * publishDelay * 1000) ||checkBound(newTemp, temp) || checkBound(newHum, hum)) {
      lastForceMsg = now;
      temp = newTemp;
      hum = newHum;
      publishData(temp, hum);
      newConnection = false;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("esp8266_bedroom", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      newConnection = true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue) {
  return !isnan(newValue) && (newValue != prevValue);
}

void publishData(float p_temperature, float p_humidity) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = (String)p_temperature;
  root["humidity"] = (String)p_humidity;
  root.prettyPrintTo(Serial);
  Serial.println("");
  char data[200];
  root.printTo(data, root.measureLength() + 1);
  client.publish(TEMP_HUM_TOPIC, data, true);
}

