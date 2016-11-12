# Folders
CLIENT_SRC=../../bmclient/src
CAPTURE_SRC=../../capture/src
SHARED_SRC=../../shared/src
SHARED_CLIENT_SRC=../../shared_client/src
WEB_SRC=../../webserver/src
BMDB_SRC=../../bmdb/src
SYNC_SRC=../../bmsync/src
SQLITE_SRC=./sqlite3

CLIENT_TEST=../../bmclient/test
CAPTURE_TEST=../../capture/test
SHARED_TEST=../../shared/test
SHARED_CLIENT_TEST=../../shared_client/test
WEB_TEST=../../webserver/test
BMDB_TEST=../../bmdb/test
SYNC_TEST=../../bmsync/test
TEST=../../test

EXE_BMCLIENT=bmclient
EXE_BMDB=bmdb
EXE_BMSYNC=bmsync
EXE_CAP=bmcapture
EXE_WEB=bmws
EXE_TEST=unittests

# Target files for Help
CLIENT_HELP=$(CLIENT_SRC)/helpText.c
BMDB_HELP=$(BMDB_SRC)/helpText.c
SYNC_HELP=$(SYNC_SRC)/helpText.c

INCLUDE_DIRS=-I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC) -I$(CLIENT_SRC) -I$(CAPTURE_SRC) -I$(WEB_SRC) -I$(BMDB_SRC) -I$(SYNC_SRC) -I$(TEST) -I/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/include/malloc
LIBS=-lm
OBJS_BMSYNC=bmsync.o syncHelp.o syncHelpText.o syncOptions.o sql.o common.o data.o db.o log.o platform.o time.o sqlite3.o
OBJS_BMDB=bmdb.o bmdbConfig.o upgrade.o bmdbVersion.o bmdbHelpText.o log.o db.o common.o platform.o data.o admin.o sqlite3.o
OBJS_BMWS=bmws.o handleConfig.o handleFile.o handleMonitor.o handleQuery.o handleSync.o handleAlert.o handleSummary.o handleExport.o handleRss.o http.o httpRequest.o data.o common.o log.o platform.o db.o alert.o clientMonitor.o clientSummary.o clientSync.o clientQuery.o clientDump.o clientAlert.o clientUtil.o time.o sqlite3.o
OBJS_CAP=captureDaemon.o net.o sql.o time.o process.o common.o db.o log.o platform.o data.o sqlite3.o
OBJS_CLIENT=bmclient.o options.o monitor.o summary.o query.o dump.o help.o helpText.o time.o common.o db.o log.o platform.o clientDump.o clientMonitor.o clientSummary.o clientQuery.o clientUtil.o data.o sqlite3.o

OBJS_TEST=CuTest.o alltests.o testUtils.o db.o log.o data.o common.o options.o clientUtil.o testClientOptions.o testConfig.o bmdbConfig.o testUpgrade.o upgrade.o testSyncOptions.o syncOptions.o testProcess.o process.o sql.o time.o testSql.o testCommon.o testTime.o testDb.o testData.o testClientSummary.o clientSummary.o testClientMonitor.o clientMonitor.o testClientDump.o clientDump.o clientAlert.o testClientQuery.o clientQuery.o testClientSync.o clientSync.o testClientUtil.o testHttpRequest.o httpRequest.o testHandleConfig.o handleConfig.o httpTesting.o handleMonitor.o handleSummary.o handleQuery.o handleSync.o handleAlert.o handleExport.o handleFile.o handleRss.o testHandleMonitor.o testHandleQuery.o testHandleSummary.o testHandleFile.o testHandleSync.o testHandleAlert.o alert.o testAlert.o testClientAlert.o testHandleRss.o

CC=gcc -Wall -arch i386 -arch x86_64 -mmacosx-version-min=10.5
#CC=gcc -Wall -arch i386 -arch ppc -arch ppc64
#CC=gcc -DTESTING -Wall -arch i386 -arch ppc -arch ppc64

#### Install or reinstall
install: all
	-sudo invoke-rc.d bitmeter stop
	-sudo invoke-rc.d bitmeterweb stop

	-sudo mv $(EXE_BMCLIENT) /usr/bin/$(EXE_BMCLIENT) 
	-sudo mv $(EXE_BMDB) /usr/bin/$(EXE_BMDB) 
	-sudo mv $(EXE_BMSYNC) /usr/bin/$(EXE_BMSYNC) 
	-sudo mv $(EXE_CAP) /usr/bin/$(EXE_CAP) 
	-sudo mv $(EXE_WEB) /usr/bin/$(EXE_WEB) 

	-sudo invoke-rc.d bitmeter start
	-sudo invoke-rc.d bitmeterweb start

#### compile everything
all: bmsync bmdb bmws bmcapture bmclient 

