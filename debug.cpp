#include "debug.h"

///////////////////////////////////////////////////////////////////////////
//   TELNET
///////////////////////////////////////////////////////////////////////////
/*
   Function called to handle Telnet clients
   https://www.youtube.com/watch?v=j9yW10OcahI
*/
#if defined DEBUG_TELNET

WiFiServer telnetServer(23);
WiFiClient telnetClient;

void handleTelnet(void) {
    if (telnetServer.hasClient()) {
        if (!telnetClient || !telnetClient.connected()) {
            if (telnetClient) {
                telnetClient.stop();
            }
            telnetClient = telnetServer.available();
        } else {
            telnetServer.available().stop();
        }
    }
}

void startTelnet(void) {
    telnetServer.begin();
    telnetServer.setNoDelay(true);
}
#endif