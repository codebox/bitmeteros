#include <stdlib.h>
#include <stdio.h>
#include "capture.h"
#include "common.h"

/* Contains platform-specific code for obtaining the network stats that we need */

#ifdef __APPLE__
	#include <sys/socket.h>
	#include <net/if.h>
	#include <net/if_types.h>
	#include <sys/sysctl.h>
	#include <string.h>
	#include <net/route.h>

	struct Data* extractDataFromIf(struct if_msghdr2 *);
	struct Data* getData(){
		int mib[6] = {CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST2, 0};
		size_t len;
		int rc = sysctl(mib, 6, NULL, &len, NULL, 0);
		if (rc<0){
		    logMsg(LOG_ERR, "sysctl returned %d\n", rc);
			return NULL;
		}
		char *buf = malloc(len);
		rc = sysctl(mib, 6, buf, &len, NULL, 0);
		if (rc<0){
			free(buf);
			logMsg(LOG_ERR, "sysctl returned %d\n", rc);
			return NULL;
		}
		int offset=0;

		struct Data* firstData = NULL;
		struct Data* thisData = NULL;

		struct if_msghdr* hdr;

		while (offset < len){
			hdr = (struct if_msghdr *) (buf + offset);

			if((hdr->ifm_type == RTM_IFINFO2) && (hdr->ifm_data.ifi_type != IFT_LOOP)){
				struct if_msghdr2 *hdr2 = (struct if_msghdr2 *)hdr;
				struct Data* thisData = extractDataFromIf(hdr2);

				appendData(&firstData, thisData);
			}

			offset += hdr->ifm_msglen;
		}
		free(buf);

		return firstData;
	}

	struct Data* extractDataFromIf(struct if_msghdr2 *ifHdr){
		struct Data* data = allocData();
		data->dl = ifHdr->ifm_data.ifi_ibytes;
	    data->ul = ifHdr->ifm_data.ifi_obytes;

		char ifName[IF_NAMESIZE];
		if_indextoname(ifHdr->ifm_index, ifName);

	    setAddress(data, ifName);

	    return data;
	}
#endif

#ifdef __linux__
	#include <string.h>
	struct Data* parseProcNetDevLine(char*, char*);
    #define PROC_NET_DEV "/proc/net/dev"

	struct Data* getData(){
		FILE* fProcNetDev = fopen(PROC_NET_DEV, "r");		//TODO just open once?

		if (fProcNetDev == NULL){
			logMsg(LOG_ERR, "Unable to open " PROC_NET_DEV);
			return NULL;
		}

		const int MAX_LINE_SIZE = 256;
		char line[MAX_LINE_SIZE];
		char* colonPos;

		struct Data* firstData = NULL;
		struct Data* thisData  = NULL;

		while (fgets(line, MAX_LINE_SIZE, fProcNetDev) != NULL) {
			if ((colonPos = strchr(line, ':')) != NULL ){
				char* ifName = calloc(32, 1);
				strncpy(ifName, line, colonPos-line);

				thisData = parseProcNetDevLine(ifName, colonPos+1);

				appendData(&firstData, thisData);
			}
		}
		fclose(fProcNetDev);

		return firstData;
	}

	struct Data* parseProcNetDevLine(char* ifName, char* line){
		unsigned long dummy, dl, ul;
		sscanf(line, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
			&dl, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &ul, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy);

		struct Data* data = allocData();
		data->dl = dl;
	    data->ul = ul;
	    setAddress(data, ifName);

	    return data;
	}

#endif

#ifdef _WIN32
	#define MAX_ADDR_BYTES 8
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <iphlpapi.h>
	static void logErrMsg(char* msg, int rc);
	static char* makeHexString(byte*);

	struct Data* getData(){
		MIB_IFTABLE* pIfTable = (MIB_IFTABLE *) malloc(sizeof (MIB_IFTABLE));
		unsigned long dwSize = sizeof (MIB_IFTABLE);

		if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
	        free(pIfTable);
	        pIfTable = (MIB_IFTABLE *) malloc(dwSize);
	    }

		int numEntries;
		struct Data* firstData = NULL;
		struct Data* thisData  = NULL;

		int rc = GetIfTable(pIfTable, &dwSize, FALSE);
		if (rc == NO_ERROR){
			MIB_IFROW* pIfRow;

			numEntries = (int) pIfTable->dwNumEntries;

			int i;
			for (i = 0; i < numEntries; i++) {
			    pIfRow = (MIB_IFROW *) & pIfTable->table[i];
			    if (pIfRow->dwType != IF_TYPE_SOFTWARE_LOOPBACK){
   					thisData = allocData();
				    thisData->dl = pIfRow->dwInOctets;
				    thisData->ul = pIfRow->dwOutOctets;

				    char hexString[MAX_ADDR_BYTES * 2 + 1];
				    makeHexString(hexString, pIfRow->dwPhysAddrLen){
				    setAddress(thisData, hexString);

					appendData(&firstData, thisData);
				}
			}

		} else {
			logErrMsg("GetIfTable Error", rc);
		}

		if (pIfTable != NULL) {
		    free(pIfTable);
		    pIfTable = NULL;
		}

		return firstData;
	}

	char HEX[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	static char* makeHexString(char* hexString, byte* data){ //TODO test
		byte* thisByte;
		for(int i = 0; i < MAX_ADDR_BYTES; i++){
			thisByte = data[i];
			hexString[j*2]     = HEX[(thisByte >> 4) & 0xF];
			hexString[j*2 + 1] = HEX[thisByte & 0x0F];
		}
		hexString[MAX_ADDR_BYTES * 2] = 0;
	}

	static void logErrMsg(char* msg, int rc) {
	    LPVOID lpMsgBuf;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	            NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
		logMsg(LOG_ERR, "%s. Code=%d Msg=%s", msg, rc, lpMsgBuf);
	    LocalFree(lpMsgBuf);
	}
#endif