#### run the unit tests
test: $(OBJS_BMSYNC) $(OBJS_BMDB) $(OBJS_BMWS) $(OBJS_CAP) $(OBJS_CLIENT) $(OBJS_TEST)
	$(CC) $(OBJS_TEST) -o $(EXE_TEST) $(LIBS)
	clear
	./$(EXE_TEST)

CuTest.o: $(TEST)/CuTest.c $(TEST)/CuTest.h
	$(CC) -c $(TEST)/CuTest.c -o CuTest.o $(INCLUDE_DIRS)

alltests.o: $(TEST)/alltests.c $(TEST)/CuTest.h $(TEST)/test.h
	$(CC) -c $(TEST)/alltests.c -o alltests.o $(INCLUDE_DIRS)

testUtils.o: $(TEST)/testUtils.c $(TEST)/CuTest.h $(SHARED_SRC)/common.h $(CAPTURE_SRC)/capture.h
	$(CC) -c $(TEST)/testUtils.c -o testUtils.o $(INCLUDE_DIRS)

testClientOptions.o: $(CLIENT_TEST)/testOptions.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h  $(CLIENT_SRC)/bmclient.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c -D _BSD_SOURCE $(CLIENT_TEST)/testOptions.c -o testClientOptions.o $(INCLUDE_DIRS)

testConfig.o: $(BMDB_TEST)/testConfig.c $(BMDB_SRC)/bmdb.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(BMDB_TEST)/testConfig.c -o testConfig.o $(INCLUDE_DIRS)

testUpgrade.o: $(BMDB_TEST)/testUpgrade.c $(BMDB_SRC)/bmdb.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(BMDB_TEST)/testUpgrade.c -o testUpgrade.o $(INCLUDE_DIRS)

testSyncOptions.o: $(SYNC_TEST)/testOptions.c $(SHARED_SRC)/common.h $(SYNC_SRC)/bmsync.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c -D _BSD_SOURCE $(SYNC_TEST)/testOptions.c -o testSyncOptions.o $(INCLUDE_DIRS)

testProcess.o: $(CAPTURE_TEST)/testProcess.c $(SHARED_SRC)/common.h $(CAPTURE_SRC)/capture.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(CAPTURE_TEST)/testProcess.c -o testProcess.o $(INCLUDE_DIRS)

testSql.o: $(CAPTURE_TEST)/testSql.c $(SHARED_SRC)/common.h $(CAPTURE_SRC)/capture.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(CAPTURE_TEST)/testSql.c -o testSql.o $(INCLUDE_DIRS)
	
testCommon.o: $(SHARED_TEST)/testCommon.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(SHARED_TEST)/testCommon.c -o testCommon.o $(INCLUDE_DIRS)

testTime.o: $(SHARED_TEST)/testTime.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(SHARED_TEST)/testTime.c -o testTime.o $(INCLUDE_DIRS)

testDb.o: $(SHARED_TEST)/testDb.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(SHARED_TEST)/testDb.c -o testDb.o $(INCLUDE_DIRS)

testData.o: $(SHARED_TEST)/testData.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(SHARED_TEST)/testData.c -o testData.o $(INCLUDE_DIRS)

testAlert.o: $(SHARED_TEST)/testAlert.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h 
	$(CC) -c $(SHARED_TEST)/testAlert.c -o testAlert.o $(INCLUDE_DIRS)

testClientSummary.o: $(SHARED_CLIENT_TEST)/testClientSummary.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientSummary.c -o testClientSummary.o $(INCLUDE_DIRS)

testClientMonitor.o: $(SHARED_CLIENT_TEST)/testClientMonitor.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientMonitor.c -o testClientMonitor.o $(INCLUDE_DIRS)

testClientDump.o: $(SHARED_CLIENT_TEST)/testClientDump.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientDump.c -o testClientDump.o $(INCLUDE_DIRS)

testClientQuery.o: $(SHARED_CLIENT_TEST)/testClientQuery.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientQuery.c -o testClientQuery.o $(INCLUDE_DIRS)

testClientSync.o: $(SHARED_CLIENT_TEST)/testClientSync.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientSync.c -o testClientSync.o $(INCLUDE_DIRS)

testClientAlert.o: $(SHARED_CLIENT_TEST)/testClientAlert.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientAlert.c -o testClientAlert.o $(INCLUDE_DIRS)

testClientUtil.o: $(SHARED_CLIENT_TEST)/testClientUtil.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(SHARED_CLIENT_TEST)/testClientUtil.c -o testClientUtil.o $(INCLUDE_DIRS)

testHttpRequest.o: $(WEB_TEST)/testHttpRequest.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHttpRequest.c -o testHttpRequest.o $(INCLUDE_DIRS)

testHandleConfig.o: $(WEB_TEST)/testHandleConfig.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleConfig.c -o testHandleConfig.o $(INCLUDE_DIRS)

