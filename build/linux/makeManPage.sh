cp ../../bmclient/man.txt bmclient.1
cp ../../bmdb/man.txt bmdb.1
cp ../../bmsync/man.txt bmsync.1
MAN_DIR=debian/usr/share/man/man1
mkdir -p $MAN_DIR
gzip -c9 bmclient.1 > $MAN_DIR/bmclient.1.gz
gzip -c9 bmdb.1 >     $MAN_DIR/bmdb.1.gz
gzip -c9 bmsync.1 >   $MAN_DIR/bmsync.1.gz
rm bmclient.1
rm bmdb.1
rm bmsync.1
