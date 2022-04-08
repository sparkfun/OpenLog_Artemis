void menuTimeStamp()
{
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Time Stamp"));
    SerialPrint(F("Current date/time: "));
    myRTC.getTime();

    char rtcDate[11]; // 10/12/2019
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
    else
      sprintf(rtcDate, "%s/%s/%s,", rtcYear, rtcMonth, rtcDay);

    SerialPrint(rtcDate);
    SerialPrint(F(" "));

    char rtcTime[13]; //09:14:37.41,
    int adjustedHour = myRTC.hour;
    if (settings.hour24Style == false)
    {
      if (adjustedHour > 12) adjustedHour -= 12;
    }
    char rtcHour[3];
    char rtcMin[3];
    char rtcSec[3];
    char rtcHundredths[3];
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
    sprintf(rtcTime, "%s:%s:%s.%s", rtcHour, rtcMin, rtcSec, rtcHundredths);

    SerialPrintln(rtcTime);

    SerialPrint(F("1) Log Date: "));
    if (settings.logDate == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("2) Log Time: "));
    if (settings.logTime == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (settings.logDate == true || settings.logTime == true)
    {
      SerialPrintln(F("3) Set RTC to compiler macro time"));
    }

    if (settings.logDate == true)
    {
      SerialPrintln(F("4) Manually set RTC date"));

      SerialPrint(F("5) Toggle date style: "));
      if (settings.dateStyle == 0) SerialPrintln(F("mm/dd/yyyy"));
      else if (settings.dateStyle == 1) SerialPrintln(F("dd/mm/yyyy"));
      else if (settings.dateStyle == 2) SerialPrintln(F("yyyy/mm/dd"));
      else SerialPrintln(F("ISO 8601"));
    }

    if (settings.logTime == true)
    {
      SerialPrintln(F("6) Manually set RTC time"));

      SerialPrint(F("7) Toggle time style: "));
      if (settings.hour24Style == true) SerialPrintln(F("24 hour"));
      else SerialPrintln(F("12 hour"));
    }

    if (settings.logDate == true || settings.logTime == true)
    {
      if (isUbloxAttached() == true)
      {
        SerialPrintln(F("8) Synchronize RTC to GPS"));
      }
      SerialPrint(F("9) Local offset from UTC: "));
      Serial.println(settings.localUTCOffset);
      if (settings.useTxRxPinsForTerminal == true)
        Serial1.println(settings.localUTCOffset);
    }

    SerialPrint(F("10) Log Microseconds: "));
    if (settings.logMicroseconds == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      settings.logDate ^= 1;
    else if (incoming == 2)
      settings.logTime ^= 1;
    else if (incoming == 10)
      settings.logMicroseconds ^= 1;
    else if (incoming == STATUS_PRESSED_X)
      return;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      return;

    if ((settings.logDate == true) || (settings.logTime == true))
    {
      //Options 3, 8, 9
      if (incoming == 3)
      {
        myRTC.setToCompilerTime(); //Set RTC using the system __DATE__ and __TIME__ macros from compiler
        SerialPrintln(F("RTC set to compiler time"));
      }
      else if ((incoming == 8) && (isUbloxAttached() == true))
      {
        myRTC.getTime(); // Get the RTC date and time (just in case getGPSDateTime fails)
        int dd = myRTC.dayOfMonth, mm = myRTC.month, yy = myRTC.year, h = myRTC.hour, m = myRTC.minute, s = myRTC.seconds, ms = (myRTC.hundredths * 10);
        bool dateValid, timeValid;
        getGPSDateTime(yy, mm, dd, h, m, s, ms, dateValid, timeValid); // Get the GPS date and time, corrected for localUTCOffset
        myRTC.setTime((ms / 10), s, m, h, dd, mm, (yy - 2000)); //Manually set RTC
        lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change
        SerialPrintln(F("RTC set to GPS (UTC) time"));
        if ((dateValid == false) || (timeValid == false))
        {
          SerialPrintln(F("\r\nWarning: the GPS time or date was not valid. Please try again.\r\n"));
        }
      }
      else if (incoming == 9)
      {
        SerialPrint(F("Enter the local hour offset from UTC (-12 to 14): "));
        float offset = (float)getDouble(menuTimeout); //Timeout after x seconds
        if (offset < -12 || offset > 14)
          SerialPrintln(F("Error: Offset is out of range"));
        else
          settings.localUTCOffset = offset;
      }
    }

    if (settings.logDate == true)
    {
      //4 and 5
      if (incoming == 4) //Manually set RTC date
      {
        myRTC.getTime();
        int dd = myRTC.dayOfMonth, mm = myRTC.month, yy = myRTC.year, h = myRTC.hour, m = myRTC.minute, s = myRTC.seconds;

        SerialPrint(F("Enter current two digit year: "));
        yy = getNumber(menuTimeout); //Timeout after x seconds
        if (yy > 2000 && yy < 2100) yy -= 2000;

        SerialPrint(F("Enter current month (1 to 12): "));
        mm = getNumber(menuTimeout); //Timeout after x seconds

        SerialPrint(F("Enter current day (1 to 31): "));
        dd = getNumber(menuTimeout); //Timeout after x seconds

        myRTC.getTime();
        h = myRTC.hour; m = myRTC.minute; s = myRTC.seconds;
        myRTC.setTime(0, s, m, h, dd, mm, yy); //Manually set RTC
        lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change
      }
      else if (incoming == 5)
      {
        settings.dateStyle += 1;
        if (settings.dateStyle == 4)
          settings.dateStyle = 0;
      }
    }

    if (settings.logTime == true)
    {
      //6 and 7
      if (incoming == 6) //Manually set time
      {
        myRTC.getTime();
        int dd = myRTC.dayOfMonth, mm = myRTC.month, yy = myRTC.year, h = myRTC.hour, m = myRTC.minute, s = myRTC.seconds;

        SerialPrint(F("Enter current hour (0 to 23): "));
        h = getNumber(menuTimeout); //Timeout after x seconds

        SerialPrint(F("Enter current minute (0 to 59): "));
        m = getNumber(menuTimeout); //Timeout after x seconds

        SerialPrint(F("Enter current second (0 to 59): "));
        s = getNumber(menuTimeout); //Timeout after x seconds

        myRTC.setTime(0, s, m, h, dd, mm, yy); //Manually set RTC
        lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change
      }
      else if (incoming == 7)
      {
        settings.hour24Style ^= 1;
      }
    }
  }
}
