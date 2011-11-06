/*global $,BITMETER,window,config*/
/*jslint sloppy: true, white: true, plusplus: true, unparam: true */

/*
Contains code that handles the Monitor page.
*/
BITMETER.getMonitorTs = function(){
 /* This value determines how stretched/squashed the graph is horizontally,
    increasing the value gives more room between data points on the graph. */
    var SQUASH_FACTOR = 4;
    return Math.floor($('#monitorDisplay').width() / SQUASH_FACTOR);
};

BITMETER.updateMonitor = function(){
 // Update the graph with new data
    function updateMonitorGraph(jsonData,  graphObj, interval){
        var allData = {}, graphData = [];
        BITMETER.forEachFilter(function(o){
                allData[o.id] = [];
            }, true);

     // Split the data into arrays, one for each filter
        $.each(jsonData, function(i,o){
            allData[o.fl].push([o.ts, o.vl]);
        });
        
        BITMETER.forEachFilter(function(o){
                graphData.push(
                    {label : o.desc, data: allData[o.id], color: BITMETER.model.getColour(o.name)}
                );
            }, true);
        
        graphObj.setData(graphData);
        
     // Redraw the graph and scale
        graphObj.setupGrid();
        graphObj.draw();
    }   
        
 // Updates the Current, Peak and Average figures displayed to the right of the graph
    function updateFigures(jsonData){
        var ts, filterStats = {}, titleTxtArr = [];
        BITMETER.forEachFilter(function(o){
                filterStats[o.id] = {filterName : o.name, total : 0, peak : 0, curr: 0, bestTs : 5};
            }, true);
        
     // Loop through all the data currently displayed on the graph and accumulate totals, peaks and best-fit current values
        $.each(jsonData, function(i,o){
            var stats = filterStats[o.fl];
            
            stats.total += o.vl;
            if (o.vl > stats.peak){
                stats.peak = o.vl;
            }
            
         /* Use the most recent values to display in the 'Current' fields to the right of the graph, but ignore
            any values which have timestamps in the future (this can happen if we have synchronised with another
            host which has a clock set slightly ahead of the local clock). We cant assume there will be a value
            with exactly ts===0, so count anything within the last 5 seconds, picking the newest values that meet 
            all these criteria. */
            if (o.ts < stats.bestTs && o.ts >= 0){
                stats.bestTs = o.ts;
                stats.curr   = o.vl;
            }
        });
        
     // Store the peak values, we will need them again later
        BITMETER.forEachFilter(function(o){
                BITMETER.model.setMonitorPeak(o.id, filterStats[o.id].peak);
            }, true);
        
     // Format the values and display them
        ts = BITMETER.getMonitorTs();
        
        $.each(filterStats, function(filterId, stats){
            $("#monitorCurrent_" + stats.filterName).html(BITMETER.formatAmount(stats.curr)     + '/s');
            $("#monitorAverage_" + stats.filterName).html(BITMETER.formatAmount(stats.total/ts) + '/s');
            $("#monitorPeak_"    + stats.filterName).html(BITMETER.formatAmount(stats.peak)     + '/s');
        });
        
        if (BITMETER.model.getMonitorSpeedInTitle()){
            BITMETER.forEachFilter(function(filter){
                titleTxtArr.push(filter.name + ': ' + BITMETER.formatAmount(filterStats[filter.id].curr));
            }, true);
            window.document.title = titleTxtArr.join('|'); 
        }
    }

 // Sends the AJAX request to get the Monitor data
    var reqTxt = BITMETER.addFiltersToRequest('monitor?ts=' + BITMETER.getMonitorTs());
    $.get(reqTxt, function(response){
            /* The response is formatted as follows, with the 'ts' values being offsets from the serverTime:
                { serverTime : 123456, 
                      data : [
                        {ts: 0, dr: 1, dl: 100, ul: 200},
                        {ts: 1, dr: 1, dl: 101, ul: 201},
                           etc
                    ]}
            */
            var responseData = response.data, dataByFilter = {},
         /* The responseData array doesn't contain entries for any timestamps where there was no data, so we need to
            add those in otherwise the graph will be misleading. */
                zeroData = [],
                prevTs = 0, allData;
            
         // Split the data up into a series of arrays, one for each filter
            $.each(responseData, function(i, o){
                if (!dataByFilter[o.fl]){
                    dataByFilter[o.fl] = [];
                }
                dataByFilter[o.fl].push(o);
            });

         // The loop will start with the newest data (smallest 'ts' offset values) and move backwards through time
            $.each(dataByFilter, function(filterId, dataArray) {
                $.each(dataArray, function(i, o){
                 /* Look for a gap between this ts and the last one we saw for this filter, if there is a gap create 
                    new objects with zero values and push them onto the zeroData array */
                    var ts = o.ts - 1;
                    while(ts > prevTs){
                        zeroData.push({ts : ts, vl : 0, fl : filterId, dr : 1});   
                        ts--;
                    }
                    prevTs = o.ts;
                });
            });
            
         // Now merge the zeroData array with responseData
            allData = responseData.concat(zeroData);
            allData.sort(function(o1,o2){
                return o1.ts - o2.ts;
            });
            
         // Finally update the display with the new data
            updateMonitorGraph(allData, BITMETER.monitorGraph, 1);
            updateFigures(allData);
            BITMETER.stopwatch.newData(response.serverTime, allData);
        });
};

