/*global $,config*/
/*jslint onevar: true, undef: true, nomen: true, eqeqeq: true, bitwise: true, regexp: true, newcap: true, immed: true, strict: false */

var BITMETER = {};

/* 
The factory function that builds the 'model' object containing the state of the application. Some of
the properties of the model are automatically saved into a cookie when new values are assigned.
*/
BITMETER.model = (function(){
    var model  = {}, values = {}, defaults;
    
 // We use these defaults when there is nothing else in memory or in the cookie
    defaults = {
    	'colour_dl'  : '#aa0000',
    	'colour_ul'  : '#00aa00',
    	'colour_idl' : '#ffaa00',
    	'colour_iul' : '#00aaff',
        'monitorSpeedInTitle' : 'false',
        'monitorScale'  : 204800,
        'historyMinScale'  : 12288000,
        'historyHourScale' : 737280000,
        'historyDayScale'  : 17694720000,
        'queryGrouping'    : 2,
        'queryResultsPerPage' : 20,
        'queryResults' : [],
        'filters' : '1,2,3,4',
        'binaryUnits' : 'true',
        'showFilterWarning' : 'true',
        'monitorRefresh' : config.monitorInterval,
        'historyRefresh' : config.historyInterval,
        'summaryRefresh' : config.summaryInterval
    };
    
    function get(key){
        var value = values[key];
        if (typeof(value) === "undefined"){
         // No value stored in memory so check the cookie
            value = $.cookie(key);
            if ((value === null) && defaults){
             // No cookie value either but some defaults were provided, so check those
                value = defaults[key];
            }
        }
        return value;
    }

    function set(key, value, persist){
        values[key] = value;
        if (persist){
            $.cookie(key, value, {expires : 30});
        }
    }

 // Summary values
    model.getSummary = function(){
        return get('summary');
    };
    model.setSummary = function(summary){
        set('summary', summary, false);
    };
 
 // Query Results
    model.getQueryResults = function(){
        return get('queryResults');
    };
    model.setQueryResults = function(queryResults){
        return set('queryResults', queryResults, false);
    };
    
 // Query Results Grouping
    model.getQueryGrouping = function(){
        return Number(get('queryGrouping'));
    };
    model.setQueryGrouping = function(queryResults){
        return set('queryGrouping', queryResults, true);
    };

 // Query Results Current Page 
    model.getQueryResultsPage = function(){
        return Number(get('queryResultsPage'));
    };
    model.setQueryResultsPage = function(queryResultsPage){
        return set('queryResultsPage', '' + queryResultsPage, false);
    };
 

 // Monitor peak values
    model.getMonitorPeak = function(filter){
    	return get('monitorPeak_' + filter) || 0;
    };
    model.setMonitorPeak = function(filter, peakValue){
        set('monitorPeak_' + filter, peakValue, false);
    };
    
 // Show/hide speeds in title bar on the Monitor page  
    model.getMonitorSpeedInTitle = function(){
        return get('monitorSpeedInTitle') === 'true';
    };
    model.setMonitorSpeedInTitle = function(showFlag){
        set('monitorSpeedInTitle', '' + showFlag, true);
    };
    
 // Scales for the Monitor and History graphs
    model.getMonitorScale = function(){
        return Number(get('monitorScale'));
    };
    model.setMonitorScale = function(scale){
        set('monitorScale', '' + scale, true);
    };
    
    model.getHistoryMinScale = function(){
        return Number(get('historyMinScale'));
    };
    model.setHistoryMinScale = function(scale){
        set('historyMinScale', '' + scale, true);
    };

    model.getHistoryHourScale = function(){
        return Number(get('historyHourScale'));
    };
    model.setHistoryHourScale = function(scale){
        set('historyHourScale', '' + scale, true);
    };

    model.getHistoryDayScale = function(){
        return Number(get('historyDayScale'));
    };
    model.setHistoryDayScale = function(scale){
        set('historyDayScale', '' + scale, true);
    };

    model.getQueryResultsPerPage = function(){
        return Number(get('queryResultsPerPage'));
    };
    model.setQueryResultsPerPage = function(resultsPerPage){
        set('queryResultsPerPage', '' + resultsPerPage, true);
    };
    
 // Colours for the plots on graphs
    model.getColour = function(filter){
        return get('colour_' + filter);
    };
    model.setColour = function(filter, colour){
        set('colour_' + filter, colour, true);
    };
    
 // Value determines which filters we display data for 
    model.getFilters = function(){
        return get('filters');
    };
    model.setFilters = function(filters){
        set('filters', filters, true);
    };

 // Determines if a warning is shown when a data filter is active
    model.getShowFilterWarning = function(){
        return get('showFilterWarning') === 'true';
    };
    model.setShowFilterWarning = function(showFilterWarning){
        set('showFilterWarning', '' + showFilterWarning, true);
    };

 // Determines whether we use binary or decimal units
    model.getBinaryUnits = function(){
        return get('binaryUnits') === 'true';
    };
    model.setBinaryUnits = function(binaryUnits){
        set('binaryUnits', '' + binaryUnits, true);
    };

 // Interval, in milliseconds, between updates on the Monitor page
    model.getMonitorRefresh = function(){
        return Number(get('monitorRefresh'));
    };
    model.setMonitorRefresh = function(monitorRefresh){
        set('monitorRefresh', monitorRefresh, true);
    };

 // Interval, in milliseconds, between updates on the History page
    model.getHistoryRefresh = function(){
        return Number(get('historyRefresh'));
    };
    model.setHistoryRefresh = function(historyRefresh){
        set('historyRefresh', historyRefresh, true);
    };

 // Interval, in milliseconds, between updates on the Summary page
    model.getSummaryRefresh = function(){
        return Number(get('summaryRefresh'));
    };
    model.setSummaryRefresh = function(summaryRefresh){
        set('summaryRefresh', summaryRefresh, true);
    };

    return model;
}());

