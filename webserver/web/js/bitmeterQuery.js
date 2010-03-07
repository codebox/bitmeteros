/*
Contains code that handles to Query page
*/
function tabShowQuery(){
	// Nothing to do here
}

$(document).ready(function(){
		function runQuery(){
			$('#queryErrBox').hide();
			var fromDate = $('#fromDate').val();
			var toDate   = $('#toDate').val();

		 // We do some validation on the date values entered by the user, empty fields don't cause an error
			var df = $('#fromDate').datepicker('option', 'dateFormat');
			var fd, td, errList=[];
			try{
				fd = $.datepicker.parseDate(df, $('#fromDate').val());
			 // Default to 0 (ie as far back in time as possible)
				fd = (fd ? fd.getTime()/1000 : 0);
			} catch (e) {
				errList.push('Invalid From Date');
			}
			
			try{
				td = $.datepicker.parseDate(df, $('#toDate').val());
			 // Default to now (ie as far forward in time as possible)
				td = (td ? td.getTime()/1000 : getTime());
			} catch (e) {
				errList.push('Invalid To Date');			
			}
			
			if (errList.length === 0){
			 // The date values were ok, so continue
				var req = '/query?from=' + fd + '&to=' + td + '&group=' + $('#queryDisplay').val();
				$.get(req, function(objQuery){
					 // Store the results, we need them elsewhere
						model.setQueryResults(doEval(objQuery));
						model.setQueryGrouping($('#queryDisplay').val());
						queryResultsGridObj[0].grid.populate();
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

		var datePickerOpts = { showOn: 'button', buttonImage: '/css/images/calendar.gif' };
		$('#fromDate').datepicker(datePickerOpts);
		$('#toDate').datepicker(datePickerOpts);
		
		WEEKDAYS = $('#fromDate').datepicker('option', 'dayNamesShort');
		MONTHS   = $('#fromDate').datepicker('option', 'monthNamesShort');

	 // Indicate what date format we are using
		var dateFormat = $('#fromDate').datepicker('option', 'dateFormat');
		$('.dateFormat').text(dateFormat);
		
	 // Set up event handlers etc
		$('#queryButton').click(runQuery);
		$('#queryErrBox').hide();
		
		var queryDialog   = $('#query .dialog').dialog(dialogOpts);
		$('#queryHelpLink').click(function(){
				queryDialog.dialog("open");
			});
		$('#queryDisplay').val(model.getQueryGrouping());
		$("#queryDisplay").change(function() {
			model.setQueryGrouping($(this).val());
		});

	 // Set up the grid, the flexigrid prefs are non-standard because we use a hacked version
		var queryResultsGridObj = $('#queryResults').flexigrid({
			preProcess: function(data){
			 // The grid needs our data in this format...
				var rows = [];
				$.each(data, function(i,o){
				 // Subtract the duration from the timestamp so the new value represents the start of the interval
					rows.push({id: o.ts, cell: [o.ts - o.dr, o.dl, o.ul]});
				});
				
				return {total : model.getQueryResults().length, page : model.getQueryResultsPage(), rows: rows};
			},
			dataType : 'json',
			colModel : [
				{display: 'Date',     name : 'ts', width : 180, sortable : true, align: 'center'},
				{display: 'Download', name : 'dl', width : 120, sortable : true, align: 'center'},
				{display: 'Upload',   name : 'ul', width : 120, sortable : true, align: 'center'}
			],
			sortname: "ts",
			sortorder: "desc",
			usepager: true,
			useRp: true,
			rp: model.getQueryResultsPerPage(),
			onRpChange : function(newValue){
			 // Called when we change the results-per-page value
				model.setQueryResultsPerPage(newValue);
			},
			showTableToggleBtn: true,
			width: 500,
			height: 200,
			getData : function(params){
			 // This is non-standard, because we don't want flexigrid to do our AJAX calls - it gets data from here instead
				var sortField = params[2].value;
				var isAscending = (params[3].value === 'asc');

				model.getQueryResults().sort(function(a,b){
				 // Sort the results according to what column header has been clicked
				 	return (a[sortField] - b[sortField]) * (isAscending ? 1 : -1);
				});

				var pageResults = [];
				var pageNum  = params[0].value;
				model.setQueryResultsPage(pageNum);
			
				return model.getQueryResults();
			},
			formatters : [
				function(ts){
					var formatTxt;
					if (model.getQueryGrouping() !== 1){
					 // Date values displayed in the grid are formatted according to how the results are grouped
						switch (model.getQueryGrouping()) {
							case 2: formatTxt = 'dd M yy'; break;
							case 3: formatTxt = 'MM yy'; break;
							case 4: formatTxt = 'yy'; break;
							case 5: formatTxt = ''; break;						
						}

						return $.datepicker.formatDate(formatTxt, new Date(ts * 1000)); 
						
					} else {
					 // We are displaying both Dates and Times in the grid because the results are grouped per-hour, so do things slightly differently
						var mod = (ts % 3600);
						var roundedTs;
						if (mod){
						 /* This should only happen if the range includes the current date/time - this value probably won't be exactly a full hour, but
						    instead will cover however long it is between the last hour boundary and now. Round up to the next full hour. */
							roundedTs = ts - mod + 3600
						} else {
							roundedTs = ts;
						}
						
						var startOfRange = new Date(roundedTs * 1000); 
					 // ts value has been adjusted to mark the start of the interval, so add 1 hour to get the end
						var endOfRange   = new Date((roundedTs + 3600) * 1000);
						
						return zeroPad(startOfRange.getHours()) + ':00-' + zeroPad(endOfRange.getHours()) + ':00 ' + $.datepicker.formatDate('dd M yy', startOfRange);
					}
				}, 
				formatAmount, 
				formatAmount
			],
			onReload : runQuery
		});			
});


