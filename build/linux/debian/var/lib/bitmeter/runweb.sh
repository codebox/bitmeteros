#!/bin/sh
WEB_URL=http://localhost:2605/index.html
BROWSER=$(which gnome-www-browser || which x-www-browser || which firefox || which www-browser)
if [ "$BROWSER" = "" ]; then
	echo For help visit $WEB_URL
else
	nohup $BROWSER $WEB_URL &
fi

