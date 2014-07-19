#!/bin/sh

export VERSION=0.7.6

./makeHelpTextC.sh
./makeManPage.sh
./makeLicense.sh

make bmcapture
make bmclient
make bmdb
make bmws
make bmsync

make clean

PACKAGE_DIR=package
sudo rm -rf $PACKAGE_DIR

BIN_DIR=$PACKAGE_DIR/usr/local/bin
mkdir -p $BIN_DIR
mv bmcapture     $BIN_DIR
mv bmclient      $BIN_DIR
mv bmws          $BIN_DIR
mv bmdb          $BIN_DIR
mv bmsync        $BIN_DIR
sudo chown root  $BIN_DIR/*
sudo chgrp admin $BIN_DIR/*

LD_DIR=$PACKAGE_DIR/System/Library/LaunchDaemons
mkdir -p $LD_DIR
cp bitmeter.plist    $LD_DIR
cp bitmeterweb.plist $LD_DIR
sudo chown root      $LD_DIR/*
sudo chgrp wheel     $LD_DIR/*
sudo chmod 644       $LD_DIR/*

MAN_DIR=$PACKAGE_DIR/usr/share/man/man1
mkdir -p $MAN_DIR
mv bmclient.1.gz $MAN_DIR
mv bmdb.1.gz     $MAN_DIR
mv bmsync.1.gz   $MAN_DIR
sudo chown root  $MAN_DIR/*
sudo chgrp admin $MAN_DIR/*

WEB_SRC="../../webserver/web"
WEB_DIR="$PACKAGE_DIR/Library/Application Support/BitMeter/www"
mkdir -p "$WEB_DIR"
cp "$WEB_SRC/index.html" "$WEB_DIR"
cp "$WEB_SRC/rss.xml" "$WEB_DIR"
cp "$WEB_SRC/favicon.ico" "$WEB_DIR"
JS_DIR="$WEB_DIR/js"
mkdir -p "$JS_DIR"
cp "$WEB_SRC"/js/*.js "$JS_DIR"
CSS_DIR="$WEB_DIR/css"
mkdir -p "$CSS_DIR"
cp "$WEB_SRC"/css/*.css "$CSS_DIR"
IMG_DIR="$CSS_DIR/images"
mkdir -p "$IMG_DIR"
cp "$WEB_SRC"/css/images/*.gif "$IMG_DIR"
cp "$WEB_SRC"/css/images/*.png "$IMG_DIR"
sudo chown -R root  "$WEB_DIR"
sudo chgrp -R admin "$WEB_DIR"
MOB_DIR="$WEB_DIR/m"
mkdir -p "$MOB_DIR"
cp "$WEB_SRC"/m/*.xml "$MOB_DIR"
MOB_JS_DIR="$WEB_DIR/m/js"
mkdir -p "$MOB_JS_DIR"
cp "$WEB_SRC"/m/js/*.js "$MOB_JS_DIR"
MOB_CSS_DIR="$WEB_DIR/m/css"
mkdir -p "$MOB_CSS_DIR"
cp "$WEB_SRC"/m/css/*.css "$MOB_CSS_DIR"

APP_DIR="$PACKAGE_DIR/Library/Application Support/BitMeter"
mkdir -p "$APP_DIR"
sudo chown root  "$APP_DIR"
sudo chgrp admin "$APP_DIR"
sudo chmod 777 "$APP_DIR"
DB_FILE="$APP_DIR/bitmeter.db.new"
cp "../bitmeter.db" "$DB_FILE"
sudo chown root  "$DB_FILE"
sudo chgrp admin "$DB_FILE"
sudo chmod 666   "$DB_FILE"
cp bmremove.sh "$APP_DIR"
sudo chown root  "$APP_DIR/bmremove.sh"
sudo chgrp admin "$APP_DIR/bmremove.sh"
sudo chmod 775   "$APP_DIR/bmremove.sh"