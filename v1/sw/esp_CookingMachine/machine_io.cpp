/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

Hardware interface layer

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include "machine_io.h"
#include "common_io.h"

MachineIo machineIOs;

MachineIo::MachineIo()
{
  // cannot call libraries methods from constructor
  // only variables initialization with constant values
}

void MachineIo::Init()
{
#ifdef VERBOSE_SERIAL
  Serial.println("MachineIo::Init()");
#endif //VERBOSE_SERIAL
  // GPIO init:
  pinMode(RY_MAIN_PIN, OUTPUT);
  digitalWrite(RY_MAIN_PIN, outOFF);

  pinMode(RY_HOT_PIN, OUTPUT);
  digitalWrite(RY_HOT_PIN, outOFF);

  // Water pouring pump:
  pinMode(RY_POUR_PIN, OUTPUT);
  digitalWrite(RY_POUR_PIN, outOFF);

  // Hot/Warm sensor:
  detachInterrupt(digitalPinToInterrupt(SENSOR_HOT_PIN));
  pinMode(SENSOR_HOT_PIN, INPUT_PULLUP);

  // Temperature:
  //pinMode(TEMP_PIN, INPUT);

  // Buzzer:
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, 0);

  //
  // User interface (LEDs and button):
  // prepare GPIOs:
  pinMode(MODULE_BLUELED_PIN, OUTPUT);
  digitalWrite(MODULE_BLUELED_PIN, 1); // init module blue LED off

  pinMode(PANEL_REDLED_PIN, OUTPUT);
  digitalWrite(PANEL_REDLED_PIN, 1); //   init red LED off

  pinMode(PANEL_GREENLED_PIN, OUTPUT);
  digitalWrite(PANEL_GREENLED_PIN, 1); // init green LED off

  pinMode(PANEL_BLUELED_PIN, OUTPUT);
  digitalWrite(PANEL_BLUELED_PIN, 1); //  init blue LED off

  // input: button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void MachineIo::SetRelays(OutSetAction_e rMain, OutSetAction_e rHot, OutSetAction_e rPump)
{
  // GPIO set
  if (rPump != OutSetAction_e::noChange)
  {
    digitalWrite(RY_POUR_PIN, (rPump == OutSetAction_e::On) ? outON : outOFF);
#ifdef VERBOSE_SERIAL
    Serial.print("Pump is now ");
    Serial.print((rPump == OutSetAction_e::On) ? "ON" : "OFF");
    Serial.print(" (GPIO");
    Serial.print(RY_POUR_PIN);
    Serial.print(" = ");
    Serial.print((rPump == OutSetAction_e::On) ? outON : outOFF);
    Serial.println(")");
#endif //VERBOSE_SERIAL
  }

  if (rMain != OutSetAction_e::noChange)
  {
    digitalWrite(RY_MAIN_PIN, (rMain == OutSetAction_e::On) ? outON : outOFF);
#ifdef VERBOSE_SERIAL
    Serial.print("MainRelay is now ");
    Serial.print((rMain == OutSetAction_e::On) ? "ON" : "OFF");
    Serial.print(" (GPIO");
    Serial.print(RY_MAIN_PIN);
    Serial.print(" = ");
    Serial.print((rMain == OutSetAction_e::On) ? outON : outOFF);
    Serial.println(")");
#endif //VERBOSE_SERIAL
  }

  if (rHot != OutSetAction_e::noChange)
  {
    digitalWrite(RY_HOT_PIN, (rHot == OutSetAction_e::On) ? outON : outOFF);
#ifdef VERBOSE_SERIAL
    Serial.print("HotRelay is now ");
    Serial.print((rHot == OutSetAction_e::On) ? "ON" : "OFF");
    Serial.print(" (GPIO");
    Serial.print(RY_HOT_PIN);
    Serial.print(" = ");
    Serial.print((rHot == OutSetAction_e::On) ? outON : outOFF);
    Serial.println(")");
#endif //VERBOSE_SERIAL
  }
}

void MachineIo::GetRelays(OutSetAction_e &rMain, OutSetAction_e &rHot, OutSetAction_e &rPump)
{
  // GPIO get
  uint8_t v;

  v = digitalRead(RY_MAIN_PIN); // not sure digitalRead() returns 1 for pin high
  rMain = (!(v == outOFF)) ? OutSetAction_e::On : OutSetAction_e::Off;

  v = digitalRead(RY_HOT_PIN); // not sure digitalRead() returns 1 for pin high
  rHot = (!(v == outOFF)) ? OutSetAction_e::On : OutSetAction_e::Off;

  v = digitalRead(RY_POUR_PIN); // not sure digitalRead() returns 1 for pin high
  rPump = (!(v == outOFF)) ? OutSetAction_e::On : OutSetAction_e::Off;
}

int MachineIo::GetTemperature()
{
  // GPIO read
  //return TEMP_NO_READING;
  int adc = 0;
  for (int i = 0; i < 32; ++i)
    adc += analogRead(TEMP_PIN);
  adc = adc >> 5;

  return adc;
}

SafeSwitchStatus MachineIo::GetSafeSwitchStatus()
{
  // pin HIGH => switch NOT armed or cannot detect (no power or heating set to HOT)
  // pin LOW  => switch armed (can only be detected when heating is set to WARM)
  if (SENSOR_HOT_PIN == 3)
    Serial.end();
  bool armed = !digitalRead(SENSOR_HOT_PIN);
  if (SENSOR_HOT_PIN == 3)
  {
    Serial.begin(UART_BAUDRATE);
#ifdef VERBOSE_SERIAL
    Serial.setDebugOutput(true); // enable system log to UART0
#endif //VERBOSE_SERIAL
  }
  if (armed)
    return SafeSwitchStatus::Armed;

  // Cannot determine status without power set to WARM.
  // This sw layer does not have to deal with machine cycle
  // so further actions cannot be taken here
  /*
  // check if power is on
  // NOTE: this doesn't work if PWM output is used
  OutSetAction_e rMain, rHot;
  GetRelays(rMain, rHot);
  if(rMain == OutSetAction_e::On && rHot == OutSetAction_e::On)
    return SafeSwitchStatus::NotArmed;
  */
  return SafeSwitchStatus::Undefined;
}

void MachineIo::SetAlarm(bool soundOn)
{
  digitalWrite(BUZZER_PIN, soundOn);
}

void MachineIo::SetLeds(OutSetAction_e r, OutSetAction_e g, OutSetAction_e b)
{
  if (r != noChange)
    digitalWrite(PANEL_REDLED_PIN, r == Off);
  if (g != noChange)
    digitalWrite(PANEL_GREENLED_PIN, g == Off);
  if (b != noChange)
  {
    digitalWrite(PANEL_BLUELED_PIN, b == Off);
    digitalWrite(MODULE_BLUELED_PIN, b == Off);
  }
}

bool MachineIo::GetButton()
{
  return digitalRead(BUTTON_PIN) != 0;
}
