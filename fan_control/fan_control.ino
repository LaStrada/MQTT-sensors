#include "secrets.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMP_HUM_TOPIC "hass/living_room/receiver/sensor"
#define CONNECTED_TOPIC "hass/living_room/receiver/startup"

#define fan_speed_topic "hass/living_room/receiver/fan_speed/set"

#define ONE_WIRE_BUS 4
#define RELAYPIN 5

int publishDelay = 60; // Seconds of delay
int forceSend = 30; // Force to send each x minutes

long lastMsg = 0;
long lastForceMsg = 0;
float temp = 0.0;
float diff = 1.0;
bool fan = false;
bool newConnection = true;
float temperatureLimit = 32.0;

WiFiClient espClient;
PubSubClient client(espClient);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress insideThermometer;

void setup() {
  Serial.begin(115200);
  pinMode(RELAYPIN, OUTPUT);
  sensorSetup();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void sensorSetup() {
  sensors.begin();
  // locate devices on the bus
  Serial.print("Locating devices...");
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");

  // report parasite power requirements
  Serial.print("Parasite power is: "); 
  if (sensors.isParasitePowerMode()) Serial.println("ON");
  else Serial.println("OFF");

  if (!sensors.getAddress(insideThermometer, 0)) {
    Serial.println("Unable to find address for Device 0"); 
  }

  Serial.print("Device 0 Address: ");
  printAddress(insideThermometer);
  Serial.println();

  // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
  sensors.setResolution(insideThermometer, 9);
 
  Serial.print("Device 0 Resolution: ");
  Serial.print(sensors.getResolution(insideThermometer), DEC); 
  Serial.println();
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) {
      Serial.print("0");
    }
    Serial.print(deviceAddress[i], HEX);
  }
}

float getTemperature(DeviceAddress deviceAddress) {
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(deviceAddress);
  return tempC;
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

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
    Serial.println("Reading temperature...");

    lastMsg = now;

    float newTemp = getTemperature(insideThermometer);
    Serial.print("Temperature: ");
    Serial.println((String)newTemp);

    if (newConnection || now - lastForceMsg > (forceSend * publishDelay * 1000) || checkBound(newTemp, temp, diff)) {
      lastForceMsg = now;
      fan = activateFan(temp);
      publishData(temp, fan);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("esp8266_receiver", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish(CONNECTED_TOPIC, "online");
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

bool activateFan(float temperature) {
  bool state = (temperature > temperatureLimit);
  if (state) {
    digitalWrite(RELAYPIN, HIGH);
  } else {
    digitalWrite(RELAYPIN, LOW);
  }
  return state;
}

void publishData(float p_temperature, float p_fan_speed) {
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = (String)p_temperature;
  root["fan_speed"] = (String)p_fan_speed;
  root.prettyPrintTo(Serial);
  Serial.println("");
  char data[200];
  root.printTo(data, root.measureLength() + 1);
  client.publish(TEMP_HUM_TOPIC, data, true);
}

