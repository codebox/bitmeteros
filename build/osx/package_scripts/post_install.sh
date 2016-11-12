#!/bin/sh

DB_DIR="/Library/Application Support/BitMeter"
DB_FILE=$DB_DIR/bitmeter.db

if [ -f "$DB_FILE" ]; then
    rm "$DB_FILE.new"
    /usr/local/bin/bmdb upgrade 7
else
    mv "$DB_FILE.new" "$DB_FILE"
fi

sudo chmod 644 /Library/LaunchDaemons/bitmeter.plist
sudo chmod 644 /Library/LaunchDaemons/bitmeterweb.plist

sudo launchctl load /Library/LaunchDaemons/bitmeter.plist
sudo launchctl load /Library/LaunchDaemons/bitmeterweb.plist

sudo chmod 777 "$DB_DIR"

open "http://codebox.org.uk/bitmeteros/installed?version=0.7.6.1&platform=osx" &

exit 0