#ifndef _COMMON_H_
#define _COMMON_H_

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif //uint8_t
//extern const char *cookingMachineName;
extern const char *COPYRIGHT PROGMEM;
extern const char *COPYRIGHTtxt PROGMEM;

//#define VERBOSE_SERIAL

/*
  pay attention when using PROGMEM with ESP8266,
  PROGMEM stores variables in EEPROM, which is not directly accessible by the ESP memory map
  see: http://www.esp8266.com/viewtopic.php?f=28&t=5549#sthash.U8vc5INj.dpuf
*/

#define bufmmssSz 11
char *strSecToMinSec(char *buf, int bLen, int sec);
char *PublicName(const char *prefixName);

#endif //_COMMON_H_
