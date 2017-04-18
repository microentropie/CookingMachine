#ifndef _myMACHINE_IF_
#define _myMACHINE_IF_

#include "machine_io.h"

enum machineHeatingStatus
{
  OFF = 0,
  WARM = 1,
  HOT = 2,
};

#define HTTP_PAGE_UPDATE_FREQ_SEC 5
#define MACHINE_STATUS_REFRESH_FREQ_SEC 2

class MachineIf
{
public:
  MachineIf();
  void Init();
  void SetHeating(machineHeatingStatus newStatus, int millisecDelay = 100);
  machineHeatingStatus GetHeating();
  int GetTemperature();

  SafeSwitchStatus GetSafeSwitchStatus();
  SafeSwitchStatus TestSafeSwitchStatus();

  void SoundAlarm(int sec);
  void ModulateSound(bool modulation);

  static String TemperatureToString(int temp);

protected:
  bool m_bUpdateSensorsStatus;

private:
  machineHeatingStatus heatingStatus;
  SafeSwitchStatus safeSwitchStatus;
  long safeSwitchStatusTimeStamp;

  bool soundOn;
  long soundOffTimeout;
};

#endif //_myMACHINE_IF_
