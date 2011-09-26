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

struct SyncPrefs{
	int version;
	int help;
	char** hosts;
	int hostCount;
	int port;
	char* alias;
	char* errMsg;
};

int parseSyncArgs(int argc, char **argv, struct SyncPrefs *prefs);
void doHelp();
void doVersion();
