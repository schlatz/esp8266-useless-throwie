// ****************************************************************
// Sketch Esp8266 DualRelais Modular(Tab)
// created: Jens Fleischer, 2018-09-11
// last mod: Jens Fleischer, 2020-08-23
// For more information visit: https://fipsok.de
// ****************************************************************
// Hardware: Esp8266, 2-Kanal Relais Modul, Logic Level MOSFET oder SSR
//
// für Relais Modul
// GND an GND
// IN1 an D5 = GPIO14
// IN2 an D6 = GPIO12
// VCC an VIN -> je nach verwendeten Esp.. möglich
// Jumper JD-VCC VCC
// alternativ ext. 5V Netzteil verwenden
//
// für Mosfet IRF3708
// Source an GND
// Mosfet1 Gate an D5 = GPIO14
// Mosfet2 Gate an D6 = GPIO12
//
// oder 3V Solid State Relais
// SSR1 Input - an GND
// SSR1 Input + an D5 = GPIO14
// SSR2 Input - an GND
// SSR2 Input + an D6 = GPIO12
//
// Software: Esp8266 Arduino Core 2.4.2 - 3.1.0
// Getestet auf: Nodemcu
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
// Diese Version von DualRelais sollte als Tab eingebunden werden.
// #include <ESP8266WebServer.h> muss im Haupttab aufgerufen werden
// Die Funktionalität des ESP8266 Webservers ist erforderlich.
// Die Funktion "dualRelais();" muss im Setup aufgerufen werden.
/**************************************************************************************/

void dualRelais() {
  const auto aktiv = LOW;                    // LOW für LOW aktive Relais oder HIGH für HIGH aktive (zB. SSR, Mosfet) einstellen
  const uint8_t relPin[] = {D4, D5};         // Pin für Relais eventuell einstellen
  
  for (auto pin : relPin) {
    digitalWrite(pin, !aktiv);
    pinMode(pin, OUTPUT);
  }
  server.on("/duorel", [=]() {
    if (server.arg(0) == "1") {
      digitalWrite(relPin[0], !digitalRead(relPin[0]));    // Relais umschalten
    }
    if (server.arg(0) == "2") {
      digitalWrite(relPin[1], !digitalRead(relPin[1]));    // Relais umschalten
    }
    Serial.printf("Relais 1 ist %s \tRelais 2 ist %s\n", digitalRead(relPin[0]) == aktiv ? "An" : "Aus", digitalRead(relPin[1]) == aktiv ? "An" : "Aus");
    server.send(200, "application/json", "[\"" + static_cast<String>(digitalRead(relPin[0]) == aktiv) + "\",\"" + static_cast<String>(digitalRead(relPin[1]) == aktiv) + "\"]");
  });
}
