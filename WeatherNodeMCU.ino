#include <Time.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include "common/Weather.h"
#include "common/conf.h" // Create this file and add the variable (OWAPIKEY) for your openweathermap API Key (see https://openweathermap.org/appid)
#include <ArduinoJson.h>
#include <Servo.h>

Ticker ticker;
WiFiClient espClient;
PubSubClient client(espClient);
Servo sunMoonServo;
Servo cloudServo;
Servo temperatureServo;

void tick() {
  // toggle state
  int state = digitalRead(16);
  digitalWrite(16, !state);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode...");
  Serial.println(WiFi.softAPIP());
  ticker.detach();
  ticker.attach(0.1, tick);
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  // Sets LED pins
  pinMode(SunRledPin, OUTPUT);
  pinMode(SunGledPin, OUTPUT);
  pinMode(SunBledPin, OUTPUT);
  pinMode(CloudRledPin, OUTPUT);
  pinMode(CloudGledPin, OUTPUT);
  pinMode(CloudBledPin, OUTPUT);

  /* Wifi setup */
  ticker.attach(0.7, tick);

  Serial.begin(115200);

  // Initialize WiFiManager and then destroy it after it's used
  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConnectTimeout(20);

  // Enable to hard reset saved settings, just remember to disable it again right after ;)
  // wifiManager.resetSettings();

  // Set the SSID name
  wifiManager.autoConnect("NodeMCURGBWifi");
  Serial.println("Connected to Wifi.");

  sunMoonServo.attach(10); // SDD3
  // cloudServo.attach(0); // D3
  // temperatureServo.attach(2); // D4

  ticker.detach();
}

void loop() {
  WiFiClient client;
  HTTPClient http;
  String WeatherHTTPURL = String("http://api.openweathermap.org/data/2.5/weather?lat=41.548&lon=-73.205&appid=" + OWAPIKEY);
  Serial.println(WeatherHTTPURL);
  Serial.print("[HTTP] OWM begin...\n");
  if (http.begin(WeatherHTTPURL)) {

    Serial.print("[HTTP] OWM GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {

        // Parsing
        const size_t bufferSize = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(8) + 370;
        DynamicJsonBuffer jsonBuffer(bufferSize);
        JsonObject& root = jsonBuffer.parseObject(http.getString());

        // Parameters
        int id = root["id"]; // 1
        const char* city = root["name"];
        String weather = root["weather"][0]["main"];
        int temp = root["main"]["temp"];
        temp = temp * 9 / 5 - 459.67; // Convert temp to F from K
        int latitude = root["coord"]["lat"];
        int longitude = root["coord"]["lon"];
        int sunrise = root["sys"]["sunrise"];
        int sunset = root["sys"]["sunset"];
        int timestamp = root["dt"];
        int gmtOffset = root["timezone"];
        String AMorPM;

        // Output to serial monitor
        Serial.print("City: ");
        Serial.println(city);
        Serial.print("Temperature: ");
        Serial.println(temp);
        Serial.print("Weather: ");
        Serial.println(weather);
        Serial.print("Lat: ");
        Serial.println(latitude);
        Serial.print("Long: ");
        Serial.println(longitude);

        // Detect if it's AM or PM, then change the servo / LEDs for the day / night detector if the timestamp is greater than the sunrise or sunset
        sunrise = sunrise + gmtOffset;
        sunset = sunset + gmtOffset;
        timestamp = timestamp + gmtOffset;

        if(isAM(timestamp)) {
          AMorPM = "AM";
        }

        if(isPM(timestamp)) {
          AMorPM = "PM";
        }

        if(timestamp > sunrise && timestamp < sunset) { // Greater than the sunrise to dusk OR 
          Serial.println("Sun is UP!");
          changeColorByHex("Sun","ffff00");

          int i;

          // Rotate servo to Sun 
          for (i = 0; i < 90; i++) {
            sunMoonServo.write(i);
            delay(30);
          }

          // TODO: Set sun LED
        }

        if(timestamp < sunset && timestamp < sunrise || timestamp > sunset) { // 12AM to sunrise OR greater than sunset to 12AM
          Serial.println("Sun is DOWN!");
          changeColorByHex("Sun","000022");

          int i;

          // Rotate servo to Moon 
          for (i = 90; i > 0; i--) {
            sunMoonServo.write(i);
            delay(30);
          }

          // TODO: Show LED moon phase (0, .25, .5, .75, 1) - 0 is new moon 1 is full moon
          Serial.print("Moon phase: ");
          Serial.println(GetPhase(year(timestamp), month(timestamp), day(timestamp)));
        }

        Serial.println(timestamp);
        Serial.println(sunrise);
        Serial.println(sunset);

        // TODO: Cloud LED Set based off of weather condition // only happens when servo is engaged
        if(weather == "Thunderstorm") {
          changeColorByHex("Cloud","ffff00"); // Yellow
        } else if(weather == "Drizzle") {
          changeColorByHex("Cloud","0000fa"); // Blue
        } else if(weather == "Rain") {
          changeColorByHex("Cloud","0000fa"); // Blue
        } else if(weather == "Snow") {
          changeColorByHex("Cloud","00f7fa"); // Cyan
        } else if(weather == "Clouds") {
          changeColorByHex("Cloud","ffffff"); // White
        } else if(weather == "Clear") {
          Serial.println("Weather is clear, shut off cloud LED");
          changeColorByHex("Cloud","000000");
          // TODO: Rotate cloud servo to unshown 
        } else {
          changeColorByHex("Cloud","212121"); // Dim LED
          Serial.print("Weather condition is some other condition: '");
          Serial.print(weather);
          Serial.println("'!");
        }

        // This gets the local time of the lat / long, a.k.a local time of device or set weather location :)
        Serial.print("Time: ");
        Serial.print(hourFormat12(timestamp));
        Serial.print(":");
        Serial.print(minute(timestamp));
        Serial.println(AMorPM);

        // if(temp < 35) {
        //   changeColorByHex("Sun","0000FF");
        // } else if (temp > 35 && temp < 70) {
        //   changeColorByHex("Sun","00FF00");
        // } else if (temp > 70) {
        //   changeColorByHex("Sun","FF0000");
        // }
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP] Unable to connect\n");
  }

  delay(10000); // Refresh every minute (eventually change this back to 60000)
}