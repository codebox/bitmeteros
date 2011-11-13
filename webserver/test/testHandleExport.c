#include <stdlib.h> 
#include <stdarg.h>
#include <stddef.h> 
#include <setjmp.h> 
#include <cmockery.h> 
#include "test.h"
#include <stdio.h>
#include "common.h"
#include "test.h"
#include "client.h"
#include "bmws.h"

/*
Contains unit tests for the handleExport module.
*/
void testHandleExport(void** state){
    addFilterRow(1, "Filter One", "f1", "ex1", NULL);
    addFilterRow(2, "Filter Two", "f2", "ex2", "host");
    addFilterRow(3, "Filter Three", "f3", "ex3", NULL);
    
    addDbRow(makeTs("2011-01-01 12:00:00"), 3600, 1, 1);
    addDbRow(makeTs("2011-01-01 13:00:00"),   60, 1, 2);
    addDbRow(makeTs("2011-01-01 14:00:00"),    1, 1, 3);
    addDbRow(makeTs("2011-01-01 14:00:00"),    1, 1, 4); // filter does not exist

    expect_string(mockWriteHeadersOk, contentType, "text/csv");
    expect_value(mockWriteHeadersOk, endHeaders, FALSE);

    expect_string(mockWriteHeader, name,  "Content-Disposition");
    expect_string(mockWriteHeader, value, "attachment;filename=bitmeterOsExport.csv");

    expect_value(mockWriteEndOfHeaders, fd, 0);

    expect_string(mockWriteText, txt, "2011-01-01,13:59:59,14:00:00,1,f3\n");
    expect_string(mockWriteText, txt, "2011-01-01,13:59:59,14:00:00,1,\n");
    expect_string(mockWriteText, txt, "2011-01-01,12:59:00,13:00:00,1,f2\n");
    expect_string(mockWriteText, txt, "2011-01-01,11:00:00,12:00:00,1,f1\n");
    
    
    processExportRequest(NULL, NULL); // request is ignored
    
    freeStmtList();
}