#ifdef UNIT_TESTING 
	#include "test.h"
#endif
#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#define HAVE_REMOTE
#include <stdio.h>
#include <stdlib.h>
#include "capture.h"
#include "common.h"
#include <string.h>
#include "pcap.h"
#include "remote-ext.h"

#define NON_PROMISCUOUS_MODE 0

/*
Contains the high-level code that is invoked by the main processing loop of the application. The code
in here co-ordinates the various data capture and storage invocations, as well as performing the required
initialisation, and termination steps.
*/

#ifdef STATS_MODE
	static struct ProcessCalls calls = {&openDb, &closeDb, &compressDb, &getNextCompressTime, &pcap_findalldevs_ex,
		&pcap_open, &pcap_setnonblock, &pcap_compile, &pcap_setfilter, &pcap_freecode, 
		&pcap_setmode, &pcap_dispatch, &pcap_freealldevs};
#else
	static struct ProcessCalls calls = {&openDb, &closeDb, &compressDb, &getNextCompressTime, &pcap_findalldevs_ex,
		&pcap_open, &pcap_setnonblock, &pcap_compile, &pcap_setfilter, &pcap_freecode, 
		&pcap_dispatch, &pcap_freealldevs};
#endif

static struct ProcessCalls getCalls(){
	#ifdef UNIT_TESTING
		return mockProcessCalls;
	#else
		return calls;
	#endif
}

static int tsCompress;
static int dbWriteInterval;
static pcap_if_t *allDevices;
struct Adapter* adapters = NULL;
struct Filter* filters;
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
static pcap_t* getFilterHandle(char* dev, char* filter);

static void setCustomLogLevel(){
 // If a custom logging level for the capture process has been set in the db then use it
	int dbLogLevel = getConfigInt(CONFIG_CAP_LOG_LEVEL, TRUE);
	if (dbLogLevel > 0) {
		setLogLevel(dbLogLevel);
	}
}

void setupCapture(){
 // Called once when the application starts - setup up the various db related things...
    getCalls().openDb();
    setCustomLogLevel();
    dbVersionCheck();
	setupDb();
	getCalls().compressDb();

	tsCompress = getCalls().getNextCompressTime();
	
 // Check how often we should write captured values to the database - the default is every second
	dbWriteInterval = getConfigInt(CONFIG_DB_WRITE_INTERVAL, TRUE);
	if (dbWriteInterval < 1){
		dbWriteInterval = 1;	
	}
	
	filters = readFilters();
	
	char errbuf[PCAP_ERRBUF_SIZE];

 // See what network devices are available
    if (getCalls().pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL, &allDevices, errbuf) == -1) {
        logMsg(LOG_ERR, "Error in pcap_findalldevs_ex: %s", errbuf);
        exit(1); //TODO
    }
    
 // Build the Adapter structs - one for each device
    pcap_if_t *device = allDevices;
    while (device != NULL) {
    	if (device->addresses != NULL){
    		struct Adapter* adapter = allocAdapter(device);
    		
	     // Each Adapter has a Total struct for each Filter
	    	struct Total*  totals = NULL;
	    	struct Filter* filter = filters;
	    	while (filter != NULL) {
	    		struct Total* total = allocTotal(filter);
	    		
	    	 // Modify the filter expression to suit the current adapter (if necessary)
	    		char* filterTxt = getFilterTxt(filter->expr, adapter);
				logMsg(LOG_INFO, "Adding filter '%s' to device %s", filterTxt, adapter->name);

				pcap_t* handle = getFilterHandle(adapter->name, filterTxt);
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

static pcap_t* getFilterHandle(char* dev, char* filter){ 
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
    
    if ((adhandle = getCalls().pcap_open(dev, 100, NON_PROMISCUOUS_MODE, 1000, NULL, errbuf)) == NULL) {
        logMsg(LOG_ERR, "Unable to open the device %s", dev);
        return NULL;
    }

	if (getCalls().pcap_setnonblock(adhandle, 1, errbuf) < 0) {
		logMsg(LOG_ERR, "Unable to set non-blocking mode on device %s: %s", dev, errbuf);
		return NULL;
	}
	
	struct bpf_program fcode;
	if (getCalls().pcap_compile(adhandle, &fcode, filter, 1, 0) < 0) {
        logMsg(LOG_ERR, "Unable to compile the packet filter '%s': %s", filter, pcap_geterr(adhandle));
        return NULL;
    }

	int rc = getCalls().pcap_setfilter(adhandle, &fcode);
    getCalls().pcap_freecode(&fcode); 	
	if (rc < 0) {
        logMsg(LOG_ERR, "Error setting the filter: %s", pcap_geterr(adhandle));
        return NULL;
    }

    #ifdef STATS_MODE
	    /* Put the interface in statistics mode */
	    if (getCalls().pcap_setmode(adhandle, MODE_STAT) < 0) { 
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
			getCalls().pcap_dispatch(total->handle, -1, packet_handler, (u_char *)total);
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
			printf("%s: %d\n", filter->name, total);
			
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
		status = getCalls().compressDb();
		if (status == FAIL){
            return FAIL;
		}
		tsCompress = getCalls().getNextCompressTime();
	}
}

void shutdownCapture(){
 // Called when the application shuts down
 	getCalls().pcap_freealldevs(allDevices);
 	freeAdapters(adapters);
 	freeFilters(filters);
	getCalls().closeDb();
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
