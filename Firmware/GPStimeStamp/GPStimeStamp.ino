
void setup() {
  Serial.begin(500000);

  Serial.println("GPS testing");

  int year = 18;
  int month = 12;
  int day = 30;
  int hour = 1;
  int minute = 15;
  int second = 5;
  int localUTCOffset = -7;

  //Example: Date roll forward
//  month = 1;
//  day = 31;
//  hour = 22;
//  localUTCOffset = 6;
  //Output should be 2/1/19 hour = 4 (no DST)

  //Example: Date roll back
//  month = 1;
//  day = 1;
//  year = 15;
//  hour = 2;
//  localUTCOffset = -7;
  //Output should be 12/31/14 hour = 19 (no DST)

  //Example: Date roll back with leap year interaction
//  month = 3;
//  day = 1;
//  year = 8; //Leap year
//  hour = 2;
//  localUTCOffset = -7;
  //Output should be 2/29/08 hour = 19 (no DST)

  //Example: UTC is 8AM, date is first day of DST, localOffset is -7,
//  month = 3;
//  day = 14;
//  year = 10;
//  hour = 8;
//  localUTCOffset = -7;
  //Output should be 3/14/10 hour = 1, we haven't sprung forward yet

  //Example: UTC is 9AM, date is first day of DST, localOffset is -7,
//  month = 3;
//  day = 14;
//  year = 10; //Leap year
//  hour = 9;
//  localUTCOffset = -7;
  //Output should be 3/14/10 hour = 3 (loose an hour)

  //Example: UTC is 7AM, date is last day of DST, localOffset is -7,
//  month = 11;
//  day = 3;
//  year = 19;
//  hour = 7;
//  localUTCOffset = -7;
  //Output should be 11/3/19 hour = 1 (not yet DST, no change)

  //Example: UTC is 8AM, date is last day of DST, localOffset is -7,
//  month = 11;
//  day = 3;
//  year = 19;
//  hour = 8;
//  localUTCOffset = -7;
  //Output should be 11/3/19 hour = 1 (repeat 1AM)

  //Example: UTC is 10AM, date is outside of DST, localOffset is -7,
  //      month = 12;
  //      day = 1;
  //      year = 19;
  //      hour = 10;
  //      localUTCOffset = -7;
  //Output should be 12/1/19 hour = 3 (No DST add)

  //Example: UTC is 5AM, we are in DST, localOffset is -7,
  //    month = 7;
  //    day = 1;
  //    year = 19;
  //    hour = 5;
  //    localUTCOffset = -7;
  //Output should be 6/30/19 hour = 23 (in DST)


  adjustToLocalDateTime(year, month, day, hour, localUTCOffset);

  String myTime = "";

  if (true)
  {
    char gpsDate[11]; //10/12/2019
    if (true)
      sprintf(gpsDate, "%02d/%02d/20%02d", month, day, year);
    else
      sprintf(gpsDate, "%02d/%02d/20%02d", day, month, year);
    myTime += String(gpsDate);
    myTime += ",";
  }

  char gpsTime[13]; //09:14:37.412
  sprintf(gpsTime, "%02d:%02d:%02d.%03d", hour, minute, second, millis() % 1000); //TODO get GPS hundredths()
  myTime += String(gpsTime);
  myTime += ",";

  Serial.println(myTime);
}

void loop() {

}

//Given the date and hour, calculate local date/time
//Adjust the hour by local hour offset
//Adjust the hour by DST as necessary
//Adjust the date as necessary
//Leap year is taken into account but does not interact with DST (DST happens later in March)
void adjustToLocalDateTime(int &year, int &month, int &day, int &hour, int localUTCOffset) {

  //Adjust UTC with local offset
  hour += localUTCOffset;

  //If the adjusted hour is outside 0 to 23, then adjust date as necessary
  correctDate(year, month, day, hour);

  //Should we correct for daylight savings time?
  if (true)
  {
    //Calculate DST adjustment based on date and local offset
    hour += findUSDSTadjustment(year, month, day, hour);

    //DST may have pushed a date change so do one more time
    correctDate(year, month, day, hour);
  }
  //  if (settings.hour24Style == false)
  //  {
  //    if (hour > 12) hour -= 12;
  //  }
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
