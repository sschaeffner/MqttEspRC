#ifndef RC_HANDLER_H
#define RC_HANDLER_H

#include "RCSwitch.h"

class RCHandler {
  public:
    void init();
    void send(unsigned long code, unsigned int length);
  private:
    RCSwitch sw;
};
#endif //RC_HANDLER_H
