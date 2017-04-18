/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

display handler

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include "common_io.h"
#include "settings.h"
#include "machine_io.h"

void ui_init() {}

void ui_splashScreen()
{
  /*
    // OLED display - begin
    Display_init();
    display.drawString(0, 0, String("Cooking"));
    display.drawString(0, 26, String("Machine"));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 53, COPYRIGHTtxt);
    display.drawString(95, 25, "2016");
    display.display();
    // OLED display - end
    yield();
  */
}
void ui_ScreenSaver() {}

char ui_getSetInitialMode()
{
  /*
    // if the button is pressed: start as AP
    display.setFont(ArialMT_Plain_24);
    byte button = 255;
    int btnCounter = 0;
    for(int i = 0; i < 10; ++i)
    {
      button = SPI.transfer(0);
      if(button == 0)
        ++btnCounter;
      delay(100);
      display.drawString(i*6, 32, ".");
      display.display();
    }
    if(btnCounter > 8)
      wifiMode = WIFI_AP;
  */
  bool buttonPressed = false;
  int btnCounter = 0;
  for (int i = 0; i < 10; ++i)
  {
    buttonPressed = !machineIOs.GetButton();
    if (buttonPressed)
      ++btnCounter;
    delay(100);
  }

  if (btnCounter > 8) return 2;
  if (btnCounter > 1) return 1;
  return 0;
}

void Console_init() {}
void Console_print(const char *p) {}
void Console_println(const char *p) {}
void Console_println(String str) {}



#include "machine_cycle.h"
#include "src/button/button.h"
extern MachineCycle machine;

void loopButton()
{
  byte bs = ButtonStatusGet();
  if (bs & (BUTTON_LONG_PRESS | BUTTON_CLICK_UP))
  {
    // an event that needs to be handled has occurred

    // check current machine status
    int mTime;
    if (machine.IsWorking())
    {
      // machine ON -> switch off  
      // (InitialWaiting is considered ON)
      Serial.println("Panel user action: Switching OFF");
      machineIOs.SetLeds(Off, Off, noChange);

      machine.StopCycle();
    }
    else
    {
      if (bs & BUTTON_CLICK_UP)
      {
        // machine OFF (or in initial delay) -> start cooking
        Serial.println("Panel user action: Switching ON");
        machineIOs.SetLeds(On, Off, noChange);

        // load saved times
        int waitSec, cookingSec, keepWarmSec;
        CookingSession cfg;
        if (!CookingSession_Load(cfg))
          MachineCycle::GetDefaultTimes(cfg.waitTimeMin, cfg.cookingTimeMin, cfg.keepWarmTimeMin);
        waitSec = 0;
        cookingSec = cfg.cookingTimeMin * 60;
        keepWarmSec = cfg.keepWarmTimeMin * 60;
        machine.SetTimes(waitSec, cookingSec, keepWarmSec);

        // start cooking:
        machine.StartCycle();
      }
    }
  }
}
