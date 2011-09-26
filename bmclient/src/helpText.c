#include "common.h" 
char* helpTxt= 
"Provides command-line access to the BitMeterOS database" EOL
" " EOL
"Usage:" EOL
"    bmclient -h|-v|-m<mode> <mode specific options>" EOL
" " EOL
"Dump all data using '-md' or '-m dump':" EOL
"    bmclient -m dump [-u <units>] [-o <output format>]" EOL
" " EOL
"    Values allowed for 'units' are:" EOL
"        b - all values displayed in bytes (default)" EOL
"        a - values displayed with abbreviated units" EOL
"        f - values displayed with full units" EOL
" " EOL
"    Values allowed for 'output format' are:" EOL
"        c - CSV format, suitable for import into a spreadsheet" EOL
"        f - fixed width format, easier to read" EOL
" " EOL
"Display a database summary using '-ms' or '-m summary':" EOL
"    bmclient -m summary" EOL
" " EOL
"Monitor current usage using '-mm' or '-m monitor'. This produces a continuously updating display of the current network usage:" EOL
"    bmclient -m monitor -f <filter name> [-t <display type>] [-w <max bar width>] [-x <max bar value>]" EOL
" " EOL
"    The 'filter name' argument is mandatory, and must match the name of an existing packet filter (use the command 'bmdb showfilters' to display a list of the available filters)" EOL
" " EOL
"    Values allowed for 'display type' are:" EOL
"        n - numeric, values are displayed numerically (default)" EOL
"        b - bar graph, values are displayed as bars of varying width (see subsequent arguments)" EOL
" " EOL
"    The 'max bar width' argument is ignored unless a display type of 'b' (bar graph) is selected." EOL
"        A numeric value must be supplied with this option, to indicate the width (in characters) of the longest bar that will be displayed (default is 69 to allow each line, including the numeric output, to fit on an 80 character display)" EOL
" " EOL
"    The 'max bar value' argument is ignored unless a display type of 'b' (bar graph) is selected." EOL
"        A numeric value must be supplied with this option, to indicate the maximum value (in bytes) that can be displayed before the bar reaches the maximum width specified by the 'max bar width' argument (default value is 100000)" EOL
" " EOL
"Query the database using '-mq' or '-m query':" EOL
"    bmclient -m query -f <filter name> -r <range> [-g <grouping>] [-u <units>]" EOL
" " EOL
"    The 'filter name' argument is mandatory, and must match the name of an existing packet filter (use the command 'bmdb showfilters' to display a list of the available filters)" EOL
"    The 'range' argument specifies the date/time range for which you want to see information. The range can consist of either 1 or 2 date components, if 2 date components are present then they must be separated by a hyphen. Each date component can be in 1 of 4 different formats, shown below:" EOL
"        yyyy        - a 4 digit year" EOL
"        yyyymm      - a year followed by a 2 digit month" EOL
"        yyyymmdd    - a year and month followed by a 2 digit day" EOL
"        yyyymmddhh  - a year, month and day, followed by a 2 digit hour" EOL
" " EOL
"    Some examples:" EOL
"        Show all data recorded during the year 2009:" EOL
"            bmclient -m query -f dl -r2009" EOL
"        Show all data recorded between April 2008 and June 2008, inclusive" EOL
"            bmclient -m query -f dl -r200804-200806" EOL
"        Show all data recorded between 4AM and 5AM on 1st January 2002" EOL
"            bmclient -m query -f dl -r2002010104" EOL
"                OR" EOL
"            bmclient -m query -f dl -r2002010104-2002010105" EOL
" " EOL
"    The 'grouping' argument specifies if/how the results of the query should be grouped when they are displayed:" EOL
"        t - do not group results, display overall total only (default)" EOL
"        h - group results by hour" EOL
"        d - group results by day" EOL
"        m - group results by month" EOL
"        y - group results by year" EOL
" " EOL
"    The 'units' argument is explained above, in the 'dump' section." EOL
" " EOL
"Display version information" EOL
"    bmclient -v" EOL
" " EOL
"Display this help" EOL
"    bmclient -h" EOL
" " EOL
"Email: rob@codebox.org.uk" EOL
"Web:   http://codebox.org.uk/bitmeteros" EOL
" " EOL
; 
