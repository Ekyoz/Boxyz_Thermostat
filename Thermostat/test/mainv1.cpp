#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <DHT.h>

#ifndef STASSID
#define STASSID "Mon WiFi"
#define STAPSK "KbDPzokTSzzTgf73ML"

#define DEFBOILERHEAT 20
#define DEFBOILERSTATE false

#define DHTPIN 2
#define DHTTYPE DHT11

#define BAUD 9600
#endif

// server settings
const int serverPort = 80;

// host settings
const char *host = "192.168.1.26";
const int hostPort = 25565;

// pin settings
const int boilerPin = 14;
const int btn1 = 12;
const int btn2 = 13;

// button stat variable
int btn1Stat;
int btn2Stat;

// variable creation
int heat;
int boilerHeat;
int boilerState;
int hostServerState;

// server creation
ESP8266WebServer server(serverPort);
WiFiClient client;
HTTPClient http;

// DHT11 object
DHT dht(DHTPIN, DHTTYPE);

// blink led
void blinkLed(uint8_t pin, int flashes, int flashesTime)
{
  for (int i = 0; i < flashes; i++)
  {
    digitalWrite(pin, HIGH);
    delay(flashesTime);
    digitalWrite(pin, LOW);
    delay(flashesTime);
  }
}

// initialise esp8266
bool Init()
{
  boilerHeat = DEFBOILERHEAT;
  boilerState = DEFBOILERSTATE;
  Serial.begin(BAUD);
  dht.begin();
  if (boilerHeat == DEFBOILERHEAT && boilerState == DEFBOILERSTATE && Serial.available() && isnan(dht.read()))
  {
    blinkLed(LED_BUILTIN, 3, 100);
    return true;
  }
  else
  {
    return false;
  }
}

// get value for set variables
void getValues(int boilerHeatArgs, float, bool boilerStateArgs)
{
}

// main page
void handleRoot()
{
  server.send(200, "text/plain", "Thermostas is on");
}

// error page
void handleNotFound()
{
  String message = "Error\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// init page
void handleInit()
{
  Init();
  if (Init())
  {
    server.send(200, "text/plain", "The thermostat has been initialized correctly !");
    Serial.println("The thermostat has been initialized correctly !");
  }
  else
  {
    server.send(400, "text/plain", "The thermostat is not initialized");
    Serial.println("The thermostat is not initialized");
  }
}

void handleState()
{
  String postBody = server.arg("plain");
  Serial.println(postBody);
}

// boilerheat page
void handleBoilerHeat()
{
  // recupere les arguments
  String boilerHeatArgs = server.arg("heat");
  if (!boilerHeatArgs.isEmpty())
  {
    boilerHeat = boilerHeatArgs.toInt();
    server.send(200, "text/plain", "Boiler heat was been set to : " + boilerHeat);
    Serial.print("Boiler heat was been set to : ");
    Serial.println(boilerHeat);
  }
  else if (boilerHeatArgs.isEmpty())
  {
    server.send(400, "text/plain", "Internal error !");
    Serial.println("Internal error !");
  }
}

// boilerState page
void handleBoilerState()
{
  String boilerStateArgs = server.arg("state");
  if (!boilerStateArgs.isEmpty())
  {
    boilerState = boilerStateArgs.toInt();
    server.send(200, "text/plain", "Boiler state was been set to");
    Serial.println("Boiler heat was been set");
  }
  else if (boilerStateArgs.isEmpty())
  {
    server.send(400, "text/plain", "Internal error !");
    Serial.println("Internal error !");
  }
}

// set wifi setting and connect to this
void wifiInit()
{
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
  // creer les differentes page
  server.on("/", handleRoot);
  server.on("/init", handleInit);
  server.on("/boilerHeat", handleBoilerHeat);
  server.on("/boilerState", handleBoilerState);
  server.on("/test", HTTP_POST, handleState);
  server.onNotFound(handleNotFound);

  // start le serveur
  server.begin();
  Serial.println("HTTP server started");
}

void setup(void)
{
  pinMode(boilerHeat, OUTPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Init();
  wifiInit();
}

void loop(void)
{
  server.handleClient();
  MDNS.update();
  if (WiFi.status() == WL_CONNECTED)
  {
    int btn1_stat = digitalRead(btn1);
    int btn2_stat = digitalRead(btn2);
    if (btn1_stat == HIGH)
    {
      Serial.println("temperature increase");
      http.begin(client, "http://" + String(host) + ":" + String(hostPort) + "/set/thermosUp");
      http.GET();
      http.end();
      boilerHeat++;
      btn1_stat = LOW;
      delay(200);
    }
    if (btn2_stat == HIGH)
    {
      Serial.println("temperature decrease");
      http.begin(client, "http://" + String(host) + ":" + String(hostPort) + "/set/thermosDown");
      http.GET();
      http.end();
      boilerHeat--;
      btn2_stat = LOW;
      delay(200);
    }
  }
  else
  {
    Serial.println("Wifi is not connected!");
    wifiInit();
    delay(1000);
  }
}