testHandleFile.o: $(WEB_TEST)/testHandleFile.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleFile.c -o testHandleFile.o $(INCLUDE_DIRS)

testHandleMonitor.o: $(WEB_TEST)/testHandleMonitor.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleMonitor.c -o testHandleMonitor.o $(INCLUDE_DIRS)

testHandleRss.o: $(WEB_TEST)/testHandleRss.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleRss.c -o testHandleRss.o $(INCLUDE_DIRS)

testHandleQuery.o: $(WEB_TEST)/testHandleQuery.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleQuery.c -o testHandleQuery.o $(INCLUDE_DIRS)

testHandleSummary.o: $(WEB_TEST)/testHandleSummary.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleSummary.c -o testHandleSummary.o $(INCLUDE_DIRS)

testHandleSync.o: $(WEB_TEST)/testHandleSync.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleSync.c -o testHandleSync.o $(INCLUDE_DIRS)

testHandleAlert.o: $(WEB_TEST)/testHandleAlert.c $(SHARED_SRC)/common.h $(TEST)/test.h $(TEST)/CuTest.h $(SHARED_CLIENT_SRC)/client.h $(WEB_SRC)/bmws.h
	$(CC) -c $(WEB_TEST)/testHandleAlert.c -o testHandleAlert.o $(INCLUDE_DIRS)

httpTesting.o: $(WEB_SRC)/http.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -D TESTING -c $(WEB_SRC)/http.c -o httpTesting.o $(INCLUDE_DIRS)

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

admin.o: $(BMDB_SRC)/admin.c 
	$(CC) -c $(BMDB_SRC)/admin.c -o admin.o $(INCLUDE_DIRS)

bmdbVersion.o: $(BMDB_SRC)/version.c $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	$(CC) -c $(BMDB_SRC)/version.c -o bmdbVersion.o $(INCLUDE_DIRS)

bmdbHelpText.o: $(BMDB_SRC)/helpText.c $(SHARED_SRC)/common.h
	$(CC) -c $(BMDB_SRC)/helpText.c -o bmdbHelpText.o $(INCLUDE_DIRS)

bmdb.o: $(BMDB_SRC)/bmdb.c $(BMDB_SRC)/bmdb.h $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	$(CC) -c $(BMDB_SRC)/bmdb.c -o bmdb.o $(INCLUDE_DIRS)

#### bmws
bmws: $(OBJS_BMWS)
	$(CC) $(OBJS_BMWS) -o $(EXE_WEB) $(LIBS)

bmws.o: $(WEB_SRC)/bmws.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/bmws.c -o bmws.o $(INCLUDE_DIRS)

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

handleSummary.o: $(WEB_SRC)/handleSummary.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleSummary.c -o handleSummary.o $(INCLUDE_DIRS)

handleExport.o: $(WEB_SRC)/handleExport.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleExport.c -o handleExport.o $(INCLUDE_DIRS)

handleRss.o: $(WEB_SRC)/handleRss.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	$(CC) -c $(WEB_SRC)/handleRss.c -o handleRss.o $(INCLUDE_DIRS)

http.o: $(WEB_SRC)/http.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/http.c -o http.o $(INCLUDE_DIRS)

httpRequest.o: $(WEB_SRC)/httpRequest.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	$(CC) -c $(WEB_SRC)/httpRequest.c -o httpRequest.o $(INCLUDE_DIRS)

sqlite3.o: $(SQLITE_SRC)/sqlite3.c $(SQLITE_SRC)/sqlite3.h
	$(CC) -c $(SQLITE_SRC)/sqlite3.c -o sqlite3.o $(INCLUDE_DIRS)

#### bmcapture standalone
bmcapture: $(OBJS_CAP)
	$(CC) $(OBJS_CAP) -o $(EXE_CAP) $(LIBS)

captureDaemon.o: $(CAPTURE_SRC)/captureDaemon.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/captureDaemon.c -o captureDaemon.o $(INCLUDE_DIRS)

captureService.o: $(CAPTURE_SRC)/captureService.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/captureService.c -o captureService.o $(INCLUDE_DIRS)
	
net.o: $(CAPTURE_SRC)/net.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/net.c -o net.o $(INCLUDE_DIRS)

sql.o: $(CAPTURE_SRC)/sql.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/sql.c -o sql.o $(INCLUDE_DIRS)

process.o: $(CAPTURE_SRC)/process.c $(CAPTURE_SRC)/capture.h $(SHARED_SRC)/common.h
	$(CC) -c $(CAPTURE_SRC)/process.c -o process.o $(INCLUDE_DIRS)

#### bmclient
bmclient: bmclienthelp $(OBJS_CLIENT)
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

monitor.o: $(CLIENT_SRC)/monitor.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
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

clean:
	rm *.o
