//Query the RTC and put the appropriately formatted (according to settings) 
//string into the passed buffer. timeStringBuffer should be at least 37 chars long
//Code modified by @DennisMelamed in PR #70
void getTimeString(char timeStringBuffer[])
{
  //reset the buffer
  timeStringBuffer[0] = '\0';

  myRTC.getTime();

  if (settings.logDate)
  {
    char rtcDate[12]; // 10/12/2019,
    char rtcDay[3];
    char rtcMonth[3];
    char rtcYear[5];
    if (myRTC.dayOfMonth < 10)
      sprintf(rtcDay, "0%d", myRTC.dayOfMonth);
    else
      sprintf(rtcDay, "%d", myRTC.dayOfMonth);
    if (myRTC.month < 10)
      sprintf(rtcMonth, "0%d", myRTC.month);
    else
      sprintf(rtcMonth, "%d", myRTC.month);
    if (myRTC.year < 10)
      sprintf(rtcYear, "200%d", myRTC.year);
    else
      sprintf(rtcYear, "20%d", myRTC.year);
    if (settings.dateStyle == 0)
      sprintf(rtcDate, "%s/%s/%s,", rtcMonth, rtcDay, rtcYear);
    else if (settings.dateStyle == 1)
      sprintf(rtcDate, "%s/%s/%s,", rtcDay, rtcMonth, rtcYear);
    else if (settings.dateStyle == 2)
      sprintf(rtcDate, "%s/%s/%s,", rtcYear, rtcMonth, rtcDay);
    else // if (settings.dateStyle == 3)
      sprintf(rtcDate, "%s-%s-%sT", rtcYear, rtcMonth, rtcDay);
    strcat(timeStringBuffer, rtcDate);
  }

  if ((settings.logTime) || ((settings.logDate) && (settings.dateStyle == 3)))
  {
    char rtcTime[16]; //09:14:37.41, or 09:14:37+00:00,
    int adjustedHour = myRTC.hour;
    if (settings.hour24Style == false)
    {
      if (adjustedHour > 12) adjustedHour -= 12;
    }
    char rtcHour[3];
    char rtcMin[3];
    char rtcSec[3];
    char rtcHundredths[3];
    char timeZoneH[4];
    char timeZoneM[4];
    if (adjustedHour < 10)
      sprintf(rtcHour, "0%d", adjustedHour);
    else
      sprintf(rtcHour, "%d", adjustedHour);
    if (myRTC.minute < 10)
      sprintf(rtcMin, "0%d", myRTC.minute);
    else
      sprintf(rtcMin, "%d", myRTC.minute);
    if (myRTC.seconds < 10)
      sprintf(rtcSec, "0%d", myRTC.seconds);
    else
      sprintf(rtcSec, "%d", myRTC.seconds);
    if (myRTC.hundredths < 10)
      sprintf(rtcHundredths, "0%d", myRTC.hundredths);
    else
      sprintf(rtcHundredths, "%d", myRTC.hundredths);
    if (settings.localUTCOffset >= 0)
    {
      if (settings.localUTCOffset < 10)
        sprintf(timeZoneH, "+0%d", (int)settings.localUTCOffset);
      else
        sprintf(timeZoneH, "+%d", (int)settings.localUTCOffset);
    }
    else
    {
      if (settings.localUTCOffset <= -10)
        sprintf(timeZoneH, "-%d", 0 - (int)settings.localUTCOffset);
      else
        sprintf(timeZoneH, "-0%d", 0 - (int)settings.localUTCOffset);
    }
    int tzMins = (int)((settings.localUTCOffset - (float)((int)settings.localUTCOffset)) * 60.0);
    if (tzMins < 0)
      tzMins = 0 - tzMins;
    if (tzMins < 10)
      sprintf(timeZoneM, ":0%d", tzMins);
    else
      sprintf(timeZoneM, ":%d", tzMins);
    if ((settings.logDate) && (settings.dateStyle == 3))
    {
      sprintf(rtcTime, "%s:%s:%s%s%s,", rtcHour, rtcMin, rtcSec, timeZoneH, timeZoneM);
      strcat(timeStringBuffer, rtcTime);      
    }
    if (settings.logTime)
    {
      sprintf(rtcTime, "%s:%s:%s.%s,", rtcHour, rtcMin, rtcSec, rtcHundredths);
      strcat(timeStringBuffer, rtcTime);
    }
  }
  
  if (settings.logMicroseconds)
  {
    // Convert uint64_t to string
    // Based on printLLNumber by robtillaart
    // https://forum.arduino.cc/index.php?topic=143584.msg1519824#msg1519824
    char microsecondsRev[20]; // Char array to hold to microseconds (reversed order)
    char microseconds[20]; // Char array to hold to microseconds (correct order)
    uint64_t microsNow = micros();
    unsigned int i = 0;
    
    if (microsNow == 0ULL) // if usBetweenReadings is zero, set tempTime to "0"
    {
      microseconds[0] = '0';
      microseconds[1] = ',';
      microseconds[2] = 0;
    }
    
    else
    {
      while (microsNow > 0)
      {
        microsecondsRev[i++] = (microsNow % 10) + '0'; // divide by 10, convert the remainder to char
        microsNow /= 10; // divide by 10
      }
      unsigned int j = 0;
      while (i > 0)
      {
        microseconds[j++] = microsecondsRev[--i]; // reverse the order
        microseconds[j] = ',';
        microseconds[j+1] = 0; // mark the end with a NULL
      }
    }
    
    strcat(timeStringBuffer, microseconds);
  }
}

