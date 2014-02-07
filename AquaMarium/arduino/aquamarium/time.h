
#ifndef _TIME_H_
#define _TIME_H_

// Number of seconds between 1-Jan-1900 and 1-Jan-1970, unix time starts 1970
// and ntp time starts 1900.
#define GETTIMEOFDAY_TO_NTP_OFFSET 2208988800UL

#define EPOCH_YR 1970
//(24L * 60L * 60L)
#define SECS_DAY 86400UL
#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)

static const char day_abbrev[] PROGMEM = "SunMonTueWedThuFriSat";

uint8_t monthlen(uint8_t isleapyear,uint8_t month);
uint8_t gmtime(const uint32_t time,char *day, char *clock);

#endif // _TIME_H_