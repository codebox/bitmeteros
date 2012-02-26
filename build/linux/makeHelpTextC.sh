CLI_SRC=../../bmclient/src
(
echo '#include "common.h"'
echo char* helpTxt=""
cat $CLI_SRC/../help.txt | sed 's/^/"/' | sed 's/$/" EOL/'
echo '"";'
) > $CLI_SRC/helpText.c

SYNC_SRC=../../bmsync/src
(
echo '#include "common.h"'
echo char* helpTxt=""
cat $SYNC_SRC/../help.txt | sed 's/^/"/' | sed 's/$/" EOL/'
echo '"";'
) > $SYNC_SRC/helpText.c

BMDB_SRC=../../bmdb/src
(
echo '#include "common.h"'
echo char* helpTxt=""
cat $BMDB_SRC/../help.txt | sed 's/^/"/' | sed 's/$/" EOL/'
echo '"";'
) > $BMDB_SRC/helpText.c
