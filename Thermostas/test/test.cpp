#include <Arduino.h>
#include <SocketIoClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define USER_SERIAL Serial

const char *ssid = STASSID;
const char *password = STAPSK;

const int heater_pin = 14;
const int btn1 = 12;
const int btn2 = 13;
char toEmit[5];

SocketIoClient webSocket;


void setup() {
    pinMode(heater_pin, OUTPUT);
    pinMode(btn1, INPUT_PULLUP);

    USER_SERIAL.begin(9600);

    connectWiFi();

    webSocket.begin("192.168.1.26", 25565);

    webSocket.on("message");
}

void loop() {
    webSocket.loop();

}

void connectWiFi(){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        USER_SERIAL.print(".");
        delay(1000);
    }
    
    USER_SERIAL.print('.');
    USER_SERIAL.println('WiFi connected');
    USER_SERIAL.print('IP Adresss : ');
    USER_SERIAL.println(WiFi.localIP());
}