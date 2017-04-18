/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

http page handler, system info

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <DateTime.h>

#include "machine_cycle.h"
#include "temperature_processing.h"
#include "common.h"
#include "settings.h"
#include "HttpUtils.h"

extern ESP8266WebServer webServer;
extern baseConnectionConfig cfg;
extern MachineIo machineIOs;

#include "WebServer.h"
#include "enumsToString.h"

extern bool IsMachineInASafeState();


char *FlashModeToString(FlashMode_t m)
{
  switch (m)
  {
  case FM_QIO:
    return "QIO";
  case FM_QOUT:
    return "QOUT";
  case FM_DIO:
    return "DIO";
  case FM_DOUT:
    return "DOUT";
  case FM_UNKNOWN:
    return "UNKNOWN";
  }
  return "?";
}

void infoHandler()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    forbiddenHandler(); // configuration modifications only possible if locally connected

  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();

  char h[2000];

  Serial.println(F("infoHandler():"));
  logDetails();
  SendHeaderAndHead(webServer, -1, "C.M. System Info");

  String s = "<tr><td>machine name</td><td>" + String(PublicName(NULL)) + "</td></tr>";
  s += "<tr><td>Source compiled</td><td>" + String(__DATE__) + " " + String(__TIME__) + "</td></tr>";
  s += "<tr><td>System started</td><td>" + String(millis()) + " ms ago (counter overflows every 71')</td></tr>";

  DateTime::UtcIso8601DateTime(h, sizeof(h), time(NULL), true);
  s += "<tr><td>Now</td><td>" + String(h) + "</td></tr>";

  uint32_t ui32;
  uint16_t ui16;
  uint8_t ui8;

  //ADC_MODE(ADC_VCC); set at startup and at out of any function if you want to use:
  //uint16_t ui16 = ESP.getVcc();
  ui16 = analogRead(A0);
  s += "<tr><td>analogRead(A0)</td><td>" + String(ui16) + "</td></tr>";

  ui8 = ESP.getBootMode();
  s += "<tr><td>getBootMode()</td><td>" + String(ui8) + "</td></tr>";

  ui8 = ESP.getBootVersion();
  s += "<tr><td>getBootVersion()</td><td>" + String(ui8) + "</td></tr>";

  ui32 = ESP.getChipId();
  s += "<tr><td>getChipId()</td><td>" + String(ui32) + "</td></tr>";

  ui8 = ESP.getCpuFreqMHz();
  s += "<tr><td>getCpuFreqMHz()</td><td>" + String(ui8) + "</td></tr>";

  ui32 = ESP.getCycleCount();
  s += "<tr><td>getCycleCount()</td><td>" + String(ui32) + "</td></tr>";

  ui32 = ESP.getFlashChipId();
  s += "<tr><td>getFlashChipId()</td><td>" + String(ui32) + "</td></tr>";

  FlashMode_t fm = ESP.getFlashChipMode();
  s += "<tr><td>getFlashChipMode()</td><td>" + String(FlashModeToString(fm)) + "</td></tr>";

  ui32 = ESP.getFlashChipRealSize();
  s += "<tr><td>getFlashChipRealSize()</td><td>" + String(ui32) + "</td></tr>";

  ui32 = ESP.getFlashChipSize();
  s += "<tr><td>getFlashChipSize()</td><td>" + String(ui32) + "</td></tr>";

  ui32 = ESP.getFlashChipSizeByChipId();
  s += "<tr><td>getFlashChipSizeByChipId()</td><td>" + String(ui32) + "</td></tr>";

  ui32 = ESP.getFlashChipSpeed();
  s += "<tr><td>getFlashChipSpeed()</td><td>" + String(ui32) + "</td></tr>";

  ui32 = ESP.getFreeHeap();
  s += "<tr><td>getFreeHeap()</td><td>" + String(ui32) + "</td></tr>";

  ui32 = ESP.getFreeSketchSpace();
  s += "<tr><td>getFreeSketchSpace()</td><td>" + String(ui32) + "</td></tr>";

  s += "<tr><td>getResetInfo()</td><td>" + ESP.getResetInfo() + "</td></tr>";

  const char *p = ESP.getSdkVersion();
  s += "<tr><td>getSdkVersion()</td><td>" + String(p) + "<br>";

  ui32 = ESP.getSketchSize();
  s += "<tr><td>getSketchSize()</td><td>" + String(ui32) + "<br>";

  snprintf(h, sizeof(h),
    "<body bgcolor='#%02X%02X%02X'>"
    "<h3>C.M. System Info</h3>"
    "<table>"
    "%s"
    "</table>"
    "<br /><a href='/config'>Cancel</a><br />"
    "%s"
    "</body>"
    "</html>"
    ,
    0xB0, 0xFF, 0xB0,
    s.c_str(),
    COPYRIGHT);

