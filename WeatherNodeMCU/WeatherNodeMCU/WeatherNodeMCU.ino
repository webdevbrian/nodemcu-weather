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
  // Sets LED pins
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);

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
}

void loop() {
  WiFiClient client;
  HTTPClient http;
  String WeatherHTTPURL = String("http://api.openweathermap.org/data/2.5/weather?q=waterbury,ctt&cnt=3&appid=" + OWAPIKEY);

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
        const char* weather = root["weather"][0]["main"];
        int temp = root["main"]["temp"];
        temp = temp * 9 / 5 - 459.67; // Convert temp to F from K
        int latitude = root["coord"]["lat"];
        int longitude = root["coord"]["lon"];
        int sunrise = root["sys"]["sunrise"];
        int sunset = root["sys"]["sunset"];

        // Output to serial monitor
        Serial.print("City:");
        Serial.println(city);
        Serial.print("Temperature:");
        Serial.println(temp);
        Serial.print("Weather:");
        Serial.println(weather);
        Serial.print("Lat:");
        Serial.println(latitude);
        Serial.print("Long:");
        Serial.println(longitude);

        // get response from timezonedb with timestamp
        // get gmtOffset from timezonedb and add that from openweathermap's sunrise and sunset timestamp hour format
        // Detect if it's AM or PM, then change the servo / LEDs for the day / night detector if the timestamp is greater than the sunrise or sunset

        String TimezoneDBHTTPURL = String("http://api.timezonedb.com/v2.1/get-time-zone?key=" + TIMEZONEDBKEY + "&format=json&by=position&lat=" + latitude + "&lng=" + longitude);
        Serial.print("[HTTP] Timezone DB begin...\n");
        if (http.begin(TimezoneDBHTTPURL)) {
          Serial.print("[HTTP]  Timezone DB GET...\n");
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

              String status = root["status"];
              int timestamp = root["timestamp"];
              int gmtOffset = root["gmtOffset"];
              String AMorPM;

              Serial.print("Response status: ");
              Serial.println(status);

              sunrise = sunrise + gmtOffset;
              sunset = sunset + gmtOffset;

              if(isAM(timestamp)) {
                AMorPM = "AM";
                Serial.println("Morning!");

                if(timestamp >= sunrise) {
                  Serial.println("Sun is UP!");
                  changeColorByHex("ffff00");
                }
              }

              if(isPM(timestamp)) {
                AMorPM = "PM";
                Serial.println("Evening!");

                if(timestamp >= sunset) {
                  Serial.println("Sun is DOWN!");
                  changeColorByHex("0000FF");
                }
              }

              // This gets the local time of the lat / long, a.k.a local time of device or set weather location :)
              Serial.print("Time: ");
              Serial.print(hourFormat12(timestamp));
              Serial.print(":");
              Serial.print(minute(timestamp));
              Serial.println(AMorPM);

              // Show moon phase (0, .25, .5, .75, 1) - 0 is new moon 1 is full moon
              Serial.print("Moon phase: ");
              Serial.println(GetPhase(year(timestamp), month(timestamp), day(timestamp)));
            }
          } else {
            // There was an error
            changeColorByHex("FF0000");
          }
        }

        // if(temp < 35) {
        //   changeColorByHex("0000FF");
        // } else if (temp > 35 && temp < 70) {
        //   changeColorByHex("00FF00");
        // } else if (temp > 70) {
        //   changeColorByHex("FF0000");
        // }
      }
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  } else {
    Serial.printf("[HTTP} Unable to connect\n");
  }

  delay(60000); // Refresh every minute
}

// calculate the current phase of the moon
float GetPhase(int nYear, int nMonth, int nDay) {
  float phase;
  double AG, IP;
  long YY, MM, K1, K2, K3, JD;
  YY = nYear - floor((12 - nMonth) / 10);
  MM = nMonth + 9;

  if (MM >= 12) {
    MM = MM - 12;
  }

  K1 = floor(365.25 * (YY + 4712));
  K2 = floor(30.6 * MM + 0.5);
  K3 = floor(floor((YY / 100) + 49) * 0.75) - 38;
  JD = K1 + K2 + nDay + 59;

  if (JD > 2299160) {
    JD = JD - K3;
  }

  IP = MyNormalize((JD - 2451550.1) / 29.530588853);
  AG = IP*29.53;
  phase = 0;

  if ((AG < 1.84566) && (phase == 0)) {
    phase = 0; //new; 0% illuminated
  }

  if ((AG < 5.53699) && (phase == 0)) {
    phase = .25; //Waxing crescent; 25% illuminated
  }

  if ((AG < 9.922831) && (phase == 0)) {
    phase = .50; //First quarter; 50% illuminated
  }

  if ((AG < 12.91963) && (phase == 0)) {
    phase = .75; //Waxing gibbous; 75% illuminated
  }

  if ((AG < 16.61096) && (phase == 0)) {
    phase = 1; //Full; 100% illuminated
  }

  if ((AG < 20.30228) && (phase == 0)) {
    phase = .75; //Waning gibbous; 75% illuminated
  }

  if ((AG < 23.99361) && (phase == 0)) {
    phase = .50; //Last quarter; 50% illuminated
  }

  if ((AG < 27.68493) && (phase == 0)) {
    phase = .25; //Waning crescent; 25% illuminated
  }

  if (phase == 0) {
    phase = 0; //default to new; 0% illuminated
  }

  return phase;
}

double MyNormalize(double v) {
  v = v - floor(v);
  if (v < 0)
  v = v + 1;
  return v;
}