//Gets the current time from GPS
//Adjust the hour by local hour offset
//Adjust the date as necessary
//
//Note: this function should only be called if we know that a u-blox GNSS is actually connected
//
void getGPSDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second, int &millisecond, bool &dateValid, bool &timeValid) {
  //Get latested date/time from GPS
  //These will be extracted from a single PVT packet
  getUbloxDateTime(year, month, day, hour, minute, second, millisecond, dateValid, timeValid);
  //Do it twice - to make sure the data is fresh
  getUbloxDateTime(year, month, day, hour, minute, second, millisecond, dateValid, timeValid);

  adjustToLocalDateTime(year, month, day, hour, minute, settings.localUTCOffset);
}

//Given the date and hour, calculate local date/time
//Adjust the hour by local hour offset
//Adjust the hour by DST as necessary
//Adjust the date as necessary
//Leap year is taken into account but does not interact with DST (DST happens later in March)
void adjustToLocalDateTime(int &year, int &month, int &day, int &hour, int &minute, float localUTCOffset) {

  //Apply any offset to UTC
  hour += (int)localUTCOffset;

  //Apply minutes offset
  int tzMins = (int)((localUTCOffset - (float)((int)localUTCOffset)) * 60.0);
  minute += tzMins;
  if (minute >= 60)
  {
    hour += 1;
    minute -= 60;
  }
  else if (minute < 0)
  {
    hour -= 1;
    minute += 60;
  }

  //If the adjusted hour is outside 0 to 23, then adjust date as necessary
  correctDate(year, month, day, hour);

  //Should we correct for daylight savings time?
  if (settings.correctForDST == true)
  {
    //Calculate DST adjustment based on date and local offset
    hour += findUSDSTadjustment(year, month, day, hour);

    //DST may have pushed a date change so do one more time
    correctDate(year, month, day, hour);
  }
}

