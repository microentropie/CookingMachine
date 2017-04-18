/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

machine base interface layer

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include "machine_if.h"
#include "temperature_processing.h"
//#include "common.h"

extern MachineIo machineIOs;


MachineIf::MachineIf()
{
  this->heatingStatus = machineHeatingStatus::OFF;
  this->safeSwitchStatus = SafeSwitchStatus::Undefined;
  this->safeSwitchStatusTimeStamp = 0;
  this->m_bUpdateSensorsStatus = false;
  this->soundOn = false;
  this->soundOffTimeout = 0L;
}

void MachineIf::Init()
{
  this->m_bUpdateSensorsStatus = false;
  machineIOs.Init();
  SetHeating(machineHeatingStatus::OFF, 0);
  TempInit();
}

void MachineIf::SetHeating(machineHeatingStatus newStatus, int millisecDelay)
{
  // GPIO set
  // delay is useful for electric noise reduction,
  // in case outputs plot inductive loads like relays
  //
  // WARM : Main Relay on
  // HEAT : both Relays on

  if (millisecDelay <= 0)
    machineIOs.SetRelays(
    (newStatus == machineHeatingStatus::WARM || newStatus == machineHeatingStatus::HOT) ? OutSetAction_e::On : OutSetAction_e::Off,
      (newStatus == machineHeatingStatus::HOT) ? OutSetAction_e::On : OutSetAction_e::Off,
      OutSetAction_e::Off);
  else
  {
    delay(millisecDelay);
    machineIOs.SetRelays(
      (newStatus == machineHeatingStatus::WARM || newStatus == machineHeatingStatus::HOT) ? OutSetAction_e::On : OutSetAction_e::Off,
      OutSetAction_e::noChange,
      OutSetAction_e::Off);
    delay(millisecDelay);
    machineIOs.SetRelays(
      OutSetAction_e::noChange,
      (newStatus == machineHeatingStatus::HOT) ? OutSetAction_e::On : OutSetAction_e::Off,
      OutSetAction_e::Off);
  }
  this->heatingStatus = newStatus;
}

machineHeatingStatus MachineIf::GetHeating()
{
  return this->heatingStatus;
}

SafeSwitchStatus MachineIf::GetSafeSwitchStatus()
{
  if (this->heatingStatus == machineHeatingStatus::WARM)
    return TestSafeSwitchStatus(); // phisically read switch status and refresh timestamp

  if (this->safeSwitchStatusTimeStamp + (HTTP_PAGE_UPDATE_FREQ_SEC + MACHINE_STATUS_REFRESH_FREQ_SEC) * 1000 <= millis())
    return SafeSwitchStatus::Undefined; // too much time elapsed, status may be changed
  return this->safeSwitchStatus;
}

SafeSwitchStatus MachineIf::TestSafeSwitchStatus()
{
#ifdef VERBOSE_SERIAL
  Serial.println("MachineIf::TestSafeSwitchStatus");
#endif //VERBOSE_SERIAL

  if (this->safeSwitchStatus != SafeSwitchStatus::Undefined &&
    this->safeSwitchStatusTimeStamp + (HTTP_PAGE_UPDATE_FREQ_SEC + MACHINE_STATUS_REFRESH_FREQ_SEC) * 1000 > millis())
  {
#ifdef VERBOSE_SERIAL
    Serial.println("  return cached value");
#endif //VERBOSE_SERIAL
    return this->safeSwitchStatus; // safe-sw read in recently, so now return cached value
  }
#ifdef VERBOSE_SERIAL
  Serial.println("  reading");
#endif //VERBOSE_SERIAL

  machineHeatingStatus oldStatus = this->heatingStatus;
  if (oldStatus != machineHeatingStatus::WARM)
  {
    // need to set full power or the safe switch status won't be detected
    SetHeating(machineHeatingStatus::WARM, 100);
    delay(100); // need to wait until hardware senses the AC Voltage
  }
  SafeSwitchStatus st = machineIOs.GetSafeSwitchStatus();
  if (oldStatus != machineHeatingStatus::WARM)
    SetHeating(oldStatus, 100); // restore power
  if (st == SafeSwitchStatus::Undefined)
    st = SafeSwitchStatus::NotArmed;

  this->safeSwitchStatus = st;
  this->safeSwitchStatusTimeStamp = millis();

  return st;
}

int MachineIf::GetTemperature()
{
  return TempConvertAdc2Temp(machineIOs.GetTemperature());
}

void MachineIf::SoundAlarm(int milliSec)
{
  if (milliSec < 0)
  {
    // immediate sound, has the precedence
    milliSec = -milliSec;
    if (milliSec > 1000)
      milliSec = 1000; // sound for no more than 1"
    machineIOs.SetAlarm(true);
    delay(milliSec); // this will lock the system for the specified time
    machineIOs.SetAlarm(false);
    return;
  }
  // normal sound does not lock the system
  this->soundOn = milliSec > 0;
  if (this->soundOn)
    this->soundOffTimeout = millis() + milliSec;
  else
    this->soundOffTimeout = 0;
}
void MachineIf::ModulateSound(bool modulation)
{
  if (millis() > this->soundOffTimeout)
    SoundAlarm(0);
  machineIOs.SetAlarm(this->soundOn && modulation);
}

