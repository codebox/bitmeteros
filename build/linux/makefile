# Folders
ifdef TESTING
CC=gcc -DUNIT_TESTING
PATH_TO_ROOT=../../..
else
CC=gcc 
PATH_TO_ROOT=../..
endif

CLIENT_SRC=$(PATH_TO_ROOT)/bmclient/src
CAPTURE_SRC=$(PATH_TO_ROOT)/capture/src
SHARED_SRC=$(PATH_TO_ROOT)/shared/src
SHARED_CLIENT_SRC=$(PATH_TO_ROOT)/shared_client/src
WEB_SRC=$(PATH_TO_ROOT)/webserver/src
BMDB_SRC=$(PATH_TO_ROOT)/bmdb/src
SYNC_SRC=$(PATH_TO_ROOT)/bmsync/src
BUILD_TEST_DIR=test
BUILD_TEST=$(PATH_TO_ROOT)/build/linux/${BUILD_TEST_DIR}

CLIENT_TEST=$(PATH_TO_ROOT)/bmclient/test
CAPTURE_TEST=$(PATH_TO_ROOT)/capture/test
SHARED_TEST=$(PATH_TO_ROOT)/shared/test
SHARED_CLIENT_TEST=$(PATH_TO_ROOT)/shared_client/test
WEB_TEST=$(PATH_TO_ROOT)/webserver/test
BMDB_TEST=$(PATH_TO_ROOT)/bmdb/test
SYNC_TEST=$(PATH_TO_ROOT)/bmsync/test
TEST=$(PATH_TO_ROOT)/test

# Target files for Help
CLIENT_HELP=$(CLIENT_SRC)\helpText.c
BMDB_HELP=$(BMDB_SRC)\helpText.c
SYNC_HELP=$(SYNC_SRC)\helpText.c

# Names of EXEs
EXE_BMCLIENT=bmclient
EXE_BMDB=bmdb
EXE_BMSYNC=bmsync
EXE_CAP=bmcapture
EXE_WEB=bmws
EXE_TEST=unittests

INCLUDE_DIRS=-I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC) -I$(CLIENT_SRC) -I$(CAPTURE_SRC) -I$(WEB_SRC) -I$(BMDB_SRC) -I$(SYNC_SRC) -I$(TEST) -I/usr/include/malloc
LIBS=-lpcap -lsqlite3 -lm
OBJS_BMSYNC=bmsync.o syncHelp.o syncHelpText.o syncOptions.o sql.o common.o data.o db.o log.o platform.o time.o filter.o nameValuePair.o clientFilter.o sync.o
OBJS_BMDB=bmdb.o bmdbConfig.o upgrade.o bmdbVersion.o bmdbHelpText.o log.o db.o common.o platform.o data.o admin.o filter.o adapter.o filters.o total.o time.o nameValuePair.o clientFilter.o
OBJS_BMWS=bmws.o handleConfig.o handleExport.o handleFile.o handleMonitor.o handleQuery.o handleSync.o handleAlert.o handleSummary.o handleRss.o http.o httpRequest.o data.o common.o log.o platform.o db.o alert.o clientMonitor.o clientSummary.o clientSync.o clientQuery.o clientDump.o clientAlert.o clientUtil.o time.o filter.o nameValuePair.o clientFilter.o bmwsCommon.o
OBJS_CAP=sql.o time.o process.o common.o db.o log.o platform.o data.o adapter.o filter.o total.o nameValuePair.o clientFilter.o
OBJS_CLIENT=bmclient.o options.o monitor.o summary.o query.o dump.o help.o helpText.o time.o common.o db.o log.o platform.o clientDump.o clientMonitor.o clientSummary.o clientQuery.o clientUtil.o data.o filter.o nameValuePair.o clientFilter.o

