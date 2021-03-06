
#include "time.h"

// isleapyear = 0-1
// month=0-11
// return: how many days a month has
//
// We could do this if ram was no issue:

uint8_t monthlen(uint8_t isleapyear,uint8_t month){
  const uint8_t mlen[2][12] = {
    {
      31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
    ,
    {
      31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
  };
  return(mlen[isleapyear][month]);
}


// gmtime -- convert calendar time (sec since 1970) into broken down time
// returns something like Fri 2007-10-19 in day and 01:02:21 in clock
// The return values is the minutes as integer. This way you can update
// the entire display when the minutes have changed and otherwise just
// write current time (clock). That way an LCD display needs complete
// re-write only every minute.
uint8_t gmtime(const uint32_t time,char *day, char *clock)
{
  char dstr[4];
  uint8_t i;
  uint32_t dayclock;
  uint16_t dayno;
  uint16_t tm_year = EPOCH_YR;
  uint8_t tm_sec,tm_min,tm_hour,tm_wday,tm_mon;

  dayclock = time % SECS_DAY;
  dayno = time / SECS_DAY;

  tm_sec = dayclock % 60UL;
  tm_min = (dayclock % 3600UL) / 60;
  tm_hour = dayclock / 3600UL;
  tm_wday = (dayno + 4) % 7; /* day 0 was a thursday */
  while (dayno >= YEARSIZE(tm_year)) {
    dayno -= YEARSIZE(tm_year);
    tm_year++;
  }
  tm_mon = 0;
  while (dayno >= monthlen(LEAPYEAR(tm_year),tm_mon)) {
    dayno -= monthlen(LEAPYEAR(tm_year),tm_mon);
    tm_mon++;
  }
  i=0;
  while (i<3){
    dstr[i]= pgm_read_byte(&(day_abbrev[tm_wday*3 + i]));
    i++;
  }
  dstr[3]='\0';
  sprintf_P(day,PSTR("%s %u-%02u-%02u"),dstr,tm_year,tm_mon+1,dayno + 1);
  sprintf_P(clock,PSTR("%02u:%02u:%02u"),tm_hour,tm_min,tm_sec);
  return(tm_min);
}

