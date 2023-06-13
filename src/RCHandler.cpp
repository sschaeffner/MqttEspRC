#include "RCHandler.h"
#include <RCSwitch.h>

void RCHandler::init() {
  sw.enableTransmit(2);//GPIO2 / D4
  sw.setPulseLength(180);
  sw.setRepeatTransmit(20);
}

void RCHandler::send(unsigned long code, unsigned int length) {
  sw.send(code, length);
}
