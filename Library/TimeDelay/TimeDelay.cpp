#include <TimeDelay.h>

TimeDelay::TimeDelay()
:timex(millis()),offset(0)
{
}

void TimeDelay::setdelay(unsigned long mils)
{
   timex=millis();
   offset=mils;
}

bool TimeDelay::chkdelay()
{
   unsigned long testval = millis();
   bool retval = false;
   if (timex + offset < testval)
   {
     timex = testval;
     retval = true;
   }
   return retval;
}

bool TimeDelay::chkdelay(unsigned long mils)
{
   unsigned long testval = millis();
   bool retval = false;
   if (timex + mils < testval)
   {
     timex = testval;
     retval = true;
   }
   return retval;
}

