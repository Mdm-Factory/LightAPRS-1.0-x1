
#include "UnitTests.h"
#include "PositionMessage.h"
#include "PositionReport.h"
#include "Arduino.h"
//#include <EEPROM.h>

void setup() {
	Serial.begin(57600);
	Serial.printf("Starting Tests");
}

void loop() {

	//UnitTests::GridIdConversion();
	UnitTests::ReportEncoding();
	UnitTests::MessageEncoding();
	UnitTests::MessageWithReportsEncoding();
  
  // repeat tests after delay
  delay(10000);
  

}
