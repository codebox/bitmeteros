#ifdef _WIN32
	#define __USE_MINGW_ANSI_STDIO 1
#endif
#define HAVE_REMOTE
#include <stdlib.h>
#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "common.h"
#include <string.h>
#include "capture.h"
#include "pcap.h"

/*
Contains unit tests for the process module.
*/

static int _compressDb(int dummy){
	//check_expected(dummy);
}
static int _getNextCompressTime(){
	return 100;
}
static int _pcap_findalldevs_ex(char *source, struct pcap_rmtauth *auth, pcap_if_t **alldevs, char *errbuf){
	pcap_if_t* device1 = malloc(sizeof(pcap_if_t));
	pcap_if_t* device2 = malloc(sizeof(pcap_if_t));

	struct sockaddr* ip1 = malloc(sizeof(struct sockaddr));
	struct sockaddr _ip1 = {1, "001234"}; // IP 49.50.51.52
	*ip1 = _ip1;
	
	struct pcap_addr* addr1 = malloc(sizeof(struct pcap_addr));
	struct pcap_addr _addr1 = {NULL, ip1, NULL, NULL, NULL};
	*addr1 = _addr1;
	
	device1->next = device2;
	device1->name = strdup("dev1");
	device1->description = NULL;
	device1->addresses = addr1;
	device1->flags = 0;
	
	struct sockaddr* ip2 = malloc(sizeof(struct sockaddr));
	struct sockaddr _ip2 = {1, "001234"}; // IP 49.50.51.52
	*ip2 = _ip2;
	
	struct pcap_addr* addr2 = malloc(sizeof(struct pcap_addr));
	struct pcap_addr _addr2 = {NULL, ip2, NULL, NULL, NULL};
	*addr2 = _addr2;
	
	device2->next = NULL;
	device2->name = strdup("dev2");
	device2->description = NULL;
	device2->addresses = addr2;
	device2->flags = 0;

	*alldevs = device1;
}
static pcap_t* _pcap_open(const char *source, int snaplen, int flags, int read_timeout, struct pcap_rmtauth *auth, char *errbuf){
	return (pcap_t*)mock();
}
static int _pcap_setnonblock(pcap_t* h, int i, char * c){
	check_expected(h);
	return 0;
}
static int _pcap_compile(pcap_t *h, struct bpf_program *p, const char *c, int i, bpf_u_int32 b){
	check_expected(h);
	check_expected(c);
	return 0;
}
static int _pcap_setfilter(pcap_t *h, struct bpf_program *p){
	check_expected(h);
	return 0;
}
static void _pcap_freecode(struct bpf_program *p){

}
#ifdef STATS_MODE	
	static int _pcap_setmode(pcap_t *h, int mode){
		check_expected(h);
		return 0;
	}
#endif
static int _pcap_close(pcap_t *h){
	check_expected(h);
	return 0;
}
static int _pcap_dispatch(pcap_t *h, int i, pcap_handler fn, u_char *u){
	check_expected(h);
	
#ifdef STATS_MODE
	int VAL = 12;
	BW_INT v = VAL;
	u_char* p = malloc(sizeof(BW_INT) * 2);
	memcpy(p+8, &v, sizeof(BW_INT));
	
	struct Total* t = allocTotal(NULL);
	(*fn)(t,0,p);
	free(p);
	assert_int_equal(VAL, t->count);
	freeTotals(t);
#else
	int VAL = 12;
	struct pcap_pkthdr header;
	header.len = VAL;
	struct Total* t = allocTotal(NULL);
	(*fn)(t,&header,0);
	assert_int_equal(VAL, t->count);
	freeTotals(t);
#endif
	return 0;
}

static void _pcap_freealldevs(pcap_if_t *device){
	pcap_if_t* nextDev;
	while (device != NULL) {
		nextDev = device->next;
		free(device->name);
		struct pcap_addr* addr = device->addresses;
		free(addr->addr);
		free(addr);
		free(device);
		device = nextDev;
	}
}
sqlite3* _openDb(int dummy){
	check_expected(dummy);
}
void _closeDb(int dummy){
	//check_expected(dummy);	
}

