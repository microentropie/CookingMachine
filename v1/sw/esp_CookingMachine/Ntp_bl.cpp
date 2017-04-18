/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

Network Time Protocol, initialization

Sources repository: https://github.com/microentropie/
*/

#include <TimeZone.h>
#include "src/time/TimeNTP.h"
#include "settings.h"

void ntp_init()
{
  struct ntpConfig ntpc;
  if (!NtpConfig_Load(ntpc))
  {
    // error reading from eeprom, set defaults
    strncpy(ntpc.Server1, "it.pool.ntp.org", sizeof(ntpc.Server1)); // extra space filled with 0s
    strncpy(ntpc.Server2, "europe.pool.ntp.org", sizeof(ntpc.Server2)); // extra space filled with 0s
    strncpy(ntpc.Server3, "pool.ntp.org", sizeof(ntpc.Server3)); // extra space filled with 0s
    ntpc.updatePeriodSeconds = 0;
    NtpConfig_Save(ntpc);
  }

  NtpTime_init(ntpc);
}
