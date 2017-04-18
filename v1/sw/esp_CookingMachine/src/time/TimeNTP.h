#ifndef _TimeNTP_H_
#define _TimeNTP_H_

extern void NtpTime_init(struct ntpConfig &ntpc);
extern void NtpTime_loop();

extern const char *TimeToString(time_t unixTime, bool printTzInfo, float timeZone, bool isDst);
extern const char *UtcTimeToString();
extern const char *LocalTimeToString(bool printTzInfo = false);

#endif //_TimeNTP_H_
