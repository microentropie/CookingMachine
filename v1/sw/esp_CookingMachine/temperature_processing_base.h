#ifndef _TEMPERATURE_BASE_
#define _TEMPERATURE_BASE_

#define NO_TEMP -500
//#define _DEBUG_  

struct ntcTranslationAdcTemp
{
  unsigned short adc; // adc value
  short temp; // temperature in °C multiplied by 10
};

#define MAX_TEMP_SETPOINTS 16
extern ntcTranslationAdcTemp NtcTable[MAX_TEMP_SETPOINTS] /*PROGMEM*/;
extern int qtNtcElem;
extern bool TempSetPointsSave(struct ntcTranslationAdcTemp Table[]);

extern int TempEntryCompare(struct ntcTranslationAdcTemp *p1, struct ntcTranslationAdcTemp *p2);

#endif //_TEMPERATURE_BASE_
