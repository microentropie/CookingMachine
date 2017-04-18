/*
Author:  Stefano Di Paolo
License:  CC-BY-SA, https://creativecommons.org/licenses/by-sa/4.0/
Date:  2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

interrupt button handler with debouncing
*/

#include <arduino.h>

#include "button.h"


static bool m_buttonOn2Off = false;
static bool m_buttonPressed = false;
static unsigned long m_buttonTimer = 0;
static bool m_buttonPressedPrev = false;
static signed char buttonPin = -1;
void ICACHE_RAM_ATTR ButtonInterrupt()
{
  if (buttonPin < 0) return;
  // NEVER use Serial.print from inside interrupts
  //Serial.println("ButtonInterrupt()"); // may cause malfunctions
  //ets_printf("ButtonInterrupt()\n"); // no malfunctions but may lose some chars
  unsigned long tmr = millis();
  if (tmr - m_buttonTimer < 50)
    return; // debouncing ...

  //m_buttonPressed = !machineIOs.GetButton(); // when button is pressed input goes LOW
  m_buttonPressed = !digitalRead(buttonPin); // when button is pressed input goes LOW
  if (m_buttonPressed == m_buttonPressedPrev)
    return; // debouncing ...

  // button status has changed
  if (!m_buttonPressed)
  {
    // button just released
    //Serial.println("ButtonInterrupt(): just released");
    m_buttonOn2Off = true; // must be user reset by calling ButtonStatusClear()
  }
  else
  {
    // button just pressed
    //m_buttonOn2Off = true; // must be user reset by calling ButtonStatusClear()
  }
  m_buttonTimer = tmr;
  m_buttonPressedPrev = m_buttonPressed;
}



enum buttonEvents
{
  Relax, // button unpressed
  ClickDown, // unsupported
  LongPress,
  LongPressMasksClickUp,
  ClickUp,
};
static buttonEvents m_buttonQueue = buttonEvents::Relax; // queue
static bool HandleQueue()
{
  // in order to have a consistent access to button status,
  // interrupts must be disabled
  noInterrupts();
  unsigned long buttonTmr = m_buttonTimer;
  bool bUp = m_buttonOn2Off;
  bool bp = m_buttonPressed;
  unsigned long tmr = millis();
  interrupts();

  if (!(m_buttonQueue == buttonEvents::LongPress ||
    m_buttonQueue == buttonEvents::LongPressMasksClickUp))
  {
    // no apparent events, check for long-press
    bool longPress = false;
    if (bp)
      longPress = (tmr - buttonTmr > 1000);
    if (longPress)
    {
      // long-press just detected: enqueue it
      m_buttonQueue = buttonEvents::LongPress;
    }
  }

  if (bUp)
  {
    switch (m_buttonQueue)
    {
    case buttonEvents::LongPress:
    default:
      // a Long press masks click-up
      // user has not consumed yet the button
      break;
    case buttonEvents::LongPressMasksClickUp:
      // a Long press masks click-up
      m_buttonQueue = buttonEvents::Relax;
      break;
    case buttonEvents::Relax:
      m_buttonQueue = buttonEvents::ClickUp;
      break;
    }
    noInterrupts();
    m_buttonOn2Off = false; // clear status
    interrupts();
  }
  return bp;
}

// returns:
// bits: 210
//       |||
//       ||\-- button is currently pressed
//       |\--- button has been pressed for 1" or more
//       \---- button Up event (needs to be reset)
// NOTE: when button is released after a long-press, "button Up event" is hidden.
// Example:
// description:              returns:
// button is released        0
// button Down (pressed)     1
// keep holding down for 1+" 3
// HIDDEN: button Up (release)       4
// ButtonStatusGet() returns 0
// button Down (pressed)     1
// button Up (release)       4
byte ButtonStatusGet()
{
  bool bp = HandleQueue();

  byte ret = (m_buttonQueue == buttonEvents::ClickUp ? BUTTON_CLICK_UP : 0) | (m_buttonQueue == buttonEvents::LongPress ? BUTTON_LONG_PRESS : 0) | (bp ? BUTTON_CURRENTLY_PRESSED : 0);
  /*
  if (ret > 1)
  {
    Serial.print("ButtonStatusGet() = ");
    Serial.print(ret, BIN);
    //Serial.print(", ms = ");
    //Serial.print(tmr - buttonTmr);
    Serial.print(", queue = ");
    Serial.print(m_buttonQueue);
    Serial.println();
  }
  */

  // dequeue
  switch (m_buttonQueue)
  {
  case buttonEvents::ClickUp:
    m_buttonQueue = buttonEvents::Relax;
    break;
  case buttonEvents::LongPress:
    m_buttonQueue = buttonEvents::LongPressMasksClickUp;
    break;
    //case buttonEvents::Relax:
    //default:
    //  ;
  }
  return ret;
}

bool ButtonInit(signed char button)
{
  buttonPin = button;
  if (buttonPin < 0)
  {
    detachInterrupt(digitalPinToInterrupt(-buttonPin));
    return false;
  }
  m_buttonOn2Off = false;
  m_buttonPressed = false;
  m_buttonTimer = millis();
  m_buttonPressedPrev = false;
  m_buttonQueue = buttonEvents::Relax;
  attachInterrupt(digitalPinToInterrupt(buttonPin), ButtonInterrupt, CHANGE);
  return true;
}
