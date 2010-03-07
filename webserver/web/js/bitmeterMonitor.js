/*
Contains code that handles the Monitor page.
*/
var monitorGraph;

function getMonitorTs(){
	return Math.floor($('#monitorDisplay').width() / 4);
}

function updateMonitor(){
// Update the graph with new data
	function updateMonitorGraph(jsonData,  graphObj, interval, showDl, showUl){
		var dlData = [];
		var ulData = [];

	 // Split the data into 2 arrays, one for UL and one for DL
		$.each(jsonData, function(i,o){
			dlData.push([o.ts, o.dl]);
			ulData.push([o.ts, o.ul]);
		});
		
		graphObj.setData([
			{color: model.getDownloadColour(), label : (showDl ? 'Download' : null), data: dlData, lines : {show : showDl}}, 
			{color: model.getUploadColour(),   label : (showUl ? 'Upload' : null),   data: ulData, lines : {show : showUl}}
		]);
		
	 // Redraw the graph and scale
		graphObj.setupGrid();
		graphObj.draw();
	}	
		
// Updates the Current, Peak and Average figures displayed to the right of the graph
	function updateFigures(jsonData){
		var showDl = model.getMonitorShowDl();
		var showUl = model.getMonitorShowUl();

	 // Loop through all the data currently displayed on the graph and accumulate totals, peaks and best-fit current values
		var dlTotal = 0, ulTotal = 0, dlPeak = 0, ulPeak = 0, dlCurr = 0, ulCurr = 0, bestTs = 5;
		$.each(jsonData, function(i,o){
			dlTotal += o.dl;
			if (o.dl > dlPeak){
				dlPeak = o.dl;
			}
			
			ulTotal += o.ul;
			if (o.ul > ulPeak){
				ulPeak = o.ul;
			}
			
		 /* Use the most recent values to display in the 'Current' fields to the right of the graph, but ignore
		 	any values which have timestamps in the future (this can happen if we have synchronised with another
		 	host which has a clock set slightly ahead of the local clock). We cant assume there will be a value
		 	with exactly ts===0, so count anything within the last 5 seconds, picking the newest values that meet 
		 	all these criteria. */
			if (o.ts < bestTs && o.ts >= 0){
				bestTs = o.ts;
				dlCurr = o.dl;
				ulCurr = o.ul;
			}
		});
		
	 // Store the peak values, we will need them again later
		model.setMonitorDlPeak(dlPeak);
		model.setMonitorUlPeak(ulPeak);

	 // Format the values and display them
	 	var ts = getMonitorTs();
		$('#monitorDlCurrent').html(showDl ? formatAmount(dlCurr)                   + '/s' : '');
		$('#monitorUlCurrent').html(showUl ? formatAmount(ulCurr)                   + '/s' : '');
		$('#monitorDlAverage').html(showDl ? formatAmount(dlTotal/ts)               + '/s' : '');
		$('#monitorUlAverage').html(showUl ? formatAmount(ulTotal/ts)               + '/s' : '');
		$('#monitorDlPeak').html(   showDl ? formatAmount(model.getMonitorDlPeak()) + '/s' : '');
		$('#monitorUlPeak').html(   showUl ? formatAmount(model.getMonitorUlPeak()) + '/s' : '');				
	}

// Sends the AJAX request to get the Monitor data
	$.get('monitor?ts=' + getMonitorTs(), function(responseTxt){
			var response = doEval(responseTxt);
			/* The response is formatted as follows, with the 'ts' values being offsets from the serverTime:
				{ serverTime : 123456, 
					  data : [
						{ts: 0, dr: 1, dl: 100, ul: 200},
						{ts: 1, dr: 1, dl: 101, ul: 201},
						   etc
					]}
			*/
			var responseData = response.data;
			
		 /* The responseData array doesn't contain entries for any timestamps where there was no data, so we need to
			add those in otherwise the graph will be misleading. */
			var zeroData = [];
			var prevTs = 0;
			
		 // The loop will start with the newest data (smallest 'ts' offset values) and move backwards through time
			$.each(responseData, function(i, o){
			 /* Look for a gap between this ts and the last one we saw, if there is a gap create new objects with empty
				DL and UL values and push them onto the zeroData array */
				var ts = o.ts - 1;
				while(ts > prevTs){
					zeroData.push({ts : ts, dl : 0, ul : 0, dr : 1});	
					ts--;
				}
				prevTs = o.ts;
			});
			
		 // Now merge the zeroData array with responseData
			var allData = responseData.concat(zeroData);
			allData.sort(function(o1,o2){
				return o1.ts - o2.ts;
			});
			
		 // Finally update the display with the new data
			updateMonitorGraph(allData, monitorGraph, 1, model.getMonitorShowDl(), model.getMonitorShowUl());
			updateFigures(allData);
		});
}
function tabShowMonitor(){
	updateMonitor();
	refreshTimer = setInterval(updateMonitor, config.monitorInterval);	
	
 // Make sure the readout values are coloured correctly
 	$('#monitorDlCurrent, #monitorDlPeak, #monitorDlAverage').css('color', model.getDownloadColour());
 	$('#monitorUlCurrent, #monitorUlPeak, #monitorUlAverage').css('color', model.getUploadColour());

	$(window).resize();
};

