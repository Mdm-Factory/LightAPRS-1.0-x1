
#include "PositionMessage.h"
#include "PositionReport.h"
#include <stdio.h>
#include <string.h>

PositionReport::PositionReport(unsigned long epoch, PositionTypes positionType, unsigned long timeIntervalSeconds){
	Epoch = epoch;
	PositionType = positionType;
	TimeIntervalSeconds = timeIntervalSeconds;
}

//  New object from pointer to binary array (i.e. decode a position report)

void PositionReport::FromBytes(unsigned char* data, unsigned long startPos) {
	_startPos = startPos;
	_buffer = data;
	unsigned short gridBytes;
	memcpy(&TimeIndex, &_buffer[_startPos], sizeof(TimeIndex));  //bytes  0..1
	memcpy(&gridBytes, &_buffer[_startPos + sizeof(TimeIndex)], sizeof(gridBytes));  //bytes 2..3

	Utc = (unsigned long)(Epoch + (TimeIntervalSeconds * TimeIndex));

	unsigned char gridSubSqaure = gridBytes & 127;  // lower 7 bits
	unsigned short gridId = gridBytes >> 7;

	GridLocation[2] = (gridSubSqaure / 10) + '0';
	GridLocation[3] = gridSubSqaure - ((gridSubSqaure / 10)) * 10 + '0';

	IdToGridLocator(GridLocation, gridId);

	AltMeters = (unsigned short)0;  // Haven't encoded this yet.
	// TODO:  Convert GridLocation back to Lat/Lon
}

//  Write the message to the data buffer, along with the 'count' position reports from 'startIndex'
void PositionReport::ToBytes(unsigned char* data, unsigned long startPos, unsigned long utc, float lat, float lon, unsigned short altMeters) {
	_startPos = startPos;
	_buffer = data;
	Utc = utc;
	TimeIndex = (unsigned short)((utc - Epoch) / TimeIntervalSeconds); 

	Lat = lat;
	Lon = lon;
	LatLonToGridSquare(GridLocation, lat, lon);

	unsigned short gridId = GridLocatorToID(GridLocation);
	unsigned short gridSubSqaure = (unsigned char)(GridLocation[2] - '0') * 10 + (unsigned char)(GridLocation[3] - '0');   //chars to number 00..99
	unsigned short gridBytes = (gridId << 7) + gridSubSqaure;		 // high 9 bits are the square 0..323, low 7 bites are the subsquare 00.99
	
	memcpy(&_buffer[_startPos], (unsigned char*)&(TimeIndex), sizeof(TimeIndex));  //bytes 0..1
	memcpy(&_buffer[_startPos + sizeof(TimeIndex)], (unsigned char*)&(gridBytes), sizeof(gridBytes));  //bytes 2..3
	
	AltMeters = altMeters;
}

unsigned short PositionReport::ByteSize() {
	return ByteSize(PositionType);
}

// STATIC Functions

// Convert Timetype/TimeInterval into interval in seconds
unsigned short PositionReport::ByteSize(PositionTypes positionType) {
	switch (positionType) {
	//case lowRes: return ?;
	case normalRes: return 4;
	//case highRes: return ?;
	}
}

// Convert lat/lon to grid, e.g. AR99
void PositionReport::LatLonToGridSquare(unsigned char *gridloc, float lat, float lon) {
	int o1, o2;
	int a1, a2;
	float remainder;
	// longitude
	remainder = lon + 180.0;
	o1 = (int)(remainder / 20.0);
	remainder = remainder - (float)o1 * 20.0;
	o2 = (int)(remainder / 2.0);
	// latitude
	remainder = lat + 90.0;
	a1 = (int)(remainder / 10.0);
	remainder = remainder - (float)a1 * 10.0;
	a2 = (int)(remainder);

	gridloc[0] = (char)o1 + 'A';
	gridloc[1] = (char)a1 + 'A';
	gridloc[2] = (char)o2 + '0';
	gridloc[3] = (char)a2 + '0';
	gridloc[4] = (char)0;
}

// Convert AA..RR to 0..323  (2^9)
unsigned short PositionReport::GridLocatorToID(unsigned char* gridLoc) {
	unsigned short col = (gridLoc[0] - 'A') * 18; 
	unsigned short row = gridLoc[1] - 'A';
	return  col + row;
}

// Convert 0..323 to AA..RR
void PositionReport::IdToGridLocator(unsigned char* gridLoc, unsigned short id) {
	unsigned short col = (id / 18) + 'A';
	unsigned short row = id - (id / 18) * 18 + 'A';
	gridLoc[0] = col;
	gridLoc[1] = row;
}
