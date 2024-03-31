#include <ESPWiFiAPI.h>
#include <Adafruit_NeoPixel.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#define LED_PIN    4
#define NUM_LEDS  256

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB;

AsyncWebServer server(80);

IPAddress AP_IP = IPAddress(192, 168, 4, 2);
//IPAddress STA_IP = IPAddress(192, 168, 15, 120);
IPAddress STA_IP = IPAddress(192, 168, 0, 120);

char *accessPointSSID = "AccessPoint";
char *accessPointPassword = "accesspoint123";

String configHash = "12345";
String resetHash = "12345";

String corpoPost;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

int smileMatrix[256] = {
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
0,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,
0,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
0,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0,
0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,
0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,
0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,
0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void setup() {
    Serial.begin(9600);

    ESPWiFiAPI api = ESPWiFiAPI(&server, AP_IP, STA_IP, accessPointSSID, accessPointPassword);
    api.setConfigHash(configHash);
    api.setResetHash(resetHash);
    api.setupWiFi();
    api.setupManager();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { 
        request->send_P(200, "text/html", "Enable"); 
    });

    server.on("/smile", HTTP_GET, [](AsyncWebServerRequest *request) { 
        for (int i = 0; i < NUM_LEDS; i++) {
          if (smileMatrix[i] == 1) {
            strip.setPixelColor(i, strip.Color(20, 20, 20));
          } else {
            strip.setPixelColor(i, strip.Color(0, 0, 0));
          }
        }
        strip.show();
        request->send_P(200, "text/html", "Done"); 
    });

    AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/segment", [](AsyncWebServerRequest *request, JsonVariant &json) {
      StaticJsonDocument<50000> data;
      int size = 0;
      if (json.is<JsonArray>()) {
        JsonArray array = json.as<JsonArray>();
        for (JsonVariant value : array) {
          size++;
        }
        RGB *list = (RGB*)malloc(size * sizeof(RGB));
        int index = 0;
        if (json.is<JsonArray>()) {
          JsonArray array = json.as<JsonArray>();
          for (JsonVariant value : array) {
            RGB rgb;

            rgb.red = value[0];
            rgb.green = value[1];
            rgb.blue = value[2];
            
            list[index] = rgb;
            index++;
          }
        }
        strip.clear();
        for (int i = 0;i < size;i++) {
          int red = list[i].red;
          int green = list[i].green;
          int blue = list[i].blue;
          strip.setPixelColor(i, strip.Color(red, green, blue));
        }
        strip.show();
        free(list);
      } else {
        Serial.println("O JSON não é um array.");
      }
      request->send(200, "application/json", "Done");
    });
    server.addHandler(handler);

    strip.begin();
    strip.show();
    server.begin();
}

void loop() {
}
