#!/bin/sh

export VERSION=0.7.5

./makeHelpTextC.sh
./makeManPage.sh

WORK_DIR=./work
WEB_DIR=../../webserver/web
DB_FILE=../bitmeter.db
if [ $(arch) = "x86_64" ]; then
    IS_64=1
else 
    IS_64=0
fi

make bmcapture
make bmclient
make bmws
make bmdb
make bmsync

mkdir -p ./debian/usr/bin/
mv ./bmcapture ./debian/usr/bin/
mv ./bmclient  ./debian/usr/bin/
mv ./bmws      ./debian/usr/bin/
mv ./bmdb      ./debian/usr/bin/
mv ./bmsync    ./debian/usr/bin/

mkdir -p ./debian/var/www/bitmeter/css/images/
mkdir -p ./debian/var/www/bitmeter/js/
mkdir -p ./debian/var/www/bitmeter/m/js/
mkdir -p ./debian/var/www/bitmeter/m/css/
cp $WEB_DIR/index.html       ./debian/var/www/bitmeter/
cp $WEB_DIR/rss.xml          ./debian/var/www/bitmeter/
cp $WEB_DIR/favicon.ico      ./debian/var/www/bitmeter/
cp $WEB_DIR/css/*.css        ./debian/var/www/bitmeter/css/
cp $WEB_DIR/css/images/*.png ./debian/var/www/bitmeter/css/images/
cp $WEB_DIR/css/images/*.gif ./debian/var/www/bitmeter/css/images/
cp $WEB_DIR/js/*.js          ./debian/var/www/bitmeter/js/
cp $WEB_DIR/m/*.xml          ./debian/var/www/bitmeter/m/
cp $WEB_DIR/m/js/*.js        ./debian/var/www/bitmeter/m/js/
cp $WEB_DIR/m/css/*.css      ./debian/var/www/bitmeter/m/css/

cp $DB_FILE ./debian/var/lib/bitmeter/bitmeter.db.new

if [ $IS_64 = 1 ]; then
    cp ./control_64 ./debian/DEBIAN/control
else
    cp ./control_32 ./debian/DEBIAN/control
fi

if [ -d $WORK_DIR ]; then
    rm -rf $WORK_DIR
fi

mkdir $WORK_DIR
tar -cpf - --exclude=CVS --exclude=.cvsignore debian| sh -c "cd $WORK_DIR; tar -xpf -"

dpkg-deb --build $WORK_DIR/debian

if [ $IS_64 = 1 ]; then
    mv $WORK_DIR/debian.deb bitmeteros_$VERSION-amd64.deb
else
    mv $WORK_DIR/debian.deb bitmeteros_$VERSION-i386.deb
fi

make clean
