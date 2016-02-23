/*

    This file is part of KBox.

    Copyright (C) 2016 Thomas Sarlandie.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include "NetServer.h"

uint8_t neopixel_pin = 4;

Adafruit_NeoPixel rgb = Adafruit_NeoPixel(1, neopixel_pin, NEO_GRB + NEO_KHZ800);
NetServer server(4242);

static const uint32_t connectedColor = rgb.Color(0x00, 0xff, 0x00);
static const uint32_t readyColor = rgb.Color(0x00, 0x00, 0xff);

void setup() {
  rgb.begin();
  rgb.setPixelColor(0, 0, 0, 0xff);
  rgb.show();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("KBox");

  Serial.begin(115200);
  Serial.setTimeout(0);
  Serial.println("Starting kbox ...");
}

uint8_t buffer[4096];

void loop() {
  server.loop();

  if (Serial.available()) {
    int read = Serial.readBytes(buffer, sizeof(buffer));
    server.writeAll(buffer, read);
  }

  if (server.clientsCount() > 0) {
    rgb.setPixelColor(0, connectedColor);
  }
  else {
    rgb.setPixelColor(0, readyColor);
  }
  rgb.show();
}
