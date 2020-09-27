#ifndef PositionStorage_h
#define PositionStorage_h
#ifdef ARDUINO 
	#include "Arduino.h"
#else
	#include <iostream>
#endif

#include "PositionMessage.h"
#include "PositionReport.h"

class PositionStorage
{
public:
	static const unsigned short STORAGE_HEADER_BYTE_SIZE = 15;  // binary encoded length of  HEADER
	PositionStorage(unsigned long startPos, unsigned short availableBytes, unsigned long Epoch, PositionReport::PositionTypes positionType,
		PositionMessage::TimeTypes timeType, unsigned short timeInterval, unsigned short reportsPerMessage);
	~PositionStorage();
	void StoreNextReport(unsigned long utc, float lat, float lon, unsigned short altMeters);
	PositionMessage* GetNextMessage(unsigned char* bytes);

	void ClearStorage();
	// parameters stored in EEprom
	unsigned char IsInitialized = 0;		//byte 0
	unsigned long Epoch;				//bytes 1..4
	unsigned long LastWriteTime;		//bytes 5..8
	unsigned short LifetimeWrites;		//bytes 9..10  total items written.  (if LifetimeWrites < IndexesAvailable
	unsigned short LastIndexWritten;	//bytes 11..12 position index (not byte index)
	unsigned short LastIndexRead;		//bytes 13..14


	unsigned short AvailableBytes;		//total bytes that may be written, including the header information
	unsigned short IndexesAvailable;	//number of indexed positions available for position reports.
	unsigned short TotalMessageSize;	//total number of bytes (message header, plus reportsPerMessage * count 

	// values for the storage/message header
	unsigned short TimeInterval;
	PositionReport::PositionTypes PositionType;
	PositionMessage::TimeTypes TimeType;
	unsigned short ReportsPerMessage;
	void DebugStats();
	PositionMessage* _tmpPositionMessage;
private:
	unsigned short _startPos;
	unsigned short _bytesAvailable;
	unsigned char* _testBuffer;   //used for testing only to simulate the EEPROM
	unsigned char* _messageBuffer;   //used for building a PositionMessage
	unsigned char* _reportBuffer;   //used for building a PositionMessage
	unsigned short _reportSize;   
	void _writeBuffer(unsigned char* bytes, unsigned short offset, unsigned short length);
	void _readBuffer(unsigned char* bytes, unsigned short offset, unsigned short length);

};

#endif