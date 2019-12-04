#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include "common/Weather.h"
#include "common/conf.h" // Create this file and add the variable (OWAPIKEY) for your openweathermap API Key (see https://openweathermap.org/appid)
#include <ArduinoJson.h>

Ticker ticker;
WiFiClient espClient;
PubSubClient client(espClient);

void tick() {
  // toggle state
  int state = digitalRead(BledPin);
  digitalWrite(BledPin, !state);
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode...");
  Serial.println(WiFi.softAPIP());
  ticker.detach();
  ticker.attach(0.1, tick);
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  // sets the pins as output
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  FadeOn(RledPin);
  FadeOn(GledPin);
  FadeOn(BledPin);

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

  ticker.detach();

  // Fade off RGB LED 
  FadeOff(RledPin);
  FadeOff(GledPin);
  FadeOff(BledPin);
}

void loop() {
  WiFiClient client;
  HTTPClient http; 
  String WeatherHTTPURL= String("http://api.openweathermap.org/data/2.5/weather?q=waterbury,ctt&cnt=3&appid=" + OWAPIKEY);

  Serial.print("[HTTP] begin...\n");
  if (http.begin(WeatherHTTPURL)) {

    Serial.print("[HTTP] GET...\n");
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
        const char* weather = root["weather"][0]["main"];
        int temp = root["main"]["temp"];
        temp = temp * 9 / 5 - 459.67;

        // Output to serial monitor
        Serial.print("City:");
        Serial.println(city);
        Serial.print("Temperature:");
        Serial.println(temp);
        Serial.print("Weather:"); 
        Serial.println(weather);

        if(temp < 35) {
          changeColorByHex("0000FF");
        } else if (temp > 35 && temp < 70) {
          changeColorByHex("00FF00");
        } else if (temp > 70) {
          changeColorByHex("FF0000");
        }
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }

  delay(120000);

// REMINDERS: - All pins must be faded in first to be able to be faded out after.
//             - Delay is set at 2ms (check _delay to see it's setting) (255 steps = 1.5seconds for fading in and out)
//             - loop will only start when wifi is connected and configured. Connect to the SSID named above from a phone or computer to configure.

// FadeOn(RledPin);
// FadeOn(GledPin);
// FadeOn(BledPin);
// FadeOff(GledPin);
// FadeOff(BledPin);
// FadeOff(RledPin);
// FadeOn(GledPin);
// FadeOff(RledPin);
// FadeOn(BledPin);
// FadeOn(RledPin);
// Pulse(RledPin, 5, 1);
// Pulse(BledPin, 5, 4);
// Pulse(GledPin, 5, 10);
// FadeOff(BledPin);
// FadeOff(GledPin);
// FadeOff(RledPin);

//  changeColorByHex("ff0000");
//  delay(200);
//  changeColorByHex("c300c5");
//  delay(200);
//  changeColorByHex("0013c5");
//  delay(200);
//  changeColorByHex("00c5c3");
//  delay(200);
//  changeColorByHex("00c54f");
//  delay(200);
//  changeColorByHex("25c500");
//  delay(200);
//  changeColorByHex("86c500");
//  delay(200);
//  changeColorByHex("ffa200");
//  delay(random(200));
//  changeColorByHex("ff3c00");
//  delay(random(200));
//  changeColorByHex("ff0000");
//  delay(random(200));

// changeColorByHex("5300ff");
// delay(random(100));
// changeColorByHex("0018ff");
// delay(random(100));
// changeColorByHex("0060ff");
// delay(random(100));
// changeColorByHex("0066ff");
// delay(random(100));

//  FadeOn(RledPin);
//  FadeOn(GledPin);
//  FadeOn(BledPin);
//  Pulse(RledPin, 5, 1);
//  Pulse(RledPin, 5, 4);
//  Pulse(RledPin, 5, 10);
//  TurnOff(RledPin);
//  TurnOff(GledPin);
//  TurnOff(BledPin);

//Serial.println("Main loop end");

}
