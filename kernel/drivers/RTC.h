#ifndef RTC_H
#define RTC_H

void rtc_get_time(int* hour, int* minute, int* second);
const char* rtc_time_to_string();

#endif
