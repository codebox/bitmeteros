#define SERVICE_NAME "BitMeterCaptureService"

#define MAXLEN_PHYSADDR 8
#define POLL_INTERVAL   1

#include <stdio.h>

struct BwData {
	unsigned long dl;
	unsigned long ul;
	unsigned long addrLen;
	unsigned char addr[MAXLEN_PHYSADDR];
	struct BwData* next;
};

struct Addr{
	int id;
	int addrSize;
	unsigned char addr[MAXLEN_PHYSADDR];
	struct Addr* next;
};

struct BwData* getData();

void setupDb();
void updateDb(int, int, struct BwData*);
void compressDb();


int getTime();
int getNextCompressTime();

int getNextMinForTs(int);
int getNextHourForTs(int);

void setupCapture();
void processCapture();
void shutdownCapture();
void logBw(struct BwData*);

