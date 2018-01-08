#ifndef RC_HANDLER_H
#define RC_HANDLER_H

#include <RCSwitch.h>

/*
 * 1 - 333107 333116
 * 2 - 333251 333260
 * 3 - 333571 333580
 * 4 - 335107 335116
 */


class RCHandler {
  public:
    void init();
    void on(int id);
    void off(int id);
  private:
    RCSwitch sw;
};
#endif //RC_HANDLER_H