#ifdef VERBOSE_SERIAL
  Serial.println(h);
  logDetails();
#endif //VERBOSE_SERIAL
  webServer.sendContent(h);
  webServer.client().stop(); // Stop is needed because we sent no content length
}



void hwtestHandler()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // configuration modifications only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeState())
    return handleConfigurationNotAllowed();

  Serial.println("hwtestHandler():");
  logDetails();
  SendHeaderAndHead(webServer, -1, "C.M. HW TEST");

  OutSetAction_e mainRelayVal, hotRelayVal, pumpVal;

  mainRelayVal = OutSetAction_e::noChange;
  hotRelayVal = OutSetAction_e::noChange;
  pumpVal = OutSetAction_e::noChange;

  bool bSet = false;
  if (webServer.args() > 0)
  {
    for (int i = 0; i < webServer.args(); i++)
    {
      if (webServer.argName(i) == String("update"))
      {
        bSet = (webServer.arg(i) == String("SET"));
      }
      else if (webServer.argName(i) == String("mainRelay"))
      {
        mainRelayVal = (webServer.arg(i) == String("ON")) ? OutSetAction_e::On : OutSetAction_e::Off;
      }
      else if (webServer.argName(i) == String("hotRelay"))
      {
        hotRelayVal = (webServer.arg(i) == String("ON")) ? OutSetAction_e::On : OutSetAction_e::Off;
      }
      else if (webServer.argName(i) == String("Pump"))
      {
        pumpVal = (webServer.arg(i) == String("ON")) ? OutSetAction_e::On : OutSetAction_e::Off;
      }
    }
  }
  if (bSet)
  {
#ifdef VERBOSE_SERIAL
    Serial.print("SetRelays: Main=>");
    Serial.print(OutSetActionToString(mainRelayVal));
    Serial.print(", Hot=>");
    Serial.print(OutSetActionToString(hotRelayVal));
    Serial.print(", Pump=>");
    Serial.println(OutSetActionToString(pumpVal));
#endif //VERBOSE_SERIAL
    machineIOs.SetRelays(mainRelayVal, hotRelayVal, pumpVal);
  }

  machineIOs.GetRelays(mainRelayVal, hotRelayVal, pumpVal);

  int adc = machineIOs.GetTemperature();

  char s[1500];
  snprintf(s, sizeof(s),
    "<body bgcolor='#%02X%02X%02X'>"
    "<h2>WARNING this page gives direct control to hardware I/Os !<br>"
    "this may result in damages to the machine, persons and the environment</h2>"
    "<b><a href='/config'>Go back now</a> if not sure on how to proceed.<b><br>"
    "<h3>Actions</h3>"
    "<br>"
    "<form method='post' action='/hwtest'>"
    "<table>"
    "<tr><td>Main Relay</td><td><input type='radio' name='mainRelay' value='ON' %s>set ON&nbsp;<input type='radio' name='mainRelay' value='OFF' %s>set OFF</td></tr>"
    "<tr><td>Hot/Warm Relay</td><td><input type='radio' name='hotRelay' value='ON' %s>set ON&nbsp;<input type='radio' name='hotRelay' value='OFF' %s>set OFF</td></tr>"
    "<tr><td>Pouring Pump</td><td><input type='radio' name='Pump' value='ON' %s>set ON&nbsp;<input type='radio' name='Pump' value='OFF' %s>set OFF</td></tr>"
    //"<tr><td>Led R G B</td><td><input name='R' value='%d' size='4'>&nbsp;<input name='G' value='%d' size='4'>&nbsp;<input name='B' value='%d' size='4'></td></tr>"
    "<tr><td>Temperature</td><td>adc=%d (%s &deg;C)</td></tr>"
    "</table>"
    "<br>"
    "<a href='/hwtest'>click to refresh</a>"
    "<br><br>"
    "<input type='submit' name='update' value='SET'>"
    "</form>"
    "</div>"
    "%s"
    "</body>"
    "</html>"
    , 0xAD, 0xD8, 0xE6, // form background color
    (mainRelayVal == OutSetAction_e::On) ? "checked" : "", (mainRelayVal == OutSetAction_e::On) ? "" : "checked",
    (hotRelayVal == OutSetAction_e::On) ? "checked" : "", (hotRelayVal == OutSetAction_e::On) ? "" : "checked",
    (pumpVal == OutSetAction_e::On) ? "checked" : "", (pumpVal == OutSetAction_e::On) ? "" : "checked",
    //0, 0, 0, // Led R G B
    adc, TemperatureToString(TempConvertAdc2Temp(adc)).c_str(),
    COPYRIGHT);
#ifdef VERBOSE_SERIAL
  Serial.println(s);
#endif //VERBOSE_SERIAL
  webServer.sendContent(s);
  webServer.client().stop(); // Stop is needed because we sent no content length
}