BITMETER.tabShowMonitor = function(){
 // Build the readout containing entries for each filter that we are displaying
    var currentReadoutHtml, averageReadoutHtml, peakReadoutHtml, hrHtml, activeFilterCount = 0, i=0;
    
    BITMETER.forEachFilter(function(){
            activeFilterCount++;
        }, true);
        
    BITMETER.forEachFilter(function(o){
            if (i++ === 0){
                currentReadoutHtml = "<tr class='monitorReadoutRow'><td rowspan='" + activeFilterCount + "' class='monitorLabel'>Current</td><td class='monitorCurrent' id='monitorCurrent_" + o.name + "'></td></tr>";
                averageReadoutHtml = "<tr class='monitorReadoutRow'><td rowspan='" + activeFilterCount + "' class='monitorLabel'>Average</td><td class='monitorAverage' id='monitorAverage_" + o.name + "'></td></tr>";
                peakReadoutHtml    = "<tr class='monitorReadoutRow'><td rowspan='" + activeFilterCount + "' class='monitorLabel'>Peak</td>   <td class='monitorPeak'    id='monitorPeak_" + o.name + "'></td></tr>";
            } else {
                currentReadoutHtml += "<tr class='monitorReadoutRow'><td class='monitorCurrent' id='monitorCurrent_" + o.name + "'></td></tr>";
                averageReadoutHtml += "<tr class='monitorReadoutRow'><td class='monitorAverage' id='monitorAverage_" + o.name + "'></td></tr>";
                peakReadoutHtml    += "<tr class='monitorReadoutRow'><td class='monitorPeak'    id='monitorPeak_" + o.name + "'></td></tr>";
            }   
        }, true);

    hrHtml = "<tr class='monitorReadoutRow'><td colspan='2'><hr/></td</tr>";    
    $('#monitorReadout table tr.monitorReadoutRow').remove();
    $('#monitorReadout table').prepend(currentReadoutHtml + hrHtml + averageReadoutHtml + hrHtml + peakReadoutHtml);
    
    BITMETER.updateMonitor();
    BITMETER.refreshTimer.set(BITMETER.updateMonitor, BITMETER.model.getMonitorRefresh());  
    
 // Make sure the readout values are coloured correctly
    BITMETER.forEachFilter(function(o){
            var colour = BITMETER.model.getColour(o.name);
            $('#monitorCurrent_' + o.name + ', #monitorPeak_' + o.name + ', #monitorAverage_' + o.name).css('color', colour);
        }, true);

    BITMETER.onTabHide.set(function(){
        $('#swDialog').dialog("close");
        BITMETER.restoreDocTitle();
    });
    
    $(window).resize();
};

