/*
 * BitMeterOS
 * http://codebox.org.uk/bitmeterOS
 *
 * Copyright (c) 2010 Rob Dawson
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
"" EOL
"Performs various admin and configuration operations on the BitMeterOS database. Running the utility without an 'action' argument will list the available actions. The following actions are defined:" EOL
"" EOL
"showconfig" EOL
"Displays all the configuration parameters currently stored in the database" EOL
"" EOL
"setconfig <name> <value>" EOL
"Updates the named configuration parameter with the specified value. If no parameter currently exists with a matching name then a new one is added." EOL
"" EOL
"vac" EOL
"Performs a vacuum operation on the database, freeing any unused space still occupied by the file" EOL
"" EOL
"version" EOL
"Displays various pieces of versioning information" EOL
"" EOL
"upgrade <level>" EOL
"Performs a database upgrade to the specified level. This operation is performed automatically by the BitMeterOS installer during software upgrades, and should not have to be executed manually." EOL
"" EOL
"webremote" EOL
"Enables remote access to the BitMeterOS web interface." EOL
"" EOL
"weblocal" EOL
"Disables remote access to the BitMeterOS web interface." EOL
"" EOL
"help" EOL
"Displays this help page." EOL
"" EOL
"Email: rob@codebox.org.uk" EOL
"Web:   http://codebox.org.uk/bitmeterOs" EOL
"";
