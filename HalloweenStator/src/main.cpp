/*
 * WebSocketServer_LEDcontrol.ino
 *
 *  Created on: 26.11.2015
 *
 */

void updateLEDs();

#include <Arduino.h>

// #include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>

#define S1_RED 0
#define S1_GREEN 0
#define S1_BLUE 0

#define S2_RED 0
#define S2_GREEN 0
#define S2_BLUE 0

#define USE_SERIAL Serial

IPAddress staticIP(192,168,44,1);
IPAddress gateway(192,168,1,9);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

uint32_t strip1[3], strip2[3];

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket.remoteIP(num);
            USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
            // USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            int16_t angle = strtol((const char *) &payload[0], NULL, 10);
            angle += 180;
            // USE_SERIAL.println(angle);
            if(angle <= 180) {
                strip1[2] = (angle*255)/180;
                strip1[0] = 255-strip1[2];

                // Strip 2 is just the opposite of strip 1
                strip2[0] = strip1[2];
                strip2[2] = strip1[0];
            } else {
                angle -= 180;
                strip1[0] = (angle*255)/180;
                strip1[2] = 255-strip1[2];

                // Strip 2 is just the opposite of strip 1
                strip2[0] = strip1[2];
                strip2[2] = strip1[0]; 
            }
            break;
    }

}

void setup() {
    //USE_SERIAL.begin(921600);
    USE_SERIAL.begin(115200);

    //USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    pinMode(S1_RED, OUTPUT);
    pinMode(S1_GREEN, OUTPUT);
    pinMode(S1_BLUE, OUTPUT);
    
    pinMode(S2_RED, OUTPUT);
    pinMode(S2_GREEN, OUTPUT);
    pinMode(S2_BLUE, OUTPUT);

    WiFi.config(staticIP, gateway, subnet);
    WiFi.begin("8Hz WAN IP");
    while(WiFi.status() != WL_CONNECTED) {
        delay(100);
    }

    Serial.println(WiFi.localIP());

    // start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    if(MDNS.begin("esp8266")) {
        USE_SERIAL.println("MDNS responder started");
    }

    // handle index
    server.on("/", []() {
        // send index.html
        server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/></body></html>");
    });

    server.begin();

    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
}

void loop() {
    webSocket.loop();
    server.handleClient();
    updateLEDs();
}

void updateLEDs() {
    analogWrite(S1_RED, strip1[0]);
    analogWrite(S1_GREEN, strip1[1]);
    analogWrite(S1_BLUE, strip1[2]);

    analogWrite(S2_RED, strip2[0]);
    analogWrite(S2_GREEN, strip2[1]);
    analogWrite(S2_BLUE, strip2[2]);

    Serial.print("S1 R:");
    Serial.print(strip1[0]);
    Serial.print(" B:");
    Serial.println(strip1[2]);

    
    Serial.print("S2 R:");
    Serial.print(strip2[0]);
    Serial.print(" B:");
    Serial.println(strip2[2]);
}