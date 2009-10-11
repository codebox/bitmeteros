#define SERVICE_NAME "BitMeterCaptureService"

#define POLL_INTERVAL   1

#include <stdio.h>

struct Data* getData();

int setupDb();
int updateDb(int, int, struct Data*);
int compressDb();

int getNextCompressTime();

void setupCapture();
int processCapture();
void shutdownCapture();
void logData(struct Data*);

