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
#ifdef TESTING
void setPrevData(struct Data* data);
#endif

int getTotalForFilter(struct Adapter* adapters, int filterId);

#define LAN_IPS_VALUE "10.0.0.0/8 or 172.16.0.0/12 or 192.168.0.0/16"

char* replace(char* src, char* target, char* replace);
char* getFilterTxt(char* filterTxt, struct Adapter* adapter);

struct ProcessCalls{
	sqlite3* (*openDb)();
	void (*closeDb)();
	int (*compressDb)();
	int (*getNextCompressTime)();
	int (*pcap_findalldevs_ex)(char *source, struct pcap_rmtauth *auth, pcap_if_t **alldevs, char *errbuf);	
	pcap_t* (*pcap_open)(const char *source, int snaplen, int flags, int read_timeout, struct pcap_rmtauth *auth, char *errbuf);
	int	(*pcap_setnonblock)(pcap_t *, int, char *);
	int	(*pcap_compile)(pcap_t *, struct bpf_program *, const char *, int, bpf_u_int32);
	int	(*pcap_setfilter)(pcap_t *, struct bpf_program *);
	void (*pcap_freecode)(struct bpf_program *);
#ifdef STATS_MODE	
	int (*pcap_setmode)(pcap_t *p, int mode);
#endif
	int	(*pcap_dispatch)(pcap_t *, int, pcap_handler, u_char *);
	void (*pcap_freealldevs)(pcap_if_t *alldevsp);
};
struct ProcessCalls mockProcessCalls;
