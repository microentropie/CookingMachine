/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

localization configuration

Sources repository: https://github.com/microentropie/
*/

#include <TimeZone.h>
#include "settings.h"

void Localization_init()
{
  struct TimeAndDateInfo lcl;
  if (Localization_Load(lcl))
    SetLocalization(lcl);
  else
  {
    // error reading from eeprom, set defaults
    SetLocalization(+1, dstCriteria::CET, // Central European Time and Dst
      '/', 32, // YYYY/MM/DD
      ':', 16);// HH:mm:SS
    lcl = GetLocalization();
    Localization_Save(lcl);
  }
}
