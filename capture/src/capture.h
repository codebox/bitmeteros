#define SERVICE_NAME "BitMeterCaptureService"

#define POLL_INTERVAL   1
#define HAVE_REMOTE

#include <stdio.h>
#include "common.h"
#include "pcap.h"


struct Data* getData();

void setupDb();
int updateDb(int, struct Data*);
int insertData(struct Data* data);
int compressDb();

int getNextCompressTime();

void setupCapture();
int processCapture();
void shutdownCapture();
void logData(struct Data*);

int getTotalForFilter(struct Adapter* adapters, int filterId);

#define LAN_IPS_VALUE "10.0.0.0/8 or 172.16.0.0/12 or 192.168.0.0/16"

char* replace(char* src, char* target, char* replace);
char* getFilterTxt(char* filterTxt, struct Adapter* adapter);

#ifdef UNIT_TESTING
    #define COMPRESS_DB mockCompressDb
    #define GET_NEXT_COMPRESS_TIME getNextCompressTime
    #define PCAP_FINDALLDEVS_EX mockPcap_findalldevs_ex
    #define PCAP_OPEN mockPcap_open
    #define PCAP_SETNONBLOCK mockPcap_setnonblock
    #define PCAP_COMPILE mockPcap_compile
    #define PCAP_SETFILTER mockPcap_setfilter
    #define PCAP_FREECODE mockPcap_freecode
    #ifdef STATS_MODE   
        #define PCAP_SETMODE mockPcap_setmode
    #endif
    #define PCAP_DISPATCH mockPcap_dispatch
    #define PCAP_FREEALLDEVS mockPcap_freealldevs
#else
    #define COMPRESS_DB compressDb
    #define GET_NEXT_COMPRESS_TIME getNextCompressTime
    #define PCAP_FINDALLDEVS_EX pcap_findalldevs_ex
    #define PCAP_OPEN pcap_open
    #define PCAP_SETNONBLOCK pcap_setnonblock
    #define PCAP_COMPILE pcap_compile
    #define PCAP_SETFILTER pcap_setfilter
    #define PCAP_FREECODE pcap_freecode
    #ifdef STATS_MODE   
        #define PCAP_SETMODE pcap_setmode
    #endif
    #define PCAP_DISPATCH pcap_dispatch
    #define PCAP_FREEALLDEVS pcap_freealldevs
#endif