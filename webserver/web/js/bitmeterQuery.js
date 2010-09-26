/*global $,BITMETER,window,config*/
/*jslint onevar: true, undef: true, nomen: true, eqeqeq: true, bitwise: true, regexp: true, newcap: true, immed: true, strict: false */

/*
Contains code that handles to Query page
*/
BITMETER.tabShowQuery = function(){
    // Nothing to do here
};

$(function(){
    var queryResultsGridObj, dateFormat, queryDialog, datePickerOpts = { showOn: 'button', buttonImage: '/css/images/calendar.gif' };

    function runQuery(){
        var fd, td, errList=[], df, reqTxt;
        $('#queryErrBox').hide();

     // We do some validation on the date values entered by the user, empty fields don't cause an error
        df = $('#fromDate').datepicker('option', 'dateFormat');
        try{
            fd = $.datepicker.parseDate(df, $('#fromDate').val());
         // Default to 0 (ie as far back in time as possible)
            fd = (fd ? fd.getTime()/1000 : 0);
        } catch (fde) {
            errList.push('Invalid From Date');
        }

        try{
            td = $.datepicker.parseDate(df, $('#toDate').val());
         // Default to now (ie as far forward in time as possible)
            td = (td ? td.getTime()/1000 : BITMETER.getTime());
        } catch (tde) {
            errList.push('Invalid To Date');
        }

        if (errList.length === 0){
         // The date values were ok, so continue
            $('#queryStatusBox').html('Searching... <img src="css/images/working.gif" alt="search in progress" />');
            $('#queryExportLink').hide();
            reqTxt = BITMETER.addAdaptersToRequest('query?from=' + fd + '&to=' + td + '&group=' + $('#queryDisplay').val());
            $.get(reqTxt, function(results){
                 // Store the results, adding in combined totals - we need them elsewhere
                    $.each(results, function(i,o){
                        o.cm = o.dl + o.ul;
                    });
                    BITMETER.model.setQueryResults(results);
                    BITMETER.model.setQueryGrouping($('#queryDisplay').val());
                    queryResultsGridObj[0].grid.populate();
                    var resultCount = results.length;
                    $('#queryStatusBox').html('Search found ' + resultCount + ' result' + (resultCount === 1 ? '' : 's') + '.');
                    if (resultCount){
                        $('#queryExportLink').show();
                        $('#queryExportLink').attr('href', reqTxt + '&csv=1');
                    } else {
                        $('#queryExportLink').attr('href', '');
                        $('#queryExportLink').hide();
                    }
                });
        } else {
         // There was a problem with the dates, show an error and don't send the query
            $('#errMsgList').html('');
            $.each(errList, function(i,msg){
                $('#errMsgList').append('<li>' + msg + '</li>');
            });
            $('#queryErrBox').show();
        }

    }

    $('#fromDate').datepicker(datePickerOpts);
    $('#toDate').datepicker(datePickerOpts);

    BITMETER.consts.weekdays = $('#fromDate').datepicker('option', 'dayNamesShort');
    BITMETER.consts.months   = $('#fromDate').datepicker('option', 'monthNamesShort');

 // Indicate what date format we are using
    dateFormat = $('#fromDate').datepicker('option', 'dateFormat');
    $('.dateFormat').text(dateFormat);

 // Set up event handlers etc
    $('#queryButton').click(function(){
        queryResultsGridObj.flexChangePage('first');
        runQuery();
    });
    $('#queryErrBox').hide();

    queryDialog = $('#query .dialog').dialog(BITMETER.consts.dialogOpts);
    $('#queryHelpLink').click(function(){
            queryDialog.dialog("open");
        });
    $('#queryDisplay').val(BITMETER.model.getQueryGrouping());
    $("#queryDisplay").change(function() {
        BITMETER.model.setQueryGrouping($(this).val());
    });

    $('#queryButton').button();

 // Set up the grid, the flexigrid prefs are non-standard because we use a hacked version
    queryResultsGridObj = $('#queryResults').flexigrid({
        preProcess: function(data){
         // The grid needs our data in this format...
            var rows = [];
            $.each(data, function(i,o){
             // Subtract the duration from the timestamp so the new value represents the start of the interval
                rows.push({id: o.ts, cell: [o.ts - o.dr, o.dl, o.ul, o.dl + o.ul]});
            });

            return {total : BITMETER.model.getQueryResults().length, page : BITMETER.model.getQueryResultsPage(), rows: rows};
        },
        dataType : 'json',
        colModel : [
            {display: 'Date',     name : 'ts', width : 180, sortable : true, align: 'center'},
            {display: 'Download', name : 'dl', width : 120, sortable : true, align: 'center'},
            {display: 'Upload',   name : 'ul', width : 120, sortable : true, align: 'center'},
            {display: 'Combined', name : 'cm', width : 120, sortable : true, align: 'center'}
        ],
        sortname: "ts",
        sortorder: "desc",
        usepager: true,
        useRp: true,
        rp: BITMETER.model.getQueryResultsPerPage(),
        onRpChange : function(newValue){
         // Called when we change the results-per-page value
            BITMETER.model.setQueryResultsPerPage(newValue);
        },
        showTableToggleBtn: true,
        width: 600,
        height: 200,
        getData : function(params){
         // This is non-standard, because we don't want flexigrid to do our AJAX calls - it gets data from here instead
            var sortField = params[2].value,
                isAscending = (params[3].value === 'asc'),
                pageNum  = params[0].value;

            BITMETER.model.getQueryResults().sort(function(a,b){
             // Sort the results according to what column header has been clicked
                return (a[sortField] - b[sortField]) * (isAscending ? 1 : -1);
            });

            BITMETER.model.setQueryResultsPage(pageNum);

            return BITMETER.model.getQueryResults();
        },
        formatters : [
            function(ts){
                var formatTxt, mod, roundedTs, startOfRange, endOfRange;

                if (BITMETER.model.getQueryGrouping() !== 1){
                 // Date values displayed in the grid are formatted according to how the results are grouped
                    switch (BITMETER.model.getQueryGrouping()) {
                        case 2: formatTxt = 'dd M yy'; break;
                        case 3: formatTxt = 'MM yy'; break;
                        case 4: formatTxt = 'yy'; break;
                        case 5: formatTxt = ''; break;
                    }

                    return $.datepicker.formatDate(formatTxt, new Date(ts * 1000));

                } else {
                 // We are displaying both Dates and Times in the grid because the results are grouped per-hour, so do things slightly differently
                    mod = (ts % 3600);
                    if (mod){
                     /* This should only happen if the range includes the current date/time - this value probably won't be exactly a full hour, but
                        instead will cover however long it is between the last hour boundary and now. Round up to the next full hour. */
                        roundedTs = ts - mod + 3600;
                    } else {
                        roundedTs = ts;
                    }

                    startOfRange = new Date(roundedTs * 1000);
                 // ts value has been adjusted to mark the start of the interval, so add 1 hour to get the end
                    endOfRange   = new Date((roundedTs + 3600) * 1000);

                    return BITMETER.zeroPad(startOfRange.getHours()) + ':00-' + BITMETER.zeroPad(endOfRange.getHours()) + ':00 ' + $.datepicker.formatDate('dd M yy', startOfRange);
                }
            },
            BITMETER.formatAmount,
            BITMETER.formatAmount,
            BITMETER.formatAmount
        ],
        onReload : runQuery
    });

 // Clear out the grid when the page loads...
    queryResultsGridObj[0].grid.populate();

});
