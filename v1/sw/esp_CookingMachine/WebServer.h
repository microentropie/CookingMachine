#ifndef _myWEBSERVER_
#define _myWEBSERVER_

extern void setupWebServer();
extern void loopWebServer();

class ESP8266WebServer;
extern void SendHeaderAndHead(ESP8266WebServer &webServer, int refreshSec, char *title);
extern void RedirectTo(char *url);
extern void logDetails();
extern void notFoundHandler();
extern void forbiddenHandler();
extern void badRequestHandler();
void handleConfigurationNotAllowed();

#endif //_myWEBSERVER_
