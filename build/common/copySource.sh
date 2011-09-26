#!/bin/sh

FROM_DIR="/Users/rob/code/eclipseWS/BitMeter OS"
TO_BUILD_DIR="/Users/rob/code/eclipseWS/BitMeter OS/build/common/src"
TO_SF_DIR="/Users/rob/code/eclipseWS/BitMeter OS Sourceforge"
HEADER_FILE="$TO_BUILD_DIR/../header.txt"
VERSION=v0.7.0
APP=BitMeterOS

copy_file()
{
	SRC=$1
	DEST=$2
	cat "$HEADER_FILE" | sed "s/#VERSION#/$VERSION/g" | sed "s/#APP#/$APP/g" > $DEST
	cat $SRC >> $DEST
}

copy_dir()
{
	SUB_DIR=$1
	echo copy_dir : $SUB_DIR
	mkdir -p "$TO_BUILD_DIR/$SUB_DIR"
	rm "$TO_SF_DIR/$SUB_DIR/*.c" 2>/dev/null
	rm "$TO_SF_DIR/$SUB_DIR/*.h" 2>/dev/null

	cd "$FROM_DIR/$SUB_DIR"
	for f in $(ls *.c *.h 2>/dev/null); do
		copy_file $f "$TO_SF_DIR/$SUB_DIR/$f"
	done
	for f in $(ls *.txt *.html *.xml *.css *.js *.gif *.png 2>/dev/null); do
		cp $f "$TO_SF_DIR/$SUB_DIR/$f"
	done	
}

rm -rf "$TO_BUILD_DIR"
mkdir "$TO_BUILD_DIR"

copy_dir bmclient/src
copy_dir bmclient/test
copy_dir bmdb
copy_dir bmdb/src
copy_dir bmdb/test
copy_dir bmsync
copy_dir bmsync/src
copy_dir bmsync/test
copy_dir capture/src
copy_dir capture/test
copy_dir shared/src
copy_dir shared/test
copy_dir shared_client/src
copy_dir shared_client/test
copy_dir webserver/src
copy_dir webserver/test
copy_dir webserver/web
copy_dir webserver/web/css
copy_dir webserver/web/css/images
copy_dir webserver/web/js
copy_dir webserver/web/m
copy_dir webserver/web/m/js
copy_dir webserver/web/m/css
