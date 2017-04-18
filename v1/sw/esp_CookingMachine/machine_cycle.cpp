/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

machine cycle layer, the heart of the machine

Sources repository: https://github.com/microentropie/
*/

#include <Arduino.h>
#include <Ticker.h>
#include "machine_cycle.h"
#include "common.h"
#include "settings.h"

// as 60 = 2^2 * 3 * 5, in order to have a neat 60" cycle (60 / StatusRefreshFreqSec = quotient without remainder),
// StatusRefreshFreqSec values at which status updates must be a multiple of 1 or more of those values,
// example: 1, 2, 3, 4, 5, 6, 10, 12, ...

MachineCycle::MachineCycle() : MachineIf()
{
  // cannot call libraries methods from constructor
  // only variables initialization with constant values
  this->m_bUpdateStatus = false;
  this->m_machineStatus = MachineStatus::Idle;
  this->m_currentSec = 0;
  //CookingSession cfg;
  SetDefaultTimes();
  this->m_alarmDurationSec = 10;

  initTimer();
}

MachineCycle::~MachineCycle()
{
  stopTimer();
}

void MachineCycle::Init()
{
#ifdef VERBOSE_SERIAL
  Serial.println("MachineCycle::Init()");
  Serial.flush();
#endif //VERBOSE_SERIAL

  MachineIf::Init();
  Reset();
  CookingSession cfg;
  if (CookingSession_Load(cfg))
  {
    this->initialWaitSec = cfg.waitTimeMin * 60;
    this->cookingSec = cfg.cookingTimeMin * 60;
    this->keepWarmSec = cfg.keepWarmTimeMin * 60;
  }
  else
  {
    SetDefaultTimes();
    cfg.waitTimeMin = this->initialWaitSec / 60;
    cfg.cookingTimeMin = this->cookingSec / 60;
    cfg.keepWarmTimeMin = this->keepWarmSec / 60;
    CookingSession_Save(cfg);
  }
}

void MachineCycle::Reset()
{
  SetHeating(machineHeatingStatus::OFF, 0);

  this->m_machineStatus = MachineStatus::Idle;
  this->m_currentSec = 0;
}

void MachineCycle::GetDefaultTimes(int &initialWait, int &cooking, int &keepWarm)
{
  //set default timing values (in seconds):
  initialWait = 5;
  cooking = 15;
  keepWarm = 1;
}

void MachineCycle::SetDefaultTimes()
{
  int initialWait, cooking, keepWarm;

  GetDefaultTimes(initialWait, cooking, keepWarm);

  //set default timing values (in seconds):
  this->initialWaitSec = initialWait * 60;
  this->cookingSec = cooking * 60;
  this->keepWarmSec = keepWarm * 60;
}

void MachineCycle::GetTimes(int *pInitialWaitSec, int *pCookingSec, int *pKeepWarmSec)
{
  if (pInitialWaitSec)
    *pInitialWaitSec = this->initialWaitSec;
  if (pCookingSec)
    *pCookingSec = this->cookingSec;
  if (pKeepWarmSec)
    *pKeepWarmSec = this->keepWarmSec;
}

bool MachineCycle::SetTimes(int initialWaitSec, int cookingSec, int keepWarmSec)
{
  if (initialWaitSec < 0 || initialWaitSec > 86400) // 0 .. 24 h
    return false;
  if (cookingSec < 0 || cookingSec > 10800) // 0 .. 3 h
    return false;
  if (keepWarmSec < 0 || keepWarmSec > 86400) // 0 .. 24 h
    return false;
  if (initialWaitSec == 0 && cookingSec == 0 && keepWarmSec == 0)
    return false; // nothing to do

  this->initialWaitSec = initialWaitSec;
  this->cookingSec = cookingSec;
  this->keepWarmSec = keepWarmSec;

  return true;
}

void MachineCycle::SetAlarmDuration(int sec)
{
  this->m_alarmDurationSec = sec;
}

