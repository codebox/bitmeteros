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
"showfilters" EOL
"Lists the names, and filter expressions, of all the current packet filters." EOL
" " EOL
"addfilter <name> <description> <filter expression>" EOL
"Adds a new packet filter, with the specified name. The filter expression argument must be valid TcpDump filter, but may include one or more of the following BitMeter-specific values:" EOL
"    {lan}     expands to an expression that specifies the IP address ranges reserved for private networks" EOL
"    {adapter} replaced with an expression containing the IP address/es of the network adapter to which the filter is applied" EOL
" " EOL
"for example:" EOL
"    bmdb addfilter webdl \"Web Downloads\" \"dst host {adapter} and src port 80\"" EOL
" " EOL
"rmfilter <filter name>" EOL
"Deletes the named packet filter, and also removes all data captured using that filter from the main data table." EOL
" " EOL
"help" EOL
"Displays this help page." EOL
" " EOL
"Email: rob@codebox.org.uk" EOL
"Web:   http://codebox.org.uk/bitmeteros" EOL
" " EOL
; 
