/*global $,BITMETER,config*/
/*jslint onevar: true, undef: true, nomen: true, eqeqeq: true, bitwise: true, regexp: true, newcap: true, immed: true, strict: false */

/*
Contains the code for the Summary page.
*/
BITMETER.tabShowSummary = function(){
	var tdTodayDl = $('#summaryTodayDl'),
	    tdTodayUl = $('#summaryTodayUl'),
	    tdTodayCm = $('#summaryTodayCm'),
	    tdMonthDl = $('#summaryMonthDl'),
	    tdMonthUl = $('#summaryMonthUl'),
	    tdMonthCm = $('#summaryMonthCm'),
	    tdYearDl  = $('#summaryYearDl'),
	    tdYearUl  = $('#summaryYearUl'),
	    tdYearCm  = $('#summaryYearCm'),
	    tdTotalDl = $('#summaryTotalDl'),
	    tdTotalUl = $('#summaryTotalUl'),
	    tdTotalCm = $('#summaryTotalCm'),
	    tdHosts   = $('#summaryHosts'),
	    tdSince   = $('#summarySince');
	
	function updateSummary(){
		function populateSummary(){
		 // Take the current Summary values out of the Model and display them
		    var summary = BITMETER.model.getSummary(), sinceDate, sinceDateTxt;
			tdTodayDl.html(BITMETER.formatAmount(summary.today.dl));
			tdTodayUl.html(BITMETER.formatAmount(summary.today.ul));
			tdTodayCm.html(BITMETER.formatAmount(summary.today.dl + summary.today.ul));
			tdMonthDl.html(BITMETER.formatAmount(summary.month.dl));
			tdMonthUl.html(BITMETER.formatAmount(summary.month.ul));
			tdMonthCm.html(BITMETER.formatAmount(summary.month.dl + summary.month.ul));
			tdYearDl.html(BITMETER.formatAmount(summary.year.dl));
			tdYearUl.html(BITMETER.formatAmount(summary.year.ul));
			tdYearCm.html(BITMETER.formatAmount(summary.year.dl + summary.year.ul));
			tdTotalDl.html(BITMETER.formatAmount(summary.total.dl));
			tdTotalUl.html(BITMETER.formatAmount(summary.total.ul));
			tdTotalCm.html(BITMETER.formatAmount(summary.total.dl + summary.total.ul));
			
			sinceDate = new Date(BITMETER.model.getSummary().since * 1000);
			sinceDateTxt = BITMETER.consts.weekdays[sinceDate.getDay()] + ', ' + sinceDate.getDate() + ' ' + 
					BITMETER.consts.months[sinceDate.getMonth()] + ' ' + sinceDate.getFullYear() + ' ' + sinceDate.getHours() + ':00:00';
			tdSince.html(sinceDateTxt);
			
			if (BITMETER.model.getSummary().hosts === null){
				$('tr#summaryHostRow').hide();
			} else {
				$('tr#summaryHostRow').show();
				if (BITMETER.model.getSummary().hosts.length === 0){
					tdHosts.html('No data from other hosts');
				} else {
					tdHosts.html('');
					$.each(BITMETER.model.getSummary().hosts, function(i,o){
						tdHosts.append(o + '<br>');	
					});
				}
			}
		}
		
		var reqTxt = BITMETER.addAdaptersToRequest('summary');
		$.get(reqTxt, function(objSummary){
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
