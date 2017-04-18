/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

Common routines

Sources repository: https://github.com/microentropie/
*/

#include <arduino.h>

#include "common.h"

const char *COPYRIGHT PROGMEM = "<p><small>&copy; 2016-2017 <a href='http://www.microentropie.com/'>microentropie.com</a></small></p>";
const char *COPYRIGHTtxt PROGMEM = "(C) 2016-2017 microentropie.com";

/*
  pay attention when using PROGMEM with ESP8266,
  PROGMEM stores variables in EEPROM, which is not directly accessible by the ESP memory map
  see: http://www.esp8266.com/viewtopic.php?f=28&t=5549
*/

char *strSecToMinSec(char *buf, int bLen, int sec)
{
  int minutes = sec / 60;
  int hours = minutes / 60;
  if (hours == 0)
    snprintf(buf, bLen, "%d'%02d\"", minutes, sec % 60);
  else
    snprintf(buf, bLen, "%dh%02d'%02d\"", hours, minutes % 60, sec % 60);
  return buf;
}



#include <ESP8266WiFi.h>
/*const */char *PublicName(const char *prefixName)
{
  static char AP_NameChar[16 + 5 + 1] = { 0 }; // prefixName + "_XXXX" + '\0'


  if (AP_NameChar[0] == 0)
  {
    // Create an unique name by appending the last two bytes of the MAC:
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    if (prefixName && *prefixName && strlen(prefixName) <= 16)
      snprintf(AP_NameChar, sizeof(AP_NameChar), "%s_%02x%02x", prefixName, mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1]);
    else
      snprintf(AP_NameChar, sizeof(AP_NameChar), "esp_%02x%02x", mac[WL_MAC_ADDR_LENGTH - 2], mac[WL_MAC_ADDR_LENGTH - 1]);
    //strlwr(AP_NameChar);
  }
  return AP_NameChar;
}

