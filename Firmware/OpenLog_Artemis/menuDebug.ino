void menuDebug()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Debug Settings"));

    Serial.print(F("1) Debug Messages: "));
    if (settings.printDebugMessages == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.printDebugMessages ^= 1;
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
