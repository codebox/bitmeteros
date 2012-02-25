#define SERVICE_NAME "BitMeterCaptureService"

#define POLL_INTERVAL   1
#define HAVE_REMOTE

#include <stdio.h>
#include "common.h"
#include "pcap.h"

#ifndef STATS_MODE
    struct LockableCounterValue{
        int count;
        time_t ts;
        struct LockableCounterValue* next;
    };
    struct LockableCounter{
        struct LockableCounterValue* values;
        int fl;
        pcap_t* handle;
        pthread_mutex_t mutex;
        struct LockableCounter* next;
    };
    struct LockableCounterValue* allocValue();
    struct LockableCounter* allocCounter();
    void addValueToCounter(struct LockableCounter* counter, time_t ts, int v);
    void freeValue(struct LockableCounterValue* value);
    void resetValueForCounter(struct LockableCounter* counter);
    void appendValue(struct LockableCounterValue** earlierValue, struct LockableCounterValue* newValue);
    void freeCounter(struct LockableCounter* counter);
    void appendCounter(struct LockableCounter** earlierCounter, struct LockableCounter* newCounter);
#endif

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
    #ifdef _WIN32
        #define PCAP_FINDALLDEVS_EX mockPcap_findalldevs_ex
    #else
        #define PCAP_FINDALLDEVS mockPcap_findalldevs
    #endif
    #ifdef _WIN32
        #define PCAP_OPEN mockPcap_open
    #else
        #define PCAP_OPEN_LIVE mockPcap_open_live
    #endif
    #define PCAP_SETNONBLOCK mockPcap_setnonblock
    #define PCAP_COMPILE mockPcap_compile
    #define PCAP_SETFILTER mockPcap_setfilter
    #define PCAP_FREECODE mockPcap_freecode
    #ifdef STATS_MODE   
        #define PCAP_SETMODE mockPcap_setmode
    #endif
    #define PCAP_DISPATCH mockPcap_dispatch
    #define PCAP_FREEALLDEVS mockPcap_freealldevs
    #define PTHREAD_MUTEX_INIT mockPthread_mutex_init
    #define PTHREAD_MUTEX_DESTROY mockPthread_mutex_destroy
    #define PTHREAD_MUTEX_LOCK mockPthread_mutex_lock
    #define PTHREAD_MUTEX_UNLOCK mockPthread_mutex_unlock
    #define PTHREAD_CREATE mockPthread_create
#else
    #define COMPRESS_DB compressDb
    #define GET_NEXT_COMPRESS_TIME getNextCompressTime
    #ifdef _WIN32
        #define PCAP_FINDALLDEVS_EX pcap_findalldevs_ex
    #else
        #define PCAP_FINDALLDEVS pcap_findalldevs
    #endif
    #ifdef _WIN32
        #define PCAP_OPEN pcap_open
    #else
        #define PCAP_OPEN_LIVE pcap_open_live
    #endif
    #define PCAP_SETNONBLOCK pcap_setnonblock
    #define PCAP_COMPILE pcap_compile
    #define PCAP_SETFILTER pcap_setfilter
    #define PCAP_FREECODE pcap_freecode
    #ifdef STATS_MODE   
        #define PCAP_SETMODE pcap_setmode
    #endif
    #define PCAP_DISPATCH pcap_dispatch
    #define PCAP_FREEALLDEVS pcap_freealldevs
    #define PTHREAD_MUTEX_INIT pthread_mutex_init
    #define PTHREAD_MUTEX_DESTROY pthread_mutex_destroy
    #define PTHREAD_MUTEX_LOCK pthread_mutex_lock
    #define PTHREAD_MUTEX_UNLOCK pthread_mutex_unlock
    #define PTHREAD_CREATE pthread_create
#endif