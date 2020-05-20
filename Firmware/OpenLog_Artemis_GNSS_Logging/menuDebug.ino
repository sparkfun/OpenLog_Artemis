void menuDebug()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Debug Settings"));

    Serial.print(F("1) Major Debug Messages: "));
    if (settings.printMajorDebugMessages == true) Serial.println(F("Enabled"));
    else Serial.println("Disabled");

    Serial.print(F("2) Minor Debug Messages: "));
    if (settings.printMinorDebugMessages == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.printMajorDebugMessages ^= 1;
    }
    else if (incoming == '2')
    {
      settings.printMinorDebugMessages ^= 1;
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