void MachineCycle::StartCycle()
{
  Serial.println("StartCycle()");
  stopTimer();
  m_machineStatus = MachineStatus::Idle;
  m_currentSec = this->initialWaitSec;
  m_machineStatus = MachineStatus::InitialWaiting;
  startTimer();
}



void MachineCycle::StopCycle()
{
  Serial.println("StopCycle()");
  stopTimer();
  SetHeating(machineHeatingStatus::OFF);
  if (m_machineStatus != MachineStatus::Idle)
    m_machineStatus = MachineStatus::END;
  machineIOs.SetLeds(Off, Off, noChange);

  //Init();
}


void MachineCycle::AsyncUpdateStatus()
{
  // this routine is async-called:
  // better not calling library functions
  // from here, may lead to unexpected results
  // if not carefully pounded.
  // Hence NO log output here:
  //Serial.println("AsyncUpdateStatus()");
  m_bUpdateSensorsStatus = true;
  m_bUpdateStatus = m_currentSec <= 0;
  if (m_bUpdateStatus)
    return;
  m_currentSec -= StatusRefreshFreqSec;
}


void MachineCycle::UpdateStatus()
{
  if (m_bUpdateSensorsStatus)
  {
    m_bUpdateSensorsStatus = false;

    if (m_machineStatus == MachineStatus::KeepWarm)
      TestSafeSwitchStatus(); // reading status is easy now (does not interfere with heating)
    else if (m_machineStatus == MachineStatus::Cooking)
    {
      // Read safe sw status and warn (beep) if cooking will not take place.
      // If the sw is not armed, cooking is not taking place, then forcing a phisical read
      // does not impact on relay contacts wearing: HOT (i.e. current) is disabled by safe switch itself.
      // If safe sw is enabled just check the first 10" and then every minute
      SafeSwitchStatus sSwStatus = GetSafeSwitchStatus();

      if (sSwStatus == SafeSwitchStatus::NotArmed ||
        sSwStatus == SafeSwitchStatus::Undefined && (m_currentSec >= cookingSec - 10 || (m_currentSec % 60) == 0))
        sSwStatus = TestSafeSwitchStatus();
      if (sSwStatus == SafeSwitchStatus::NotArmed)
      {
        // Warn user that cooking did not start (bad !) or
        // ended due to safe switch toggle (cooking completed in advance)
        SoundAlarm(-10);
      }
    }
    else if (m_machineStatus == MachineStatus::InitialWaiting)
    {
      // read status and warn (beep) if cooking will not take place;
      // as reading sw status interferes with heating,
      // it is tested only at specified moments: for the first 10" and then once every minute
      SafeSwitchStatus sSwStatus = GetSafeSwitchStatus();

      if (sSwStatus == SafeSwitchStatus::Undefined && (m_currentSec >= initialWaitSec - 10 || (m_currentSec % 60) == 0))
        sSwStatus = TestSafeSwitchStatus();
      if (sSwStatus == SafeSwitchStatus::NotArmed)
      {
        // Warn user that cooking will not start
        SoundAlarm(-10);
      }
    }
  }

  if (!m_bUpdateStatus)    return; // nothing to do
  m_bUpdateStatus = false;

#ifdef VERBOSE_SERIAL
  Serial.println("UpdateStatus() begin");
#endif //VERBOSE_SERIAL

  // change status
  Serial.print("Machine Status change: ");
  Serial.print(StatusToString((MachineStatus)m_machineStatus));
  Serial.print(" => ");
  ++m_machineStatus;
  Serial.println(StatusToString((MachineStatus)m_machineStatus));

  if (m_machineStatus == MachineStatus::InitialWaiting)
  {
    m_currentSec = initialWaitSec;
    machineIOs.SetLeds(On, On, noChange);
  }
  else if (m_machineStatus == MachineStatus::Cooking)
  {
    m_currentSec = cookingSec;
    machineIOs.SetLeds(On, Off, noChange);
  }
  else if (m_machineStatus == MachineStatus::KeepWarm)
  {
    m_currentSec = keepWarmSec;
    machineIOs.SetLeds(Off, On, noChange);
  }
  else if (m_machineStatus >= MachineStatus::END)
  {
    //m_status = MachineStatus::Idle;
    m_currentSec = 0;
    machineIOs.SetLeds(Off, Off, noChange);
  }

  if (m_machineStatus == MachineStatus::Cooking)
    SetHeating(machineHeatingStatus::HOT);
  else if (m_machineStatus == MachineStatus::KeepWarm)
  {
    SetHeating(machineHeatingStatus::WARM);
    SoundAlarm(this->m_alarmDurationSec * 1000);
  }
  else
    SetHeating(machineHeatingStatus::OFF);

  if (m_machineStatus == MachineStatus::Idle ||
    m_machineStatus == MachineStatus::END)
    StopCycle();

#ifdef VERBOSE_SERIAL
  Serial.print("UpdateStatus() end:");
  Serial.print("m_status=");
  Serial.print(StatusToString((MachineStatus)m_machineStatus));
  Serial.print(", m_currentSec=");
  Serial.println(m_currentSec);
#endif //VERBOSE_SERIAL
}

