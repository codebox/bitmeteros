/*
Contains the code for the Summary page.
*/
function tabShowSummary(){
	var tdTodayDl = $('#summaryTodayDl');
	var tdTodayUl = $('#summaryTodayUl');
	var tdMonthDl = $('#summaryMonthDl');
	var tdMonthUl = $('#summaryMonthUl');
	var tdYearDl  = $('#summaryYearDl');
	var tdYearUl  = $('#summaryYearUl');
	var tdTotalDl = $('#summaryTotalDl');
	var tdTotalUl = $('#summaryTotalUl');
	
	function updateSummary(){
		function populateSummary(){
		 // Take the current Summary values out of the Model and display them
			tdTodayDl.html(formatAmount(model.getSummary().today.dl));
			tdTodayUl.html(formatAmount(model.getSummary().today.ul));
			tdMonthDl.html(formatAmount(model.getSummary().month.dl));
			tdMonthUl.html(formatAmount(model.getSummary().month.ul));
			tdYearDl.html(formatAmount(model.getSummary().year.dl));
			tdYearUl.html(formatAmount(model.getSummary().year.ul));
			tdTotalDl.html(formatAmount(model.getSummary().total.dl));
			tdTotalUl.html(formatAmount(model.getSummary().total.ul));
		}
		$.get('summary', function(objSummary){
			 // Get the next set of summary data from the server and display it
				model.setSummary(doEval(objSummary));
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
