
#include "PositionMessage.h"
#include <stdio.h>
#include <string.h>

PositionMessage::PositionMessage() {

}

//  New object from header values
PositionMessage::PositionMessage(unsigned char* data, unsigned long startPos, unsigned long epoch, PositionReport::PositionTypes positionType,
	TimeTypes timeType, int timeInterval, unsigned short reportsCount) {

	_startPos = startPos;
	_buffer = data;

	Epoch = epoch;
	PositionType = positionType;
	TimeType = timeType;
	TimeInterval = timeInterval;
	ReportsCount = reportsCount;

	ToBytes(data, startPos);
	//_reports_buffer = new unsigned char[reportsCount * PositionReport::BYTE_SIZE];
}

//  New object from binary array (i.e. decode a message)
void PositionMessage::FromBytes(unsigned char* data, unsigned long startPos) {
	//https://stackoverflow.com/questions/29380065/copying-integers-from-char-array-depending-on-number-of-bytes-in-c

	_startPos = startPos;
	_buffer = data;
	// HEADER
	memcpy(&Epoch, &_buffer[startPos], sizeof(Epoch));  //bytes  0..3

	unsigned char timevalues = _buffer[startPos + 4];  //byte 4
	TimeType = static_cast<TimeTypes>(timevalues >> 6);  //higher 2 bits
	TimeInterval = timevalues & 63;  // lower 6 bits
									  
	unsigned char positionValues = _buffer[startPos + 5];  //byte 5
	PositionType = static_cast<PositionReport::PositionTypes>(positionValues >> 6);  //higher 2 bits
	ReportsCount = positionValues & 63;  // lower 6 bits

	// LIST OF POSITION REPORTS

}

//  Write the message to the data buffer, along with the 'count' position reports from 'startIndex'
void PositionMessage::ToBytes(unsigned char* data, unsigned long startPos){
	_startPos = startPos;
	_buffer = data;

	// HEADER
	memcpy(&_buffer[startPos], (unsigned char*)&(Epoch), sizeof(Epoch));  //EPOCH bytes 0..3

	unsigned char timevalues;
	timevalues = (TimeType << 6) + TimeInterval;
	memcpy(&_buffer[startPos + 4], (unsigned char*)&(timevalues), sizeof(timevalues));  //byte 4

	unsigned char positionvalues;
	positionvalues = (PositionType << 6) + ReportsCount;
	memcpy(&_buffer[startPos + 5], (unsigned char*)&(positionvalues), sizeof(positionvalues));  //byte 5

	// LIST OF POSITION REPORTS

}

// Add next position report
PositionReport PositionMessage::AddReport(unsigned long utc, float lat, float lon, unsigned short altMeters) {
	
	// TODO:  Should we reuse this PositionReport object for efficiency?
	PositionReport newReport(Epoch, PositionType, TimeIntervalSeconds());
	unsigned long nextBytePosition = _startPos + HEADER_BYTE_SIZE + (newReport.ByteSize() * _lastIndexAdded);
	newReport.ToBytes(_buffer, nextBytePosition, utc, lat, lon, altMeters);
	_lastIndexAdded++;
	return newReport;
}

// Return PositionReport at index i
PositionReport PositionMessage::ReadReport(unsigned char index) {

	PositionReport nextReport(Epoch, PositionReport::PositionTypes::normalRes, TimeIntervalSeconds());
	unsigned long nextBytePosition = _startPos + HEADER_BYTE_SIZE + (nextReport.ByteSize()  * _lastIndexRead);
	nextReport.FromBytes(_buffer, nextBytePosition);
	_lastIndexRead++;
	return nextReport;
}

// Convert Timetype/TimeInterval into interval in seconds
int PositionMessage::TimeIntervalSeconds() {
	switch (TimeType) {
		case seconds: return TimeInterval;
		case minutes: return 60 * TimeInterval;
		case hours: return 60 * 60 * TimeInterval;
		case days: return 60 * 60 * 24 * TimeInterval;
	}
}


