#!/bin/sh
launchctl unload /System/Library/LaunchDaemons/bitmeter.plist
launchctl unload /System/Library/LaunchDaemons/bitmeterweb.plist
exit 0