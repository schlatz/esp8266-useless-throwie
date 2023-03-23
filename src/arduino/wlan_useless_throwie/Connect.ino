
// ****************************************************************
// Sketch Esp8266 Login Manager mit Captive Portal und optischer Anzeige
// created: Jens Fleischer, 2021-01-05
// last mod: Jens Fleischer, 2021-11-29
// For more information visit: https://fipsok.de
// ****************************************************************
// Hardware: Esp8266
// Software: Esp8266 Arduino Core 2.6.3 / 2.7.4 / 3.0.2
// Getestet auf: Nodemcu, Wemos D1 Mini Pro
/******************************************************************
  Copyright (c) 2021 Jens Fleischer. All rights reserved.

  This file is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This file is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*******************************************************************/
// Diese Version von Login Manager sollte als Tab eingebunden werden.
// #include <LittleFS.h> #include <ESP8266WebServer.h> müssen im Haupttab aufgerufen werden
// Die Funktionalität des ESP8266 Webservers und des LittleFS Tab ist erforderlich.
// Die Funktion "connectWifi();" muss im Setup eingebunden werden.
// Die Oneboard LED blinkt beim Verbindungsaufbau zum Netzwerk und leuchtet im AP Modus dauerhaft.
// Die Zugangsdaten werden nicht menschenlesbar im Dateisystem gespeichert.
/**************************************************************************************/

/**
   Folgendes muss im Webserver Tab vor dem "setup()" eingefügt werden.

   #include <DNSServer.h>
   const byte DNS_PORT = 53;
   DNSServer dnsServer;

   Der DNS Server muss im loop aufgerufen werden.

   void loop() {
     dnsServer.processNextRequest();
     reStation();
   }
*/

//#define CONFIG                                        // Einkommentieren wenn der ESP dem Router die IP mitteilen soll.

#ifdef CONFIG
IPAddress staticIP(192, 168, 178, 99);                  // statische IP des NodeMCU ESP8266
IPAddress gateway(192, 168, 178, 1);                    // IP-Adresse des Router
IPAddress subnet(255, 255, 255, 0);                     // Subnetzmaske des Netzwerkes
IPAddress dns(192, 168, 178, 1);                        // DNS Server
#endif

const char HTML[] PROGMEM =  R"(<!DOCTYPE HTML>
<html lang="de">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      button{width:11em;height:2.5em}
      body{background: #87cefa; text-align: center;}
    </style>
    <title>Login Manager</title>
  </head>
  <body>
    <!-- style="position: absolute; height: 100%; border: none" -->
    <iframe src="ul.htm" style="position: absolute; height: 100%; border: none">
     
    </iframe>
  </body>
</html>)";
const char JSON[] PROGMEM = R"("<h3>Die Zugangsdaten wurden übertragen. Eine Verbindung zum Netzwerk wird hergestellt.</h3>")";

char ssid[33] {" "};
char password[65];
constexpr char key {129};

void connectWifi() {
  IPAddress apIP(172, 217, 28, 1);
  IPAddress netMsk(255, 255, 255, 0);
  File file = LittleFS.open("/wifi.dat", "r");
  if (file) {
    file.read(reinterpret_cast<uint8_t*>(&ssid), sizeof(ssid));
    file.read(reinterpret_cast<uint8_t*>(&password), sizeof(password));
    file.close();
    for (auto &c : ssid) c ^= key;                      // Dechiffrierung SSID
    for (auto &c : password) c ^= key;                  // Dechiffrierung Passwort
  }
  WiFi.disconnect();
  WiFi.persistent(false);                               // Auskommentieren wenn Netzwerkname und Passwort in den Flash geschrieben werden sollen.
  WiFi.mode(WIFI_STA);                                  // Station-Modus
  WiFi.begin(ssid, password);
#ifdef CONFIG
  WiFi.config(staticIP, gateway, subnet, dns);
#endif
  uint8_t i {0};
  while (WiFi.status() != WL_CONNECTED) {
    pinMode(LED_BUILTIN, OUTPUT);                       // OnBoardLed Nodemcu, Wemos D1 Mini Pro
    digitalWrite(LED_BUILTIN, 0);                       // Led blinkt während des Verbindungsaufbaus
    delay(500);
    digitalWrite(LED_BUILTIN, 1);
    delay(500);
    Serial.printf(" %i sek\n", ++i);
    if (WiFi.status() == WL_NO_SSID_AVAIL || i > 29) {  // Ist die SSID nicht erreichbar, wird ein eigenes Netzwerk erstellt.
      digitalWrite(LED_BUILTIN, 0);                     // Dauerleuchten der Led zeigt den AP Modus an.
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);                               // Soft-Access-Point-Modus
      Serial.println(PSTR("\nVerbindung zum Router fehlgeschlagen !\nStarte Soft AP"));
      WiFi.softAPConfig(apIP, apIP, netMsk);
      if (WiFi.softAP("EspConfig")) {
        Serial.println(PSTR("Verbinde dich mit dem Netzwerk \"EspConfig\".\n"));
      }
      break;
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf(PSTR("\nVerbunden mit: %s\nEsp8266 IP: %s\n"), WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    display.println(WiFi.localIP().toString().c_str());
 
  display.display();
  }
  dnsServer.start(DNS_PORT, "*", apIP);
  server.on("/wifisave", HTTP_POST, handleWifiSave);
  server.onNotFound(handleRoot);
}

void handleWifiSave() {
  if (server.hasArg("ssid") && server.hasArg("passwort")) {
    strcpy(ssid, server.arg(0).c_str());
    strcpy(password, server.arg(1).c_str());
    for (auto &c : ssid) c ^= key;                      // Chiffrierung SSID
    for (auto &c : password) c ^= key;                  // Chiffrierung Passwort
    File file = LittleFS.open("/wifi.dat", "w");
    file.write(reinterpret_cast<uint8_t*>(&ssid), sizeof(ssid));
    file.write(reinterpret_cast<uint8_t*>(&password), sizeof(password));
    file.close();
    server.send(200, "application/json", JSON);
    delay(500);
    connectWifi();
  }
}

void handleRoot() {
  if (WiFi.status() != WL_CONNECTED) {                  // Besteht keine Verbindung zur Station wird das Formular gesendet.
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    //server.send(200, "text/html", HTML);
    File f = LittleFS.open("ul.htm", "r"); server.streamFile(f, mime::getContentType("ul.htm")); f.close();
  }
  else {
    if (!handleFile(server.urlDecode(server.uri()))) {
      if (server.urlDecode(server.uri()).endsWith("/")) sendResponce();
      server.send(404, "text/plain", "FileNotFound");
    }
  }
}

void reStation() {                                      // Der Funktionsaufruf "reStation();" sollte im "loop" stehen.
  static unsigned long previousMillis;                  // Nach Stromausfall startet der Esp.. schneller als der Router.
  constexpr unsigned long INTERVAL (3e5);               // Im AP Modus aller 5 Minuten prüfen ob der Router verfügbar ist.
  if (millis() - previousMillis >= INTERVAL) {
    previousMillis += INTERVAL;
    if (WiFi.status() != WL_CONNECTED) connectWifi();
  }
}
