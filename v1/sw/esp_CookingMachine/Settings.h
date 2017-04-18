#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <IPAddress.h>
#include <serializeEeprom.h>



struct baseConnectionConfig
{
  // station connection params:
  char hostName[30]; // and AP ssid
  char ssid[30];
  char password[30];
  bool bUseStaticIp;
  IPAddress ip;
  IPAddress gateway;
  IPAddress subnet;
  //
  // User access control
  char userName[16];
  char userPassword[16];
  unsigned char userGroup;
  //
  // AP parameters:
  char ApPassword[30];
  IPAddress ApIp;
};

struct CookingSession
{
  int waitTimeMin;
  int cookingTimeMin;
  int keepWarmTimeMin;
};


//---
bool baseConnectionConfig_Save(baseConnectionConfig &cfg);
bool baseConnectionConfig_Load(baseConnectionConfig &cfg);

class baseConnectionConfigEEP : public baseConnectionConfig
{
public:
  bool Save();
  bool Load();
};

//---
bool CookingSession_Save(CookingSession &cs);
bool CookingSession_Load(CookingSession &cs);

/*
class CookingSessionEEP : public CookingSession
{
public:
  bool Save();
  bool Load();
};
*/

//---
#include <TimeZone.h>

bool Localization_Save(TimeAndDateInfo &cs);
bool Localization_Load(TimeAndDateInfo &cs);

/*
class LocalizationEEP : public TimeAndDateInfo
{
public:
  bool Save();
  bool Load();
};
*/

#include "src/time/ntp_model.h"

//---
bool NtpConfig_Save(ntpConfig &ntpc);
bool NtpConfig_Load(ntpConfig &ntpc);

/*
class NtpConfigEEP : public ntpConfig
{
public:
  bool Save();
  bool Load();
};
*/

#endif //_SETTINGS_H_
