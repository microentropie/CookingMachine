/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

Main project entry.
The following libraries are required:
* https://github.com/microentropie/Sort
* https://github.com/microentropie/serializeEeprom
* https://github.com/microentropie/DateTimeManipulation

Sources repository: https://github.com/microentropie/
*/

#include <TimeZone.h>

#include "common.h"
#include "common_io.h"
#include "WebServer.h"
#include "WiFi.h"

#include "machine_cycle.h"
#include "settings.h"
#include "src/time/TimeNTP.h"

#include "ui.h"
#include "src/button/button.h"

MachineCycle machine;

extern bool loadWiFiConfig();
extern void Localization_init();
extern void ntp_init();

extern WiFiMode wifiMode;
void setup()
{
  Serial.begin(UART_BAUDRATE);
#ifdef VERBOSE_SERIAL
  Serial.setDebugOutput(true); // enable system log to UART0
#endif //VERBOSE_SERIAL

  Serial.println();
  Serial.println();
  Serial.println("------------ FRESH FROM RESET :-) ------------");
  Serial.println(F(__FILE__));
  Serial.println(String(F(__DATE__)) + " " + String(F(__TIME__)));
  //Serial.println(F(__TIMESTAMP__));
  Serial.println(COPYRIGHTtxt);
  Serial.print(F("This is: "));  Serial.println(PublicName("CookingMachine"));
  Serial.print(F("CpuFreqMHz:        ")); Serial.println(ESP.getCpuFreqMHz());
  Serial.print(F("FlashChipRealSize: ")); Serial.println(ESP.getFlashChipRealSize());
  Serial.print(F("FreeHeap:          ")); Serial.println(ESP.getFreeHeap());
  Serial.print(F("FreeSketchSpace:   ")); Serial.println(ESP.getFreeSketchSpace());
  Serial.print(F("SketchSize:        ")); Serial.println(ESP.getSketchSize());
  Serial.println(F("Reset info:"));
  Serial.println(ESP.getResetInfo());
  Serial.println("----------------------------------------------");
  Serial.println();
  Serial.flush();

  machine.Init();
  //ui_init();
  yield();

  //ui_splashScreen();

  bool ok = loadWiFiConfig();

  if (!ok || ui_getSetInitialMode() == 2) // config error o boot with button ON ?
    wifiMode = WIFI_AP;
  else
    wifiMode = WIFI_STA;
  Console_init();
  //Console_print("This is: ");
  //Console_println(PublicName(NULL));

  Serial.print("WiFiMode=");
  Serial.println(WiFiModeToString(wifiMode));

  // Connect to WiFi network or create an AP
  wifiMode = setupWiFi(wifiMode); // if unable to connect to an AP, becomes an AP itself

  setupWebServer();
  yield();

  Localization_init();
  yield();

  ntp_init();
  yield();

  // interrupts:
  ButtonInit(BUTTON_PIN);
}

void loop()
{
  loopWiFi();
  loopWebServer();
  machine.UpdateStatus();
  NtpTime_loop();

  loopButton();

  // ESP blue LED will flash as a good health indicator
  if (wifiMode == WIFI_STA)
    machineIOs.SetLeds(noChange, noChange, (((millis() / 125) & 7) == 0) ? On : Off); // 1 Hz blink with 12.5% duty cycle
  else
    machineIOs.SetLeds(noChange, noChange, (((millis() / 125) & 7) != 0) ? On : Off); // 1 Hz blink with 87.5% duty cycle
  machine.ModulateSound(((millis() / 63) & 7) == 0); // 2 Hz blink with 12.5% duty cycle (1.984... Hz)

  //ui_ScreenSaver();
  yield();
}

