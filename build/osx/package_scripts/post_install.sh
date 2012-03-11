#!/bin/sh

DB_DIR="/Library/Application Support/BitMeter"
DB_FILE=$DB_DIR/bitmeter.db

if [ -f "$DB_FILE" ]; then
    rm "$DB_FILE.new"
    /usr/local/bin/bmdb upgrade 8
else
    mv "$DB_FILE.new" "$DB_FILE"
fi

sudo chmod 644 /System/Library/LaunchDaemons/bitmeter.plist
sudo chmod 644 /System/Library/LaunchDaemons/bitmeterweb.plist

sudo launchctl load /System/Library/LaunchDaemons/bitmeter.plist
sudo launchctl load /System/Library/LaunchDaemons/bitmeterweb.plist

sudo chmod 777 "$DB_DIR"

exit 0