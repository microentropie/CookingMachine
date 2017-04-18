/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

WiFi parameters

Sources repository: https://github.com/microentropie/
*/

#include <ESP8266WebServer.h>
#include "common.h"
#include "settings.h"



baseConnectionConfig cfg;

bool loadWiFiConfig()
{
  // read stored configuration
  baseConnectionConfigEEP cfgc;

  bool ok = cfgc.Load();
  if (ok)
  {
    cfg = (baseConnectionConfig)cfgc;
    if (cfg.ssid[0] == '\0')
    {
      Serial.print("empty configuration has been retrieved from eeprom; ");
      ok = false;
    }
  }

  //ok = false; // debug
  if (!ok)
  {
    Serial.println("ERROR reading from EEPROM, forcing defaults");

    // set default parameters
    strncpy(cfgc.hostName, PublicName(NULL), sizeof(cfgc.hostName)); // buffer padded with \0s
    strncpy(cfgc.ssid, "YOUR A.P. NAME", sizeof(cfgc.ssid));     // buffer padded with \0s
    strncpy(cfgc.password, "YOUR A.P. PASSWORD", sizeof(cfgc.password)); // buffer padded with \0s
    //cfgc.bUseStaticIp = false;
    cfgc.bUseStaticIp = true;
    cfgc.ip.fromString("192.168.0.111");
    cfgc.gateway.fromString("192.168.0.1");
    cfgc.subnet.fromString("255.255.255.0");
    //
    strncpy(cfgc.userName, "admin", sizeof(cfgc.userName)); // buffer padded with \0s
    strncpy(cfgc.userPassword, "esp8266", sizeof(cfgc.userPassword)); // buffer padded with \0s
    cfgc.userGroup = 0;
    //
    strncpy(cfgc.ApPassword, "12345678", sizeof(cfgc.ApPassword)); // buffer padded with \0s
    cfgc.ApIp.fromString("192.168.44.1");
    cfgc.Save();
    cfg = (baseConnectionConfig)cfgc;
  }
  //cfg.bUseStaticIp = true; // DEBUG
  //ok = true; // DEBUG

#ifdef VERBOSE_SERIAL
  Serial.print("cfg.hostName=");
  Serial.println(cfg.hostName);

  Serial.print("cfg.ssid=");
  Serial.println(cfg.ssid);

  Serial.print("cfg.password=");
  Serial.println(cfg.password);

  Serial.print("cfg.bUseStaticIp=");
  Serial.println(cfg.bUseStaticIp);

  Serial.print("cfg.ip=");
  Serial.println(cfg.ip);

  Serial.print("cfg.gateway=");
  Serial.println(cfg.gateway);

  Serial.print("cfg.subnet=");
  Serial.println(cfg.subnet);

  Serial.print("cfg.ApPassword=");
  Serial.println(cfg.ApPassword);

  Serial.print("cfg.ApIp=");
  Serial.println(cfg.ApIp);

  Serial.print("cfg.userName=");
  Serial.println(cfg.userName);

  Serial.print("cfg.userPassword=");
  Serial.println(cfg.userPassword);
#endif //VERBOSE_SERIAL

  return ok;
}

const char *WiFiModeToString(WiFiMode wifiMode)
{
  switch (wifiMode)
  {
  case WIFI_OFF:
    return "WIFI_OFF";
  case WIFI_STA:
    return "WIFI_STA";
  case WIFI_AP:
    return "WIFI_AP";
  case WIFI_AP_STA:
    return "WIFI_AP_STA";
  }
  return "?";
}

