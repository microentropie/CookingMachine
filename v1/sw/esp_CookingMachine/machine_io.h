#ifndef _myMACHINE_IO_
#define _myMACHINE_IO_

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif //uint8_t

#define TEMP_NO_READING (-500)

enum OutSetAction_e
{
  Off, On, noChange
};

enum SafeSwitchStatus
{
  Undefined, Armed, NotArmed
};

class MachineIo
{
public:
  MachineIo();
  void Init();
  void SetRelays(OutSetAction_e rMain, OutSetAction_e rHot, OutSetAction_e rPump);
  void GetRelays(OutSetAction_e &rMain, OutSetAction_e &rHot, OutSetAction_e &rPump);
  int  GetTemperature();
  SafeSwitchStatus GetSafeSwitchStatus();
  void SetLeds(OutSetAction_e r, OutSetAction_e g, OutSetAction_e b);
  void SetAlarm(bool soundOn);

  bool GetButton();
};
#define outON 0
#define outOFF (!outON)

extern MachineIo machineIOs;

#endif //_myMACHINE_IO_