OBJS_TEST=cmockery.o alltests.o filter.o testFilter.o adapter.o testAdapter.o nameValuePair.o total.o db.o log.o testUtils.o data.o common.o platform.o time.o alert.o testAlert.o testCommon.o clientDump.o testClientDump.o bmclient.o testBmclient.o dump.o testDump.o clientUtil.o testData.o clientAlert.o testClientAlert.o clientMonitor.o testClientMonitor.o clientQuery.o testClientQuery.o clientSummary.o testClientSummary.o clientSync.o testClientSync.o testClientUtil.o bmdbConfig.o testConfig.o testDb.o handleConfig.o handleExport.o testHandleConfig.o testTime.o process.o testProcess.o sql.o monitor.o summary.o query.o help.o options.o http.o handleMonitor.o httpRequest.o handleSummary.o handleQuery.o handleSync.o handleAlert.o handleRss.o handleFile.o helpText.o testSql.o testClientOptions.o testSyncOptions.o syncOptions.o upgrade.o testUpgrade.o testHandleSummary.o testHandleFile.o testHttpRequest.o testHandleQuery.o testHandleMonitor.o testHandleAlert.o testHandleRss.o testNameValuePair.o testTotal.o testHandleSync.o testClientFilter.o clientFilter.o testSync.o sync.o mocks.o testHandleExport.o testHttp.o bmwsCommon.o testBmwsCommon.o

#### compile everything
all: bmsync bmdb bmws bmcapture bmclient 

#### run the unit tests
    
_test: $(OBJS_TEST)
	$(CC) $(OBJS_TEST) -o $(EXE_TEST) $(LIBS)
	clear
	./$(EXE_TEST)

.PHONY: test
test: 
	make TESTING=1 -e -C ${BUILD_TEST_DIR} -f ../makefile _test

cmockery.o: $(TEST)/cmockery.c $(TEST)/cmockery.h 
	$(CC) -c $(TEST)/cmockery.c -o cmockery.o $(INCLUDE_DIRS)

alltests.o: $(TEST)/alltests.c $(TEST)/cmockery.h $(TEST)/test.h
	$(CC) -c $(TEST)/alltests.c -o alltests.o $(INCLUDE_DIRS)

mocks.o: $(TEST)/mocks.c $(TEST)/cmockery.h $(TEST)/test.h
	$(CC) -c $(TEST)/mocks.c -o mocks.o $(INCLUDE_DIRS)

testUtils.o: $(TEST)/testUtils.c $(TEST)/cmockery.h $(SHARED_SRC)/common.h $(CAPTURE_SRC)/capture.h
	$(CC) -c $(TEST)/testUtils.c -o testUtils.o $(INCLUDE_DIRS)

testClientOptions.o: $(CLIENT_TEST)/testOptions.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h  $(CLIENT_SRC)/bmclient.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c -D _BSD_SOURCE $(CLIENT_TEST)/testOptions.c -o testClientOptions.o $(INCLUDE_DIRS)

testBmclient.o: $(CLIENT_TEST)/testBmclient.c $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(CLIENT_TEST)/testBmclient.c -o testBmclient.o $(INCLUDE_DIRS)

testDump.o: $(CLIENT_TEST)/testDump.c $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(CLIENT_TEST)/testDump.c -o testDump.o $(INCLUDE_DIRS)

testConfig.o: $(BMDB_TEST)/testConfig.c $(BMDB_SRC)/bmdb.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(BMDB_TEST)/testConfig.c -o testConfig.o $(INCLUDE_DIRS)

testUpgrade.o: $(BMDB_TEST)/testUpgrade.c $(BMDB_SRC)/bmdb.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(BMDB_TEST)/testUpgrade.c -o testUpgrade.o $(INCLUDE_DIRS)

testSyncOptions.o: $(SYNC_TEST)/testOptions.c $(SHARED_SRC)/common.h $(SYNC_SRC)/bmsync.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c -D _BSD_SOURCE $(SYNC_TEST)/testOptions.c -o testSyncOptions.o $(INCLUDE_DIRS)

testSync.o: $(SYNC_TEST)/testSync.c $(SHARED_SRC)/common.h $(SYNC_SRC)/bmsync.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c -D _BSD_SOURCE $(SYNC_TEST)/testSync.c -o testSync.o $(INCLUDE_DIRS)
	
