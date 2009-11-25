CLIENT_SRC=../../bmclient/src
DAEMON_SRC=../../capture/src
SHARED_SRC=../../shared/src
SHARED_CLIENT_SRC=../../shared_client/src
WEB_SRC=../../webserver/src
BMDB_SRC=../../bmdb/src

bmdb: bmdb.o bmdbConfig.o upgrade.o bmdbVersion.o log.o db.o common.o platform.o data.o
	gcc bmdb.o bmdbConfig.o upgrade.o bmdbVersion.o log.o db.o common.o platform.o data.o -o bmdb -lsqlite3 -lm

bmdbConfig.o: $(BMDB_SRC)/config.c $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	gcc -c $(BMDB_SRC)/config.c -o bmdbConfig.o -I$(SHARED_SRC) -I$(BMDB_SRC)

upgrade.o: $(BMDB_SRC)/upgrade.c $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	gcc -c $(BMDB_SRC)/upgrade.c -o upgrade.o -I$(SHARED_SRC) -I$(BMDB_SRC)

bmdbVersion.o: $(BMDB_SRC)/version.c $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	gcc -c $(BMDB_SRC)/version.c -o bmdbVersion.o -I$(SHARED_SRC) -I$(BMDB_SRC)

bmdb.o: $(BMDB_SRC)/bmdb.c $(BMDB_SRC)/bmdb.h $(SHARED_SRC)/common.h $(BMDB_SRC)/bmdb.h
	gcc -c $(BMDB_SRC)/bmdb.c -o bmdb.o -I $(SHARED_SRC) -I$(BMDB_SRC)

bmws: bmws.o handleConfig.o handleFile.o handleMonitor.o handleQuery.o handleSummary.o http.o httpRequest.o data.o common.o log.o platform.o db.o clientMonitor.o clientSummary.o clientQuery.o clientUtil.o time.o
	gcc bmws.o handleConfig.o handleFile.o handleMonitor.o handleQuery.o handleSummary.o http.o httpRequest.o data.o common.o log.o platform.o db.o clientMonitor.o clientSummary.o clientQuery.o clientUtil.o time.o -o bmws -lsqlite3 -lm

bmws.o: $(WEB_SRC)/bmws.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	gcc -c $(WEB_SRC)/bmws.c -o bmws.o -I$(SHARED_SRC) 

handleConfig.o: $(WEB_SRC)/handleConfig.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(WEB_SRC)/handleConfig.c -o handleConfig.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

handleFile.o: $(WEB_SRC)/handleFile.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	gcc -c $(WEB_SRC)/handleFile.c -o handleFile.o -I$(SHARED_SRC)

handleMonitor.o: $(WEB_SRC)/handleMonitor.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(WEB_SRC)/handleMonitor.c -o handleMonitor.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

handleQuery.o: $(WEB_SRC)/handleQuery.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(WEB_SRC)/handleQuery.c -o handleQuery.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

handleSummary.o: $(WEB_SRC)/handleSummary.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(WEB_SRC)/handleSummary.c -o handleSummary.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

http.o: $(WEB_SRC)/http.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	gcc -c $(WEB_SRC)/http.c -o http.o -I$(SHARED_SRC)

httpRequest.o: $(WEB_SRC)/httpRequest.c $(WEB_SRC)/bmws.h $(SHARED_SRC)/common.h
	gcc -c $(WEB_SRC)/httpRequest.c -o httpRequest.o -I$(SHARED_SRC)

bmcapture: captureDaemon.o net.o sql.o time.o process.o common.o db.o log.o platform.o data.o
	gcc captureDaemon.o net.o sql.o time.o process.o common.o db.o log.o platform.o data.o -o bmcapture -lsqlite3 -lm

captureDaemon.o: $(DAEMON_SRC)/captureDaemon.c $(DAEMON_SRC)/capture.h $(SHARED_SRC)/common.h
	gcc -c $(DAEMON_SRC)/captureDaemon.c -o captureDaemon.o -I$(SHARED_SRC)

net.o: $(DAEMON_SRC)/net.c $(DAEMON_SRC)/capture.h $(SHARED_SRC)/common.h
	gcc -c $(DAEMON_SRC)/net.c -o net.o -I$(SHARED_SRC)

