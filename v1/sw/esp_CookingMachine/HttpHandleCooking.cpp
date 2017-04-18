/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

http page handler, cooking

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <DateTime.h>

#include "machine_cycle.h"
#include "temperature_processing.h"
#include "common.h"
#include "settings.h"
#include "ui.h"
#include "enumsToString.h"


extern MachineCycle machine;

extern ESP8266WebServer webServer;
extern baseConnectionConfig cfg;


static bool gTestMode = false;
int gWaitSec, gCookingSec, gKeepWarmSec;

#include "WebServer.h"
#include "HttpUtils.h"


// set initialWaitSec to get food cooked at specified time
bool SetInitialDelayFromEndTime(int cookingSec, const char *cookingEndTime, int &initialWaitSec, String &errMsg)
{
  time_t tm;
  if (!DateTime::TryParseTime(cookingEndTime, ':', tm))
  {
    errMsg = String("Invalid time entered: '") + cookingEndTime + "'";
    initialWaitSec = -1;
    return false;
  }
  DateTime now = DateTime::Now();
  //printf("now     = %s\n", TimeToString(now, false, /*timezone=*/1, /*isDst=*/true));
  DateTime startCookingTime = now.Date();
  startCookingTime.AddSeconds(tm);
  startCookingTime.AddSeconds(-cookingSec);
  struct TimeAndDateInfo tzi = GetLocalization();
  startCookingTime = DateTime::LocalToUtc(startCookingTime, tzi);
  //printf("endTime = %s\n", TimeToString(startCookingTime, false, /*timezone=*/1, /*isDst=*/true));
  // decide if user wants food cooked today or tomorrow
  DateTime *pNowLess12h = new DateTime(now);
  pNowLess12h->AddHours(-12);
  bool ok = false;
  //printf("now-12h = %s\n", TimeToString(*pNowLess12h, false, /*timezone=*/1, /*isDst=*/true));
  bool isToday = DateTime::Compare(pNowLess12h, startCookingTime) < 0;
  if (isToday)
  {
    // want food today, check if now is too late
    ok = DateTime::Compare(startCookingTime, now) >= 0;
  }
  else
  {
    // want food cooked tomorrow
    startCookingTime.AddDays(1);
    ok = true;
  }
  if (ok)
    initialWaitSec = (int)(startCookingTime - now);
  else
  {
    DateTime *pValidEndTime = new DateTime(now);
    pValidEndTime->AddSeconds(cookingSec);
    pValidEndTime->AddMinutes(1);
    errMsg = String("Food cannot be cooked in time");
    initialWaitSec = 0; // will need to ask user if start cooking immediately
  }

  return ok;
}

char *NowAndSeconds(char *buf, int bufSize, int addSeconds)
{
  time_t tm = DateTime::Now() + addSeconds;
  DateTime::LocalTimeToString(buf, bufSize, tm, TimePrintOption_e::SecondsNo);
  return buf;
}

char *HtmlPrepareStatusDiv(char *s, int maxLen,
  const char *pageAddress, MachineStatus mStatus, machineHeatingStatus heat, int temperature, SafeSwitchStatus sSwStatus)
{
  char TimeBuf[30];

  snprintf(s, maxLen,
    "<div>"
    "<h3>Status</h3>"
    "<table>"
    "<tr><td>Safety Switch:</td><td%s>%s</td></tr>"
    "<tr><td>Status:</td><td>%s</td></tr>"
    "<tr><td>Heating:</td><td>%s</td></tr>"
    "<tr><td>Temperature:</td><td>%s &deg;C</td></tr>"
    "<tr><td>Page updated:</td><td>%s</td></tr>"
    "</table>"
    "</div>"
    ,
    (sSwStatus == SafeSwitchStatus::NotArmed) ? " bgcolor='red'" : "",
    (sSwStatus == SafeSwitchStatus::NotArmed) ? (String("<FONT color='white'><b>") + SafeSwitchStatusToString(sSwStatus) + String("</b></FONT>")).c_str() : SafeSwitchStatusToString(sSwStatus),
    MachineCycle::StatusToString(mStatus),
    HeatingStatusToString(heat),
    TemperatureToString(temperature).c_str(),
    //pageAddress,
    DateTime::LocalTimeToString(TimeBuf, sizeof(TimeBuf), TimePrintOption_e::SecondsYes)
  );
}

