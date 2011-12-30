#ifdef _WIN32
    #define __USE_MINGW_ANSI_STDIO 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "capture.h"
#include <pcap/pcap.h>
#include "pcap.h"
#include "remote-ext.h"

#define NON_PROMISCUOUS_MODE 0
#define SNAP_LEN 65535

/*
Contains the high-level code that is invoked by the main processing loop of the application. The code
in here co-ordinates the various data capture and storage invocations, as well as performing the required
initialisation, and termination steps.
*/

static int tsCompress;
static int dbWriteInterval;
static pcap_if_t *allDevices;
struct Adapter* adapters;
struct Filter* filters;
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
static pcap_t* getFilterHandle(char* dev, char* filter, int promiscuousMode);

static void setCustomLogLevel(){
 // If a custom logging level for the capture process has been set in the db then use it
    int dbLogLevel = getConfigInt(CONFIG_CAP_LOG_LEVEL, TRUE);
    if (dbLogLevel > 0) {
        setLogLevel(dbLogLevel);
    }
}

void setupCapture(){
    adapters = NULL;
    
 // Called once when the application starts - setup up the various db related things...
    OPEN_DB();
    setCustomLogLevel();
    dbVersionCheck();
    setupDb();
    COMPRESS_DB();

    tsCompress = GET_NEXT_COMPRESS_TIME();
    
 // Check how often we should write captured values to the database - the default is every second
    dbWriteInterval = getConfigInt(CONFIG_DB_WRITE_INTERVAL, TRUE);
    if (dbWriteInterval < 1){
        dbWriteInterval = 1;    
    }
    
    filters = readFilters();
    
    char errbuf[PCAP_ERRBUF_SIZE];

 // See what network devices are available
 	#ifdef _WIN32
	    if (PCAP_FINDALLDEVS_EX(PCAP_SRC_IF_STRING, NULL, &allDevices, errbuf) == -1) {
    	    logMsg(LOG_ERR, "Error in pcap_findalldevs_ex: %s", errbuf);
        	exit(1); //TODO
	    }
    #else
	    if (PCAP_FINDALLDEVS(&allDevices, errbuf) == -1) {
    	    logMsg(LOG_ERR, "Error in pcap_findalldevs: %s", errbuf);
        	exit(1); //TODO
	    }
    #endif
    
 // Are we capturing in promiscuous mode?
    int promiscuousMode;
    if (getConfigInt(CONFIG_CAP_PROMISCUOUS, TRUE) == TRUE) {
        promiscuousMode = PCAP_OPENFLAG_PROMISCUOUS;
    } else {
        promiscuousMode = NON_PROMISCUOUS_MODE;
    }

 // Build the Adapter structs - one for each device
    pcap_if_t *device = allDevices;

    while (device != NULL) {
    	int isLoopback = (device->flags & PCAP_IF_LOOPBACK);

        if (device->addresses != NULL && !isLoopback){
            struct Adapter* adapter = allocAdapter(device);
         // Each Adapter has a Total struct for each Filter
            struct Total*  totals = NULL;
            struct Filter* filter = filters;
            while (filter != NULL) {
                struct Total* total = allocTotal(filter);
                
             // Modify the filter expression to suit the current adapter (if necessary)
                char* filterTxt = getFilterTxt(filter->expr, adapter);
                logMsg(LOG_INFO, "Adding filter '%s' to device %s", filterTxt, adapter->name);

                pcap_t* handle = getFilterHandle(adapter->name, filterTxt, promiscuousMode);
                if (handle != NULL) {
                    total->handle = handle;
                    appendTotal(&totals, total);
                } else {
                    free(total);
                }
                free(filterTxt);
                filter = filter->next;  
            }
            appendTotal(&(adapter->total), totals); 
            appendAdapter(&adapters, adapter);
        }
        
        device = device->next;
    }
}

