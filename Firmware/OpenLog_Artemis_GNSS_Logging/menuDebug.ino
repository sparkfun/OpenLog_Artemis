void menuDebug(bool *printMajorDebugMessages, bool *printMinorDebugMessages)
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Debug Settings"));

    Serial.print(F("1) Major Debug Messages : "));
    if (*printMajorDebugMessages == true) Serial.println(F("Enabled"));
    else Serial.println("Disabled");

    Serial.print(F("2) Minor Debug Messages : "));
    if (*printMinorDebugMessages == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      *printMajorDebugMessages ^= 1;
    }
    else if (incoming == '2')
    {
      *printMinorDebugMessages ^= 1;
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
