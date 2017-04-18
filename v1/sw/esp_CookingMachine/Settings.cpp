/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

EEPROM save interface

Sources repository: https://github.com/microentropie/
*/

#include "settings.h"

bool baseConnectionConfigEEP::Save()
{
  return eepromIf<baseConnectionConfig>::Save((baseConnectionConfig *)this, EEPROM_log_RW);
}

bool baseConnectionConfigEEP::Load()
{
  return eepromIf<baseConnectionConfig>::Load((baseConnectionConfig *)this, EEPROM_log_RW);
}

// initialize the signature for this type
//                                                            C O N N
template<> int eepromIf<baseConnectionConfig>::Signature = 0x434F4E4E;
template<> int eepromIf<baseConnectionConfig>::baseOffset = 0;

bool baseConnectionConfig_Save(baseConnectionConfig &cfg)
{
  return eepromIf<baseConnectionConfig>::Save(&cfg, EEPROM_log_RW);
}
bool baseConnectionConfig_Load(baseConnectionConfig &cfg)
{
  return eepromIf<baseConnectionConfig>::Load(&cfg, EEPROM_log_RW);
}


//----------------------------------------------------------------
// initialize the signature for this type
//                                                      C S E S
template<> int eepromIf<CookingSession>::Signature = 0x43534553;
template<> int eepromIf<CookingSession>::baseOffset = 300;

bool CookingSession_Save(CookingSession &cs)
{
  return eepromIf<CookingSession>::Save(&cs, EEPROM_log_RW);
}
bool CookingSession_Load(CookingSession &cs)
{
  return eepromIf<CookingSession>::Load(&cs, EEPROM_log_RW);
}


//----------------------------------------------------------------
// initialize the signature for this type
//                                                      L C L Z
template<> int eepromIf<TimeAndDateInfo>::Signature = 0x4C434C5A;
template<> int eepromIf<TimeAndDateInfo>::baseOffset = 500;

bool Localization_Save(TimeAndDateInfo &lcl)
{
  return eepromIf<TimeAndDateInfo>::Save(&lcl, EEPROM_log_RW);
}
bool Localization_Load(TimeAndDateInfo &lcl)
{
  return eepromIf<TimeAndDateInfo>::Load(&lcl, EEPROM_log_RW);
}

// initialize the signature for this type
//                                                s N T P
template<> int eepromIf<ntpConfig>::Signature = 0x734E5450;
template<> int eepromIf<ntpConfig>::baseOffset = 550;

bool NtpConfig_Save(ntpConfig &ntpc)
{
  return eepromIf<ntpConfig>::Save(&ntpc, EEPROM_log_RW);
}
bool NtpConfig_Load(ntpConfig &ntpc)
{
  return eepromIf<ntpConfig>::Load(&ntpc, EEPROM_log_RW);
}
