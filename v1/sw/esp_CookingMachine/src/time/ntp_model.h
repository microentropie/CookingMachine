#ifndef _NTP_MODEL_H_
#define _NTP_MODEL_H_

struct ntpConfig
{
  // EEprom has not enought space to hold a full long address
  char Server1[80];
  char Server2[80];
  char Server3[80];
  long updatePeriodSeconds;
};

#endif //_NTP_MODEL_H_
