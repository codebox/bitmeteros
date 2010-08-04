/*
Contains the code for the Summary page.
*/
function tabShowSummary(){
	var tdTodayDl = $('#summaryTodayDl');
	var tdTodayUl = $('#summaryTodayUl');
	var tdTodayCm = $('#summaryTodayCm');
	var tdMonthDl = $('#summaryMonthDl');
	var tdMonthUl = $('#summaryMonthUl');
	var tdMonthCm = $('#summaryMonthCm');
	var tdYearDl  = $('#summaryYearDl');
	var tdYearUl  = $('#summaryYearUl');
	var tdYearCm  = $('#summaryYearCm');
	var tdTotalDl = $('#summaryTotalDl');
	var tdTotalUl = $('#summaryTotalUl');
	var tdTotalCm = $('#summaryTotalCm');
	var tdHosts   = $('#summaryHosts');
	var tdSince   = $('#summarySince');
	
	function updateSummary(){
		function populateSummary(){
		 // Take the current Summary values out of the Model and display them
		    var summary = model.getSummary();
			tdTodayDl.html(formatAmount(summary.today.dl));
			tdTodayUl.html(formatAmount(summary.today.ul));
			tdTodayCm.html(formatAmount(summary.today.dl + summary.today.ul));
			tdMonthDl.html(formatAmount(summary.month.dl));
			tdMonthUl.html(formatAmount(summary.month.ul));
			tdMonthCm.html(formatAmount(summary.month.dl + summary.month.ul));
			tdYearDl.html(formatAmount(summary.year.dl));
			tdYearUl.html(formatAmount(summary.year.ul));
			tdYearCm.html(formatAmount(summary.year.dl + summary.year.ul));
			tdTotalDl.html(formatAmount(summary.total.dl));
			tdTotalUl.html(formatAmount(summary.total.ul));
			tdTotalCm.html(formatAmount(summary.total.dl + summary.total.ul));
			
			var sinceDate = new Date(model.getSummary().since * 1000);
			var sinceDateTxt = WEEKDAYS[sinceDate.getDay()] + ', ' + sinceDate.getDate() + ' ' + MONTHS[sinceDate.getMonth()] + ' ' + (1900 + sinceDate.getYear()) + ' ' + sinceDate.getHours() + ':00:00';
			tdSince.html(sinceDateTxt);
			
			if (model.getSummary().hosts === null){
				$('tr#summaryHostRow').hide();
			} else {
				$('tr#summaryHostRow').show();
				if (model.getSummary().hosts.length === 0){
					tdHosts.html('No data from other hosts');
				} else {
					tdHosts.html('');
					$.each(model.getSummary().hosts, function(i,o){
						tdHosts.append(o + '<br>');	
					});
				}
			}
		}
		
		var reqTxt = addAdaptersToRequest('summary');
		$.get(reqTxt, function(objSummary){
			 // Get the next set of summary data from the server and display it
				model.setSummary(objSummary);
				populateSummary();
			});
	}
	updateSummary();
	refreshTimer = setInterval(updateSummary, config.summaryInterval);
};

$(document).ready(function(){
	 // Handler that opens the Help dialog box
		var summaryDialog = $('#summary .dialog').dialog(dialogOpts);
		$('#summaryHelpLink').click(function(){
				summaryDialog.dialog("open");
			});

});
