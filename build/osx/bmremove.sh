if [ "${UID}" != "0" ]; then
exec /usr/bin/sudo "${0}";
exit
fi

echo This script will completely remove BitMeter OS and all its data from your Mac. 
echo Are you sure you want to continue [y/n]?
read ANSWER
if [ "$ANSWER" != "y" ]; then
echo BitMeter OS has not been removed.
exit 0
fi

launchctl unload /System/Library/LaunchDaemons/bitmeter.plist
launchctl unload /System/Library/LaunchDaemons/bitmeterweb.plist
killall bmclient 2>/dev/null 
killall bmdb 2>/dev/null
killall bmsync 2>/dev/null

rm /usr/local/bin/bmclient
rm /usr/local/bin/bmcapture
rm /usr/local/bin/bmws
rm /usr/local/bin/bmdb
rm /usr/local/bin/bmsync

rm -rf "/Library/Application Support/BitMeter"
rm -f /Library/Logs/bitmeter.log
rm /System/Library/LaunchDaemons/bitmeter.plist
rm /System/Library/LaunchDaemons/bitmeterweb.plist
rm /usr/share/man/man1/bmclient.1.gz
rm /usr/share/man/man1/bmdb.1.gz
rm /usr/share/man/man1/bmsync.1.gz

echo BitMeter OS has been removed.

