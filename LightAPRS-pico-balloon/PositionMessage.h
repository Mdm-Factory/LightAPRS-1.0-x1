#ifndef PositionMessage_h
#define PositionMessage_h

#include "PositionMessage.h"
#include "PositionReport.h"

class PositionMessage
{
public:
	static const unsigned short HEADER_BYTE_SIZE = 8;  // binary encoded length of message HEADER
	static const unsigned short MAX_REPORTS_PER_MESSAGE = 64;  // based on bits allocated for this value
	enum TimeTypes:unsigned short { seconds = 0, minutes = 1, hours = 2, days = 3 };  //Future use:  { seconds = 0, minutes = 1, hours = 2, days = 3 };

	PositionMessage();
	PositionMessage(unsigned char* data, unsigned long startPos, unsigned long epoch, PositionReport::PositionTypes positionType,
		TimeTypes timeType, int timeInterval, unsigned char reportsCount);

	void FromBytes(unsigned char* data, unsigned long startPos);
	void ToBytes(unsigned char* data, unsigned long startPos);

	PositionReport AddReport(unsigned long utc, float lat, float lon, unsigned short altMeters);
	PositionReport ReadReport(unsigned char index);
	int TimeIntervalSeconds();
	PositionReport::PositionTypes PositionType;
	TimeTypes TimeType;                
	unsigned long Epoch;
	unsigned short TimeInterval;	  //Time Interval (0..64) 	(every 1 day, 4 hours, 30 seconds, etc)  Zero is OFF
	unsigned short ReportsCount;  // 0..64

private:
	int _lastIndexAdded = 0;
	int _lastIndexRead = 0;
	unsigned char* _buffer;
	unsigned long _startPos;
};

#endif
