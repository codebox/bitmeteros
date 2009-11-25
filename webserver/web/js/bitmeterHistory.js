/*
Contains code that manages the History page.
*/
var historyDisplayMinutes, historyDisplayHours, historyDisplayDays;

// Gets called when we click on the History tab
function tabShowHistory(){
 /* Redraws one of the bar graphs with new data. jsonData is just an array of standard data objects like this:
        {ts: 1234567, dr: 1, dl: 100, ul: 200}
	sorted by ascending ts */
	function updateGraph(jsonData,  graphObj, fnTsToXValue){
		var dlData = [];
		var ulData = [];

	 // Separate the data out into 2 arrays, one for UL and one for DL
		$.each(jsonData, function(i,o){
			var xValue = fnTsToXValue ? fnTsToXValue(o.ts) : o.ts;
			dlData.push([xValue, o.dl]);
			ulData.push([xValue, o.ul]);
		});
		
		graphObj.getOptions().xaxis.min = 0;
		graphObj.setData([
			{color: "rgb(255,0,0)", label : 'Download', data: dlData}, 
			{color: "rgb(0,255,0)", label : 'Upload',   data: ulData}
		]);
		
		graphObj.setupGrid();
		graphObj.draw();
	}

	function updateHistory(){
		var now = Math.floor(getTime());
		
		/* Sends the AJAX request for the Minutes graph. The results returned by a /query request don't have sufficient
		   granularity for what we need here, so call /monitor instead, and sort the values in minute-sized groups. */
		var minGraphTs = historyDisplayMinutes.getOptions().xaxis.max;
		$.get('monitor?ts=' + minGraphTs, function(responseTxt){
		        /* We get back an object like this, with ts values expressed as an offset from the serverTime value:
					{ serverTime : 123456, 
					  data : [
						{ts: 0, dr: 1, dl: 100, ul: 200},
						{ts: 1, dr: 1, dl: 101, ul: 201},
						   etc
					]}
				*/
				var response = doEval(responseTxt);
				var jsonData   = response.data;
				var serverTime = response.serverTime;
				historyDisplayMinutes.serverTime = serverTime;
				
				var minuteBuckets = {};
				var newestRoundedTs = 0;
				$.each(jsonData, function(i,o){
				 // The response data contains offsets rather than real timestamps
					o.ts = (serverTime - o.ts);
					
				 // Round the timestamp UP to the next minute, unless we are already on a minute boundary
					var secondsPastTheMinute = o.ts % 60;
					var roundedTs;
					if (secondsPastTheMinute > 0){
						roundedTs = o.ts + (60 - secondsPastTheMinute);
					} else {
						roundedTs = o.ts;
					}
					
					if (roundedTs > newestRoundedTs){
						newestRoundedTs = roundedTs;
					}
					if (!minuteBuckets[roundedTs]){
					 /* This is the first value we have encountered for this particular minute, so create a
					    new entry in the minuteBuckets object, settings the ul/dl values to 0. */
						minuteBuckets[roundedTs] = {ts: roundedTs, dr: 60, dl: 0, ul: 0};
					}
					
				 // Add the ul/dl values onto the totals for the appropriate minute in the minuteBucket object
					minuteBuckets[roundedTs].dl += o.dl;
					minuteBuckets[roundedTs].ul += o.ul;
				});
				
				var roundedJsonData = [];
			 // Now extract the totals from the minuteBuckets object into an array and send it to the graph
				$.each(minuteBuckets, function(k,v){
					roundedJsonData.push(v);
				});
				
				updateGraph(roundedJsonData, historyDisplayMinutes, function(ts){
						return newestRoundedTs - ts;
					});
			});
		
		/* Sends the AJAX request for the Hours graph */
		var hourGraphMax = now;
		var hourGraphMin = now - historyDisplayHours.getOptions().xaxis.max;
		$.get('query?from=' + hourGraphMin + '&to=' + hourGraphMax + '&group=1', function(arrData){
				var jsonData = doEval(arrData);
				var now = getTime();
				var modSeconds = now % 3600;
				var secondsUntilNextFullHour = (modSeconds === 0 ? 0 : 3600 - modSeconds);
				updateGraph(jsonData, historyDisplayHours, function(ts){return now - ts + secondsUntilNextFullHour;});
			});
			
		/* Sends the AJAX request for the Days graph */
		var dayGraphMax = now;
		var dayGraphMin = now - historyDisplayDays.getOptions().xaxis.max;
		$.get('query?from=' + dayGraphMin + '&to=' + dayGraphMax + '&group=2', function(arrData){
				var jsonData = doEval(arrData);
				var now = getTime();
				var modSeconds = now % 86400;
				var secondsUntilNextFullDay = (modSeconds === 0 ? 0 : 86400 - modSeconds);
				updateGraph(jsonData, historyDisplayDays, function(ts){return now - ts + secondsUntilNextFullDay;});
			});
	}
	
	updateHistory();
	refreshTimer = setInterval(updateHistory, config.historyInterval);	
};

