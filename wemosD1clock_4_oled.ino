// ****************************************************************
// Sketch Esp8266 WiFi NTP Lokalzeit und UTC
// created: Jens Fleischer, 2019-05-05
// last mod: Jens Fleischer, 2020-07-16
// For more information visit: https://fipsok.de
// ****************************************************************
// Hardware: Esp8266
// Software: Esp8266 Arduino Core 2.6.0 - 2.7.4
// Getestet auf: Nodemcu, Wemos D1 Mini Pro, Sonoff Switch, Sonoff Dual
/******************************************************************
  Copyright (c) 2019 Jens Fleischer. All rights reserved.

  This file is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This file is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*******************************************************************/
// Automatische Umstellung zwischen Sommer- und Normalzeit
/**************************************************************************************/

#include <ESP8266WiFi.h>
#include <time.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//String hours, minutes, seconds;
int outputSecond, outputMinute, outputHour;
long lastSecond = millis();

struct tm lt;         // http://www.cplusplus.com/refrence/ctime/tm/
struct tm utc;

const char* const PROGMEM NTP_SERVER[] = {"fritz.box", "de.pool.ntp.org", "at.pool.ntp.org", "ch.pool.ntp.org", "ptbtime1.ptb.de", "europe.pool.ntp.org"};
const char* const PROGMEM DAY_NAMES[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
const char* const PROGMEM DAY_SHORT[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};
const char* const PROGMEM MONTH_NAMES[] = {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
const char* const PROGMEM MONTH_SHORT[] = {"Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"};

const char* ssid = "unhb.de";             // << kann bis zu 32 Zeichen haben
const char* password = "wurstwasser";  // << mindestens 8 Zeichen jedoch nicht länger als 64 Zeichen

#define PIN D4
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN);

void setupTime() {                              // deinen NTP Server einstellen (von 0 - 5 aus obiger Liste) alternativ lassen sich durch Komma getrennt bis zu 3 Server angeben
  configTime("CET-1CEST,M3.5.0,M10.5.0/3", NTP_SERVER[1]);  // Zeitzone einstellen https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
  // für Core Versionen vor 2.6.0. folgende Konfiguration verwenden.
  //configTime(0, 0, NTP_SERVER[1]);
  //setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.printf("\n\nSketchname: %s\nBuild: %s\t\tIDE: %d.%d.%d\n%s\n\n",
                (__FILE__), (__TIMESTAMP__), ARDUINO / 10000, ARDUINO % 10000 / 100, ARDUINO % 100 / 10 ? ARDUINO % 100 : ARDUINO % 10, ESP.getFullVersion().c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nVerbunden mit: " + WiFi.SSID());
  setupTime();
  delay(250);

  strip.begin();
  strip.setBrightness(28);
  // brightness = 0...255
  strip.show();

  // initialize with the I2C addr 0x78 / mit I2C-Adresse 0x78 initialisieren
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  
  // random start seed / zufälligen Startwert für Random-Funtionen initialisieren
  randomSeed(analogRead(0));

}

#define DRAW_DELAY 118
#define D_NUM 47

void loop() {
  static char buf[30];                                    // je nach Format von "strftime" eventuell anpassen
  static time_t lastsec {0};
  time_t now = time(&now);
  localtime_r(&now, &lt);
  gmtime_r(&now, &utc);
  if (lt.tm_sec != lastsec) {
    lastsec = lt.tm_sec;
    if (!(time(&now) % 86400)) {                          // einmal am Tag die Zeit vom NTP Server holen o. jede Stunde "% 3600" aller zwei "% 7200"
      setupTime();
    }
  }
    strftime (buf, sizeof(buf), "%d.%m.%Y %T %A", &utc);  // http://www.cplusplus.com/reference/ctime/strftime/
    Serial.printf("Koordinierte  Weltzeit %s\n", buf);
    strftime (buf, sizeof(buf), "%d.%m.%Y %T", &lt);      // http://www.cplusplus.com/reference/ctime/strftime/
    Serial.printf("Mitteleuropäische Zeit %s %s\n", buf, DAY_NAMES[utc.tm_wday]);
    Serial.printf("UTC: %.2d:%.2d:%.2d\n", utc.tm_hour, utc.tm_min, utc.tm_sec);
    Serial.printf("%s: %.2d:%.2d:%.2d %s\n\n", *tzname, lt.tm_hour, lt.tm_min, lt.tm_sec, lt.tm_isdst ? "Sommerzeit" : "Normalzeit");

    if ((millis() - lastSecond) > 1000)
      {
        strip.setPixelColor(outputSecond / 1, 0, 0, 0);
        strip.setPixelColor(outputMinute / 1, 0, 0, 0);
        strip.setPixelColor(outputHour * 5, 0, 0, 0);

    /*String currentTime = String(currentHour) + ':' + String(currentMinute) + ':' + String(currentSecond);
    Serial.println(currentTime);
    Serial.println(currentHour);
    Serial.println(currentMinute);
    Serial.println(currentSecond); */

    outputSecond = lt.tm_sec;
    outputMinute = lt.tm_min;
    outputHour = lt.tm_hour;
    if (outputHour > 12) outputHour = outputHour - 12;
    
    if (lt.tm_sec > 59)
    { outputSecond = 0;
      outputSecond++;
      if (lt.tm_min > 59) {
        outputMinute = 0;
        outputHour++;
        if (lt.tm_hour > 12) outputHour = 0;
      }
    }

    strip.setPixelColor(outputSecond / 1, 0, 0, 255); //RGB   Sekunden (blau)
    strip.setPixelColor(outputMinute / 1, 0, 255, 0); // Minuten (gruen)
    strip.setPixelColor(outputHour * 5, 255, 0, 0);   // Stunden (rot)
    strip.show();

    //Serial.printf("%.2d:%.2d:%.2d", lt.tm_hour, lt.tm_min, lt.tm_sec);
    Serial.printf("%.2d", outputHour);
    Serial.printf("%.2d", outputMinute); 
    Serial.printf("%.2d", outputSecond);
      }

   
    display.clearDisplay();
    // set text color / Textfarbe setzen
    display.setTextColor(WHITE);
    // set text size / Textgroesse setzen
    display.setTextSize(1);
    // set text cursor position / Textstartposition einstellen
    display.setCursor(1,0);
    // show text / Text anzeigen
    display.println("    aktuelle Zeit");
    display.println("---------------------");
    display.setTextSize(2);
    strftime (buf, sizeof(buf), " %H:%M:%S", &lt);
    display .println(buf);
    display.println("----------");
    display.setTextSize(2);
    strftime (buf, sizeof(buf), "%d.%m.%Y", &lt);
    display.println(buf);
    display.display();
    delay(250);
    display.clearDisplay();
   
   
}
