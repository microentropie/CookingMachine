/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

web server

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

#include "common.h"
#include "settings.h"
//#include "ui.h"
#include "HttpUtils.h"

// server authentication credentials:
static char ota_relativeUrl[20];// /*PROGMEM*/ = "/esp8266httpupdate"; //"/update";

extern WiFiMode wifiMode;

ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;
extern baseConnectionConfig cfg;

extern void handleConfigRoot();
extern void handleConfigWifi();
extern void handleConfigWifiSave();
extern void handleConfigNtc();
extern void handleConfigNtcSave();
extern void handleConfigLocalization();
extern void handleConfigLocalizationSave();
extern void handleConfigNtp();
extern void handleConfigNtpSave();
extern void handleConfigRestart();
extern void handleShutDown();

//
extern void cookingMachineHandler();
//
extern void hwtestHandler();
//
extern void infoHandler();
extern void logDetails();

void SendHeaderAndHead(ESP8266WebServer &webServer, int refreshSec, char *title)
{
  webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer.sendHeader("Pragma", "no-cache");
  webServer.sendHeader("Expires", "-1");
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.sendContent("<html><head>");
  if (refreshSec > 0)
    webServer.sendContent(String() + "<meta http-equiv='refresh' content='" + String(refreshSec) + "' />");
  webServer.sendContent(String() + F(
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<style>h2, h3{ padding-bottom: 0px; margin-bottom: 1px; }</style>"
    "<meta http-equiv='Content-Type' content='text/html; charset=iso-8859-1'>"
    "<link href='https://fonts.googleapis.com/css?family=Roboto:400,300,500' rel='stylesheet' type='text/css'>"
    "<title>") + String(title) + F("</title>"
      "</head>")
  );
}

void notFoundHandler()
{
  Serial.println(F("notFoundHandler():"));
  webServer.send(404, F("text/plain"), String("404 Not found\r\n") + webServer.uri());
  logDetails();
}



void forbiddenHandler()
{
  Serial.println(F("forbiddenHandler():"));
  webServer.send(403, "text/plain", "403 Forbidden");
  logDetails();
}



void badRequestHandler()
{
  Serial.println(F("badRequestHandler():"));
  webServer.send(400, "text/plain", "400 Bad Request");
  logDetails();
}


void handleConfigurationNotAllowed()
{
  Serial.println(F("handleConfigurationNotAllowed():"));
  logDetails();

  SendHeaderAndHead(webServer, -1, PublicName(NULL));
  webServer.sendContent(F(
    "<body>"
    "<h2>Cooking Machine config</h2>"
    "Cannot enter maintenance mode:<br>"
    "machine is currently working !<br><br>"
    "<a href='/'>Home</a>"
  ));
  webServer.sendContent(COPYRIGHT);
  webServer.sendContent(F("</body></html>"));
  webServer.client().stop(); // Stop is needed because we sent no content length
}

extern bool IsMachineInASafeState();
void handleUpdate()
{
  if (!isLocalAddress(webServer.client().remoteIP()))
    return forbiddenHandler(); // update only possible if locally connected
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
    return webServer.requestAuthentication();
  if (!IsMachineInASafeState())
    return handleConfigurationNotAllowed(); // machine must be idle

  Serial.println(F("handleUpdate():"));
  logDetails();

  // redirect to /esp8266httpupdate
  webServer.sendHeader(F("Location"), ota_relativeUrl, true);
  webServer.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  webServer.sendHeader(F("Pragma"), F("no-cache"));
  webServer.sendHeader(F("Expires"), F("-1"));
  webServer.send(302, F("text/plain"), F(""));  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.client().stop(); // Stop is needed because we sent no content length
}


void RedirectTo(char *url)
{
  webServer.sendHeader(F("Location"), url, true);
  webServer.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  webServer.sendHeader(F("Pragma"), F("no-cache"));
  webServer.sendHeader(F("Expires"), "-1");
  webServer.send(302, F("text/plain"), "");  // Empty content inhibits Content-length header so we have to close the socket ourselves.
  webServer.client().stop(); // Stop is needed because we sent no content length
}

String methodToString(HTTPMethod method)
{
  switch (method)
  {
  case HTTP_GET:
    return "GET";
  case HTTP_POST:
    return "POST";
  case HTTP_PUT:
    return "PUT";
  case HTTP_PATCH:
    return "PATCH";
  case HTTP_DELETE:
    return "DELETE";
  }
  return "Unknown";
}

void logDetails()
{
#ifdef VERBOSE_SERIAL
  Serial.println(String(F("URL is: ")) + webServer.uri());
  Serial.println(String(F("HTTP Method on request was: ")) +
    methodToString(webServer.method()));
  // Print how many properties we received and then print their names
  // and values.
  Serial.println(String(F("Number of query properties: ")) + String(webServer.args()));
  int i;
  for (i = 0; i < webServer.args(); i++)
    Serial.println(" - " + webServer.argName(i) + " = " + webServer.arg(i));
#endif //VERBOSE_SERIAL
}
//#include "esp8266_peri.h"
void setupWebServer()
{
  unsigned long randNumber = RANDOM_REG32; //ESP8266_DREG(0x20E44)
  snprintf(ota_relativeUrl, sizeof(ota_relativeUrl), "/U%u", randNumber);

  webServer.on("/", cookingMachineHandler);
  webServer.on("/config", handleConfigRoot);
  webServer.on("/configwifi", handleConfigWifi);
  webServer.on("/configwifisave", handleConfigWifiSave);
  webServer.on("/configt", handleConfigNtc);
  webServer.on("/configtsave", handleConfigNtcSave);
  webServer.on("/configlocaliz", handleConfigLocalization);
  webServer.on("/configlocalizsave", handleConfigLocalizationSave);
  webServer.on("/configntp", handleConfigNtp);
  webServer.on("/configntpsave", handleConfigNtpSave);
  webServer.on("/info", infoHandler);
  webServer.on("/hwtest", hwtestHandler);
  webServer.on("/update", handleUpdate);
  webServer.on("/config-restart-now", handleConfigRestart);
  webServer.on("/ShutDown", handleShutDown);
  webServer.on("/favicon.ico", []()
  {
    webServer.send(404, "text/plain", "");
  });
  webServer.onNotFound(notFoundHandler);

  //if (wifiMode == WIFI_STA) // temp, remove:
  //if(wifiMode == WIFI_AP)
  httpUpdater.setup(&webServer, ota_relativeUrl, cfg.userName, cfg.userPassword);
  webServer.begin();
  Serial.println("Web Server is on");
  if (wifiMode == WIFI_STA) // temp, remove:
  //if(wifiMode == WIFI_AP)
  {
    Serial.print("OTA updater at: ");
    Serial.println(ota_relativeUrl);
  }
}

void loopWebServer()
{
  webServer.handleClient();
}
