#ifndef PositionReport_h
#define PositionReport_h

#include "PositionReport.h"
#include "PositionMessage.h"



class PositionReport
{
public:
	//static const int BYTE_SIZE = 5;  // binary encoded length of a single position report  (would be varible with other PositionTypes)
	enum PositionTypes:unsigned short { normalRes = 1 }; //normal = AR99 + ALT in 10M  (Future use: lowRes = 0,highRes = 2)

	PositionReport(unsigned long epoch, PositionTypes positionType, unsigned long timeIntervalSeconds);  //TODO:  Pass in the PositionType (HiRes/NormalRes/LowRes)

	void FromBytes(unsigned char* data, unsigned long startPos);  //decode byte message
	void ToBytes(unsigned char* data, unsigned long startPos, unsigned long utc, float lat, float lon, unsigned short altMeters);
	unsigned short ByteSize();


	static void LatLonToGridSquare(unsigned char *gridloc, float lat, float lon);
	static unsigned short GridLocatorToID(unsigned char* gridLoc);
	static void IdToGridLocator(unsigned char* gridLoc, unsigned short id);
	static unsigned short ByteSize(PositionTypes positionType);

	// Time
	unsigned long Epoch;
	unsigned long Utc;
	unsigned short TimeIntervalSeconds;  
	unsigned short TimeIndex;  // time slot  up to 65K

	// Geo
	PositionTypes PositionType;
	float Lat;
	float Lon;
	unsigned char GridLocation[4];   //e.g. AR99

	// Alt
	unsigned short AltMeters = 0;

private:
	unsigned char* _buffer;
	unsigned long _startPos;
};

#endif
