#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <DHT.h>

/*-------------------VARIABLE------------------------*/

// Wifi settings
const char *ssid = "Mon WiFi";
const char *password = "KbDPzokTSzzTgf73ML";
const int reconnectTry = 5;

// Default boiler settings
const int defHeat = 20;
const bool defState = false;

// Host settings
const char *hostIp = "192.168.1.26";
const int hostPort = 25565;
const String URL_Base = "http://" + String(hostIp) + ":" + String(hostPort);

// Server settings
const int serverPort = 80;

// DHT11
int dhtHeat;

// Pins
const int boilerPin = 14;
const int addBtn = 12;
const int delBtn = 13;
const int dhtPin = 2;
const int infoLedPin = 0;

/*-------------------JSON TRANSMITTED VARIABLE------------------------*/

// Boiler
bool boilerState;
int boilerHeat;
int boilerHeatMax;
int boilerHeatMin;
int boilerHeatDef;
// Server
bool serverState;

/*-------------------OBJECTS------------------------*/

// Server object
ESP8266WebServer server(serverPort);
WiFiClient client;
HTTPClient http;

// DHT object
DHT dht(dhtPin, DHT11);

// Json documents
DynamicJsonDocument boilerJsonDoc(1024);

/*-------------------FUNCTIONS------------------------*/

void setVar(String json)
{
    DeserializationError error = deserializeJson(boilerJsonDoc, json);
    if (error)
    {
        Serial.println("Response of server is invalid");
    }
    else
    {
        boilerState = boilerJsonDoc["boilerState"];
        Serial.println("State: " + String(boilerState));
        boilerHeat = boilerJsonDoc["boilerHeat"];
        Serial.println("Heat: " + String(boilerHeat));
        boilerHeatMax = boilerJsonDoc["settings"]["heat"]["max"];
        Serial.println("Max: " + String(boilerHeatMax));
        boilerHeatMin = boilerJsonDoc["settings"]["heat"]["min"];
        Serial.println("Min: " + String(boilerHeatMin));
        boilerHeatDef = boilerJsonDoc["settings"]["heat"]["def"];
        Serial.println("Def: " + String(boilerHeatDef));
    }
}

/*-------------------HANDLE------------------------*/

void handleVar()
{
    String postBody = server.arg("plain");
    setVar(postBody);
    server.send(201);
}

void handleRoot()
{
    server.send(200, "text/plain", "Thermostas is on");
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

/*-------------------FUNCTIONS------------------------*/

void blinkLed(int flashes, int flashesTimeOne, int flashesTimeTwo)
{
    for (int i = 0; i < flashes; i++)
    {
        digitalWrite(infoLedPin, HIGH);
        delay(flashesTimeOne);
        digitalWrite(infoLedPin, LOW);
        delay(flashesTimeTwo);
    }
}

void varInit()
{
    String response;
    String URL = URL_Base + "/initBoiler";
    http.begin(client, URL);
    int responseCode = http.GET();
    if (responseCode > 0)
    {
        response = http.getString();
        setVar(response);
        Serial.println("boiler initialised");
    }
    else
    {
        Serial.print("Error code: " + responseCode);
    }
    http.end();
}

void wifiInit()
{
    int connectFailed = 0;

    WiFi.mode(WIFI_STA);

    Serial.println("");
    Serial.println("Try to connect to : '" + String(ssid) + "'");

    if (WiFi.status() != WL_CONNECTED || connectFailed < reconnectTry)
    {
        digitalWrite(infoLedPin, LOW);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED)
        {
            blinkLed(1, 500, 1000);
            if (WiFi.status() == WL_CONNECT_FAILED)
            {
                blinkLed(4, 200, 200);
                connectFailed++;
                break;
            }
        }
    }

    if (WiFi.status() == WL_CONNECT_FAILED)
    {
        Serial.println("");
        Serial.println("Connection to : '" + ssid + "' failed. Try to reconnect");
        wifiInit();
    }

    // when the wifi is connected
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("Connected to : '" + WiFi.SSID() + "'");
        Serial.println("IP address: " + WiFi.localIP().toString());
    }
}

void serverInit()
{
    server.on("/", handleRoot);
    server.on("/setVar", handleVar);
    server.onNotFound(handleNotFound);

    // start le serveur
    MDNS.begin("esp8266");
    server.begin();
    Serial.println("HTTP server started");
    blinkLed(2, 500, 500);
}

void (*resetFunc)(void) = 0;
/*-------------------PROGRAMME------------------------*/

void setup()
{
    pinMode(boilerHeat, OUTPUT);
    pinMode(infoLedPin, OUTPUT);
    pinMode(dhtPin, INPUT);
    pinMode(addBtn, INPUT);
    pinMode(delBtn, INPUT);
    Serial.begin(9600);
    wifiInit();
    serverInit();
    varInit();
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        server.handleClient();
        MDNS.update();
    }
    if (WiFi.status() != WL_CONNECTED)
    {
    }
}