testProcess.o: $(CAPTURE_TEST)/testProcess.c $(SHARED_SRC)/common.h $(CAPTURE_SRC)/capture.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(CAPTURE_TEST)/testProcess.c -o testProcess.o $(INCLUDE_DIRS)

testSql.o: $(CAPTURE_TEST)/testSql.c $(SHARED_SRC)/common.h $(CAPTURE_SRC)/capture.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(CAPTURE_TEST)/testSql.c -o testSql.o $(INCLUDE_DIRS)
	
testCommon.o: $(SHARED_TEST)/testCommon.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testCommon.c -o testCommon.o $(INCLUDE_DIRS)

testTime.o: $(SHARED_TEST)/testTime.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testTime.c -o testTime.o $(INCLUDE_DIRS)

testFilter.o: $(SHARED_TEST)/testFilter.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testFilter.c -o testFilter.o $(INCLUDE_DIRS)

testAdapter.o: $(SHARED_TEST)/testAdapter.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testAdapter.c -o testAdapter.o $(INCLUDE_DIRS)

testDb.o: $(SHARED_TEST)/testDb.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testDb.c -o testDb.o $(INCLUDE_DIRS)

testData.o: $(SHARED_TEST)/testData.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testData.c -o testData.o $(INCLUDE_DIRS)

testNameValuePair.o: $(SHARED_TEST)/testNameValuePair.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testNameValuePair.c -o testNameValuePair.o $(INCLUDE_DIRS)

testAlert.o: $(SHARED_TEST)/testAlert.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testAlert.c -o testAlert.o $(INCLUDE_DIRS)

testClientSummary.o: $(SHARED_CLIENT_TEST)/testClientSummary.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientSummary.c -o testClientSummary.o $(INCLUDE_DIRS)

testClientFilter.o: $(SHARED_CLIENT_TEST)/testClientFilter.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientFilter.c -o testClientFilter.o $(INCLUDE_DIRS)

testClientMonitor.o: $(SHARED_CLIENT_TEST)/testClientMonitor.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientMonitor.c -o testClientMonitor.o $(INCLUDE_DIRS)

testClientDump.o: $(SHARED_CLIENT_TEST)/testClientDump.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientDump.c -o testClientDump.o $(INCLUDE_DIRS)

testClientQuery.o: $(SHARED_CLIENT_TEST)/testClientQuery.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientQuery.c -o testClientQuery.o $(INCLUDE_DIRS)

testClientSync.o: $(SHARED_CLIENT_TEST)/testClientSync.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientSync.c -o testClientSync.o $(INCLUDE_DIRS)

testClientAlert.o: $(SHARED_CLIENT_TEST)/testClientAlert.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientAlert.c -o testClientAlert.o $(INCLUDE_DIRS)

testClientUtil.o: $(SHARED_CLIENT_TEST)/testClientUtil.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientUtil.c -o testClientUtil.o $(INCLUDE_DIRS)

testHttpRequest.o: $(WEB_TEST)/testHttpRequest.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHttpRequest.c -o testHttpRequest.o $(INCLUDE_DIRS)

testHttp.o: $(WEB_TEST)/testHttp.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHttp.c -o testHttp.o $(INCLUDE_DIRS)

testHandleConfig.o: $(WEB_TEST)/testHandleConfig.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleConfig.c -o testHandleConfig.o $(INCLUDE_DIRS)

testHandleFile.o: $(WEB_TEST)/testHandleFile.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleFile.c -o testHandleFile.o $(INCLUDE_DIRS)

testHandleMonitor.o: $(WEB_TEST)/testHandleMonitor.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleMonitor.c -o testHandleMonitor.o $(INCLUDE_DIRS)

testHandleRss.o: $(WEB_TEST)/testHandleRss.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleRss.c -o testHandleRss.o $(INCLUDE_DIRS)

testHandleQuery.o: $(WEB_TEST)/testHandleQuery.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleQuery.c -o testHandleQuery.o $(INCLUDE_DIRS)

testHandleSummary.o: $(WEB_TEST)/testHandleSummary.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleSummary.c -o testHandleSummary.o $(INCLUDE_DIRS)

testHandleExport.o: $(WEB_TEST)/testHandleExport.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleExport.c -o testHandleExport.o $(INCLUDE_DIRS)

