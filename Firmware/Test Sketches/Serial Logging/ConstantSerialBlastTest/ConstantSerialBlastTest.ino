/*
  Throws lots of serial at OpenLog Artemis
*/

int ledPin =  13; //Status LED connected to digital pin 13

int testAmt = 3;
//At 9600, testAmt of 4 takes about 1 minute, 10 takes about 3 minutes
//At 57600, testAmt of 10 takes about 1 minute, 40 takes about 5 minutes
//At 115200, testAmt of 30 takes about 1 minute

unsigned long startTime = 0;

void setup()
{
  pinMode(ledPin, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Run OpenLog Test");

  int rate = 115200 * 4;
  Serial1.begin(rate);

  Serial.print("Current output baud rate: ");
  Serial.println(rate);

  Serial.println("s)end test blob of text");
  Serial.println("1)Set baud to 115200");
  Serial.println("2)Set baud to 230400");
  Serial.println("3)Set baud to 460800");
  Serial.println("4)Set baud to 500000");
  Serial.println("5)Set baud to 691200");
  Serial.println("6)Set baud to 921600");
  Serial.println("a)Increase blob size");
  Serial.println("d)Decrease blob size");

  //enableBurstMode(); //Go to 96MHz
}

void loop()
{
  if (Serial.available())
  {
    byte incoming = Serial.read();

    if (incoming == 's')
    {
      Serial.println("Sending blob");
      sendBlob();
    }
    else if (incoming == '1')
    {
      Serial.println("Serial set to: 115200");
      Serial1.begin(115200);
    }
    else if (incoming == '2')
    {
      Serial.println("Serial set to: 230400");
      Serial1.begin(230400);
    }
    else if (incoming == '3')
    {
      Serial.println("Serial set to: 460800");
      Serial1.begin(460800);
    }
    else if (incoming == '4')
    {
      Serial.println("Serial set to: 500000");
      Serial1.begin(500000);
    }
    else if (incoming == '5')
    {
      Serial.println("Serial set to: 691200");
      Serial1.begin(691200);
    }
    else if (incoming == '6')
    {
      Serial.println("Serial set to: 921600");
      Serial1.begin(921600);
    }
    else if (incoming == 'a')
    {
      testAmt++;
      Serial.print("testAmt: ");
      Serial.println(testAmt);
    }
    else if (incoming == 'z')
    {
      testAmt--;
      Serial.print("testAmt: ");
      Serial.println(testAmt);
    }
    else if (incoming == '\r' || incoming == '\n')
    {
      //Do nothing
    }
    else
    {
      Serial.println("Not recognized");
    }
  }

  //Blink the Status LED because we're done!
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
}

void sendBlob()
{
  startTime = millis();

  //Each test is 100 lines. 10 tests is 1000 lines (11,000 characters)
  for (int numofTests = 0 ; numofTests < testAmt ; numofTests++)
  {
    //This loop will print 100 lines of 100 characters each
    for (int k = 33; k < 43 ; k++)
    {
      //Print one line of 100 characters with marker in the front (markers go from '!' to '*')
      Serial1.write(k); //Print the ASCII value directly: ! then " then #, etc
      Serial1.print(":abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOP-!#\n");
      //delay(50);

      //Then print 9 lines of 100 characters with new line at the end of the line
      for (int i = 1 ; i < 10 ; i++)
      {
        Serial1.print(i, DEC);
        Serial1.print(":abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOP-!#\n");
        //delay(50);
      }

      if (digitalRead(ledPin) == 0) //Turn the status LED on/off as we go
        digitalWrite(ledPin, HIGH);
      else
        digitalWrite(ledPin, LOW);
    }
  } //End numofTests loop

  unsigned long totalCharacters = (long)testAmt * 100 * 100;
  Serial.print("Characters pushed: ");
  Serial.flush();
  Serial.println(totalCharacters);
  Serial.print("Time taken (s): ");
  Serial.flush();
  Serial.println((millis() - startTime) / 1000.0, 2);
  Serial.println("Done!");
  Serial.flush();
}
