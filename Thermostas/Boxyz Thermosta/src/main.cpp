#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID "Mon WiFi"
#define STAPSK "KbDPzokTSzzTgf73ML"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
const char *host = "192.168.1.26";
const int port = 25565;

const int heater_pin = 14;
const int btn1 = 12;
const int btn2 = 13;

int btn1_stat;
int btn2_stat;

int heat = 20;
bool state = false;

ESP8266WebServer server(80);
WiFiClient client;
HTTPClient http;

void handleRoot()
{
  server.send(200, "text/plain", "Thermostas");
}

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

void handleInit(){
  String state_arg = server.arg("state");
  if(state_arg == "true"){
    state = true;
  }
  if(state_arg == "false"){
    state = false;
  }
  int heat_arg = server.arg("heat").toInt();
  heat = heat_arg;
  server.send(200, "text/plain", "initialised");
  Serial.println("initialised");
  delay(50);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
}

void handleHeat()
{
  // recupere les arguments
  String heater_arg = server.arg("heater");
  String heat_arg = server.arg("heat");
  if (heater_arg != "" && heat_arg != "")
  {
    // convertion en int
    int heater_int = heater_arg.toInt();
    int heat_int = heat_arg.toInt();
    Serial.println("Console -> Heater : " + String(heater_int) + " , Heat : " + String(heat_int));
    // Verification des arguments
    if ((heater_int == 0) || (heater_int == 1))
    {
      // commande heater
      if (heater_int == 1)
      {
        digitalWrite(heater_pin, HIGH);
        heat = heat_int;
        Serial.println("Console -> Heater run to " + String(heat));
        server.send(200, "text/plain", "Heater run to " + String(heat));
        delay(50);
      }
      if (heater_int == 0)
      {
        digitalWrite(heater_pin, LOW);
        Serial.println("Console -> Heater stop");
        server.send(200, "text/plain", "Heater stop");
        delay(50);
      }
    }
    else
    {
      Serial.println("Console -> Error : arguments is not good");
      server.send(404, "text/plain", "Error argument");
      delay(50);
    }
  }
  else
  {
    Serial.println("Console -> Error : Arguments missings");
    server.send(404, "text/plain", "Arguments missings");
    delay(50);
  }
}

void setup(void)
{
  pinMode(heater_pin, OUTPUT);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.connect(host, port);
  client.println("getTemp");
  String line = client.readStringUntil('\n');
  heat = line.toInt();
  Serial.println("Heat :" + heat);

  if (MDNS.begin("esp8266"))
  {
    Serial.println("MDNS responder started");
  }
  // creer les differentes page
  server.on("/", handleRoot);
  server.on("/heat", handleHeat);
  server.on("/init", handleInit);
  server.onNotFound(handleNotFound);

  // start le serveur
  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
}

void loop(void)
{
  server.handleClient();
  MDNS.update();
  if (WiFi.status() == WL_CONNECTED)
  {
    btn1_stat = digitalRead(btn1);
    btn2_stat = digitalRead(btn2);
    if (btn1_stat == HIGH)
    {
      Serial.println("temp_add");
      http.begin(client, "http://192.168.1.26:25565/set/thermosUp");
      int httpResponseCode = http.GET();
      http.end();
      heat++;
      delay(200);
    }
    if (btn2_stat == HIGH)
    {
      Serial.println("temp_del");
      http.begin(client, "http://192.168.1.26:25565/set/thermosDown");
      int httpResponseCode = http.GET();
      http.end();
      heat--;
      delay(200);
    }
  }
  else
  {
    Serial.println("Wifi is not connected!");
    WiFi.begin(ssid, password);
    delay(1000);
  }
}