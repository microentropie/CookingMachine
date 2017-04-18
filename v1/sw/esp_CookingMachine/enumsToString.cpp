/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

enum -> string

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include "machine_if.h"
//#include "machine_io.h"

const char *OutSetActionToString(OutSetAction_e osStatus)
{
  switch (osStatus)
  {
  case OutSetAction_e::Off:
    return "Off";
  case OutSetAction_e::On:
    return "On";
  case OutSetAction_e::noChange:
    return "noChange";
  }
  return "?";
}

String TemperatureToString(int temp)
{
  int integerPart = temp / 10;
  int decimalPart = temp % 10;
  return String(integerPart) + String(".") + String(decimalPart);
}

const char *HeatingStatusToString(machineHeatingStatus hStts)
{
  switch (hStts)
  {
  case OFF:
    return "OFF";
  case WARM:
    return "WARM";
  case HOT:
    return "HOT";
  }
  return "?";
}

const char *SafeSwitchStatusToString(SafeSwitchStatus sSwStatus)
{
  switch (sSwStatus)
  {
  case Undefined:
    return "Undefined";
  case Armed:
    return "Ok";
  case NotArmed:
    return "Needs Reset";
  }
  return "?";
}
