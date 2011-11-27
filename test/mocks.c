#define _GNU_SOURCE
#include <test.h> 
#include <stdarg.h> 
#include <stdio.h>
#include "common.h"
#include <stdlib.h> 
#include <stdarg.h>
#include <setjmp.h> 
#include <cmockery.h> 
#include <bmclient.h> 
#include "pcap.h"
#include "remote-ext.h"

void mockDoHelp(int dummy){
    check_expected(dummy);
}
void mockDoBmClientVersion(int dummy){
    check_expected(dummy);
}
void mockDoSummary(int dummy){
    check_expected(dummy);
}
void mockDoMonitor(int dummy){
    check_expected(dummy);
}
void mockDoQuery(int dummy){
    check_expected(dummy);
}
void mockDoDump(int dummy){
    check_expected(dummy);
}
void mockSetLogLevel(int level){
    check_expected(level);
}
struct Prefs _prefs;
int mockParseArgs(int argc, char** argv, struct Prefs* prefs){
    *prefs = _prefs;
    return (int) mock();
}
void mockDbVersionCheck(int dummy){
    check_expected(dummy);
}
sqlite3* mockOpenDb(int dummy){
    check_expected(dummy);
}
void mockCloseDb(int dummy){
    check_expected(dummy);
}
void mockToTime(char* c, time_t t){
    sprintf(c, "T%d", t);
}
void mockToDate(char* c, time_t t){
    sprintf(c, "D%d", t);
}
void mockFormatAmountByUnits(const BW_INT v, char* c, int units){
    check_expected(units);
    sprintf(c, "%d", (int)v);
}
char* getRecvLine(){
    return (char*)mock();
}
int mockRecv(SOCKET fd, char* buffer, int a, int b){
    char* txt = getRecvLine();
    if (txt != NULL){
        buffer[0] = txt[0]; 
        buffer[1] = 0;
        return 1;
    } else {
        return 0;
    }
}
int mockSend(SOCKET fd, char* buffer, int a, int b){
    check_expected(buffer);
}

int mockCompressDb(int dummy){
    //check_expected(dummy);
}
int mockGetNextCompressTime(){
    return 100;
}
int mockPcap_findalldevs_ex(char *source, struct pcap_rmtauth *auth, pcap_if_t **alldevs, char *errbuf){
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
pcap_t* mockPcap_open(const char *source, int snaplen, int flags, int read_timeout, struct pcap_rmtauth *auth, char *errbuf){
    check_expected(flags);
    return (pcap_t*)mock();
}
int mockPcap_setnonblock(pcap_t* h, int i, char * c){
    check_expected(h);
    return 0;
}
int mockPcap_compile(pcap_t *h, struct bpf_program *p, const char *c, int i, bpf_u_int32 b){
    check_expected(h);
    check_expected(c);
    return 0;
}
int mockPcap_setfilter(pcap_t *h, struct bpf_program *p){
    check_expected(h);
    return 0;
}
void mockPcap_freecode(struct bpf_program *p){

}
#ifdef STATS_MODE   
    int mockPcap_setmode(pcap_t *h, int mode){
        check_expected(h);
        return 0;
    }
#endif
int mockPcap_close(pcap_t *h){
    check_expected(h);
    return 0;
}
int mockPcap_dispatch(pcap_t *h, int i, pcap_handler fn, u_char *u){
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

void mockPcap_freealldevs(pcap_if_t *device){
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

void mockWriteHeadersServerError(SOCKET fd, char* msg, ...){
    check_expected(msg);
}
void mockWriteHeadersOk(SOCKET fd, char* contentType, int endHeaders){
    check_expected(contentType);
    check_expected(endHeaders);
}
void mockWriteHeadersForbidden(SOCKET fd, char* msg){
    check_expected(msg);
}
void mockWriteText(SOCKET fd, char* txt){
    check_expected(txt);
}

void mockWriteNumValueToJson(SOCKET fd, char* key, BW_INT value){
    check_expected(key);
    check_expected(value);
}
void mockWriteTextValueToJson(SOCKET fd, char* key, char* value){
    check_expected(key);
    check_expected(value);
}
void mockWriteTextArrayToJsonValue(char* value){
    check_expected(value);
}
void mockWriteTextArrayToJson(SOCKET fd, char* key, char** values){
    if (key != NULL){
        check_expected(key);
    }
    
    while (*values != NULL){
        mockWriteTextArrayToJsonValue(*values);
        values++;
    }
}
void mockWriteHeader(SOCKET fd, char* name, char* value){
    check_expected(name);
    check_expected(value);
}
void mockWriteEndOfHeaders(SOCKET fd){
    check_expected(fd);
}
void mockWriteDataToJsonTs(time_t ts){
    check_expected(ts);
}
void mockWriteDataToJsonVl(BW_INT vl){
    check_expected(vl);
}
void mockWriteDataToJsonDr(int dr){
    check_expected(dr);
}
void mockWriteDataToJsonTg(int fl){
    check_expected(fl);
}
void mockWriteDataToJson(SOCKET fd, struct Data* data){
    while(data != NULL){
        mockWriteDataToJsonTs(data->ts);
        mockWriteDataToJsonVl(data->vl);
        mockWriteDataToJsonDr(data->dr);
        mockWriteDataToJsonTg(data->fl);
        data = data->next;
    }
}
size_t mockFread(void* buffer, size_t a, size_t b, FILE* c){
    char* txt = mock();
    strcpy(buffer, txt);
    return strlen(txt);
}
void mockWriteData(SOCKET fd, char* data, int len){
    check_expected(data);
}
void mockCheckFilterValues(int id, char* name){
    check_expected(id);
    check_expected(name);
}
void mockWriteFilterData(SOCKET fd, struct Filter* filter){
    mockCheckFilterValues(filter->id, filter->name);
}

void mockCheckDataValues(int filterId, int value){
    check_expected(filterId);
    check_expected(value);
}
void mockWriteSyncData(SOCKET fd, struct Data* data){
    mockCheckDataValues(data->fl, data->vl);
}
