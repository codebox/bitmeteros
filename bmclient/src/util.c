#include <stdio.h>
#include "bmclient.h"
#include "client.h"
#include "common.h"

extern struct Prefs prefs;

void formatAmounts(const BW_INT dl, const BW_INT ul, char* dlTxt, char *ulTxt, int units){
	switch (units) {
		case PREF_UNITS_BYTES:
			sprintf(dlTxt, "%llu", dl);
			sprintf(ulTxt, "%llu", ul);
			break;

		case PREF_UNITS_ABBREV:
		case PREF_UNITS_FULL:
			formatAmount(dl, 0, (prefs.units == PREF_UNITS_ABBREV), dlTxt);
			formatAmount(ul, 0, (prefs.units == PREF_UNITS_ABBREV), ulTxt);
			break;

		default:
			//TODO error
			break;
	}
}
