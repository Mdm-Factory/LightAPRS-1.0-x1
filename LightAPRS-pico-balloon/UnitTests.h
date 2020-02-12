#include "PositionMessage.h"
#include "PositionReport.h"

class UnitTests
{
public:
	UnitTests();

	static void  GridIdConversion();
	static void  ReportEncoding();
	static void  MessageEncoding();
	static void  MessageWithReportsEncoding();

};


UnitTests::UnitTests()
{
}

// Tests converting AA..AR to 00..323 and back
void UnitTests::GridIdConversion() {
	Serial.printf("TestGridIdConversion \n");
	int id = 0;
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 18; j++) {
			unsigned char gridLoc[2];
			gridLoc[0] = (unsigned char)(65 + i);
			gridLoc[1] = (unsigned char)(65 + j);

			unsigned char gridLoc2[2];
			PositionReport::IdToGridLocator(gridLoc2, id);
			Serial.printf("%c%c %lu %lu %c%c \n", gridLoc[0], gridLoc[1], id, PositionReport::GridLocatorToID(gridLoc), gridLoc2[0], gridLoc2[1]);
			id++;
		}
	}
	Serial.printf("\n");
}

void  UnitTests::ReportEncoding() {
	Serial.printf("TestReportEncoding \n");
	unsigned long epoch = 1581456190;
	unsigned short timeIntervalSeconds = 60 * 60 * 2;
	unsigned char* buffer;
	buffer = new unsigned char[PositionReport::ByteSize(PositionReport::PositionTypes::normalRes)];
	//unsigned char buffer[4];

	// Encode report
	PositionReport report1(epoch, PositionReport::PositionTypes::normalRes, timeIntervalSeconds);  // time interval is 2 hours (in seconds)
	report1.ToBytes(buffer, 0, (unsigned long)(epoch + 86400), float(30.0), (float)(-97.0), (unsigned long)(999));
	Serial.printf("%lu %c%c%c%c %d \n", report1.Utc, report1.GridLocation[0], report1.GridLocation[1], report1.GridLocation[2],report1.GridLocation[3], report1.AltMeters);

	// Decode report
	PositionReport report2(epoch, PositionReport::PositionTypes::normalRes, timeIntervalSeconds);  // time interval is 2 hours (in seconds)
	report2.FromBytes(buffer, 0);
	Serial.printf("%lu %c%c%c%c %d \n", report2.Utc, report1.GridLocation[0], report2.GridLocation[1], report2.GridLocation[2],
		report2.GridLocation[3], report2.AltMeters);

	delete buffer;  // Cleanup!
	Serial.printf("\n");
}

void  UnitTests::MessageEncoding() {
	Serial.printf("TestMessageEncoding \n");
	unsigned char gridLoc[4];
	unsigned long epoch = 1581456190;
	unsigned short reportCount = 0;
	unsigned short timeInterval = 4;
	PositionMessage::TimeTypes timeType = PositionMessage::TimeTypes::hours;
	PositionReport::PositionTypes positionType = PositionReport::PositionTypes::normalRes;

	unsigned char* buffer;
	buffer = new unsigned char[PositionMessage::HEADER_BYTE_SIZE + PositionReport::ByteSize(positionType) * reportCount];

	// Encode message
	PositionMessage message1(buffer, 0, epoch, positionType, timeType, timeInterval, reportCount);
	unsigned char data[PositionMessage::HEADER_BYTE_SIZE];
	message1.ToBytes(data, 0);
	
	// Decode message
	PositionMessage message2;
	message2.FromBytes(data, 0);

	Serial.printf("%lu %d %d %d \n", message1.Epoch, message1.PositionType, message1.TimeType, message1.TimeInterval);
	Serial.printf("%lu %d %d %d \n", message2.Epoch, message2.PositionType, message2.TimeType, message2.TimeInterval);

	delete buffer;  // Cleanup!
	Serial.printf("\n");

}

void  UnitTests::MessageWithReportsEncoding() {
	Serial.printf("MessageWithReportsEncoding \n");
	unsigned long epoch = 1581456190;
	unsigned short  reportCount = 60;   //Need to test all 64 possible reports THERE IS A BOUNDARY BUG AT 64
	unsigned short timeInterval = 2;
	PositionMessage::TimeTypes timeType = PositionMessage::TimeTypes::hours;
	PositionReport::PositionTypes positionType = PositionReport::PositionTypes::normalRes;

	unsigned char* buffer;
	buffer = new unsigned char[PositionMessage::HEADER_BYTE_SIZE + PositionReport::ByteSize(positionType) * reportCount];

	// Encode message
	PositionMessage message1(buffer, 0, epoch, positionType, timeType, timeInterval, reportCount);
	for (int i=0; i < reportCount; i++) {
		unsigned long utc = epoch + message1.TimeIntervalSeconds() * i;  // for testing, make each report the next time slot
		PositionReport report = message1.AddReport(utc, 30.0 + i*2, -97.0 + i*2, (unsigned short)i);
		Serial.printf("...AddReport %lu %c%c%c%c %d \n", report.Utc, report.GridLocation[0], report.GridLocation[1], report.GridLocation[2],
			report.GridLocation[3], report.AltMeters);
	}
	Serial.printf("%d Reports Added \n", message1.ReportsCount);
	
	// Decode message
	PositionMessage message2;
	message2.FromBytes(buffer, 0);
	Serial.printf("%d Reports Read \n", message2.ReportsCount);
	for (int i = 0; i < message2.ReportsCount; i++) {
		PositionReport report = message2.ReadReport(i);
		Serial.printf("...ReadReport %lu %c%c%c%c %d \n", report.Utc, report.GridLocation[0], report.GridLocation[1], report.GridLocation[2],
			report.GridLocation[3], report.AltMeters);
	}

	delete buffer;  // Cleanup!
	Serial.printf("\n");
}


