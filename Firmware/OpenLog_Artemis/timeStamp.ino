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
    char rtcDate[12]; //10/12/2019,
    if (settings.americanDateStyle == true)
      sprintf(rtcDate, "%02d/%02d/20%02d,", myRTC.month, myRTC.dayOfMonth, myRTC.year);
    else
      sprintf(rtcDate, "%02d/%02d/20%02d,", myRTC.dayOfMonth, myRTC.month, myRTC.year);
    strcat(timeStringBuffer, rtcDate);
  }

  if (settings.logTime)
  {
    char rtcTime[13]; //09:14:37.41,
    int adjustedHour = myRTC.hour;
    if (settings.hour24Style == false)
    {
      if (adjustedHour > 12) adjustedHour -= 12;
    }
    sprintf(rtcTime, "%02d:%02d:%02d.%02d,", adjustedHour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
    strcat(timeStringBuffer, rtcTime);
  }
  
  if (settings.logMicroseconds)
  {
    char microseconds[11]; //
    sprintf(microseconds, "%lu,", micros());
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

  adjustToLocalDateTime(year, month, day, hour, settings.localUTCOffset);
}

//Given the date and hour, calculate local date/time
//Adjust the hour by local hour offset
//Adjust the hour by DST as necessary
//Adjust the date as necessary
//Leap year is taken into account but does not interact with DST (DST happens later in March)
void adjustToLocalDateTime(int &year, int &month, int &day, int &hour, int localUTCOffset) {

  //Apply any offset to UTC
  hour += localUTCOffset;

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
      if (year % 4 == 0 && day == 30)
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
          if (year % 4 == 0) day = 29;
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
