#include "ESP8266WiFi.h"
#include "ESPAsyncWebServer.h"

#include "html.h"

#define SSID "Millennianno's Ideapad Slim 3"
#define PASSWORD "mille1219"
#define PORT 80

AsyncWebServer server(PORT);

String temperature;
String turbidity;
String water_level;
String ph;

void SerialPrintIPAddress() {
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("LOCAL IP: ");
  Serial.println(WiFi.localIP());
}

size_t sliceSeparator(String& msg, const char *sep = "/") {
  size_t separator = msg.indexOf(sep);
  msg = msg.substring(separator+1);
  return msg.indexOf("/");
}

void setup() {
  Serial.begin(115200);
  // wifi init
  Serial.println("\nStarting WiFi");
  WiFi.begin(SSID, PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  SerialPrintIPAddress();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", temperature.c_str());
  });
  server.on("/ph", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", ph.c_str());
  });
  server.on("/water-level", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", water_level.c_str());
  });
  server.on("/turbidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", turbidity.c_str());
  });

  server.begin();
}

void loop() {}

void serialEvent() {
  if (Serial.available() > 0) {
    /*
    interactive: GET GAINS PUMP 
    or SET GAINS PUMP 0.7 0.1 0.5
    */
    String msg = Serial.readString();
    msg.trim();
    if(msg.equals("IPADDR")) {
      SerialPrintIPAddress();
      return;
    }

    /*
    msg format: "temp/ph/water_level/turbidity"
    */
    size_t separator = msg.indexOf("/");
    temperature = msg.substring(0, separator);
    separator = sliceSeparator(msg);
    ph = msg.substring(0, separator);
    separator = sliceSeparator(msg);
    water_level = msg.substring(0, separator);
    turbidity = msg.substring(separator+1);
  }
}




