#ifndef _BUTTON_H_
#define _BUTTON_H_

#define BUTTON_CURRENTLY_PRESSED 1
#define BUTTON_LONG_PRESS        2
#define BUTTON_CLICK_UP          4

bool ButtonInit(signed char button);

// returns a combination of BUTTON_...
byte ButtonStatusGet();

#endif //_BUTTON_H_