testHandleSync.o: $(WEB_TEST)/testHandleSync.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleSync.c -o testHandleSync.o $(INCLUDE_DIRS)

testHandleAlert.o: $(WEB_TEST)/testHandleAlert.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleAlert.c -o testHandleAlert.o $(INCLUDE_DIRS)

testBmwsCommon.o: $(WEB_TEST)/testBmwsCommon.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testBmwsCommon.c -o testBmwsCommon.o $(INCLUDE_DIRS)

testTotal.o: $(SHARED_TEST)/testTotal.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/cmockery.h 
	$(CC) -c $(SHARED_TEST)/testTotal.c -o testTotal.o $(INCLUDE_DIRS)

httpTesting.o: $(WEB_SRC)/http.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/http.c -o httpTesting.o $(INCLUDE_DIRS)

processTesting.o: $(CAPTURE_SRC)/process.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/process.c -o processTesting.o $(INCLUDE_DIRS)


#### bmsync
bmsync: bmsynchelp $(OBJS_BMSYNC)
	$(CC) $(OBJS_BMSYNC) -o $(EXE_BMSYNC) $(LIBS)

bmsynchelp:
	echo '#include "common.h"' > $(SYNC_HELP)
	echo 'char* helpTxt=' >> $(SYNC_HELP)
	cat $(SYNC_SRC)/../help.txt | sed 's/^/"/' | sed 's/$$/" EOL/'  >> $(SYNC_HELP)
	echo ';' >> $(SYNC_HELP)
	
bmsync.o: $(SYNC_SRC)/bmsync.c $(SHARED_SRC)/common.h $(SYNC_SRC)/bmsync.h $(CAPTURE_SRC)/capture.h $(WEB_SRC)/bmws.h
	$(CC) -c $(SYNC_SRC)/bmsync.c -o bmsync.o $(INCLUDE_DIRS)

sync.o: $(SYNC_SRC)/sync.c $(SHARED_SRC)/common.h $(SYNC_SRC)/bmsync.h $(CAPTURE_SRC)/capture.h $(WEB_SRC)/bmws.h
	$(CC) -c $(SYNC_SRC)/sync.c -o sync.o $(INCLUDE_DIRS)

syncHelp.o: $(SYNC_SRC)/help.c $(SHARED_SRC)/common.h $(SYNC_SRC)/bmsync.h
	$(CC) -c $(SYNC_SRC)/help.c -o syncHelp.o $(INCLUDE_DIRS)

syncHelpText.o: $(SYNC_SRC)/helpText.c $(SHARED_SRC)/common.h
	$(CC) -c $(SYNC_SRC)/helpText.c -o syncHelpText.o $(INCLUDE_DIRS)

syncOptions.o: $(SYNC_SRC)/options.c $(SHARED_SRC)/common.h $(SYNC_SRC)/bmsync.h
	$(CC) -c $(SYNC_SRC)/options.c -o syncOptions.o $(INCLUDE_DIRS)

#### bmdb
bmdb: bmdbhelp $(OBJS_BMDB)
	$(CC) $(OBJS_BMDB) -o $(EXE_BMDB) $(LIBS)

bmdbhelp:
	echo '#include "common.h"' > $(BMDB_HELP)
	echo 'char* helpTxt=' >> $(BMDB_HELP)
	cat $(BMDB_SRC)/../help.txt | sed 's/^/"/' | sed 's/$$/" EOL/'  >> $(BMDB_HELP)
	echo ';' >> $(BMDB_HELP)

bmdbConfig.o: $(BMDB_SRC)/config.c $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	$(CC) -c $(BMDB_SRC)/config.c -o bmdbConfig.o $(INCLUDE_DIRS)

upgrade.o: $(BMDB_SRC)/upgrade.c $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	$(CC) -c $(BMDB_SRC)/upgrade.c -o upgrade.o $(INCLUDE_DIRS)

filters.o: $(BMDB_SRC)/filters.c $(SHARED_SRC)/common.h
	$(CC) -c $(BMDB_SRC)/filters.c -o filters.o $(INCLUDE_DIRS)

