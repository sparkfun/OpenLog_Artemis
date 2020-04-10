void menuDebug()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Debug Settings");

    Serial.print("1) Debug Messages: ");
    if (settings.printDebugMessages == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.println("x) Exit");

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
