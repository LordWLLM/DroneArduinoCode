#include <Arduino.h>
#include <SingleWireSerial.h>

SingleWireSerial mySerial(false);

// put function declarations here:

void setup()
{
  // put your setup code here, to run once:
  mySerial.begin(9600);
}

void loop()
{
  // put your main code here, to run repeatedly:
  mySerial.write(0xA0);
  delay(1000);
}

// put function definitions here: