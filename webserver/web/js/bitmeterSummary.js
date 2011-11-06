/*global $,BITMETER,config*/
/*jslint sloppy: true, white: true, plusplus: true, unparam: true */

/*
Contains the code for the Summary page.
*/
BITMETER.tabShowSummary = function(){
    var table = $('#summaryTable');
    
    function updateSummary(){
        function populateSummary(){
         // Take the current Summary values out of the Model and display them
            var summary = BITMETER.model.getSummary(), sinceDate, sinceDateTxt, html = [], hostsTxt;
            
            html.push('<tr><th class="filterNameContainer">&nbsp;</th><th>Today</th><th>Month</th><th>Year</th><th>Total</th></tr>');

            function getFilterValueFromArray(arr, filterId){
                var val;
                $.each(arr, function(i,o){
                    if (o.fl === filterId){
                        val = o.vl;
                        return false;   
                    }
                }); 
                return val;
            }
            
            BITMETER.forEachFilter(function(o){
                var todayVal = getFilterValueFromArray(summary.today, o.id),
                    monthVal = getFilterValueFromArray(summary.month, o.id),
                    yearVal  = getFilterValueFromArray(summary.year,  o.id),
                    totalVal = getFilterValueFromArray(summary.total, o.id);
                
                html.push('<tr style="color: ' + BITMETER.model.getColour(o.name) + '"><td class="name">' + o.desc + '</td><td class="value">' + BITMETER.formatAmount(todayVal) + '</td><td class="value">' + BITMETER.formatAmount(monthVal) + '</td><td class="value">' + 
                        BITMETER.formatAmount(yearVal) + '</td><td class="value">' + BITMETER.formatAmount(totalVal) + '</td></tr>');
            });
            
            sinceDate = new Date(BITMETER.model.getSummary().since * 1000);
            sinceDateTxt = BITMETER.consts.weekdays[sinceDate.getDay()] + ', ' + sinceDate.getDate() + ' ' + 
                    BITMETER.consts.months[sinceDate.getMonth()] + ' ' + (1900 + sinceDate.getYear()) + ' ' + sinceDate.getHours() + ':00:00';
            html.push('<tr><td class="name">Since</td><td colspan="4" class="value">' + sinceDateTxt + '</td></tr>');
            
            if (BITMETER.model.getSummary().hosts){
                if (BITMETER.model.getSummary().hosts.length === 0){
                    hostsTxt = 'No data from other hosts';
                } else {
                    hostsTxt = '';
                    $.each(BITMETER.model.getSummary().hosts, function(i,o){
                        hostsTxt += (o + '<br>');   
                    });
                }
                html.push('<tr><td class="name">Hosts</td><td colspan="4" class="value">' + hostsTxt + '</td></tr>');               
            }
            table.html(html.join(''));
        }
        
        $.get('/summary', function(objSummary){
             // Get the next set of summary data from the server and display it
                BITMETER.model.setSummary(objSummary);
                populateSummary();
            });
    }
    updateSummary();
    BITMETER.refreshTimer.set(updateSummary, BITMETER.model.getSummaryRefresh());
};

$(function(){
 // Handler that opens the Help dialog box
    var summaryDialog = $('#summary .dialog').dialog(BITMETER.consts.dialogOpts);
    $('#summaryHelpLink').click(function(){
            summaryDialog.dialog("open");
        });
});
