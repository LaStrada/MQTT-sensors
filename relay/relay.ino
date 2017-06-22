#include "secrets.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define RELAYPIN 5

#define HERACAM_STATE_TOPIC "hass/entrance/heracam/switch"
#define HERACAM_SET_STATE_TOPIC "hass/entrance/heracam/switch/set"
#define HERACAM_STARTUP_TOPIC "hass/entrance/heracam/startup"

WiFiClient espClient;
PubSubClient client(espClient);

bool relayStatus = true;

void setup() {
  Serial.begin(115200);
  pinMode(RELAYPIN, OUTPUT);
  relayStatus = "off";
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("esp8266_heracam", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish(HERACAM_STARTUP_TOPIC, "online");
      client.subscribe(HERACAM_SET_STATE_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  if (String(topic) == HERACAM_SET_STATE_TOPIC) {
    if (payload[0] == '0') {
      relayStatus = false;
      digitalWrite(RELAYPIN, LOW);
      Serial.println("off");
      client.publish(HERACAM_STATE_TOPIC, "0", true);
    } else if (payload[0] == '1') {
      relayStatus = true;
      digitalWrite(RELAYPIN, HIGH);
      Serial.println("on");
      client.publish(HERACAM_STATE_TOPIC, "1", true);
    }
  } else if (String(topic) == HERACAM_STATE_TOPIC) {
    if (relayStatus == false) {
      client.publish(HERACAM_STATE_TOPIC, "0", true);
    } else {
      client.publish(HERACAM_STATE_TOPIC, "1", true);
    }
  }
}

