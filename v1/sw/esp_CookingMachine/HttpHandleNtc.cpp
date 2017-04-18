/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

http page handler, temperature configuration

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WebServer.h>
#include <sort_bubble.h>

#include "machine_io.h"
#include "common.h"
#include "settings.h"
#include "temperature_processing_base.h"

//extern MachineCycle machine;
extern MachineIo machineIOs;
extern WiFiMode wifiMode;

extern ESP8266WebServer webServer;

extern baseConnectionConfig cfg;

#include "WebServer.h"
#include "HttpUtils.h"

static String errMsg;

void handleConfigNtc()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    forbiddenHandler(); // configuration modifications only possible if locally connected

  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();

  Serial.println("handleConfigNtc():");
  logDetails();

  SendHeaderAndHead(webServer, -1, PublicName(NULL));
  webServer.sendContent(
    "<body>"
    "<h2>Cooking Machine config</h2>"
  );

  if (errMsg != NULL && errMsg.length() > 0)
    webServer.sendContent(String("<p style='color:red'>") + errMsg + String("</p>"));

  webServer.sendContent(
    "\r\n<br /><form method='POST' action='configtsave'>"
    "NTC temperature probe setPoints"
    "<table><tr><th>n.</th><th>adc</th><th>temp (&deg;C x 10)</th></tr>"
  );
  for (int i = 0; i < MAX_TEMP_SETPOINTS; ++i)
    webServer.sendContent(String() + "<tr><td>" + String(i + 1) + "</td><td>" +
      "<input type='text' placeholder='tool tip' name='adc" + String(i) + "' value = '" + String(NtcTable[i].adc) + "'/></td><td>" +
      "<input type='text' placeholder='tool tip' name='temp" + String(i) + "' value = '" + String(NtcTable[i].temp) + "'/></td></tr>");
  webServer.sendContent(
    "</table>\r\n<br />"
    "To add a new setPoint: write it in any free (adc=0) table entry, will be autom. sorted when Saving<br />"
    "To remove an entry: set its adc=0, will be autom. removed when Saving<br />"
    "<br />"
    "<br />"
    "<input type='submit' name='Cancel' value='Cancel'/>&nbsp;&nbsp;&nbsp;"
    "<input type='submit' name='Save' value='Save'/>&nbsp;&nbsp;&nbsp;"
    "<input type='submit' name='Default' value='Load Defaults'/>"
    "</form>"
    //"<p>return the <a href='/'>home page</a></p>"
    //"<p>restart the machine <a href='/'>NOW</a></p>"
  );
  webServer.sendContent(COPYRIGHT);
  webServer.sendContent("</body></html>");
  webServer.client().stop(); // Stop is needed because we sent no content length
}

/* Handle the WLAN save form and redirect to WLAN config page again */
void handleConfigNtcSave()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    forbiddenHandler(); // configuration modifications only possible if locally connected

  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();

  Serial.println("handleConfigNtcSave():");
  logDetails();
  if (webServer.args() != 33)
  {
    Serial.print("Bad number of args; args=");
    Serial.println(webServer.args());
    badRequestHandler();
    return;
  }
  errMsg = String("");

  if (webServer.hasArg("Defaults"))
  {
    // reset NTC parameters to the default ones
  }
  if (webServer.hasArg("Save"))
  {
    ntcTranslationAdcTemp *NtcTableTemp[MAX_TEMP_SETPOINTS];
    int qtNtcElemTemp = 0;
    for (int i = 0; i < MAX_TEMP_SETPOINTS; ++i)
      NtcTableTemp[i] = new ntcTranslationAdcTemp();

    for (int i = 0; i < MAX_TEMP_SETPOINTS; ++i)
    {
      NtcTableTemp[i]->adc = webServer.arg("adc" + String(i)).toInt();
      NtcTableTemp[i]->temp = webServer.arg("temp" + String(i)).toInt();
      if (NtcTableTemp[i]->adc == 0)
        NtcTableTemp[i]->temp = 0; // clear empty setPoints
      if (NtcTableTemp[i]->adc > 0)
        ++qtNtcElemTemp;
    }
    bool bOk = qtNtcElemTemp > 2;
    errMsg = String("ERROR: must have at least 4 setPoints");
    if (bOk)
    {
      bubbleSort<ntcTranslationAdcTemp>(NtcTableTemp, MAX_TEMP_SETPOINTS, TempEntryCompare); // does not use stack
      // check is duplicate entries exist;
      for (int i = 1; i < MAX_TEMP_SETPOINTS; ++i)
      {
        if (NtcTableTemp[i - 1]->adc == NtcTableTemp[i]->adc && NtcTableTemp[i]->adc > 0)
        {
          bOk = false;
          errMsg = String("ERROR Save aborted, latest valid setPoints reloaded:<br> 2+ adc=") + String(NtcTableTemp[i]->adc) + " values were inserted !";
          break;
        }
      }
    }
    if (bOk)
    {
      errMsg = String("");
      // update table in memory and save
      for (int i = 0; i < MAX_TEMP_SETPOINTS; ++i)
        NtcTable[i] = *NtcTableTemp[i];

      TempSetPointsSave(NULL);

      Serial.print("  debuginfo (saveNtc) qtPoints: ");//debug
      Serial.println(qtNtcElemTemp);
    }
    else
    {
      Serial.println(errMsg);
    }

    for (int i = 0; i < MAX_TEMP_SETPOINTS; ++i) // clean memory
      delete NtcTableTemp[i];
  }
  // redirect to /configt
  webServer.sendHeader("Location", webServer.hasArg("Cancel") ? "config" : "configt", true);
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.send(302, "text/plain", "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.client().stop(); // Stop is needed because we sent no content length
}