static pcap_t* getFilterHandle(char* dev, char* filter, int promiscuousMode){ 
    pcap_t *adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];

	#ifdef _WIN32
	    if ((adhandle = PCAP_OPEN(dev, SNAP_LEN, promiscuousMode, 1000, NULL, errbuf)) == NULL) {
    	    logMsg(LOG_ERR, "Unable to open the device %s", dev);
        	return NULL;
	    }
	#else
	    if ((adhandle = PCAP_OPEN_LIVE(dev, SNAP_LEN, promiscuousMode, 1000, errbuf)) == NULL) {
    	    logMsg(LOG_ERR, "Unable to open the device %s", dev);
        	return NULL;
	    }
	#endif

    if (PCAP_SETNONBLOCK(adhandle, 1, errbuf) < 0) {
        logMsg(LOG_ERR, "Unable to set non-blocking mode on device %s: %s", dev, errbuf);
        return NULL;
    }
    
    struct bpf_program fcode;
    if (PCAP_COMPILE(adhandle, &fcode, filter, 1, 0) < 0) {
        logMsg(LOG_ERR, "Unable to compile the packet filter '%s': %s", filter, pcap_geterr(adhandle));
        return NULL;
    }

    int rc = PCAP_SETFILTER(adhandle, &fcode);
    PCAP_FREECODE(&fcode);  
    if (rc < 0) {
        logMsg(LOG_ERR, "Error setting the filter: %s", pcap_geterr(adhandle));
        return NULL;
    }

    #ifdef STATS_MODE
        /* Put the interface in statistics mode */
        if (PCAP_SETMODE(adhandle, MODE_STAT) < 0) { 
            logMsg(LOG_ERR, "Error putting device %s into statistics mode", dev);
            return NULL;
        }
    #endif

    return adhandle;
}

int processCapture(){
 // Called continuously by the main processing loop of the application
    int ts = getTime();

 // Update the Totals for each Adapter
    struct Adapter* adapter = adapters;
    while(adapter != NULL) {
        struct Total* total = adapter->total;
        while(total != NULL){
            PCAP_DISPATCH(total->handle, -1, packet_handler, (u_char *)total);
            total = total->next;    
        }
		
        adapter = adapter->next;
    } 
    
    int status;
 // Accumulate a grand total for each Filter, across all the Adapters
    struct Filter* filter = filters;
    while(filter != NULL){
        int total = getTotalForFilter(adapters, filter->id);
        if (total > 0){
            struct Data data = makeData();
            data.ts = ts;
            data.vl = total;
            data.fl = filter->id;
            
            status = updateDb(POLL_INTERVAL, &data);
            if (status == FAIL) {
                return FAIL;
            }
            logData(&data);
        }
        
        filter = filter->next;
    }
    
 // Is it time to compress the database yet?
    if (ts > tsCompress) {
        status = COMPRESS_DB();
        if (status == FAIL){
            return FAIL;
        }
        tsCompress = GET_NEXT_COMPRESS_TIME();
    }
}

void shutdownCapture(){
 // Called when the application shuts down
    PCAP_FREEALLDEVS(allDevices);
    freeAdapters(adapters);
    freeFilters(filters);
    CLOSE_DB();
}

void logData(struct Data* data){
    if (isLogDebug()){
        while(data != NULL){
            logMsg(LOG_DEBUG, "%d VAL=%llu %d", getTime(), data->vl, data->fl);
            data = data->next;
        }
    }
}

char* getFilterTxt(char* filterTxt, struct Adapter* adapter) {
    if (filterTxt == NULL){
        return NULL;    
    } else {
        char* result = strdup(filterTxt);
        if (strstr(filterTxt, ADAPTER_IPS) != NULL){
            char* tmp = replace(result, ADAPTER_IPS, adapter->ips);
            free(result);
            result = tmp;
        }
        if (strstr(filterTxt, LAN_IPS) != NULL){
            char* tmp = replace(result, LAN_IPS, LAN_IPS_VALUE);
            free(result);
            result = tmp;
        }
        return result;
    }
}

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data){
    #ifdef STATS_MODE
        struct Total* total = (struct Total*)param;
        total->count += *(LONGLONG*)(pkt_data + 8);
    #else
        struct Total* total = (struct Total*)param;
        total->count += header->len;
    #endif
}
