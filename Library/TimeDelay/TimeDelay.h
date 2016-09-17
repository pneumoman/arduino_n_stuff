#ifndef timedelay_h
#define timedelay_h

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
 #include <pins_arduino.h>
#endif

class TimeDelay
{
    unsigned long timex;
    unsigned long offset;
    public:
    TimeDelay();
    void setdelay(unsigned long mils);
    bool chkdelay();
    bool chkdelay(unsigned long mils);
};

#endif
