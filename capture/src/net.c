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

	struct BwData* extractDataFromIf(struct if_msghdr2 *);
	struct BwData* getData(){
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

		struct BwData* firstData = NULL;
		struct BwData* lastData = NULL;

		struct if_msghdr* hdr;

		while (offset < len){
			hdr = (struct if_msghdr *) (buf + offset);

			if((hdr->ifm_type == RTM_IFINFO2) && (hdr->ifm_data.ifi_type != IFT_LOOP)){
				struct if_msghdr2 *hdr2 = (struct if_msghdr2 *)hdr;

				struct BwData* thisData = extractDataFromIf(hdr2);

				if (firstData==NULL){
					lastData = firstData = thisData;
				} else {
					lastData->next = thisData;
					lastData = thisData;
				}
			}

			offset += hdr->ifm_msglen;
		}
		free(buf);

		return firstData;
	}

	struct BwData* extractDataFromIf(struct if_msghdr2 *ifHdr){
		struct BwData* data = malloc(sizeof(struct BwData));
		data->dl   = ifHdr->ifm_data.ifi_ibytes;
	    data->ul   = ifHdr->ifm_data.ifi_obytes;
	    data->next = NULL;

		char ifName[IF_NAMESIZE];
		if_indextoname(ifHdr->ifm_index, ifName);

		int nameLen = strlen(ifName);
		data->addrLen = nameLen;
	    memcpy(&(data->addr), ifName, nameLen);

	    return data;
	}
#endif

#ifdef __linux__
	#include <string.h>
	struct BwData* parseProcNetDevLine(char*, char*);
    #define PROC_NET_DEV "proc/net/dev"

	struct BwData* getData(){
		FILE* fProcNetDev = fopen(PROC_NET_DEV, "r");		//TODO just open once?

		if (fProcNetDev == NULL){
			logMsg(LOG_ERR, "Unable to open " PROC_NET_DEV);
			return NULL;
		}

		const int MAX_LINE_SIZE = 256;
		char line[MAX_LINE_SIZE];
		char* colonPos;

		struct BwData* firstData = NULL;
		struct BwData* lastData = NULL;

		while (fgets(line, MAX_LINE_SIZE, fProcNetDev) != NULL) {
			if ((colonPos = strchr(line, ':')) != NULL ){
				char* ifName = calloc(32, 1);
				strncpy(ifName, line, colonPos-line);

				struct BwData* thisData = parseProcNetDevLine(ifName, colonPos+1);
				if (firstData==NULL){
					lastData = firstData = thisData;
				} else {
					lastData->next = thisData;
					lastData = thisData;
				}
			}
		}
		fclose(fProcNetDev);

		return firstData;
	}

	struct BwData* parseProcNetDevLine(char* ifName, char* line){
		unsigned long dummy, dl, ul;
		sscanf(line, "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
			&dl, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &ul, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy, &dummy);

		struct BwData* data = malloc(sizeof(struct BwData));
		data->dl      = dl;
	    data->ul      = ul;
	    data->addrLen = strlen(ifName);
	    data->next    = NULL;
	    memcpy(&(data->addr), ifName, strlen(ifName));

	    return data;
	}

#endif

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <iphlpapi.h>
	static void logErrMsg(char* msg, int rc);
	static unsigned char NULL_ADDRESS[MAXLEN_PHYSADDR] = {0,0,0,0,0,0,0,0};
	
	struct BwData* getData(){
		MIB_IFTABLE* pIfTable = (MIB_IFTABLE *) malloc(sizeof (MIB_IFTABLE));
		unsigned long dwSize = sizeof (MIB_IFTABLE);

		if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
	        free(pIfTable);
	        pIfTable = (MIB_IFTABLE *) malloc(dwSize);
	    }

		unsigned long ul=0;
		unsigned long dl=0;
		int numEntries;
		struct BwData* firstData = NULL;
		struct BwData* lastData  = NULL;
		struct BwData* thisData  = NULL;

		int rc = GetIfTable(pIfTable, &dwSize, FALSE);
		if (rc == NO_ERROR){
			MIB_IFROW* pIfRow;

			numEntries = (int) pIfTable->dwNumEntries;

			int i;
			for (i = 0; i < numEntries; i++) {
				thisData   = malloc(sizeof(struct BwData));

			    pIfRow = (MIB_IFROW *) & pIfTable->table[i];
			    if (pIfRow->dwType != IF_TYPE_SOFTWARE_LOOPBACK){
				    thisData->dl      = pIfRow->dwInOctets;
				    thisData->ul      = pIfRow->dwOutOctets;
				    thisData->addrLen = pIfRow->dwPhysAddrLen;
				    memcpy(&(thisData->addr), &(pIfRow->bPhysAddr), pIfRow->dwPhysAddrLen);
				} else {
					thisData->dl      = 0;
					thisData->ul      = 0;
					thisData->addrLen = MAXLEN_PHYSADDR;
					memcpy(&(thisData->addr), NULL_ADDRESS, MAXLEN_PHYSADDR);
				}
				thisData->next = NULL;

				if (firstData==NULL){
					firstData = thisData;
				} else {
					lastData->next = thisData;
				}
				lastData = thisData;
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

	static void logErrMsg(char* msg, int rc) {
	    LPVOID lpMsgBuf;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	            NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
		logMsg(LOG_ERR, "%s. Code=%d Msg=%s", msg, rc, lpMsgBuf);
	    LocalFree(lpMsgBuf);
	}
#endif