int timeToCooked_sec(MachineStatus mStatus, int mTime, int cookingSec)
{
  switch (mStatus)
  {
  case MachineStatus::InitialWaiting:
    return mTime + cookingSec;
    break;

  case MachineStatus::Cooking:
    return mTime;
    break;

  case MachineStatus::KeepWarm:
    return 0;
    break;

  default:
    // cooking completed or aborted
    return -1;
    break;
  }
}

void ShowCookingPage()
{
  int mTime;
  MachineStatus mStatus;
  machine.GetStatus(&mStatus, &mTime);
  int initialWaitSec, cookingSec, keepWarmSec;
  machine.GetTimes(&initialWaitSec, &cookingSec, &keepWarmSec);

  int secToCooked = timeToCooked_sec(mStatus, mTime, cookingSec);
  char bufSecToCooked[bufmmssSz];
  if (secToCooked >= 0)
    snprintf(bufSecToCooked, sizeof(bufSecToCooked), "%d' ", -((secToCooked + 30) / 60));
  else
    bufSecToCooked[0] = '\0';

  int autoUpdateFreq = HTTP_PAGE_UPDATE_FREQ_SEC;
  if (mStatus == MachineStatus::END)
    autoUpdateFreq = 60;
  else if (gTestMode)
    autoUpdateFreq = MACHINE_STATUS_REFRESH_FREQ_SEC;

  char bufTm1[bufmmssSz], bufTm2[bufmmssSz], bufTm3[bufmmssSz], bufTm0[bufmmssSz], bufTm[10];
  char s[600];
  snprintf(s, sizeof(s), "%s%s", bufSecToCooked, PublicName(NULL));
  SendHeaderAndHead(webServer, autoUpdateFreq, s);

  snprintf(s, sizeof(s),
    "<body bgcolor='#%02X%02X%02X'>"
    "<h2>%s</h2>"
    "<p><small>(page auto-updates every %d\") <a href='/'>refresh now</a></small></p>"
    , 255, 255, 170,  // background color
    PublicName(NULL),
    autoUpdateFreq);
  webServer.sendContent(s);

  HtmlPrepareStatusDiv(s, sizeof(s),
    "/", mStatus, machine.GetHeating(), machine.GetTemperature(), machine.GetSafeSwitchStatus());
  webServer.sendContent(s);

  switch (mStatus)
  {
  default:
    snprintf(s, sizeof(s),
      "<div>"
      "<h3>Process</h3>"
      "cooking completed (or aborted)"
      "</div>"
    );
    break;

  case MachineStatus::InitialWaiting:
    snprintf(s, sizeof(s),
      "<div>"
      "<table>"
      "<h3>Process</h3>"
      "<tr><td style='color:red;'>[%s] <b>will start in %s</b></td></tr>"
      "<tr><td>cooking time %s, ready at %s</td></tr>"
      "<tr><td>will keep warm for %s</td></tr>"
      "</table>"
      "</div>"
      ,
      strSecToMinSec(bufTm1, bufmmssSz, initialWaitSec), strSecToMinSec(bufTm0, bufmmssSz, mTime),
      strSecToMinSec(bufTm2, bufmmssSz, cookingSec),
      NowAndSeconds(bufTm, sizeof(bufTm), mTime + cookingSec),
      strSecToMinSec(bufTm3, bufmmssSz, keepWarmSec));
    break;

  case MachineStatus::Cooking:
    snprintf(s, sizeof(s),
      "<div>"
      "<h3>Process</h3>"
      "<table>"
      "<tr><td>%s delayed start completed</td></tr>"
      "<tr><td style='color:red;'>[%s] cooking, ready in <b>%s</b> at <b>%s</b></td></tr>"
      "<tr><td>will keep warm for %s</td></tr>"
      "</table>"
      "</div>"
      ,
      strSecToMinSec(bufTm1, bufmmssSz, initialWaitSec),
      strSecToMinSec(bufTm2, bufmmssSz, cookingSec), strSecToMinSec(bufTm0, bufmmssSz, mTime),
      NowAndSeconds(bufTm, sizeof(bufTm), mTime),
      strSecToMinSec(bufTm3, bufmmssSz, keepWarmSec));
    break;

  case MachineStatus::KeepWarm:
    snprintf(s, sizeof(s),
      "<div>"
      "<h3>Process</h3>"
      "<table>"
      "<tr><td>%s delayed start completed</td></tr>"
      "<tr><td>%s cooking completed</td></tr>"
      "<tr><td style='color:red;'>[%s] <b>keeping warm, %s</b></td></tr>"
      "</table>"
      "</div>"
      ,
      strSecToMinSec(bufTm1, bufmmssSz, initialWaitSec),
      strSecToMinSec(bufTm2, bufmmssSz, cookingSec),
      strSecToMinSec(bufTm3, bufmmssSz, keepWarmSec), strSecToMinSec(bufTm0, bufmmssSz, mTime));
    break;
  }
  webServer.sendContent(s);

  if (mStatus == MachineStatus::Idle || mStatus == MachineStatus::END)
  {
    snprintf(s, sizeof(s),
      "<div>"
      "<h3>Commands</h3>"
      "<form method='post' action='/'>"
      "<input type='submit' name='reset' value='RESET'>"
      "</form>"
      "</div>"
      "%s"
      "</body>"
      "</html>"
      , COPYRIGHT);
  }
  else
  {
    snprintf(s, sizeof(s),
      "<div>"
      "<h3>Commands</h3>"
      "<form method='post' action='/'>"
      "<input type='submit' name='stop' value='STOP' style='color:white; background-color:red;'>"
      "</form>"
      "</div>"
      "%s"
      "</body>"
      "</html>"
      , COPYRIGHT);
  }
  webServer.sendContent(s);

#ifdef VERBOSE_SERIAL
  Serial.println(s);
#endif //VERBOSE_SERIAL
  webServer.client().stop(); // Stop is needed because we sent no content length 
  //webServer.send(200, "text/html", s);
}



