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

    Serial.print((String)rtcDate);
    Serial.print(" ");

    char rtcTime[12]; //09:14:37.41
    sprintf(rtcTime, "%02d:%02d:%02d.%02d", myRTC.hour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
    Serial.println((String)rtcTime);

    Serial.println("1) Manually set RTC date");
    Serial.println("2) Manually set RTC time");
    Serial.print("3) Synchronize RTC to GPS");
    if (qwiicOnline.uBlox == false)
    {
      Serial.println(": GPS offline");
    }

    Serial.print("4) Use GPS as timestamp source: ");
    if (qwiicOnline.uBlox)
    {
      if (settings.getRTCfromGPS == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    else
      Serial.println("GPS offline");

    Serial.print("5) Log date: ");
    if (settings.logDate == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("6) Toggle date style: ");
    if (settings.americanDateStyle == true) Serial.println("mm/dd/yyyy");
    else Serial.println("dd/mm/yyyy");

    Serial.print("7) Toggle time style: ");
    if (settings.hour24Style == true) Serial.println("24 hour");
    else Serial.println("12 hour");

    Serial.print("8) Local offset from GPS UTC: ");
    Serial.println(settings.localUTCOffset);

    Serial.print("9) Correct for USA based DST using GPS UTC Date/Time: ");
    if (qwiicOnline.uBlox)
    {
      if (settings.correctForDST == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    else
      Serial.println("GPS offline");

    Serial.println("x) Exit");

    byte incoming = getByteChoice(10); //Timeout after 10 seconds

    if (incoming == '1')
    {
      myRTC.getTime();
      int dd = myRTC.dayOfMonth, mm = myRTC.month, yy = myRTC.year, h = myRTC.hour, m = myRTC.minute, s = myRTC.seconds;

      Serial.print("Enter current two digit year: ");
      yy = getNumber(10); //Timeout after 10 seconds
      if (yy > 2000 && yy < 2100) yy -= 2000;

      Serial.print("Enter current month (1 to 12): ");
      mm = getNumber(10); //Timeout after 10 seconds

      Serial.print("Enter current day (1 to 31): ");
      dd = getNumber(10); //Timeout after 10 seconds

      myRTC.setTime(h, m, s, 0, dd, mm, yy); //Manually set RTC
    }
    else if (incoming == '2')
    {
      myRTC.getTime();
      int dd = myRTC.dayOfMonth, mm = myRTC.month, yy = myRTC.year, h = myRTC.hour, m = myRTC.minute, s = myRTC.seconds;

      Serial.print("Enter current hour (0 to 23): ");
      h = getNumber(10); //Timeout after 10 seconds

      Serial.print("Enter current minute (0 to 59): ");
      m = getNumber(10); //Timeout after 10 seconds

      Serial.print("Enter current second (0 to 59): ");
      s = getNumber(10); //Timeout after 10 seconds

      myRTC.setTime(h, m, s, 0, dd, mm, yy); //Manually set RTC
    }
    else if (incoming == '3')
    {
      if (qwiicOnline.uBlox == false)
        Serial.println("Error: GPS offline");
      else
      {
        uint8_t year, month, day;
        uint8_t hour, minute, second;

        //Get time from cell module
        //lte.clock(&year, &month, &day, &hour, &minute, &second, &timezone);

        //myRTC.setTime(hour, minute, second, 0, day, month, year); //Manually set RTC

        Serial.println("RTC synchronized to GPS");
      }
    }
    else if (incoming == '4')
      settings.getRTCfromGPS ^= 1;
    else if (incoming == '5')
      settings.logDate ^= 1;
    else if (incoming == '6')
      settings.americanDateStyle ^= 1;
    else if (incoming == '7')
      settings.hour24Style ^= 1;
    else if (incoming == '8')
    {
      Serial.print("Enter the local hour offset from UTC (-12 to 14): ");
      int offset = getNumber(10); //Timeout after 10 seconds
      if (offset < -12 || offset > 14)
        Serial.println("Error: Offset is out of range");
      else
        settings.localUTCOffset = offset;
    }
    else if (incoming == '9')
      settings.correctForDST ^= 1;
    else if (incoming == 'x')
      return;
    else if (incoming == 255)
      return;
    else
      printUnknown(incoming);
  }
}