$(document).ready(function(){
	 // Set up the event handlers for the Show Upload/Download checkboxes
		$('#monitorShowDl').click(function(){
			var isChecked = $(this).is(":checked");
			model.setMonitorShowDl(isChecked);
			updateMonitor();
		});
		$('#monitorShowUl').click(function(){
			var isChecked = $(this).is(":checked");
			model.setMonitorShowUl(isChecked);
			updateMonitor();
		});
		
	 // Set the initial state of the Show Upload/Download checkboxes, based on what we have saved from last time
		$('#monitorShowDl').attr('checked', model.getMonitorShowDl());
		$('#monitorShowUl').attr('checked', model.getMonitorShowUl());
		
	 // Prepare the graph
		var monitorDisplayObj = $('#monitorDisplay');
		
		function setupGraph(){
			monitorGraph = $.plot(monitorDisplayObj, [{color: model.getDownloadColour(), data: []}, {color: model.getUploadColour(), data: []}], {
					yaxis: {min: 0, ticks : makeYAxisIntervalFn(5), tickFormatter: formatAmount},
					xaxis: {max: getMonitorTs(), min: 0, tickFormatter: function(v){ 
						 // The horizontal scale shows an offset from the current time, in MM:SS format
							var secs = v % 60;
							var mins = Math.floor(v / 60);
							return mins + ':' + zeroPad(secs);
						}},
					series: {lines : {show: true, fill: true}},
					legend : {
						labelFormatter: function(label, series) {
							return '<span id="#monitorLabel' + label + '">' + label + '</span>';
						}
					}
				});
				
		 // Set the initial y-axis scale for the graph
			applyScale(monitorGraph, model.getMonitorScale());
			
		};
		setupGraph();
		
	 // Stretch the graph when the window is resized
		var readoutWidth = $('#monitorReadout table').width();
		var panel = $('#monitor');
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
			var newScale = model.getMonitorScale() * 2;
			if (applyScale(monitorGraph, newScale)){
				model.setMonitorScale(newScale);
			}
		});			
		$('#monitorScaleDown').click(function(){
		 // We just halve the scale each time when 'Down' is pressed
			var newScale = model.getMonitorScale() / 2;
			if (applyScale(monitorGraph, newScale)){
				model.setMonitorScale(newScale);
			}
		});			
		
	 // Show the Help dialog box when the help link is clicked
		var monitorDialog = $('#monitor .dialog').dialog(dialogOpts);
		$('#monitorHelpLink').click(function(){
				monitorDialog.dialog("open");
			});

	});
