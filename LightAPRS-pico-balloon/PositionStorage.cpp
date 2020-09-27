#include "PositionStorage.h"
#include "PositionMessage.h"
#include "PositionReport.h"
#ifdef ARDUINO 
#include <eeprom.h>
#else
#include <iostream>
#endif

// check eeprom to see we have already initialized
PositionStorage::PositionStorage(unsigned long startPos, unsigned short availableBytes, unsigned long epoch, PositionReport::PositionTypes positionType,
	PositionMessage::TimeTypes timeType, unsigned short timeInterval, unsigned short reportsPerMessage){
	
#ifndef ARDUINO 
	_testBuffer = new unsigned char[availableBytes];
#endif

	_startPos = startPos;
	AvailableBytes = availableBytes;
	PositionType = positionType;
	TimeType = timeType;
	TimeInterval = timeInterval;
	ReportsPerMessage = reportsPerMessage;
	
	_readBuffer(&IsInitialized, 0, 1);  //first byte 
	if (IsInitialized == 1) {
		//Read and set these values.
		_readBuffer((unsigned char*)&Epoch, 1, sizeof(Epoch));   //does not copy bytes
		_readBuffer((unsigned char*)&LastWriteTime, 5, sizeof(LastWriteTime));
		_readBuffer((unsigned char*)&LifetimeWrites, 9, sizeof(LifetimeWrites));
		_readBuffer((unsigned char*)&LastIndexWritten, 11, sizeof(LastIndexWritten));
		_readBuffer((unsigned char*)&LastIndexRead, 13, sizeof(LastIndexRead));
	}
	else {
		// Initialize:  Write Epoch and set the following to zero: LifetimeWrites,LastIndexWritten,LastIndexRead
		IsInitialized = 1;
		Epoch = epoch;
		LastWriteTime = 0;
		LifetimeWrites = 0;
		LastIndexWritten = 0;
		LastIndexRead = 0;
		_writeBuffer((unsigned char*)&IsInitialized, 0, sizeof(IsInitialized));   //does not copy bytes
		_writeBuffer((unsigned char*)&Epoch, 1, sizeof(Epoch));   //does not copy bytes
		_writeBuffer((unsigned char*)&LastWriteTime, 5, sizeof(LastWriteTime));
		_writeBuffer((unsigned char*)&LifetimeWrites, 9, sizeof(LifetimeWrites));
		_writeBuffer((unsigned char*)&LastIndexWritten, 11, sizeof(LastIndexWritten));
		_writeBuffer((unsigned char*)&LastIndexRead, 13, sizeof(LastIndexRead));
	}

	// based on available bytes, how many position reports can we store?
	_reportSize = PositionReport::ByteSize(PositionType);
	IndexesAvailable = (availableBytes - STORAGE_HEADER_BYTE_SIZE) / _reportSize;
	TotalMessageSize = PositionMessage::HEADER_BYTE_SIZE + (_reportSize * reportsPerMessage);
	// create buffer and temp object to build a message returned in GetNextMessage()
	_messageBuffer = new unsigned char[TotalMessageSize];  
	_reportBuffer = new unsigned char[_reportSize];

	//PositionMessage _tmpPositionMessage(_messageBuffer, 0, Epoch, positionType, timeType, timeInterval, reportsPerMessage);
	_tmpPositionMessage = new PositionMessage(_messageBuffer, 0, Epoch, positionType, timeType, timeInterval, reportsPerMessage);
	//PositionMessage* test =  new PositionMessage(_messageBuffer, 0, Epoch, positionType, timeType, timeInterval, reportsPerMessage);
}

PositionStorage::~PositionStorage()
{
#ifndef ARDUINO 
	delete _testBuffer;
#endif
	delete _messageBuffer;
}

// set to unitialized.
void PositionStorage::ClearStorage() {
	IsInitialized = 0;
	_writeBuffer(&IsInitialized, 0, 1);  //first byte 
}

void PositionStorage::StoreNextReport(unsigned long utc, float lat, float lon, unsigned short altMeters) {
	LastWriteTime = utc;
	LifetimeWrites++;
	LastIndexWritten++;
	if (LastIndexWritten >= IndexesAvailable) { LastIndexWritten = 0; }  //roll over
	_writeBuffer((unsigned char*)&LastWriteTime, 5, sizeof(LastWriteTime));
	_writeBuffer((unsigned char*)&LifetimeWrites, 9, sizeof(LifetimeWrites));
	_writeBuffer((unsigned char*)&LastIndexWritten, 9, sizeof(LastIndexWritten));  //save current index

	// Write position report data
	//_tmpPositionMessage.ClearReports();
    PositionReport report1(Epoch, PositionType, _tmpPositionMessage->TimeIntervalSeconds());
	//PositionReport report1(Epoch, PositionType, 7200);
	report1.ToBytes(_reportBuffer, 0, utc, lat, lon, altMeters);
	_writeBuffer(_reportBuffer, STORAGE_HEADER_BYTE_SIZE + (_reportSize * LastIndexWritten), _reportSize);

}

PositionMessage* PositionStorage::GetNextMessage(unsigned char* bytes) {
	unsigned short nextIndexToRead = LastIndexRead;

	_tmpPositionMessage->ClearReports();  // since we are reusing the same object
	// read the reports
	for (int i = 0; i < ReportsPerMessage; i++) {
		nextIndexToRead++;
		if (nextIndexToRead >= IndexesAvailable) { nextIndexToRead = 0; }  //roll over
		// read the next reports
		_readBuffer(_reportBuffer, STORAGE_HEADER_BYTE_SIZE + (_reportSize * nextIndexToRead), _reportSize);
		
		// Add report bytes to the message object
		_tmpPositionMessage->AddReport(_reportBuffer);
	}

	// save the last index we just read
	LastIndexRead = nextIndexToRead;
	_writeBuffer((unsigned char*)&LastIndexRead, 13, sizeof(LastIndexRead));
	
	_tmpPositionMessage->ToBytes(bytes, 0);
	return _tmpPositionMessage;
}

void PositionStorage::_writeBuffer(unsigned char* bytes, unsigned short offset, unsigned short length) {
#ifdef ARDUINO 
	//write to EEPROM
	for (unsigned short i = 0; i < length; i++) {
		EEPROM.update(offset + i, bytes[i]);
	}
#else
	//write to _testBuffer
	for (unsigned short i = 0; i < length; i++) {
		_testBuffer[offset + i] = bytes[i];
	}
#endif
}

void PositionStorage::_readBuffer(unsigned char* bytes, unsigned short offset, unsigned short length) {
#ifdef ARDUINO 
	//read from EEPROM
	for (unsigned short i = 0; i < length; i++) {
		EEPROM.read(offset + i, bytes[i]);
	}
#else
	//read from _testBuffer
	for (unsigned short i = 0; i < length; i++) {
		bytes[i] = _testBuffer[offset + i];
	}
#endif
}

void PositionStorage::DebugStats() {
	printf("LastWriteTime\\LifetimeWrites\\LastIndexWritten\\LastIndexRead %lu\\%d\\%d\\%d\n", LastWriteTime,
		LifetimeWrites, LastIndexWritten, LastIndexRead);
}