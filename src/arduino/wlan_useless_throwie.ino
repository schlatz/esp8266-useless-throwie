
// ****************************************************************
// Sketch Esp8266 Webserver Modular(Tab)
// created: Jens Fleischer, 2018-05-16
// last mod: Jens Fleischer, 2020-12-28
// For more information visit: https://fipsok.de
// ****************************************************************
// Hardware: Esp8266
// Software: Esp8266 Arduino Core 2.4.2 - 3.1.1
// Getestet auf: Nodemcu, Wemos D1 Mini Pro, Sonoff Switch, Sonoff Dual
/******************************************************************
  Copyright (c) 2018 Jens Fleischer. All rights reserved.

  This file is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This file is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*******************************************************************/
// Der WebServer Tab ist der Haupt Tab mit "setup" und "loop".
// #include <LittleFS.h> bzw. #include <FS.h> und #include <ESP8266WebServer.h> 
// müssen im Haupttab aufgerufen werden.
// Ein Connect Tab ist erforderlich.
// Inklusive Arduino OTA-Updates (Erfordert freien Flash-Speicher)
/**************************************************************************************/

#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>         // https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html
#include <LittleFS.h>           // Library für Dateisystem LittleFS
//#include <FS.h>               // Library für Dateisystem Spiffs einkommentieren wenn erforderlich

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// SCL GPIO5
// SDA GPIO4
#define OLED_RESET 0  // GPIO0
Adafruit_SSD1306 display(OLED_RESET);


ESP8266WebServer server(80);

#include <DNSServer.h>
const byte DNS_PORT = 53;
DNSServer dnsServer;


String sketchName() {           // Dateiname für den Admin Tab ab EspCoreVersion 2.6.0
  char file[sizeof(__FILE__)] = __FILE__;
  char * pos = strrchr(file, '.'); *pos = '\0';
  return file;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.printf("\nSketchname: %s\nBuild: %s\t\tIDE: %d.%d.%d\n%s\n\n",
                (__FILE__), (__TIMESTAMP__), ARDUINO / 10000, ARDUINO % 10000 / 100, ARDUINO % 100 / 10 ? ARDUINO % 100 : ARDUINO % 10, ESP.getFullVersion().c_str());

   // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("http://");
  display.println("192.168.");
  display.println("    4.1");
  display.display();
  delay(2000);
  setupFS();                    // setupFS(); oder spiffs(); je nach Dateisystem
  connectWifi();
//  admin();
  //bme280();
  //bme280Duo();
  //dht22();
  //ds18b20();
  espboardLed();
  //onboardLed();               // Die Tabs die du nutzen möchtest, musst du Einkommentieren
  //setupTime();
  //ds18b20list();
  //bh1750();
  //dualRelais();
  //singleTimerSwitch();

  ArduinoOTA.onStart([]() {
    //save();                   // Wenn Werte vor dem Neustart gespeichert werden sollen
  });
  ArduinoOTA.begin();
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  reStation();
  ArduinoOTA.handle();
  server.handleClient();
  if (millis() < 0x2FFF || millis() > 0xFFFFF0FF) {    // Die Funktion "runtime()" wird nur für den Admin Tab gebraucht.
//    runtime();                                         // Auskommentieren falls du den Admin Tab nicht nutzen möchtest.
  }
}
