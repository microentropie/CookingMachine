#ifndef _myMACHINE_CYCLE_
#define _myMACHINE_CYCLE_

#include "machine_if.h"

enum MachineStatus
{
  Idle = 0,
  InitialWaiting = 1,
  Cooking = 2,
  KeepWarm = 3,
  END = 4,
};

//typedef void (*callback_t)(void); 

class MachineCycle : public MachineIf
{
public:
  //static void (MachineCycle::*pTimerCallback)() = NULL;
  //typedef void (MachineCycle::*membFunPtr)();

  MachineCycle();
  virtual ~MachineCycle();

  void Init();
  void Reset();
  void GetTimes(int *pInitialWaitSec, int *pCookingSec, int *pKeepWarmSec);
  bool SetTimes(int initialWaitSec, int cookingSec, int keepWarmSec);
  static void GetDefaultTimes(int &initialWait, int &cooking, int &keepWarm);
  void SetDefaultTimes();
  void SetAlarmDuration(int sec);

  void StartCycle();
  void StopCycle();
  void GetStatus(MachineStatus *pStatus, int *pTimeValue);
  bool IsWorking();

  static const char *StatusToString(MachineStatus status);

protected:
  // status variables:
  int m_machineStatus;
  int m_currentSec;

  // set times:
  int initialWaitSec;
  int cookingSec;
  int keepWarmSec;
  int StatusRefreshFreqSec = MACHINE_STATUS_REFRESH_FREQ_SEC;

private:
  void initTimer();
  void startTimer();
  void stopTimer();
  bool m_bUpdateStatus;
  int  m_alarmDurationSec;

public:

  void UpdateStatus();
  void AsyncUpdateStatus();
  //static void TickFunctionCaller(MachineCycle *p);
  //membFunPtr pTimerCallback;
  //callback_t  pTimerCallbackF;
};

#endif //_myMACHINE_CYCLE_
