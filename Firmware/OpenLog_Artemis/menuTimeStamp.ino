void menuTimeStamp()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Time Stamp");
    Serial.print("Current date/time: ");
    myRTC.getTime();

    char rtcDate[11]; //10/12/2019
    if (settings.americanDateStyle == true)
      sprintf(rtcDate, "%02d/%02d/20%02d", myRTC.month, myRTC.dayOfMonth, myRTC.year);
    else
      sprintf(rtcDate, "%02d/%02d/20%02d", myRTC.dayOfMonth, myRTC.month, myRTC.year);

    Serial.print(rtcDate);
    Serial.print(" ");

    char rtcTime[12]; //09:14:37.41
    int adjustedHour = myRTC.hour;
    if (settings.hour24Style == false)
    {
      if (adjustedHour > 12) adjustedHour -= 12;
    }
    sprintf(rtcTime, "%02d:%02d:%02d.%02d", adjustedHour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
    Serial.println(rtcTime);

    Serial.print("1) Log Date: ");
    if (settings.logDate == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("2) Log Time: ");
    if (settings.logTime == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.logDate == true || settings.logTime == true)
    {
      Serial.println("3) Set RTC to compiler macro time");
    }

    if (settings.logDate == true)
    {
      Serial.println("4) Manually set RTC date");

      Serial.print("5) Toggle date style: ");
      if (settings.americanDateStyle == true) Serial.println("mm/dd/yyyy");
      else Serial.println("dd/mm/yyyy");
    }

    if (settings.logTime == true)
    {
      Serial.println("6) Manually set RTC time");

      Serial.print("7) Toggle time style: ");
      if (settings.hour24Style == true) Serial.println("24 hour");
      else Serial.println("12 hour");
    }

    if (settings.logDate == true || settings.logTime == true)
    {
      if (isUbloxAttached() == true)
      {
        Serial.println("8) Synchronize RTC to GPS");
      }
      Serial.print("9) Local offset from UTC: ");
      Serial.println(settings.localUTCOffset);
  
    }

    Serial.println("x) Exit");

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      settings.logDate ^= 1;
    else if (incoming == 2)
      settings.logTime ^= 1;
    else if (incoming == STATUS_PRESSED_X)
      return;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      return;

    if (settings.logDate == true || settings.logTime == true)
    {
      //Options 3, 8, 9
      if (incoming == 3)
      {
        myRTC.setToCompilerTime(); //Set RTC using the system __DATE__ and __TIME__ macros from compiler
        Serial.println("RTC set to compiler time");
      }
      else if ((incoming == 8) && (isUbloxAttached() == true))
      {
        myRTC.getTime(); // Get the RTC date and time (just in case getGPSDateTime fails)
        int dd = myRTC.dayOfMonth, mm = myRTC.month, yy = myRTC.year, h = myRTC.hour, m = myRTC.minute, s = myRTC.seconds, ms = (myRTC.hundredths * 10);
        bool dateValid, timeValid;
        getGPSDateTime(yy, mm, dd, h, m, s, ms, dateValid, timeValid); // Get the GPS date and time, corrected for localUTCOffset
        myRTC.setTime(h, m, s, (ms / 10), dd, mm, (yy - 2000)); //Manually set RTC
        lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change
        Serial.println("RTC set to GPS (UTC) time");
        if ((dateValid == false) || (timeValid == false))
        {
          Serial.println("\nWarning: the GPS time or date was not valid. Please try again.\n");
        }
      }
      else if (incoming == 9)
      {
        Serial.print("Enter the local hour offset from UTC (-12 to 14): ");
        int offset = getNumber(menuTimeout); //Timeout after x seconds
        if (offset < -12 || offset > 14)
          Serial.println("Error: Offset is out of range");
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

        Serial.print("Enter current two digit year: ");
        yy = getNumber(menuTimeout); //Timeout after x seconds
        if (yy > 2000 && yy < 2100) yy -= 2000;

        Serial.print("Enter current month (1 to 12): ");
        mm = getNumber(menuTimeout); //Timeout after x seconds

        Serial.print("Enter current day (1 to 31): ");
        dd = getNumber(menuTimeout); //Timeout after x seconds

        myRTC.setTime(h, m, s, 0, dd, mm, yy); //Manually set RTC
        lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change
      }
      else if (incoming == 5)
      {
        settings.americanDateStyle ^= 1;
      }
    }

    if (settings.logTime == true)
    {
      //6 and 7
      if (incoming == 6) //Manually set time
      {
        myRTC.getTime();
        int dd = myRTC.dayOfMonth, mm = myRTC.month, yy = myRTC.year, h = myRTC.hour, m = myRTC.minute, s = myRTC.seconds;

        Serial.print("Enter current hour (0 to 23): ");
        h = getNumber(menuTimeout); //Timeout after x seconds

        Serial.print("Enter current minute (0 to 59): ");
        m = getNumber(menuTimeout); //Timeout after x seconds

        Serial.print("Enter current second (0 to 59): ");
        s = getNumber(menuTimeout); //Timeout after x seconds

        myRTC.setTime(h, m, s, 0, dd, mm, yy); //Manually set RTC
        lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change
      }
      else if (incoming == 7)
      {
        settings.hour24Style ^= 1;
      }
    }
  }
}