$(document).ready(function(){
     // Manage the floating info div that appears when we hover over bars on the graph
		var floaterVisible = false;
		function floatMouseMoveHandler(e){
			$('#floater').css({'left' : e.clientX + 15, 'top' : e.clientY });
		}
		
		function showFloater(evObj){
		 // Display the floating window that appears when we hover over the bars
			var cOffset = $(this).offset();
			
			$('#floater').css({'left' : evObj.clientX - cOffset.left, 'top' : evObj.clientY - cOffset.top});
			$('#floater').fadeIn(500);
			
			$(this).bind('mousemove', floatMouseMoveHandler);
			    
			floaterVisible = true;
		}
		function hideFloater(evObj){
		 // Hide the floating window
			floaterVisible = false;				
			$('#floater').fadeOut(500);
			
			$(this).unbind('mousemove', floatMouseMoveHandler);
		}			

		function buildHoverHandler(graph, fnFormatter, fnGetTime){
		 // Create a 'plothover' event handler function for the specified graph
			return function (event, pos, item) {
				if (floaterVisible){
					if (item) {
					 // Get the UL and DL values corresponding to the bar we are hovering over
						var idx = item.dataIndex;
						var data = graph.getData();
						var dl = data[0].data[idx][1];
						var ul = data[1].data[idx][1];
						
					 // Calculate the date/time for this bar
						var tick = data[0].data[idx][0];
						var time = fnGetTime(tick);
		
					 // Populate the hover box with the date/time and totals
						$('#floater').html(fnFormatter(time, dl, ul));
						
					} else {
				     // If we aren't currently over a bar then empty the hover box
						$('#floater').html('');
					}
				}
			}
		}
		
		var WEEKDAYS = $('#fromDate').datepicker('option', 'dayNamesShort');
		var MONTHS   = $('#fromDate').datepicker('option', 'monthNamesShort');

		function makeHourRange(toHour){
			var fromHour = (toHour === 0 ? 23 : toHour - 1);
			return zeroPad(fromHour) + ':00 - ' + zeroPad(toHour) + ':00';
		}
		function makeMinRange(toHour, toMin){
			var fromHour = (toMin === 0 ? (toHour === 0 ? 23 : toHour - 1) : toHour);
			var fromMin  = (toMin === 0 ? 59 : toMin - 1);
			return zeroPad(fromHour) + ':' + zeroPad(fromMin) + ' - ' + zeroPad(toHour) + ':' + zeroPad(toMin);
		}

     // Set up the Minutes graph
		var historyDisplayMinutesObj = $("#historyDisplayMinutes");
		historyDisplayMinutes = $.plot(historyDisplayMinutesObj, [{color: "rgb(255,0,0)", data: []}, {color: "rgb(0,255,0)", data: []}], {
				yaxis: {min: 0, tickFormatter: formatAmount, ticks : makeYAxisIntervalFn(2)},
				xaxis: {max: 60 * 100, min: 0, ticks: function(axis){ 
				     // The labels on the x-axis of the Minutes graph should just show time in hours and minutes, at 15 minute intervals
						var arrTicks = [];
						var now;
						if (typeof(historyDisplayMinutes) === 'undefined'){
							now = new Date();
						} else {
							now = new Date(historyDisplayMinutes.serverTime * 1000);							
						}
						
					 /* Initialise 'time' to the last 15-minute boundary that we have passed, eg if
					    the time is now 12:18:32 then we set time to 12:15:00 */
						var time = (now.getTime() - (now.getMinutes() % 15) * 1000 * 60 - now.getSeconds() * 1000);
						
					 // Calculate the time beyond which we won't have any more x-axis labels to draw
						var minTime = now.getTime() - (axis.max * 1000);
						
					 /* Starting at the initial 15-minute boundary, create labels in HH:MM format and store them in
					    the arrTicks array, moving back 15 minutes between each one, until we hit the lower limit 
						that we just calculated. */
						while (time >= minTime){
					     // This is the graph x-value where the label will appear
							var tick = (now.getTime() - time)/1000;
							
							var date  = new Date(time);
							var hours = zeroPad(date.getHours());
							var mins  = zeroPad(date.getMinutes());
							
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
		applyScale(historyDisplayMinutes, model.getHistoryMinScale());
		
	 // Set up the 'hover' event handler for the Minutes graph.
		historyDisplayMinutesObj.bind("plothover", buildHoverHandler(
			historyDisplayMinutes, 
			function(time, dl, ul){
				var timeTxt = makeMinRange(time.getHours(), time.getMinutes());
				return timeTxt + '<br>DL: ' + formatAmount(dl) + ' UL: ' + formatAmount(ul);
			},
			function(tick){
				return new Date((historyDisplayMinutes.serverTime + 60 - tick) * 1000);
			}
		));
			 
		historyDisplayMinutesObj.hover(showFloater, hideFloater);
		
	 // Set up the click events for the Scale Up and Scale Down arrows
		$('#historyMinutesScaleUp').click(function(){
		 // We just double the scale each time when 'Up' is pressed
			var newScale = model.getHistoryMinScale() * 2;
			if (applyScale(historyDisplayMinutes, newScale)){
				model.setHistoryMinScale(newScale);
			}
		});			
		$('#historyMinutesScaleDown').click(function(){
		 // We just halve the scale each time when 'Down' is pressed
			var newScale = model.getHistoryMinScale() / 2;
			if (applyScale(historyDisplayMinutes, newScale)){
				model.setHistoryMinScale(newScale);
			}
		});						


	 // Set up the Hours graph
		var historyDisplayHoursObj = $("#historyDisplayHours");
		historyDisplayHours = $.plot(historyDisplayHoursObj, [{color: "rgb(255,0,0)", data: []}, {color: "rgb(0,255,0)", data: []}], {
				yaxis: {max: 3000000, min: 0, tickFormatter: formatAmount, ticks : makeYAxisIntervalFn(2)},
				xaxis: {max: 3600 * 100, min: 0, ticks: function(axis){ 
					 // The labels on the x-axis of the Hours graph should appear at 12-hour intervals
						var arrTicks = [];
						var now = new Date();
						
					 /* Initialise 'time' to the last 12-hour boundary that we have passed, eg. if the
					    time is 13:15;01 then we set time to 12:00:00 */
						var time = (now.getTime() - ((now.getHours() % 12) * 1000 * 60 * 60) - (now.getMinutes() * 1000 * 60) - now.getSeconds() * 1000);
						
					 // Calculate the time beyond which we won't have any more x-axis labels to draw
						var minTime = now.getTime() - (axis.max * 1000);
						
					 /* Starting at the initial 12-hour boundary, create labels in DDD HH:00 format and store them in
					    the arrTicks array, moving back 12 hours between each one, until we hit the lower limit 
						that we just calculated. */
						while (time >= minTime){
						 // This is the graph x-value where the label will appear
							var tick = (now.getTime() - time)/1000;
							
							var date  = new Date(time);
							var hours = zeroPad(date.getHours()) + ':00';
							var day   = WEEKDAYS[date.getDay()];
							
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
		applyScale(historyDisplayHours, model.getHistoryHourScale());
		
	 // Set up the 'hover' event handler for the Hours graph
		historyDisplayHoursObj.bind("plothover", buildHoverHandler(
			historyDisplayHours, 
			function(time, dl, ul){
				var timeTxt = WEEKDAYS[time.getDay()] + ' ' + time.getDate() + '  ' + makeHourRange(time.getHours());
				return timeTxt + '<br>DL: ' + formatAmount(dl) + ' UL: ' + formatAmount(ul);
			},
			function(tick){
				return new Date((getTime() + 3600 - tick) * 1000);
			}
		));
		
		
	 // Set up the click events for the Scale Up and Scale Down arrows
		historyDisplayHoursObj.hover(showFloater, hideFloater);
		$('#historyHoursScaleUp').click(function(){
		 // We just double the scale each time when 'Up' is pressed
			var newScale = model.getHistoryHourScale() * 2;
			if (applyScale(historyDisplayHours, newScale)){
				model.setHistoryHourScale(newScale);
			}
		});			
		$('#historyHoursScaleDown').click(function(){
		 // We just halve the scale each time when 'Down' is pressed
			var newScale = model.getHistoryHourScale() / 2;
			if (applyScale(historyDisplayHours, newScale)){
				model.setHistoryHourScale(newScale);
			}
		});	
					
					
	 // Set up the Days graph
		var historyDisplayDaysObj = $("#historyDisplayDays");
		historyDisplayDays = $.plot(historyDisplayDaysObj, [{color: "rgb(255,0,0)", data: []}, {color: "rgb(0,255,0)", data: []}], {
				yaxis: {max: 30000000, min: 0, tickFormatter: formatAmount, ticks : makeYAxisIntervalFn(2)},
				xaxis: {max: 3600 * 24 * 100, min: 0, ticks: function(axis){ 
				     // The labels on the x-axis of the Days graph should appear at 7-day intervals
						var arrTicks = [];
						var now = new Date();
						
					 // Calculate the start of the current day
						var time = (now.getTime() - (now.getHours() * 1000 * 60 * 60) - (now.getMinutes() * 1000 * 60) - now.getSeconds() * 1000);
						
					 // Calculate the time beyond which we won't have any more x-axis labels to draw
						var minTime = now.getTime() - (axis.max * 1000);
						
					 /* Starting at the initial day boundary, create labels in DD MMM format and store them in
					    the arrTicks array, moving back 7 days between each one, until we hit the lower limit 
						that we just calculated. */
						while (time >= minTime){
						 // This is the graph x-value where the label will appear
							var tick = (now.getTime() - time)/1000;
							var date = new Date(time);
							var month = MONTHS[date.getMonth()];
							var day  = date.getDate();
							
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
		applyScale(historyDisplayDays, model.getHistoryDayScale());
		
	 // Set up the 'hover' event handler for the Days graph.
		historyDisplayDaysObj.bind("plothover", buildHoverHandler(
			historyDisplayDays, 
			function(time, dl, ul){
				var timeTxt = WEEKDAYS[time.getDay()] + ' ' + time.getDate() + ' ' + MONTHS[time.getMonth()];
				return timeTxt + '<br>DL: ' + formatAmount(dl) + ' UL: ' + formatAmount(ul);
			},
			function(tick){
				return new Date(new Date().getTime() - tick * 1000);
			}
		));
	 
	 // Set up the click events for the Scale Up and Scale Down arrows
		historyDisplayDaysObj.hover(showFloater, hideFloater);
		$('#historyDaysScaleUp').click(function(){
		 // We just double the scale each time when 'Up' is pressed
			var newScale = model.getHistoryDayScale() * 2;
			if (applyScale(historyDisplayDays, newScale)){
				model.setHistoryDayScale(newScale);
			}
		});			
		$('#historyDaysScaleDown').click(function(){
		 // We just halve the scale each time when 'Down' is pressed
			var newScale = model.getHistoryDayScale() / 2;
			if (applyScale(historyDisplayDays, newScale)){
				model.setHistoryDayScale(newScale);
			}
		});		
		
	 // Show the Help dialog box when the help link is clicked
		var historyDialog = $('#history .dialog').dialog(dialogOpts);
		$('#historyHelpLink').click(function(){
				historyDialog.dialog("open");
			});
						
	});



