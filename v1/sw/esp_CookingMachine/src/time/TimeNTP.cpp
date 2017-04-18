/*
Author:  Stefano Di Paolo
License:  CC-BY-SA, https://creativecommons.org/licenses/by-sa/4.0/
Date:  2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

Network Time Protocol and localization business logic
*/

#include <Arduino.h>
#include <time.h>
#include <sntp.h>
#include <ESP8266WiFi.h>
#include <TimeZone.h>
#include <DateTime.h>
#include "ntp_model.h"



static bool timeSynch_ed = false;
void NtpTime_init(struct ntpConfig &ntpc)
{
  char TimeBuf[30];


  timeSynch_ed = false;

  const char *pBrief, *pLong;
  struct TimeAndDateInfo lcl = GetLocalization();

  Serial.print(F("time zone: "));
  Serial.print(lcl.timeZone);
  Serial.print(F(", DST criteria: "));
  if (DstCriteriaStrings(lcl.dst, &pBrief, &pLong) >= 0)
    Serial.println(pBrief);
  else
    Serial.println(F("ERROR"));
  Serial.print(F("date separator: '"));
  Serial.print(lcl.dateSeparator);
  Serial.print(F("', Date format: ("));
  Serial.print(lcl.dateFormat);
  Serial.print(F(") "));
  if (DateFormatStrings(lcl.dateFormat, &pBrief, &pLong) >= 0)
    Serial.println(pBrief);
  else
    Serial.println(F("ERROR"));
  Serial.print(F("time separator: '"));
  Serial.print(lcl.timeSeparator);
  Serial.print(F("', Time format: ("));
  Serial.print(lcl.timeFormat);
  Serial.print(F(") "));
  if (TimeFormatStrings(lcl.timeFormat, &pBrief, &pLong) >= 0)
    Serial.println(pBrief);
  else
    Serial.println(F("ERROR"));

  WiFiMode wifiMode = WiFi.getMode();

  if (wifiMode == WIFI_STA)
  {
    Serial.print(F("date/time needs to be set: "));
    Serial.print(DateTime::UtcIso8601DateTime(TimeBuf, sizeof(TimeBuf), time(NULL), true));
    Serial.println();

    Serial.println(F("sNTP time servers:"));
    //struct ntpConfig ntpc;
    //NtpConfig_Load(ntpc);
    Serial.print(F(" 1: ")); Serial.println(ntpc.Server1);
    Serial.print(F(" 2: ")); Serial.println(ntpc.Server2);
    Serial.print(F(" 3: ")); Serial.println(ntpc.Server3);
    Serial.print(F("sNTP time initialization in progress "));
    configTime(0, 0, ntpc.Server1, ntpc.Server2, ntpc.Server3);
    Serial.println();
  }
  else
  {
    Serial.println(F("date/time can't be set, sNTP can't be synchronized: no internet !"));
    timeSynch_ed = true; // inhibits loop
  }
}

unsigned long nextSyncTime = 0;
void NtpTime_loop()
{
  if (timeSynch_ed)
    ;
  else
    if (nextSyncTime <= millis())
    {
      timeSynch_ed = (time(NULL) > 152524800UL); // 2016/01/01
      if (!timeSynch_ed)
        nextSyncTime += 1000;
      else
      {
        char TimeBuf[30];

        Serial.println(String() + F("sNTP time synchronized in ") + String(millis()) + F(" ms after startup:"));
        time_t tm = time(NULL);
        Serial.println(DateTime::UtcIso8601DateTime(TimeBuf, sizeof(TimeBuf), tm, true));
        Serial.print(DateTime::LocalIso8601DateTime(TimeBuf, sizeof(TimeBuf), tm, true));
        Serial.println(F(" local time"));
      }
    }
}
