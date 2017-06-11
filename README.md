# Hass-MQTT-sensors
Home Assistant - MQTT sensors (ESP8266)

## List of MQTT nodes:

### Surround receiver sensor
My receiver is placed inside my tv bench with a closed door. I had a temperature sensor placed above the receiver for a couple of weeks, and the highest temperature I measured was 38 degrees celsius. There was a couple of holes in the back, but not big enough to replace the air. What I did was to create a new hole in the back, big enough to mount a 120mm x 120mm fan. This fan moves new air into the tv bench ([positive vs negative pressure](http://www.tomshardware.com/reviews/cooling-air-pressure-heatsink,3058-5.html)).

Hardware:
- NodeMCU v2
- 12v power supply
- DS18B20 temperature sensor
- TIP120 transistor

Schematic
Code (Arduino)

### Temperature + Humidity (bedroom)
Just a simple temperature / humidity sensor, placed in my bedroom.

Hardware:
- ESP8266 WeMos D1
- DHT22 (better resolution than DHT11)

Code (Arduino)

### Camera control (relay)
I wanted my network camera to be turned off when I'm home, and on when I'm leaving. This is just a simple MQTT relay.

Hardware:
- NodeMCU v2
- Relay

Code (Arduino)

### Kegerator
I have a [kegerator](https://en.wikipedia.org/wiki/Kegerator) in the guest room. What I needed was a couple of sensors to monitor temperature and if the door was closed.

Hardware:
- NodeMCU v2
- DS18B20 temperature sensor
- Door sensor

In progress...

### Door bell
I want to get a notification if somebody rings my doorbell. I'm not able to hear the doorbell now if any doors to the entrance is closed and/or if we are listing to music or the TV is on.

Hardware:
- ESP8266 WeMos D1

In progress...
