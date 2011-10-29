#include <test.h> 
#include <stdarg.h> 
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "capture.h"

/*
Contains unit tests for the sql module.
*/

void setup();

static int getDataRowCount(){
	return getRowCount("SELECT * FROM data");	
}
void testUpdateDbNull(void** state) {
 // Check that nothing is added to the d/b if we pass in a NULL list
    int rowsBefore = getDataRowCount();
    updateDb(1, NULL);
    int rowsAfter = getDataRowCount();
    assert_int_equal(rowsBefore, rowsAfter);
}

void testUpdateDbMultiple(void** state) {
 // Check that the correct number of rows are added when we pass in multiple structs
    int rowsBefore = getDataRowCount();
    struct Data data3 = { 3, 3, 5, 1, NULL};
    struct Data data2 = { 2, 2, 5, 1, &data3};
    struct Data data1 = { 1, 1, 5, 1, &data2};
    
    updateDb(1,&data1);
    int rowsAfter = getDataRowCount();
    assert_int_equal(rowsBefore + 3, rowsAfter);
    freeStmtList();
}

void testGetNextCompressTime(void** state){
 // Check that the next d/b compress interval is calculated correctly, based on the current time
    int now = 1234;
    setTime(now);
    assert_int_equal(now + 3600, getNextCompressTime());
}

void testCompressSec1Filter(void** state){
 // Check that database second->minute compression is performed correctly for a single filter
    int now = 7200;
    setTime(now);
    addDbRow(3601, 1,  1, 1);
    addDbRow(3600, 1,  2, 1);
    addDbRow(3599, 1,  4, 1);
    addDbRow(3598, 1,  8, 1);
    addDbRow(3597, 1, 16, 1);

    compressDb();

    struct Data row2 = {3600, 60, 30, 1, NULL};
    struct Data row1 = {3601, 1,   1, 1, &row2};
    
    checkTableContents(&row1);
    freeStmtList();
}

void testCompressSecMultiFilters(void** state){
 // Check that database second->minute compression is performed correctly for multiple filter
    int now = 7200;
    setTime(now);
    emptyDb();
    addDbRow(3601, 1,   1, 1);
    addDbRow(3601, 1,   2, 2);
    addDbRow(3601, 1,   4, 3);
    addDbRow(3600, 1,   8, 1);
    addDbRow(3600, 1,  16, 2);
    addDbRow(3600, 1,  32, 3);
    addDbRow(3599, 1,  64, 1);
    addDbRow(3598, 1, 128, 2);
    addDbRow(3597, 1, 256, 3);
    compressDb();

    struct Data row6 = {3600, 60,  288, 3, NULL};
    struct Data row5 = {3600, 60,  144, 2, &row6};
    struct Data row4 = {3600, 60,   72, 1, &row5};
    struct Data row3 = {3601, 1,     4, 3, &row4};
    struct Data row2 = {3601, 1,     2, 2, &row3};
	struct Data row1 = {3601, 1,     1, 1, &row2};        
	
    checkTableContents(&row1);
    freeStmtList();
}

void testCompressSecMultiIterations(void** state){
 /* Check that database second->minute compression is performed correctly for multiple filters where
    the data is spread out over time such that multiple compressed rows for each
    filter will result. */
    int now = 7200;
    setTime(now);
    emptyDb();
    addDbRow(3601, 1,     1, 0);
    addDbRow(3601, 1,     2, 1);
    addDbRow(3601, 1,     4, 2);
    addDbRow(3600, 1,     8, 0);
    addDbRow(3600, 1,    16, 1);
    addDbRow(3600, 1,    32, 2);
    addDbRow(3599, 1,    64, 0);
    addDbRow(3598, 1,   128, 1);
    addDbRow(3597, 1,   256, 2);
    addDbRow(3540, 1,   512, 0);
    addDbRow(3540, 1,  1024, 1);
    addDbRow(3540, 1,  2048, 2);
    addDbRow(3539, 1,  4096, 0);
    addDbRow(3538, 1,  8192, 1);
    addDbRow(3537, 1, 16384, 2);
    compressDb();
    
    struct Data row9 = {3540, 60,18432, 2, NULL};
    struct Data row8 = {3540, 60, 9216, 1, &row9};
    struct Data row7 = {3540, 60, 4608, 0, &row8};
    struct Data row6 = {3600, 60,  288, 2, &row7};
    struct Data row5 = {3600, 60,  144, 1, &row6};
    struct Data row4 = {3600, 60,   72, 0, &row5};
    struct Data row3 = {3601, 1,     4, 2, &row4};
    struct Data row2 = {3601, 1,     2, 1, &row3};
    struct Data row1 = {3601, 1,     1, 0, &row2};
    
    checkTableContents(&row1);
    freeStmtList();
}

void testCompressMin1Filter(void** state){
 // Check that database minute->hour compression is performed correctly for a single filter
    int now = 86400 + 3600;
    setTime(now);
    emptyDb();
    addDbRow(3601, 60,  1, 1);
    addDbRow(3600, 60,  2, 1);
    addDbRow(3599, 60,  4, 1);
    addDbRow(3598, 60,  8, 1);
    addDbRow(3597, 60, 16, 1);
    compressDb();
    
    struct Data row2 = {3600, 3600, 30, 1, NULL};
    struct Data row1 = {3601,   60,  1, 1, &row2};

    checkTableContents(&row1);
    freeStmtList();
}

void testCompressMinMultiFilters(void** state){
 // Check that database minute->hour compression is performed correctly for multiple adapters
    int now = 86400 + 3600;
    setTime(now);
    emptyDb();
    addDbRow(3601, 60,   1, 0);
    addDbRow(3601, 60,   2, 1);
    addDbRow(3601, 60,   4, 2);
    addDbRow(3600, 60,   8, 0);
    addDbRow(3600, 60,  16, 1);
    addDbRow(3600, 60,  32, 2);
    addDbRow(3599, 60,  64, 0);
    addDbRow(3598, 60, 128, 1);
    addDbRow(3597, 60, 256, 2);
    compressDb();
    
    struct Data row6 = {3600, 3600, 288, 2, NULL};
    struct Data row5 = {3600, 3600, 144, 1, &row6};
    struct Data row4 = {3600, 3600,  72, 0, &row5};
    struct Data row3 = {3601,   60,   4, 2, &row4};
    struct Data row2 = {3601,   60,   2, 1, &row3};
    struct Data row1 = {3601,   60,   1, 0, &row2};
    
    checkTableContents(&row1); 
    freeStmtList();
}