void ShowIdlePage()
{
  int mTime;
  MachineStatus mStatus;
  machine.GetStatus(&mStatus, &mTime);
  int initialWaitSec, cookingSec, keepWarmSec;
  machine.GetTimes(&initialWaitSec, &cookingSec, &keepWarmSec);

  SendHeaderAndHead(webServer, -1, PublicName(NULL));

  char s[2000];
  snprintf(s, sizeof(s),
    "<body bgcolor='#%02X%02X%02X'>"
    "<h2>%s</h2>"
    "<p><small><a href='/'>refresh page now</a></small></p>"
    , 0xAD, 0xD8, 0xE6,
    PublicName(NULL));
  webServer.sendContent(s);

  HtmlPrepareStatusDiv(s, sizeof(s),
    "/", mStatus, machine.GetHeating(), machine.GetTemperature(), machine.TestSafeSwitchStatus());
  webServer.sendContent(s);

  // propose a valid end time (considering and initial wait = 1')
  // i.e. give user 1' to accept proposed time
  //time_t now = DateTime::Now();
  DateTime *pEndTime = new DateTime(DateTime::Now());
  pEndTime->AddSeconds(cookingSec);
  pEndTime->AddMinutes(1);
  char TimeBuf[30];
  DateTime::LocalTimeToString(TimeBuf, sizeof(TimeBuf), *pEndTime, TimePrintOption_e::SecondsNo);
  snprintf(s, sizeof(s),
    "<h3>Actions</h3>"
    "<form method='post' action='/'>"
    "<table>"
    "<tbody>"
    "<tr>"
    "<td>Schedule by:</td><td>"
    "<input name='timeChoice' onclick='handleClick(this);' value='delay' type='radio'>delay<br>"
    "<input name='timeChoice' onclick='handleClick(this);' value='readyAt' checked type='radio'>end time"
    "</td>"
    "<td rowspan='4'>"
    "<br><br>"
    "<button type='submit' name='action' value='LOAD' title='Load default values'>LOAD</button>"
    "<br><br>"
    "<button type='submit' name='action' value='SAVE' title='Save values as the new defaults'>SAVE</button>"
    "</td></tr>"
    "<tr class='trWaitMinutes' style='display: none;'>"
    "<td>Delayed Start</td>"
    "<td><input name='waitMinutes' value='%d' size='4'>&nbsp;'</td>"
    "</tr>"
    "<tr><td>Cooking time</td><td><input name='cookingMinutes' value='%d' size='4'>&nbsp;'</td></tr>"
    "<tr class='trReadyAt'>"
    "<td>Ready at</td><td><input name='cookingHhMm' value='%s' size='7' title='enter hh:mm (24h) or h:mm am/pm'></td>"
    "</tr>"
    "<tr><td>Keep warm for</td><td><input name='keepWarmMinutes' value='%d' size='4'>&nbsp;'</td></tr>"
    "</tbody>"
    "</table>"
    "<br>"
    "<input type='submit' name='action' value='START'>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
    "<button type='submit' name='action' value='TEST' title='Switches on heat for 2\"'>TEST</button>"
    "</form>"
    "</div>"
    "%s"
    "<script>"
    // http://stackoverflow.com/questions/17518035/showing-hiding-table-rows-with-javascript-can-do-with-id-how-to-do-with-clas
    "function toggle_by_class(cls, on) {"
    "var lst = document.getElementsByClassName(cls);"
    "for (var i = 0; i < lst.length; ++i) {"
    "lst[i].style.display = on ? '' : 'none';"
    "}"
    "}"
    "function handleClick(myRadio) {"
    "if (myRadio.value == 'delay') {"
    "toggle_by_class('trWaitMinutes', true);"
    "toggle_by_class('trReadyAt', false);"
    "}"
    "if (myRadio.value == 'readyAt') {"
    "toggle_by_class('trWaitMinutes', false);"
    "toggle_by_class('trReadyAt', true);"
    "}"
    "}"
    "</script>"
    "</body>"
    "</html>"
    ,
    initialWaitSec / 60, cookingSec / 60,
    TimeBuf,
    keepWarmSec / 60,
    COPYRIGHT);
  webServer.sendContent(s);

#ifdef VERBOSE_SERIAL
  Serial.println(s);
#endif //VERBOSE_SERIAL
  webServer.client().stop(); // Stop is needed because we sent no content length 
  //webServer.send(200, "text/html", s);
}

