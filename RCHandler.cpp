#include "RCHandler.h"
#include <RCSwitch.h>

void RCHandler::init() {
  sw.enableTransmit(2);//GPIO2 / D4
  sw.setPulseLength(180);
}

void RCHandler::on(int id) {
  sw.send(id, 24);
}
void RCHandler::off(int id) {
  sw.send((id & 0xFFFFF0) | 0b1100, 24);
}

