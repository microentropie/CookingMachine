/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

http utilities.

Sources repository: https://github.com/microentropie/
*/

#include "IpAddress.h"

bool isLocalAddress(IPAddress ip)
{
  // https://en.wikipedia.org/wiki/Private_network

  if (ip[0] == 192 && ip[1] == 168)
    return true;
  if (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31)
    return true;
  if (ip[0] == 10)
    return true;
  return false;
}