/*
 - timeChoice = readyAt
 - waitMinutes = 1
 - cookingMinutes = 15
 - cookingHhMm = 10:39 am
 - keepWarmMinutes = 1
 - action = START
*/
void cookingMachineHandler()
{
  if (!webServer.authenticate(cfg.userName, cfg.userPassword))
  {
    webServer.requestAuthentication();
    return;
  }
#ifdef VERBOSE_SERIAL
  Serial.println("cookingMachineHandler():");
#else
  Serial.println("cookingMachineHandler()");
#endif //VERBOSE_SERIAL
  logDetails();

  int mTime;
  MachineStatus mStatus;
  machine.GetStatus(&mStatus, &mTime);

  int initialWaitSec, cookingSec, keepWarmSec;
  String cookinkEndTime;
  bool bInitialDelayByEndTime = false;
  bool bStop = false;
  bool bTest = false;
  bool bSave = false;
  bool bLoad = false;
  bool bStart = false;
  bool bReset = false;
  bool bSetTime = false;
  if (webServer.args() > 0)
  {
    for (int i = 0; i < webServer.args(); i++)
    {
      if (webServer.argName(i) == String("timeChoice"))
      {
        bInitialDelayByEndTime = webServer.arg(i) == String("readyAt");
        bSetTime = true;
      }
      else if (webServer.argName(i) == String("waitMinutes"))
      {
        initialWaitSec = atoi(webServer.arg(i).c_str()) * 60;
        bSetTime = true;
      }
      else if (webServer.argName(i) == String("cookingMinutes"))
      {
        cookingSec = atoi(webServer.arg(i).c_str()) * 60;
        bSetTime = true;
      }
      else if (webServer.argName(i) == String("cookingHhMm"))
      {
        cookinkEndTime = webServer.arg(i);
        bSetTime = true;
      }
      else if (webServer.argName(i) == String("keepWarmMinutes"))
      {
        keepWarmSec = atoi(webServer.arg(i).c_str()) * 60;
        bSetTime = true;
      }
      else if (webServer.argName(i) == String("action"))
      {
        bStart = webServer.arg(i) == String("START");
        bTest = webServer.arg(i) == String("TEST");
        bSave = webServer.arg(i) == String("SAVE");
        bLoad = webServer.arg(i) == String("LOAD");
      }
      else if (webServer.argName(i) == String("stop"))
        bStop = true;
      else if (webServer.argName(i) == String("reset"))
        bReset = true;
    }
  }

  if (bSetTime)
  {
#ifdef VERBOSE_SERIAL
    Serial.print("set Time: ");
    Serial.print("time choice: ");
    Serial.print(bInitialDelayByEndTime ? "readyAt" : "delay");
    Serial.print(", iniW: ");
    Serial.print(initialWaitSec);
    Serial.print(", cT: ");
    Serial.print(cookingSec);
    Serial.print(", endTm: ");
    Serial.print(cookinkEndTime);
    Serial.print(", kW: ");
    Serial.println(keepWarmSec);
#endif //VERBOSE_SERIAL

    if (bInitialDelayByEndTime)
    {
      // set initialWaitSec to get food cooked at specified time
      String errMsg;
      bool ok = SetInitialDelayFromEndTime(cookingSec, cookinkEndTime.c_str(), initialWaitSec, errMsg);
      if (initialWaitSec >= 0)
      {
        if (initialWaitSec <= 0)
          initialWaitSec = HTTP_PAGE_UPDATE_FREQ_SEC;
        machine.SetTimes(initialWaitSec, cookingSec, keepWarmSec);
      }
    }
    machine.SetTimes(initialWaitSec, cookingSec, keepWarmSec);
  }
  if (bStop)
  {
    Serial.println("user action: Stop");
    machine.StopCycle();
  }
  else if (bReset)
  {
    Serial.println("user action: Reset");
    machine.Reset();
  }
  else if (bTest)
  {
    Serial.println("user action: Test");
    Serial.flush();
    gWaitSec = initialWaitSec;
    gCookingSec = cookingSec;
    gKeepWarmSec = keepWarmSec;
    machine.SetTimes(0, MACHINE_STATUS_REFRESH_FREQ_SEC, 0);
    machine.SetAlarmDuration(0); // no alarm at end
    machine.StartCycle();
    gTestMode = true;
    //machine.Reset();
    //machine.SetTimes(initialWaitSec, cookingSec, keepWarmSec);
  }
  else if (bStart)
  {
    Serial.println("user action: Start cooking");
    Serial.flush();
    machine.SetAlarmDuration(10);
    machine.StartCycle();
  }
  else if (bLoad)
  {
    Serial.println("user action: Load");
    Serial.flush();
    CookingSession cs;

    if (CookingSession_Load(cs))
    {
      initialWaitSec = cs.waitTimeMin * 60;
      cookingSec = cs.cookingTimeMin * 60;
      keepWarmSec = cs.keepWarmTimeMin * 60;
      machine.SetTimes(initialWaitSec, cookingSec, keepWarmSec);
    }
    //else
    //  error
  }
  else if (bSave)
  {
    Serial.println("user action: Save");
    Serial.flush();
    CookingSession cs;

    cs.waitTimeMin = initialWaitSec / 60;
    cs.cookingTimeMin = cookingSec / 60;
    cs.keepWarmTimeMin = keepWarmSec / 60;
    CookingSession_Save(cs);
  }

  machine.GetStatus(&mStatus, &mTime);
  if (mStatus == MachineStatus::Idle ||
    gTestMode && mStatus == MachineStatus::END)
  {
    if (mStatus == MachineStatus::END)
    {
      machine.Reset();
      machine.SetTimes(gWaitSec, gCookingSec, gKeepWarmSec);
    }
    gTestMode = false;
    ShowIdlePage();
    //machineIOs.SetLeds(Off, noChange, noChange);
  }
  else
  {
    ShowCookingPage();
    //machineIOs.SetLeds(On, noChange, noChange);
  }
}