bool MachineCycle::IsWorking()
{
  return (MachineStatus)this->m_machineStatus != MachineStatus::Idle &&
    (MachineStatus)this->m_machineStatus != MachineStatus::END;
}

void MachineCycle::GetStatus(MachineStatus *pStatus, int *pTimeValue)
{
  if (pStatus)
    *pStatus = (MachineStatus)this->m_machineStatus;
  if (pTimeValue)
    *pTimeValue = this->m_currentSec;
}

const char *MachineCycle::StatusToString(MachineStatus stts)
{
  switch (stts)
  {
  case Idle:
    return "Idle";
  case InitialWaiting:
    return "InitialWaiting";
  case Cooking:
    return "Cooking";
  case KeepWarm:
    return "KeepWarm";
  case END:
    return "End";
  }
  return "?";
}


// -- time functions - begin
Ticker ticker;

// create a pure function, required by Ticker class.
// This implementation prevents from creating more than one instances
// of MachineCycle class. An error is issued via Serial if any.
// Think this is not a problem and could eventually be extended
// creating an array of class-instance-pointer & function.
MachineCycle *pClassInstancePointer = NULL;
void pureFnTimerCallback()
{
  // warning this is an async call and may interfere with other libraries,
  // so NO libraries call if possible and as quick as possible
  if (!pClassInstancePointer) return;
  pClassInstancePointer->AsyncUpdateStatus();
  // NO yield():
  //yield();
}

void MachineCycle::initTimer()
{
  pClassInstancePointer = NULL;
  //#ifdef VERBOSE_SERIAL
  //  // better not running Serial...: this method is called before setup()
  //  Serial.println("initTimer()");
  //#endif //VERBOSE_SERIAL
}

void MachineCycle::startTimer()
{
#ifdef VERBOSE_SERIAL
  Serial.println("startTimer()");
  Serial.flush();
#endif //VERBOSE_SERIAL
  if (pClassInstancePointer)
  {
    Serial.println("Error cannot start a new timer, too many instances of MachineCycle class running");
    Serial.flush();
    return;
  }
  pClassInstancePointer = this;
  //os_timer_setfn(&myTimer, timerCallback, NULL);
  //os_timer_arm(&myTimer, 1000, 1); // fire event every second

 //pTimerCallback = &MachineCycle::InternalUpdateStatus;
 //pTimerCallbackF = (callback_t)&MachineCycle::InternalUpdateStatus;
 //(*this.*pTimerCallback)();
 //(this->*pTimerCallback)();
 //pTimerCallbackF();
  ticker.attach_ms(StatusRefreshFreqSec * 1000, pureFnTimerCallback); // fire event every <StatusRefreshFreqSec> seconds
}

void MachineCycle::stopTimer()
{
  //os_timer_disarm(&myTimer);
  ticker.detach();
  pClassInstancePointer = NULL;
#ifdef VERBOSE_SERIAL
  Serial.println("stopTimer()");
#endif //VERBOSE_SERIAL
}
// -- time functions - end

