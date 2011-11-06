/*global $,BITMETER,window,config*/
/*jslint sloppy: true, white: true, plusplus: true, unparam: true */

/*
Contains code that manages the History page.
*/

BITMETER.getHistoryMinutesTs = function(){
    return Math.floor($("#historyDisplayMinutes").width() / 6);
};
BITMETER.getHistoryHoursTs = function(){
    return Math.floor($("#historyDisplayHours").width() / 6);
};
BITMETER.getHistoryDaysTs = function(){
    return Math.floor($("#historyDisplayDays").width() / 6);
};

BITMETER.updateHistory = function(){
 /* Redraws one of the bar graphs with new data. jsonData is just an array of standard data objects like this:
        {ts: 1234567, dr: 1, vl: 100, fl: 2} */
    function updateGraph(jsonData,  graphObj, fnTsToXValue){
        var allData = {}, valuesForCurrentTs = {}, currentTs = 0, graphData = [];
        
        jsonData = jsonData.sort(function(a,b){
            return a.ts - b.ts;
        });
        
        BITMETER.forEachFilter(function(f){
                allData[f.id] = [];
                valuesForCurrentTs[f.id] = 0;
            }, true);

        function appendValuesForTsToAllData(ts){
            var xValue = fnTsToXValue ? fnTsToXValue(ts) : ts;
            BITMETER.forEachFilter(function(f){
                allData[f.id].push([xValue, valuesForCurrentTs[f.id]]);
                valuesForCurrentTs[f.id] = 0;
            }, true);           
        }

     // Split the data into arrays, one for each filter
        $.each(jsonData, function(i,o){
            if (currentTs && (o.ts !== currentTs)){
                appendValuesForTsToAllData(currentTs);
            }
            
            valuesForCurrentTs[o.fl] += o.vl;
            currentTs = o.ts;
        });
        appendValuesForTsToAllData(currentTs);
        
        graphObj.getOptions().xaxis.min = 0;
        
        BITMETER.forEachFilter(function(o){
                graphData.push(
                    {label : o.desc, data: allData[o.id], color: BITMETER.model.getColour(o.name)}
                );
            }, true);
        
        graphObj.setData(graphData);
        graphObj.setupGrid();
        graphObj.draw();
    }   
    
    var hourGraphMax, hourGraphMin, hourGraphReqTxt, dayGraphMax, dayGraphMin, dayGraphReqTxt, 
        minGraphTs, minGraphReqTxt, now = Math.floor(BITMETER.getTime());
    
 /* Sends the AJAX request for the Minutes graph. The results returned by a /query request don't have sufficient
    granularity for what we need here, so call /monitor instead, and sort the values in minute-sized groups. */
    minGraphTs     = BITMETER.historyDisplayMinutes.getOptions().xaxis.max;
    minGraphReqTxt = BITMETER.addFiltersToRequest('monitor?ts=' + 60 * BITMETER.getHistoryMinutesTs());
    $.get(minGraphReqTxt, function(response){
            /* We get back an object like this, with ts values expressed as an offset from the serverTime value:
                { serverTime : 123456, 
                  data : [
                    {ts: 0, dr: 1, vl: 100, fl: 1},
                    {ts: 1, dr: 1, vl: 101, fl: 1},
                       etc
                ]}
            */
            var jsonData        = response.data, 
                serverTime      = response.serverTime, 
                minuteBuckets   = {},
                roundedJsonData = [];
            
            BITMETER.historyDisplayMinutes.serverTime = serverTime;
            
            $.each(jsonData, function(i,o){
             // The response data contains offsets rather than real timestamps
                o.ts = (serverTime - o.ts);
                
             // Round the timestamp UP to the next minute, unless we are already on a minute boundary
                var roundedTs, secondsPastTheMinute = o.ts % 60;
                if (secondsPastTheMinute > 0){
                    roundedTs = o.ts + (60 - secondsPastTheMinute);
                } else {
                    roundedTs = o.ts;
                }
                
                if (!minuteBuckets[roundedTs]){
                 /* This is the first value we have encountered for this particular minute, so create a
                    new entry in the minuteBuckets object, settings the value to 0. */
                    minuteBuckets[roundedTs] = {};
                    BITMETER.forEachFilter(function(o){
                            minuteBuckets[roundedTs][o.id] = {ts: roundedTs, dr: 60, vl: 0, fl: o.id};  
                        }, true);
                }
                
             // Add the ul/dl values onto the totals for the appropriate minute in the minuteBucket object
                minuteBuckets[roundedTs][o.fl].vl += o.vl;
            });
            
         // Now extract the totals from the minuteBuckets object into an array and send it to the graph
            $.each(minuteBuckets, function(k,v){
                BITMETER.forEachFilter(function(o){
                        roundedJsonData.push(v[o.id]);
                    }, true);
            });
            
            updateGraph(roundedJsonData, BITMETER.historyDisplayMinutes, function(ts){
                    return now - ts - (now % 60) + 60;
                });
        });
    
    /* Sends the AJAX request for the Hours graph */
    hourGraphMax = now;
    hourGraphMin = now - BITMETER.historyDisplayHours.getOptions().xaxis.max;
    hourGraphReqTxt = BITMETER.addFiltersToRequest('query?from=' + hourGraphMin + '&to=' + hourGraphMax + '&group=1');
    $.get(hourGraphReqTxt, function(jsonData){
            var now = BITMETER.getTime(),
                modSeconds = now % 3600,
                secondsUntilNextFullHour = (modSeconds === 0 ? 0 : 3600 - modSeconds);
                
            updateGraph(jsonData, BITMETER.historyDisplayHours, function(ts){return now - ts + secondsUntilNextFullHour;});
        });
        
    /* Sends the AJAX request for the Days graph */
    dayGraphMax = now;
    dayGraphMin = now - BITMETER.historyDisplayDays.getOptions().xaxis.max;
    dayGraphReqTxt = BITMETER.addFiltersToRequest('query?from=' + dayGraphMin + '&to=' + dayGraphMax + '&group=2');
    $.get(dayGraphReqTxt, function(jsonData){
            var now = BITMETER.getTime(),
                modSeconds = now % 86400,
                secondsUntilNextFullDay = (modSeconds === 0 ? 0 : 86400 - modSeconds);
                
            updateGraph(jsonData, BITMETER.historyDisplayDays, function(ts){return now - ts + secondsUntilNextFullDay;});
        });
};

