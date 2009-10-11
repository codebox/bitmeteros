#include <time.h>
#define QUERY_GROUP_HOURS  1
#define QUERY_GROUP_DAYS   2
#define QUERY_GROUP_MONTHS 3
#define QUERY_GROUP_YEARS  4
#define QUERY_GROUP_TOTAL  5

struct Summary{
	struct Data* today;
	struct Data* month;
	struct Data* year;
	struct Data* total;
	time_t tsMin;
	time_t tsMax;
};

struct ValueBounds{
	unsigned long long min;
	unsigned long long max;
};

struct Data* getMonitorValues(int ts);
struct Summary getSummaryValues();
struct ValueBounds* calcTsBounds();
struct Data* calcMaxValues();
struct Data* getQueryValues();
void getDumpValues(void (*callback)(struct Data*));