//If the given hour is outside 0 to 23, then adjust date and hour as necessary
void correctDate(int &year, int &month, int &day, int &hour)
{
  //Adjust date forwards if the local hour offset causes it
  if (hour > 23)
  {
    hour -= 24;
    day++;
    bool adjustMonth = false;
    if (month == 1 && day == 32)
      adjustMonth = true;
    else if (month == 2)
    {
      if (year % 4 == 0 && day == 30) // Note: this will fail in 2100. 2100 is not a leap year.
        adjustMonth = true;
      else if (day == 29)
        adjustMonth = true;
    }
    else if (month == 3 && day == 32)
      adjustMonth = true;
    else if (month == 4 && day == 31)
      adjustMonth = true;
    else if (month == 5 && day == 32)
      adjustMonth = true;
    else if (month == 6 && day == 31)
      adjustMonth = true;
    else if (month == 7 && day == 32)
      adjustMonth = true;
    else if (month == 8 && day == 32)
      adjustMonth = true;
    else if (month == 9 && day == 31)
      adjustMonth = true;
    else if (month == 10 && day == 32)
      adjustMonth = true;
    else if (month == 11 && day == 31)
      adjustMonth = true;
    else if (month == 11 && day == 32)
      adjustMonth = true;

    if (adjustMonth == true)
    {
      month++;
      day = 1;
      if (month == 13)
      {
        month = 1;
        year++;
      }
    }
  }

  //Adjust date backwards if the local hour offset causes it
  if (hour < 0)
  {
    hour += 24;
    day--;
    if (day == 0)
    {
      //Move back a month and reset day to the last day of the new month
      month--;
      switch (month)
      {
        case 0: //December
          year--;
          month = 12;
          day = 31;
          break;
        case 1: //January
          day = 31;
          break;
        case 2: //February
          if (year % 4 == 0) day = 29; // Note: this will fail in 2100. 2100 is not a leap year.
          else day = 28;
          break;
        case 3: //March
          day = 31;
          break;
        case 4: //April
          day = 30;
          break;
        case 5: //May
          day = 31;
          break;
        case 6: //June
          day = 30;
          break;
        case 7: //July
          day = 31;
          break;
        case 8: //August
          day = 31;
          break;
        case 9: //September
          day = 30;
          break;
        case 10: //October
          day = 31;
          break;
        case 11: //November
          day = 30;
          break;
      }
    }
  }
}

//Given a year/month/day/current UTC/local offset give me the amount to adjust the current hour
//Clocks adjust at 2AM so we need the local hour as well
int findUSDSTadjustment(int year, byte month, byte day, byte localHour)
{
  //Since 2007 DST starts on the second Sunday in March and ends the first Sunday of November
  //Let's just assume it's going to be this way for awhile (silly US government!)
  //Example from: http://stackoverflow.com/questions/5590429/calculating-daylight-savings-time-from-only-date

  //boolean dst = false; //Assume we're not in DST
  if (month > 3 && month < 11) return (1); //DST is happening!
  if (month < 3 || month > 11) return (0); //DST is not happening

  int firstSunday = getFirstSunday(year, month);
  int secondSunday = firstSunday + 7;

  //In March, we are in DST if we are on or after the second sunday.
  if (month == 3)
  {
    if (day > secondSunday) return (1); //We are in later march
    if (day < secondSunday) return (0); //No DST
    if (day == secondSunday)
    {
      if (localHour >= 2) return (1); //It's after 2AM, spring forward, add hour to clock
      else return (0); //It's before 2AM, no DST
    }
  }

  //In November we must be before the first Sunday to be DST.
  if (month == 11)
  {
    if (day < firstSunday) return (1); //Still in DST
    if (day > firstSunday) return (0); //No DST
    if (day == firstSunday) //Today is the last day of DST
    {
      //At 2AM the clock gets moved back to 1AM
      //When we check the local hour we need to assume DST is still being applied (at least for the 12, 1AM, 2AM checks)
      if ((localHour + 1) >= 2) return (0); //It's 2AM or later, fall back, remove hour from clock
      else return (1); //It's before 2AM, continue adding DST
    }
  }

  return (0); //We should not get here
}

//Given year/month, find day of first Sunday
byte getFirstSunday(int year, int month)
{
  int day = 1;
  while (dayOfWeek(year, month, day) != 0)
    day++;
  return (day);
}

//Given the current year/month/day
//Returns 0 (Sunday) through 6 (Saturday) for the day of the week
//From: http://en.wikipedia.org/wiki/Calculating_the_day_of_the_week
//This function assumes the month from the caller is 1-12
char dayOfWeek(int year, int month, int day)
{
  //Devised by Tomohiko Sakamoto in 1993, it is accurate for any Gregorian date:
  static int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4  };
  year -= month < 3;
  return (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
}