static void setupMocks(){
	#ifdef STATS_MODE
		struct ProcessCalls processCalls = {&_openDb, &_closeDb, &_compressDb, &_getNextCompressTime, &_pcap_findalldevs_ex,
				&_pcap_open, &_pcap_setnonblock, &_pcap_compile, &_pcap_setfilter, &_pcap_freecode, 
				&_pcap_setmode, &_pcap_dispatch, &_pcap_freealldevs};
	#else
		struct ProcessCalls processCalls = {&_openDb, &_closeDb, &_compressDb, &_getNextCompressTime, &_pcap_findalldevs_ex,
				&_pcap_open, &_pcap_setnonblock, &_pcap_compile, &_pcap_setfilter, &_pcap_freecode, 
				&_pcap_dispatch, &_pcap_freealldevs};
	#endif
	mockProcessCalls = processCalls;
	
	struct TotalCalls _totalCalls = {&_pcap_close};
	mockTotalCalls = _totalCalls;
}

void setupForProcessTest(void** state){
	setupTestDb(state);
	setConfigIntValue(CONFIG_DB_VERSION, DB_VERSION);
	setConfigIntValue(CONFIG_CAP_KEEP_SEC_LIMIT, 3600);   
	setConfigIntValue(CONFIG_CAP_KEEP_MIN_LIMIT, 86400);   
	setConfigIntValue(CONFIG_CAP_COMPRESS_INTERVAL, 3600);

	setupMocks();
}

void teardownForProcessTest(void** state){
	tearDownTestDb(state);
}

void testProcess(void** state){
	setupForProcessTest(0);            
	char* filter1 = "port 80";
	char* filter2 = "port 90";
	addFilterRow(1, "Filter 1", "f1", filter1, NULL);
	addFilterRow(2, "Filter 2", "f2", filter2, NULL);
	
	int pcapOpenHandle1 = 100;
	int pcapOpenHandle2 = 101;
	int pcapOpenHandle3 = 102;
	int pcapOpenHandle4 = 103;
	
	will_return(_pcap_open, pcapOpenHandle1);
	will_return(_pcap_open, pcapOpenHandle2);
	will_return(_pcap_open, pcapOpenHandle3);
	will_return(_pcap_open, pcapOpenHandle4);
	expect_call(_openDb);
	//expect_call(_compressDb);
	
	expect_value(_pcap_setnonblock, h, pcapOpenHandle1);
	expect_value(_pcap_setnonblock, h, pcapOpenHandle2);
	expect_value(_pcap_setnonblock, h, pcapOpenHandle3);
	expect_value(_pcap_setnonblock, h, pcapOpenHandle4);
	
	expect_value(_pcap_compile, h, pcapOpenHandle1);
	expect_value(_pcap_compile, h, pcapOpenHandle2);
	expect_value(_pcap_compile, h, pcapOpenHandle3);
	expect_value(_pcap_compile, h, pcapOpenHandle4);
	
	expect_string(_pcap_compile, c, filter1);
	expect_string(_pcap_compile, c, filter2);
	expect_string(_pcap_compile, c, filter1);
	expect_string(_pcap_compile, c, filter2);
                                      
	expect_value(_pcap_setfilter, h, pcapOpenHandle1);
	expect_value(_pcap_setfilter, h, pcapOpenHandle2);
	expect_value(_pcap_setfilter, h, pcapOpenHandle3);
	expect_value(_pcap_setfilter, h, pcapOpenHandle4);

#ifdef STATS_MODE
	expect_value(_pcap_setmode, h, pcapOpenHandle1);
	expect_value(_pcap_setmode, h, pcapOpenHandle2);
	expect_value(_pcap_setmode, h, pcapOpenHandle3);
	expect_value(_pcap_setmode, h, pcapOpenHandle4);
#endif

	expect_value(_pcap_close, h, pcapOpenHandle1);
	expect_value(_pcap_close, h, pcapOpenHandle2);
	expect_value(_pcap_close, h, pcapOpenHandle3);
	expect_value(_pcap_close, h, pcapOpenHandle4);

	expect_value(_pcap_dispatch, h, pcapOpenHandle1);
	expect_value(_pcap_dispatch, h, pcapOpenHandle2);
	expect_value(_pcap_dispatch, h, pcapOpenHandle3);
	expect_value(_pcap_dispatch, h, pcapOpenHandle4);

	setupCapture();
	processCapture();
	shutdownCapture();
	freeStmtList();
	tearDownTestDb(0);
}