BITMETER.stopwatch = (function(){
    var sw = {}, timer, time, dlTotal, ulTotal, dlAvg, ulAvg, dlMax, ulMax, dlMin, ulMin, 
        isRunning = false, latestServerTime = 0;
        
    sw.start = function(){
        if (!timer){
            timer = window.setInterval(function(){
                time++;
                sw.handler(sw);
            }, 1000);   
            isRunning = true;
        }
    };
    sw.stop = function(){
        if (timer){
            window.clearInterval(timer);
            timer = null;   
            latestServerTime = 0;
            sw.handler(sw);
            isRunning = false;
        }
    };
    sw.isRunning = function(){
        return isRunning;  
    };
    sw.reset = function(){
        time = dlTotal = ulTotal = dlAvg = ulAvg = dlMax = ulMax = 0;  
        dlMin = ulMin = Number.MAX_VALUE;
        latestServerTime = 0;
        if (sw.handler){
            sw.handler(sw);
        }
    };
    sw.setHandler = function(callback){
        this.handler = callback;
    };
    function getTotalsAfterTs(ts, data){
        var dlTotal = 0, ulTotal = 0;
        $.each(data, function(i,o){
            if (o.ts < ts) {
                dlTotal += o.dl;   
                ulTotal += o.ul;
            } else {
                return false; // data is sorted on ts so stop looping   
            }
        });
        return [dlTotal, ulTotal];
    }
    function updateTotals(dl, ul){
        dlTotal += dl;
        ulTotal += ul;
        dlAvg = (time === 0 ? 0 : (dlTotal / time));
        ulAvg = (time === 0 ? 0 : (ulTotal / time)); 
        dlMax = (dl > dlMax ? dl : dlMax);
        ulMax = (ul > ulMax ? ul : ulMax);
        dlMin = (dl < dlMin ? dl : dlMin); 
        ulMin = (ul < ulMin ? ul : ulMin);
    }
    sw.newData = function(serverTime, data){
        var totals, secondsSinceLastData;
        if (isRunning){
            if (latestServerTime === 0) {
             // This is the first batch of data since the stopwatch started
                secondsSinceLastData = 1;
            } else {
                secondsSinceLastData = serverTime - latestServerTime;
            }
            totals = getTotalsAfterTs(secondsSinceLastData, data);
            updateTotals(totals[0], totals[1]);
            latestServerTime = serverTime - data[0].ts;
        }
    };
    sw.getTime = function(){
        return time;
    };
    sw.getDlTotal = function(){
        return dlTotal;
    };
    sw.getUlTotal = function(){
        return ulTotal;
    };
    sw.getDlAvg = function(){
        return dlAvg;
    };
    sw.getUlAvg = function(){
        return ulAvg;
    };
    sw.getDlMax = function(){
        return dlMax;
    };
    sw.getUlMax = function(){
        return ulMax;
    };
    sw.getDlMin = function(){
        return dlMin === Number.MAX_VALUE ? 0 : dlMin;
    };
    sw.getUlMin = function(){
        return ulMin === Number.MAX_VALUE ? 0 : ulMin;
    };
    sw.reset();
    return sw;
}());

