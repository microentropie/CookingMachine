/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2016-12-18

(rice) Cooking Machine (http://www.microentropie.com)
Arduino ESP8266 based.

Temperature processing

Sources repository: https://github.com/microentropie/
*/

#include <serializeEeprom.h>
#include "temperature_processing_base.h"

ntcTranslationAdcTemp NtcTable[MAX_TEMP_SETPOINTS] /*PROGMEM*/;
int qtNtcElem = 0;

bool TempInit()
{
  // load temperature settings table
  if (!TempSetPointsLoad())
  {
#ifdef _DEBUG_  
    Serial.println("Temp SetPoints: error reading from EEPROM.");
    Serial.println("Saving default values.");
#endif //_DEBUG_  
    qtNtcElem = 0;          // adc  °C x10
    NtcTable[qtNtcElem++] = { 73,  266 };
    NtcTable[qtNtcElem++] = { 80,  280 };
    NtcTable[qtNtcElem++] = { 108,  325 };
    NtcTable[qtNtcElem++] = { 348,  583 };
    NtcTable[qtNtcElem++] = { 381,  634 };
    NtcTable[qtNtcElem++] = { 944, 1000 };
    NtcTable[qtNtcElem++] = { 1001, 1600 };
    // warning: ALL table entries MUST be ordered by adc ascending
    for (int i = qtNtcElem; i < MAX_TEMP_SETPOINTS; ++i)
      NtcTable[i] = { 0, 0 };
    TempSetPointsSave(NULL);
  }

  // check if conversion table is ok (sorted ascending)
  if (qtNtcElem < 2)
    return false;
  unsigned int prevAdc = NtcTable[0].adc;
  for (int i = 1; i < qtNtcElem; ++i)
  {
    if (NtcTable[i].adc <= prevAdc)
      return false;
    prevAdc = NtcTable[i].adc;
  }
  return true;
}

int TempConvertAdc2Temp(unsigned int adc)
{
  //if(adc >= 1024)
  //  return NO_TEMP;
#ifdef _DEBUG_  
  Serial.println();
#endif //_DEBUG_  
  int iTempL = 0;
  int iTempH = 1;
  int i;
  for (i = 0; i < qtNtcElem; ++i)
  {
#ifdef _DEBUG_  
    Serial.print("NtcTable[");
    Serial.print(i);
    Serial.print("].adc="); Serial.println(NtcTable[i].adc);
#endif //_DEBUG_  
    if (NtcTable[i].adc > adc)
      break;
  }
#ifdef _DEBUG_  
  Serial.print("i="); Serial.println(i);
#endif //_DEBUG_  
  if (i == 0)
  {
    // value below 1st reference
    iTempL = 0;
    iTempH = 1;
  }
  else if (i >= qtNtcElem)
  {
    // value above last reference
    iTempL = qtNtcElem - 2;
    iTempH = qtNtcElem - 1;
  }
  else
  {
    iTempL = i - 1;
    iTempH = i;
  }

  double mul = (double)(NtcTable[iTempH].temp - NtcTable[iTempL].temp) / (double)((int)NtcTable[iTempH].adc - (int)NtcTable[iTempL].adc);
#ifdef _DEBUG_  
  Serial.print("iTempL="); Serial.println(iTempL);
  Serial.print("iTempH="); Serial.println(iTempH);
  Serial.print("dividend (temp diff)="); Serial.println(NtcTable[iTempH].temp - NtcTable[iTempL].temp);
  Serial.print("divisor (adc diff)="); Serial.println((int)NtcTable[iTempH].adc - (int)NtcTable[iTempL].adc);
  Serial.print("mul=");  Serial.println(mul);
  Serial.print("adc=");  Serial.println(adc);
  Serial.print("NtcTable[iTempL].adc=");  Serial.println(NtcTable[iTempL].adc);
  Serial.print("adc - NtcTable[iTempL].adc=");  Serial.println((int)adc - (int)NtcTable[iTempL].adc);
  Serial.print("NtcTable[iTempL].temp=");  Serial.println(NtcTable[iTempL].temp);
  return (int)((double)((int)adc - (int)NtcTable[iTempL].adc) * mul) + NtcTable[iTempL].temp;
#endif //_DEBUG_  
  return (int)((double)((int)adc - (int)NtcTable[iTempL].adc) * mul) + NtcTable[iTempL].temp;
}

//                                T E M P
#define TempSetPointsSignature 0x54454D50
#define TempSetPointsOffset 400
bool TempSetPointsSave(struct ntcTranslationAdcTemp Table[])
{
  if (!Table)
    Table = NtcTable;
  return serialize2eeprom::Save(EEPROM_log_RW, TempSetPointsSignature, TempSetPointsOffset, Table, sizeof(NtcTable));
}

bool TempSetPointsLoad()
{
  qtNtcElem = 0;
  bool ok = serialize2eeprom::Load(EEPROM_log_RW, TempSetPointsSignature, TempSetPointsOffset, NtcTable, sizeof(NtcTable));
  if (ok)
    for (int i = 0, qtNtcElem = 0; i < MAX_TEMP_SETPOINTS && NtcTable[i].adc > 0; ++i, ++qtNtcElem); // count elements
#ifdef _DEBUG_  
  Serial.print("Loaded ");
  Serial.print(qtNtcElem);
  Serial.println(" temperature setpoints from EEPROM.");
#endif //_DEBUG_  
  return ok;
}

int TempEntryCompare(struct ntcTranslationAdcTemp *p1, struct ntcTranslationAdcTemp *p2)
{
  int adc1 = (int)p1->adc;
  int adc2 = (int)p2->adc;
  // adc==0 is considered NOT AN ENTRY = free table entry, thus will be sorted to the end
  if (adc1 == 0)
    adc1 = 32767;
  if (adc2 == 0)
    adc2 = 32767;
  return adc1 - adc2;
}

