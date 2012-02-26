cp ../../bmclient/man.txt bmclient.1
cp ../../bmdb/man.txt bmdb.1
cp ../../bmsync/man.txt bmsync.1
gzip -c9 bmclient.1 > bmclient.1.gz
gzip -c9 bmdb.1 > bmdb.1.gz
gzip -c9 bmsync.1 > bmsync.1.gz
rm bmclient.1
rm bmdb.1
rm bmsync.1
