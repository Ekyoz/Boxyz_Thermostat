#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

#ifndef STASSID
#define STASSID "Mon WiFi"
#define STAPSK "KbDPzokTSzzTgf73ML"

#define DHTPIN 2
#define DHTTYPE DHT11

#endif

WiFiClient client;
HTTPClient http;

void setup()
{
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to : '" + WiFi.SSID() + "'");
  Serial.println("IP address: " + WiFi.localIP().toString());

  if (MDNS.begin("esp8266"))
  {
    Serial.println("MDNS responder started");
  }
}

void loop()
{
  MDNS.update();
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("test");
    http.begin(client, "http://192.168.1.26:25565/");
    http.GET();
    http.end();
    delay(1000);
  }
}