admin.o: $(BMDB_SRC)/admin.c 
	$(CC) -c $(BMDB_SRC)/admin.c -o admin.o $(INCLUDE_DIRS)

bmdbVersion.o: $(BMDB_SRC)/version.c $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	$(CC) -c $(BMDB_SRC)/version.c -o bmdbVersion.o $(INCLUDE_DIRS)

bmdbHelpText.o: $(BMDB_SRC)/helpText.c $(SHARED_SRC)/common.h
	$(CC) -c $(BMDB_SRC)/helpText.c -o bmdbHelpText.o $(INCLUDE_DIRS)

bmdb.o: $(BMDB_SRC)/bmdb.c $(BMDB_SRC)/bmdb.h $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	$(CC) -c $(BMDB_SRC)/bmdb.c -o bmdb.o $(INCLUDE_DIRS)

#### bmws
bmws: $(OBJS_BMWS) $(SHARED_SRC)/common.h
	$(CC) $(OBJS_BMWS) -o $(EXE_WEB) $(LIBS)

bmws.o: $(WEB_SRC)/bmws.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/bmws.c -o bmws.o $(INCLUDE_DIRS)

bmwsCommon.o: $(WEB_SRC)/bmwsCommon.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/bmwsCommon.c  -o bmwsCommon.o $(INCLUDE_DIRS)

handleConfig.o: $(WEB_SRC)/handleConfig.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleConfig.c -o handleConfig.o $(INCLUDE_DIRS)

handleFile.o: $(WEB_SRC)/handleFile.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/handleFile.c -o handleFile.o $(INCLUDE_DIRS)

handleMonitor.o: $(WEB_SRC)/handleMonitor.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleMonitor.c -o handleMonitor.o $(INCLUDE_DIRS)

handleSync.o: $(WEB_SRC)/handleSync.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleSync.c -o handleSync.o $(INCLUDE_DIRS)
	
handleAlert.o: $(WEB_SRC)/handleAlert.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleAlert.c -o handleAlert.o $(INCLUDE_DIRS)

handleQuery.o: $(WEB_SRC)/handleQuery.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleQuery.c -o handleQuery.o $(INCLUDE_DIRS)

handleExport.o: $(WEB_SRC)/handleExport.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleExport.c -o handleExport.o $(INCLUDE_DIRS)                                         	

handleSummary.o: $(WEB_SRC)/handleSummary.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleSummary.c -o handleSummary.o $(INCLUDE_DIRS)

handleRss.o: $(WEB_SRC)/handleRss.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleRss.c -o handleRss.o $(INCLUDE_DIRS)

http.o: $(WEB_SRC)/http.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/http.c -o http.o $(INCLUDE_DIRS)

httpRequest.o: $(WEB_SRC)/httpRequest.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/httpRequest.c -o httpRequest.o $(INCLUDE_DIRS)

#### bmcapture
bmcapture: captureDaemon.o $(OBJS_CAP) $(SHARED_SRC)/common.h
	$(CC) captureDaemon.o $(OBJS_CAP) -o $(EXE_CAP) $(LIBS)

captureDaemon.o: $(CAPTURE_SRC)/captureDaemon.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/captureDaemon.c -o captureDaemon.o $(INCLUDE_DIRS)

sql.o: $(CAPTURE_SRC)/sql.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/sql.c -o sql.o $(INCLUDE_DIRS)

process.o: $(CAPTURE_SRC)/process.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/process.c -o process.o $(INCLUDE_DIRS)

adapter.o: $(SHARED_SRC)/adapter.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/adapter.c -o adapter.o $(INCLUDE_DIRS)

total.o: $(SHARED_SRC)/total.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/total.c -o total.o $(INCLUDE_DIRS)

filter.o: $(SHARED_SRC)/filter.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/filter.c -o filter.o $(INCLUDE_DIRS)

#### bmclient
bmclient: bmclienthelp $(OBJS_CLIENT) $(SHARED_SRC)/common.h
	$(CC) $(OBJS_CLIENT) -o $(EXE_BMCLIENT) $(LIBS)

