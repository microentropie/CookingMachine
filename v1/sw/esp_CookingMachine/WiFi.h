#ifndef _myWiFi_
#define _myWiFi_

#include <IPAddress.h>
#include <ESP8266WiFiType.h>

extern WiFiMode setupWiFi(WiFiMode mode);
extern void loopWiFi();

extern void disconnectWiFi();

extern const char *WiFiModeToString(WiFiMode wifiMode);

#endif //_myWiFi_