sql.o: $(DAEMON_SRC)/sql.c $(DAEMON_SRC)/capture.h $(SHARED_SRC)/common.h
	gcc -c $(DAEMON_SRC)/sql.c -o sql.o -I$(SHARED_SRC)

process.o: $(DAEMON_SRC)/process.c $(DAEMON_SRC)/capture.h $(SHARED_SRC)/common.h
	gcc -c $(DAEMON_SRC)/process.c -o process.o -I$(SHARED_SRC)

bmclient: bmclient.o options.o monitor.o summary.o query.o dump.o help.o helpText.o time.o common.o db.o log.o platform.o clientDump.o clientMonitor.o clientSummary.o clientQuery.o clientUtil.o data.o
	gcc bmclient.o options.o monitor.o summary.o query.o dump.o help.o helpText.o time.o common.o db.o log.o platform.o clientDump.o clientMonitor.o clientSummary.o clientQuery.o clientUtil.o data.o -o bmclient -lsqlite3 -lm

bmclient.o: $(CLIENT_SRC)/bmclient.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(CLIENT_SRC)/bmclient.c -o bmclient.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

clientDump.o: $(SHARED_CLIENT_SRC)/clientDump.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(SHARED_CLIENT_SRC)/clientDump.c -o clientDump.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

clientMonitor.o: $(SHARED_CLIENT_SRC)/clientMonitor.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(SHARED_CLIENT_SRC)/clientMonitor.c -o clientMonitor.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC) 

clientSummary.o: $(SHARED_CLIENT_SRC)/clientSummary.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(SHARED_CLIENT_SRC)/clientSummary.c -o clientSummary.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

clientQuery.o: $(SHARED_CLIENT_SRC)/clientQuery.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(SHARED_CLIENT_SRC)/clientQuery.c -o clientQuery.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

clientUtil.o: $(SHARED_CLIENT_SRC)/clientUtil.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(SHARED_CLIENT_SRC)/clientUtil.c -o clientUtil.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)
 
options.o: $(CLIENT_SRC)/options.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(CLIENT_SRC)/options.c -o options.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

monitor.o: $(CLIENT_SRC)/monitor.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/platform.c $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(CLIENT_SRC)/monitor.c -o monitor.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

summary.o: $(CLIENT_SRC)/summary.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(CLIENT_SRC)/summary.c -o summary.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

query.o: $(CLIENT_SRC)/query.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(CLIENT_SRC)/query.c -o query.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

dump.o: $(CLIENT_SRC)/dump.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(CLIENT_SRC)/dump.c -o dump.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

help.o: $(CLIENT_SRC)/help.c $(CLIENT_SRC)/bmclient.h $(SHARED_SRC)/common.h $(SHARED_CLIENT_SRC)/client.h
	gcc -c $(CLIENT_SRC)/help.c -o help.o -I$(SHARED_SRC) -I$(SHARED_CLIENT_SRC)

helpText.o: $(CLIENT_SRC)/helpText.c $(SHARED_SRC)/common.h
	gcc -c $(CLIENT_SRC)/helpText.c -o helpText.o -I$(SHARED_SRC)

time.o: $(SHARED_SRC)/time.c $(SHARED_SRC)/common.h
	gcc -c $(SHARED_SRC)/time.c -o time.o

common.o: $(SHARED_SRC)/common.c $(SHARED_SRC)/common.h
	gcc -c $(SHARED_SRC)/common.c -o common.o -I$(SHARED_SRC)

db.o: $(SHARED_SRC)/db.c $(SHARED_SRC)/common.h
	gcc -c $(SHARED_SRC)/db.c -o db.o -I$(SHARED_SRC)

log.o: $(SHARED_SRC)/log.c $(SHARED_SRC)/common.h
	gcc -c $(SHARED_SRC)/log.c -o log.o -I$(SHARED_SRC)

platform.o: $(SHARED_SRC)/platform.c $(SHARED_SRC)/common.h
	gcc -c $(SHARED_SRC)/platform.c -o platform.o -I$(SHARED_SRC)

data.o: $(SHARED_SRC)/data.c $(SHARED_SRC)/common.h
	gcc -c $(SHARED_SRC)/data.c -o data.o -I$(SHARED_SRC)

clean:
	rm *.o
