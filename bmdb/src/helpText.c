/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2011 Rob Dawson
 *
 * Licensed under the GNU General Public License
 * http://www.gnu.org/licenses/gpl.txt
 *
 * This file is part of BitMeterOS.
 *
 * BitMeterOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BitMeterOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BitMeterOS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
char* helpTxt=
"bmdb [<action>]" EOL
" " EOL
"Performs various admin and configuration operations on the BitMeterOS database. Running the utility without an 'action' argument will list the available actions. The following actions are defined:" EOL
" " EOL
"showconfig" EOL
"Displays all the configuration parameters currently stored in the database" EOL
" " EOL
"setconfig <name> <value>" EOL
"Updates the named configuration parameter with the specified value. If no parameter currently exists with a matching name then a new one is added." EOL
" " EOL
"rmconfig <name>" EOL
"Removes the configuration parameter with the specified name from the database" EOL
" " EOL
"vac" EOL
"Performs a vacuum operation on the database, freeing any unused space still occupied by the file" EOL
" " EOL
"version" EOL
"Displays various pieces of versioning information" EOL
" " EOL
"upgrade <level>" EOL
"Performs a database upgrade to the specified level. This operation is performed automatically by the BitMeterOS installer during software upgrades, and should not have to be executed manually." EOL
" " EOL
"weblocal" EOL
"Disables remote access to the BitMeterOS web interface." EOL
" " EOL
"webremote" EOL
"Enables non-administrative remote access to the BitMeter web interface. This allows remote users to view the web interface, but blocks access to any features which can alter the contents of the BitMeter database, or start/stop any BitMeter processes." EOL
" " EOL
"webremoteadmin" EOL
"Enables administrative remote access to the BitMeter web interface. Remote users have the same levels of privilege as users accessing the interface locally." EOL
" " EOL
"webstop" EOL
"Stops the BitMeterOS web server process, removing all access to the web interface." EOL
" " EOL
"webstart" EOL
"Start the BitMeterOS web server process, restoring access to the web interface." EOL
" " EOL
"capstop" EOL
"Stop the BitMeterOS data capture process, so that no bandwidth data will be logged to the database." EOL
" " EOL
"capstart" EOL
"Starts the BitMeterOS data capture process, so that bandwidth data will be logged to the database." EOL
" " EOL
"purge" EOL
"Deletes all BitMeterOS bandwidth data from the database." EOL
" " EOL
"help" EOL
"Displays this help page." EOL
" " EOL
"Email: rob@codebox.org.uk" EOL
"Web:   http://codebox.org.uk/bitmeteros" EOL
" " EOL
;
