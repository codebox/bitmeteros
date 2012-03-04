#include "common.h"
#include "pcap.h"
#ifndef _WIN32
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

static char* makeAdapterIpList(pcap_if_t *device);

struct Adapter* allocAdapter(pcap_if_t *device){
    struct Adapter* adapter = malloc(sizeof(struct Adapter));

    adapter->name  = strdup(device->name);
    adapter->ips   = makeAdapterIpList(device);
    adapter->next  = NULL;
    adapter->total = NULL;
        
    return adapter;
}

void freeAdapters(struct Adapter* adapter){
    while (adapter != NULL) {
        struct Adapter* next = adapter->next;
        if (adapter->name != NULL) {
            free(adapter->name);
        }
        if (adapter->ips != NULL) {
            free(adapter->ips);
        }
        freeTotals(adapter->total);
        
        free(adapter);
        
        adapter = next;
    }
}

void appendAdapter(struct Adapter** earlierAdapter, struct Adapter* newAdapter){
    if (*earlierAdapter == NULL){
        *earlierAdapter = newAdapter;
    } else {
        struct Adapter* curr = *earlierAdapter;
        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = newAdapter;
    }
}

static char* makeAdapterIpList(pcap_if_t *device){
    struct pcap_addr *address = device->addresses;
    char* addrList = NULL;
    struct sockaddr_in *ipaddr;
    char* iptxt;
    char* tmpAddrList;
    
    while (address != NULL) {
        ipaddr = (struct sockaddr_in*) address->addr;
        iptxt = strdup(inet_ntoa((struct in_addr) ipaddr->sin_addr));
        
        if (addrList != NULL) {
            int len = strlen(addrList) + 4 + strlen(iptxt) + 1;
            tmpAddrList = malloc(len);
            sprintf(tmpAddrList, "%s or %s", addrList, iptxt);
            tmpAddrList[len-1] = 0;
            
            free(addrList);
            free(iptxt);
        } else {
            tmpAddrList = iptxt;
        }
        
        addrList = tmpAddrList;
        address = address->next;
    }
    return addrList;
}