$(function(){
    $('#monitorShowSpeedsInTitle').click(function(){
        var isChecked = $(this).is(":checked");
        BITMETER.model.setMonitorSpeedInTitle(isChecked);
        if (isChecked){
            BITMETER.updateMonitor();
        } else {
            BITMETER.restoreDocTitle();
        }
    });
    
 // Set the initial state of the Show Speeds in Title checkbox, based on what we have saved from last time
    $('#monitorShowSpeedsInTitle').attr('checked', BITMETER.model.getMonitorSpeedInTitle());
    
 // Prepare the graph
    var monitorDisplayObj = $('#monitorDisplay'),
        readoutWidth = $('#monitorReadout table').width(),
        panel = $('#monitor'),
        swDialog, monitorDialog,
        swReadout = $('#swReadout'),
        swDlTotal = $('#swDlTotal'),
        swUlTotal = $('#swUlTotal'),
        swDlAvg   = $('#swDlAvg'),
        swUlAvg   = $('#swUlAvg'),
        swDlMax   = $('#swDlMax'),
        swUlMax   = $('#swUlMax'),
        swDlMin   = $('#swDlMin'),
        swUlMin   = $('#swUlMin');

    function setupGraph(){
        var arrData = [];
        BITMETER.forEachFilter(function(){
                arrData.push({data: []});
            }, true);
        
        BITMETER.monitorGraph = $.plot(monitorDisplayObj, arrData, {
                yaxis: {min: 0, ticks : BITMETER.makeYAxisIntervalFn(5), tickFormatter: BITMETER.formatAmount},
                xaxis: {max: BITMETER.getMonitorTs(), min: 0, tickFormatter: function(v){ 
                     // The horizontal scale shows an offset from the current time, in MM:SS format
                        var secs = v % 60,
                            mins = Math.floor(v / 60);
                        return mins + ':' + BITMETER.zeroPad(secs);
                    }},
                series: {lines : {show: true, fill: true}},
                legend : {
                    labelFormatter: function(label, series) {
                        return '<span id="#monitorLabel' + label + '">' + label + '</span>';
                    }
                }
            });
            
     // Set the initial y-axis scale for the graph
        BITMETER.applyScale(BITMETER.monitorGraph, BITMETER.model.getMonitorScale());
        
    }
    setupGraph();
    
 // Stretch the graph when the window is resized
    $(window).resize(function() {
        var graphWidth = panel.width() - readoutWidth - 50;
        if (graphWidth > 200){
            monitorDisplayObj.width(graphWidth);
            setupGraph();
        }
    });
    $(window).resize();
    
    
 // Set up the click events for the Scale Up and Scale Down arrows
    $('#monitorScaleUp').click(function(){
     // We just double the scale each time when 'Up' is pressed
        var newScale = BITMETER.model.getMonitorScale() * 2;
        if (BITMETER.applyScale(BITMETER.monitorGraph, newScale)){
            BITMETER.model.setMonitorScale(newScale);
        }
    });         
    $('#monitorScaleDown').click(function(){
     // We just halve the scale each time when 'Down' is pressed
        var newScale = BITMETER.model.getMonitorScale() / 2;
        if (BITMETER.applyScale(BITMETER.monitorGraph, newScale)){
            BITMETER.model.setMonitorScale(newScale);
        }
    });         
    
 // Open the Stopwatch dialog when the icon is clicked
    swDialog = $('#swDialog').dialog({
        autoOpen : false,
        resizable : false,
        buttons :  { "OK" : function() { 
            $(this).dialog("close"); 
        }},
        close : function(){
            BITMETER.stopwatch.stop();
            BITMETER.stopwatch.reset();
        }
    });
    
    function setSwButtonState(){
        if (BITMETER.stopwatch.isRunning()){
            $('#swStopGo').button("option", "icons", {primary: 'ui-icon-pause'});
        } else {
            $('#swStopGo').button("option", "icons", {primary: 'ui-icon-play'});
        }
    }
    
    $('#monitorStopwatchIcon').click(function(){
            //$('.swDlValue').css('color', BITMETER.model.getDownloadColour());
            //$('.swUlValue').css('color', BITMETER.model.getUploadColour());
            swDialog.dialog("open");
            setSwButtonState();
        });

    BITMETER.stopwatch.setHandler(function(sw){
        swReadout.html(BITMETER.formatInterval(sw.getTime(), BITMETER.formatInterval.TINY));
        swDlTotal.html(BITMETER.formatAmount(sw.getDlTotal()));
        swUlTotal.html(BITMETER.formatAmount(sw.getUlTotal()));
        swDlAvg.html(BITMETER.formatAmount(sw.getDlAvg()) + '/s');
        swUlAvg.html(BITMETER.formatAmount(sw.getUlAvg()) + '/s');
        swDlMax.html(BITMETER.formatAmount(sw.getDlMax()) + '/s');
        swUlMax.html(BITMETER.formatAmount(sw.getUlMax()) + '/s');
        swDlMin.html(BITMETER.formatAmount(sw.getDlMin()) + '/s');
        swUlMin.html(BITMETER.formatAmount(sw.getUlMin()) + '/s');                                    
    });
    $('#swReset').button({ icons: {primary: 'ui-icon-seek-first'}});
    $('#swStopGo').button({ icons: {primary: 'ui-icon-play'}});

    $('#swStopGo').click(function(){
        if (BITMETER.stopwatch.isRunning()){
            BITMETER.stopwatch.stop();
        } else {
            BITMETER.stopwatch.start();    
        }
        setSwButtonState();
    });
    $('#swReset').click(function(){
        BITMETER.stopwatch.reset();
    });
    $('#swReset').click();
 // Show the Help dialog box when the help link is clicked
    monitorDialog = $('#monitor .dialog').dialog(BITMETER.consts.dialogOpts);
    $('#monitorHelpLink').click(function(){
            monitorDialog.dialog("open");
        });

});