bmclienthelp:
	echo '#include "common.h"' > $(CLIENT_HELP)
	echo 'char* helpTxt=' >> $(CLIENT_HELP)
	cat $(CLIENT_SRC)/../help.txt | sed 's/^/"/' | sed 's/$$/" EOL/' >> $(CLIENT_HELP)
	echo ';' >> $(CLIENT_HELP)

bmclient.o: $(CLIENT_SRC)/bmclient.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(CLIENT_SRC)/bmclient.c -o bmclient.o $(INCLUDE_DIRS)

clientDump.o: $(SHARED_CLIENT_SRC)/clientDump.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_SRC)/clientDump.c -o clientDump.o $(INCLUDE_DIRS)

clientAlert.o: $(SHARED_CLIENT_SRC)/clientAlert.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_SRC)/clientAlert.c -o clientAlert.o $(INCLUDE_DIRS)

clientFilter.o: $(SHARED_CLIENT_SRC)/clientFilter.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_SRC)/clientFilter.c -o clientFilter.o $(INCLUDE_DIRS)

clientMonitor.o: $(SHARED_CLIENT_SRC)/clientMonitor.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_SRC)/clientMonitor.c -o clientMonitor.o $(INCLUDE_DIRS)

clientSummary.o: $(SHARED_CLIENT_SRC)/clientSummary.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_SRC)/clientSummary.c -o clientSummary.o $(INCLUDE_DIRS)

clientQuery.o: $(SHARED_CLIENT_SRC)/clientQuery.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_SRC)/clientQuery.c -o clientQuery.o $(INCLUDE_DIRS)

clientSync.o: $(SHARED_CLIENT_SRC)/clientSync.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_SRC)/clientSync.c -o clientSync.o $(INCLUDE_DIRS)

clientUtil.o: $(SHARED_CLIENT_SRC)/clientUtil.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_SRC)/clientUtil.c -o clientUtil.o $(INCLUDE_DIRS)
 
options.o: $(CLIENT_SRC)/options.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(CLIENT_SRC)/options.c -o options.o $(INCLUDE_DIRS)

monitor.o: $(CLIENT_SRC)/monitor.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/platform.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(CLIENT_SRC)/monitor.c -o monitor.o $(INCLUDE_DIRS)

summary.o: $(CLIENT_SRC)/summary.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(CLIENT_SRC)/summary.c -o summary.o $(INCLUDE_DIRS)

query.o: $(CLIENT_SRC)/query.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(CLIENT_SRC)/query.c -o query.o $(INCLUDE_DIRS)

dump.o: $(CLIENT_SRC)/dump.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(CLIENT_SRC)/dump.c -o dump.o $(INCLUDE_DIRS)

help.o: $(CLIENT_SRC)/help.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(CLIENT_SRC)/help.c -o help.o $(INCLUDE_DIRS)

helpText.o: $(CLIENT_SRC)/helpText.c $(SHARED_SRC)/common.h
	$(CC) -c $(CLIENT_SRC)/helpText.c -o helpText.o $(INCLUDE_DIRS)

time.o: $(SHARED_SRC)/time.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/time.c -o time.o $(INCLUDE_DIRS)

common.o: $(SHARED_SRC)/common.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/common.c -o common.o $(INCLUDE_DIRS)

db.o: $(SHARED_SRC)/db.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/db.c -o db.o $(INCLUDE_DIRS)

log.o: $(SHARED_SRC)/log.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/log.c -o log.o $(INCLUDE_DIRS)

platform.o: $(SHARED_SRC)/platform.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/platform.c -o platform.o $(INCLUDE_DIRS)

data.o: $(SHARED_SRC)/data.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/data.c -o data.o $(INCLUDE_DIRS)

alert.o: $(SHARED_SRC)/alert.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/alert.c -o alert.o $(INCLUDE_DIRS)

nameValuePair.o: $(SHARED_SRC)/nameValuePair.c $(SHARED_SRC)/common.h
	$(CC) -c $(SHARED_SRC)/nameValuePair.c -o nameValuePair.o $(INCLUDE_DIRS)

clean:
	-rm *.o
