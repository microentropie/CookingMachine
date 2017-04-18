/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

WiFi initialization and handling

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#ifdef _USE_UPNP_
#include <ESP8266SSDP.h>
#endif //_USE_UPNP_

#include "common.h"
#include "settings.h"
#include "machine_io.h"

#include "ui.h"

extern baseConnectionConfig cfg;

WiFiMode wifiMode = WIFI_STA;

static MDNSResponder mdns;

void setupWiFiAP()
{
  if (WiFi.getMode() != WIFI_AP)
  {
    WiFi.mode(WIFI_AP);
    delay(100);
  }
  Serial.print("Starting AP ... ");
  Console_println("Config Mode");
  Console_println("Starting AP ...");

  const char *p = PublicName(NULL);
  WiFi.softAPConfig(cfg.ApIp, cfg.ApIp, IPAddress(255, 255, 255, 0));
  WiFi.softAP(p, cfg.ApPassword); // pw MUST be min 8 max 63 chars
  delay(10);
  Serial.println("running.");
  Serial.print("ssid: '");
  Serial.print(p);
  Serial.print("', pw: '");
  Serial.print(cfg.ApPassword);
  Serial.print("', ip: ");
  Serial.println(WiFi.softAPIP());

  Console_println("Connect to ssid:");
  Console_println(p);
  Console_println("Then visit:");
  Console_println(WiFi.softAPIP().toString() + "/");
}

bool setupWiFiStation()
{
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
    delay(100);
  }
  const char *Msg[] =
  {
    "Connecting to ",
  };
  Console_println(Msg[0]);
  Console_println(cfg.ssid);

  const char *p = PublicName(NULL);
  WiFi.hostname(p);
  Serial.print(String(Msg[0]) + "'" + String(cfg.ssid) + "',");
#ifdef VERBOSE_SERIAL
  Serial.print(" password: '");
  Serial.print(cfg.password);
  Serial.print("'");
#else VERBOSE_SERIAL
#endif VERBOSE_SERIAL

  WiFi.begin(cfg.ssid, cfg.password);
  if (cfg.bUseStaticIp)
    WiFi.config(cfg.ip, cfg.gateway, cfg.subnet); // force a fixed IP
  yield(); //delay(10);
  if (cfg.bUseStaticIp)
  {
    Serial.print(" forcing fixed IP: ");
    Serial.print(cfg.ip);
    Serial.print(" (gw: ");
    Serial.print(cfg.gateway);
    Serial.print(", mask: ");
    Serial.print(cfg.subnet);
    Serial.print(")");
  }
  Serial.println();
  Serial.flush();
  // Wait for connection
  int timeout = 60; // 30" timeout
  while (timeout >= 0 && WiFi.status() != WL_CONNECTED)
  {
    machineIOs.SetLeds(noChange, noChange, (timeout & 1) ? On : Off); // 1 Hz blink with 50% duty cycle
    delay(500);
#ifdef VERBOSE_SERIAL
    Serial.print('[');
    Serial.print(WiFi.status()),
      Serial.print(']'),
      Console_print(".");
#else VERBOSE_SERIAL
    Serial.print(".");
    Console_print(".");
#endif VERBOSE_SERIAL
    --timeout;
  }
  if (timeout < 0)
  {
    Serial.println("\nConnection error !");
    return false;
  }
  Serial.println("");
  Serial.print("Connected, ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Console_println("connected");
  Console_println("IP: " + WiFi.localIP().toString());

  if (mdns.begin(p, WiFi.localIP())) // advertize domain name
  {
    mdns.addService("http", "tcp", 80); // advertise DNS-SD services

    Serial.print("mDNS responder started: http://");
    Serial.print(p);
    Serial.print(".local = ");
    Serial.println(WiFi.localIP());
    Serial.println("NOTE: a bonjour/avahi/nss-mdns service active is required on host");
  }
  else
    Serial.println("Error, mDNS responder NOT started.");
  return true;
}

void disconnectWiFi()
{
  WiFi.disconnect();
}

#ifdef _USE_UPNP_
void setupSSDP()
{
  Serial.println("Starting SSDP...");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName("Philips hue clone");
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("index.html");
  SSDP.setModelName("Philips hue bridge 2012");
  SSDP.setModelNumber("929000226503");
  SSDP.setModelURL("http://www.meethue.com");
  SSDP.setManufacturer("Royal Philips Electronics");
  SSDP.setManufacturerURL("http://www.philips.com");
  SSDP.begin();
}
#endif //_USE_UPNP_

WiFiMode setupWiFi(WiFiMode mode)
{
  WiFi.persistent(false);

  if (mode == WIFI_STA)
  {
    Serial.println("WiFi mode = WIFI_STA");
    // the standard mode
    if (setupWiFiStation())
#ifdef _USE_UPNP_
      setupSSDP();
#else
      ;
#endif //_USE_UPNP_
    else
      setupWiFiAP();
  }
  else
  {
    // setup needed
    setupWiFiAP();
  }
  wifiMode = WiFi.getMode();
  return wifiMode;
}

void loopWiFi()
{
  if (wifiMode == WIFI_STA)
    mdns.update();
  yield();
}

