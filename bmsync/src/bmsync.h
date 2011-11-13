#ifdef _WIN32
    #include <winsock2.h>
#endif
#define OPT_HELP    'h'
#define OPT_VERSION 'v'
#define OPT_PORT    'p'
#define OPT_ALIAS   'a'

#define HTTP_EOL "\r\n"
#define ERR_OPT_NO_ARGS "No arguments were supplied"
#define ERR_NO_HOST "No host name/IP address was supplied"
#define ERR_MULTIPLE_HOSTS_ONE_ALIAS "Multiple hosts were specified with a single alias, this probably isn't what you want to do"
#define ERR_BAD_PORT "Bad port number"

#define SYNC_NAME "bmsync"
#define MAX_REQUEST_LEN   1024
#define DEFAULT_PORT 2605
#define DEFAULT_HTTP_PORT 80
#define MAX_ADDR_LEN 64
#define MAX_LINE_LEN 128

#define MSG_CONNECTING "Connecting..."
#define MSG_CONNECTED  "Connected"

struct SyncPrefs{
    int version;
    int help;
    char** hosts;
    int hostCount;
    int port;
    char* alias;
    char* errMsg;
};

struct RemoteFilter{
    int remoteId;
    int localId;
    struct RemoteFilter* next;
};

int parseSyncArgs(int argc, char **argv, struct SyncPrefs *prefs);
void doHelp();
void doVersion();
time_t getMaxTsForHost(char* alias);
struct Filter* parseFilterRow(char* row, char* host);
struct Data* parseDataRow(char* row);
int startsWith(char* txt, char* start);
int getLocalId(struct RemoteFilter* remoteFilter, int filterId);
int getLocalFilter(struct Filter *remoteFilter);
void appendRemoteFilter(struct RemoteFilter** remoteFilters, struct RemoteFilter* newRemoteFilter);
void removeDataForDeletedFiltersFromThisHost(char* host, struct RemoteFilter* remoteFilter);
int readLine(SOCKET fd, char* line);
int httpHeadersOk(SOCKET fd);
int parseData(SOCKET fd, char* alias, int* rowCount);
int sendRequest(SOCKET fd, time_t ts, char* host, int port);