// Gets called when we click on the History tab
BITMETER.tabShowHistory = function(){
    BITMETER.refreshTimer.set(BITMETER.updateHistory, BITMETER.model.getHistoryRefresh());
    $(window).resize();
};

$(function(){
 // Manage the floating info div that appears when we hover over bars on the graph
    var historyDisplayMinutesObj = $("#historyDisplayMinutes"),
        historyDisplayHoursObj   = $("#historyDisplayHours"),
        historyDisplayDaysObj    = $("#historyDisplayDays"),
        historyDialog, panel;

    function showFloater(evObj){
     // Display the floating window that appears when we hover over the bars
        BITMETER.infoFloat.show($(this), evObj);
        $(this).css('cursor', 'crosshair');
    }
    function hideFloater(){
        BITMETER.infoFloat.hide();          
        $(this).css('cursor', '');
    }           

    function buildHoverHandler(graph, fnFormatter, fnGetTime){
     // Create a 'plothover' event handler function for the specified graph
        return function (event, pos, item) {
            if (BITMETER.infoFloat.isDisplayed()){
                if (item) {
                 // Get the values corresponding to the bar we are hovering over
                    var idx = item.dataIndex, tick, time, valObj = [], data, i;

                    data = graph.getData();
                    
                 // Calculate the date/time for this bar
                    tick = item.datapoint[0];
                    time = fnGetTime(tick);

                    i=0;
                    BITMETER.forEachFilter(function(f){
                            var dataArrayForFilter = data[i].data, dataPair;
                            dataPair = dataArrayForFilter ? dataArrayForFilter[idx] : null;
                            valObj[i] = dataPair ? dataPair[1] : 0;
                            i++;
                        }, true);
    
                 // Populate the hover box with the date/time and totals
                    BITMETER.infoFloat.setHTML(fnFormatter(time, valObj));
                    
                } else {
                 // If we aren't currently over a bar then empty the hover box
                    BITMETER.infoFloat.setHTML('');
                }
            }
        };
    }
    
    function makeHourRange(toHour){
        var fromHour = (toHour === 0 ? 23 : toHour - 1);
        return BITMETER.zeroPad(fromHour) + ':00 - ' + BITMETER.zeroPad(toHour) + ':00';
    }
    function makeMinRange(toHour, toMin){
        var fromHour = (toMin === 0 ? (toHour === 0 ? 23 : toHour - 1) : toHour),
            fromMin  = (toMin === 0 ? 59 : toMin - 1);
        return BITMETER.zeroPad(fromHour) + ':' + BITMETER.zeroPad(fromMin) + ' - ' + BITMETER.zeroPad(toHour) + ':' + BITMETER.zeroPad(toMin);
    }

    function buildHoverTable(dateTimeTxt, valObj, intervalInSecs){
        var i=0, arrHtml = [
            '<table>',
                '<tr><td class="historyHoverTime" colspan="2">', dateTimeTxt, '</td></tr>',
                '<tr>'];
        
        BITMETER.forEachFilter(function(filter){
            var cssColour = BITMETER.model.getColour(filter.name), v = valObj[i];
            
            arrHtml.push('<tr>');
            arrHtml.push(    '<td style="color: ' + cssColour + '" class="historyHoverDir">' + filter.desc + '</td>');
            arrHtml.push(    '<td style="color: ' + cssColour + '" class="historyHoverData">', BITMETER.formatAmount(v), '</td>');
            arrHtml.push(    '<td style="color: ' + cssColour + '" class="historyHoverData">[ ', BITMETER.formatAmount(v/intervalInSecs), '/s ]</td>');
            arrHtml.push('</tr>');
            i++;
        }, true);        

        arrHtml.push('</tr></table>');
        
        return arrHtml.join('');
    }
    
 // Set up the Minutes graph
    function setupMinutesGraph(){
        //BITMETER.historyDisplayMinutes = $.plot(historyDisplayMinutesObj, [{color: BITMETER.model.getDownloadColour(), data: []}, {color: BITMETER.model.getUploadColour(), data: []}], {
        BITMETER.historyDisplayMinutes = $.plot(historyDisplayMinutesObj, [{data: []}, {data: []}], {
                yaxis: {min: 0, tickFormatter: BITMETER.formatAmount, ticks : BITMETER.makeYAxisIntervalFn(2)},
                xaxis: {max: 60 * BITMETER.getHistoryMinutesTs(), min: 0, ticks: function(axis){ 
                     // The labels on the x-axis of the Minutes graph should just show time in hours and minutes, at 15 minute intervals
                        var arrTicks = [], now, time, minTime, tick, date, hours, mins;
                        if (typeof(BITMETER.historyDisplayMinutes) === 'undefined'){
                            now = new Date();
                        } else {
                            now = new Date(BITMETER.historyDisplayMinutes.serverTime * 1000);                           
                        }
                    
                     /* Initialise 'time' to the last 15-minute boundary that we have passed, eg if
                        the time is now 12:18:32 then we set time to 12:15:00 */
                        time = (now.getTime() - (now.getMinutes() % 15) * 1000 * 60 - now.getSeconds() * 1000);
                    
                     // Calculate the time beyond which we won't have any more x-axis labels to draw
                        minTime = now.getTime() - (axis.max * 1000);
                    
                     /* Starting at the initial 15-minute boundary, create labels in HH:MM format and store them in
                        the arrTicks array, moving back 15 minutes between each one, until we hit the lower limit 
                        that we just calculated. */
                        while (time >= minTime){
                         // This is the graph x-value where the label will appear
                            tick = (now.getTime() - time)/1000;
                        
                            date  = new Date(time);
                            hours = BITMETER.zeroPad(date.getHours());
                            mins  = BITMETER.zeroPad(date.getMinutes());
                        
                            arrTicks.push([tick, hours + ':' + mins]);
                        
                         // Move back 15 minutes for the next label
                            time -= 15 * 60 * 1000;
                        }
                        return arrTicks;
                    }
                },
                series: {bars : {show: true, fill: true, barWidth: 60, lineWidth: 1}},
                grid: {hoverable: true}
            });
        
     // Set the initial y-axis scale for the Minutes graph
        BITMETER.applyScale(BITMETER.historyDisplayMinutes, BITMETER.model.getHistoryMinScale());
        
     // Set up the 'hover' event handler for the Minutes graph.
        historyDisplayMinutesObj.unbind("plothover");
        historyDisplayMinutesObj.bind("plothover", buildHoverHandler(
            BITMETER.historyDisplayMinutes, 
            function(time, valObj){
                var timeTxt = makeMinRange(time.getHours(), time.getMinutes()),
                    SECS_PER_MIN = 60;
                
                return buildHoverTable(timeTxt, valObj, SECS_PER_MIN);
            },
            function(tick){
                return new Date((BITMETER.historyDisplayMinutes.serverTime + 60 - tick) * 1000);
            }
        ));
        historyDisplayMinutesObj.hover(showFloater, hideFloater);
    }
    setupMinutesGraph();
    
    
 // Set up the click events for the Scale Up and Scale Down arrows
    $('#historyMinutesScaleUp').click(function(){
     // We just double the scale each time when 'Up' is pressed
        var newScale = BITMETER.model.getHistoryMinScale() * 2;
        if (BITMETER.applyScale(BITMETER.historyDisplayMinutes, newScale)){
            BITMETER.model.setHistoryMinScale(newScale);
        }
    });         
    $('#historyMinutesScaleDown').click(function(){
     // We just halve the scale each time when 'Down' is pressed
        var newScale = BITMETER.model.getHistoryMinScale() / 2;
        if (BITMETER.applyScale(BITMETER.historyDisplayMinutes, newScale)){
            BITMETER.model.setHistoryMinScale(newScale);
        }
    });                     


 // Set up the Hours graph
    function setupHoursGraph(){
        //BITMETER.historyDisplayHours = $.plot(historyDisplayHoursObj, [{color: BITMETER.model.getDownloadColour(), data: []}, {color: BITMETER.model.getUploadColour(), data: []}], {
        BITMETER.historyDisplayHours = $.plot(historyDisplayHoursObj, [{data: []}, {data: []}], {
                yaxis: {max: 3000000, min: 0, tickFormatter: BITMETER.formatAmount, ticks : BITMETER.makeYAxisIntervalFn(2)},
                xaxis: {max: 3600 * BITMETER.getHistoryHoursTs(), min: 0, ticks: function(axis){ 
                     // The labels on the x-axis of the Hours graph should appear at 12-hour intervals
                        var arrTicks = [], now = new Date(), time, minTime, tick, date, hours, day;
                    
                     /* Initialise 'time' to the last 12-hour boundary that we have passed, eg. if the
                        time is 13:15;01 then we set time to 12:00:00 */
                        time = (now.getTime() - ((now.getHours() % 12) * 1000 * 60 * 60) - (now.getMinutes() * 1000 * 60) - now.getSeconds() * 1000);
                    
                     // Calculate the time beyond which we won't have any more x-axis labels to draw
                        minTime = now.getTime() - (axis.max * 1000);
                    
                     /* Starting at the initial 12-hour boundary, create labels in DDD HH:00 format and store them in
                        the arrTicks array, moving back 12 hours between each one, until we hit the lower limit 
                        that we just calculated. */
                        while (time >= minTime){
                         // This is the graph x-value where the label will appear
                            tick = (now.getTime() - time)/1000;
                        
                            date  = new Date(time);
                            hours = BITMETER.zeroPad(date.getHours()) + ':00';
                            day   = BITMETER.consts.weekdays[date.getDay()];
                        
                            arrTicks.push([tick, day + ' ' + hours]);
                        
                         // Move back 12 hours for the next label
                            time -= 12 * 3600 * 1000;
                        }
                        return arrTicks;
                    }
                },
                series: {bars : {show: true, fill: true, barWidth: 3600, lineWidth: 1}},
                grid: {hoverable: true}
            });
        
     // Set the initial y-axis scale for the Hours graph
        BITMETER.applyScale(BITMETER.historyDisplayHours, BITMETER.model.getHistoryHourScale());
        
     // Set up the 'hover' event handler for the Hours graph
        historyDisplayHoursObj.unbind("plothover");
        historyDisplayHoursObj.bind("plothover", buildHoverHandler(
            BITMETER.historyDisplayHours, 
            function(time, valObj){
                var timeTxt = BITMETER.consts.weekdays[time.getDay()] + ' ' + time.getDate() + '  ' + makeHourRange(time.getHours()),
                    SECS_PER_HOUR = 3600;
                
                return buildHoverTable(timeTxt, valObj, SECS_PER_HOUR);
            },
            function(tick){
                return new Date((BITMETER.getTime() + 3600 - tick) * 1000);
            }
        ));

        historyDisplayHoursObj.hover(showFloater, hideFloater);         
    }
    setupHoursGraph();
    
 // Set up the click events for the Scale Up and Scale Down arrows
    $('#historyHoursScaleUp').click(function(){
     // We just double the scale each time when 'Up' is pressed
        var newScale = BITMETER.model.getHistoryHourScale() * 2;
        if (BITMETER.applyScale(BITMETER.historyDisplayHours, newScale)){
            BITMETER.model.setHistoryHourScale(newScale);
        }
    });         
    $('#historyHoursScaleDown').click(function(){
     // We just halve the scale each time when 'Down' is pressed
        var newScale = BITMETER.model.getHistoryHourScale() / 2;
        if (BITMETER.applyScale(BITMETER.historyDisplayHours, newScale)){
            BITMETER.model.setHistoryHourScale(newScale);
        }
    }); 
                
                
 // Set up the Days graph
    function setupDaysGraph(){
        //BITMETER.historyDisplayDays = $.plot(historyDisplayDaysObj, [{color: BITMETER.model.getDownloadColour(), data: []}, {color: BITMETER.model.getUploadColour(), data: []}], {
        BITMETER.historyDisplayDays = $.plot(historyDisplayDaysObj, [{data: []}, {data: []}], {
                yaxis: {max: 30000000, min: 0, tickFormatter: BITMETER.formatAmount, ticks : BITMETER.makeYAxisIntervalFn(2)},
                xaxis: {max: 3600 * 24 * BITMETER.getHistoryDaysTs(), min: 0, ticks: function(axis){ 
                     // The labels on the x-axis of the Days graph should appear at 7-day intervals
                        var arrTicks = [], now = new Date(), time, minTime, tick, date, month, day;
                    
                     // Calculate the start of the current day
                        time = (now.getTime() - (now.getHours() * 1000 * 60 * 60) - (now.getMinutes() * 1000 * 60) - now.getSeconds() * 1000);
                    
                     // Calculate the time beyond which we won't have any more x-axis labels to draw
                        minTime = now.getTime() - (axis.max * 1000);
                    
                     /* Starting at the initial day boundary, create labels in DD MMM format and store them in
                        the arrTicks array, moving back 7 days between each one, until we hit the lower limit 
                        that we just calculated. */
                        while (time >= minTime){
                         // This is the graph x-value where the label will appear
                            tick  = (now.getTime() - time)/1000;
                            date  = new Date(time);
                            month = BITMETER.consts.months[date.getMonth()];
                            day   = date.getDate();
                        
                            arrTicks.push([tick, day + ' ' + month]);
                        
                         // Move back 7 days for the next label
                            time -= 7 * 3600 * 24 * 1000;
                        }
                        return arrTicks;
                    }
                },
                series: {bars : {show: true, fill: true, barWidth: 3600 * 24, lineWidth: 1}},
                grid: {hoverable: true}
            });
        
     // Set the initial y-axis scale for the Days graph
        BITMETER.applyScale(BITMETER.historyDisplayDays, BITMETER.model.getHistoryDayScale());

     // Set up the 'hover' event handler for the Days graph.
        historyDisplayDaysObj.unbind("plothover");
        historyDisplayDaysObj.bind("plothover", buildHoverHandler(
            BITMETER.historyDisplayDays, 
            function(time, valObj){
                var timeTxt = BITMETER.consts.weekdays[time.getDay()] + ' ' + time.getDate() + ' ' + BITMETER.consts.months[time.getMonth()],
                    SECS_PER_DAY = 86400;
                
                return buildHoverTable(timeTxt, valObj, SECS_PER_DAY);
            },
            function(tick){
                return new Date(new Date().getTime() - tick * 1000);
            }
        ));

        historyDisplayDaysObj.hover(showFloater, hideFloater);          
    }
    setupDaysGraph();
    
 // Set up the click events for the Scale Up and Scale Down arrows      
    $('#historyDaysScaleUp').click(function(){
     // We just double the scale each time when 'Up' is pressed
        var newScale = BITMETER.model.getHistoryDayScale() * 2;
        if (BITMETER.applyScale(BITMETER.historyDisplayDays, newScale)){
            BITMETER.model.setHistoryDayScale(newScale);
        }
    });         
    $('#historyDaysScaleDown').click(function(){
     // We just halve the scale each time when 'Down' is pressed
        var newScale = BITMETER.model.getHistoryDayScale() / 2;
        if (BITMETER.applyScale(BITMETER.historyDisplayDays, newScale)){
            BITMETER.model.setHistoryDayScale(newScale);
        }
    });     
    
 // Show the Help dialog box when the help link is clicked
    historyDialog = $('#history .dialog').dialog(BITMETER.consts.dialogOpts);
    $('#historyHelpLink').click(function(){
            historyDialog.dialog("open");
        });

 // Stretch the graphs when the window is resized
    panel = $('#history');
    $(window).resize(function() {
        var graphWidth = panel.width() -50;
        if (graphWidth > 200){
            historyDisplayMinutesObj.width(graphWidth);
            setupMinutesGraph();
            historyDisplayHoursObj.width(graphWidth);
            setupHoursGraph();
            historyDisplayDaysObj.width(graphWidth);
            setupDaysGraph();
            
            BITMETER.updateHistory();
        }
    });
});
