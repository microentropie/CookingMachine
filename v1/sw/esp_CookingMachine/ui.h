#ifndef _UI_H_
#define _UI_H_

void ui_init();

void ui_splashScreen();
void ui_ScreenSaver();

char ui_getSetInitialMode();

void Console_init();
void Console_print(const char *p);
void Console_println(const char *p);
void Console_println(String str);

void loopButton();

#endif //_UI_H_
