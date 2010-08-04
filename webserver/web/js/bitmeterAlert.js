function setHoverStatus(icon, name, target, current){
	function floatMouseMoveHandler(e){
		$('#floater').css({'left' : e.pageX + 15, 'top' : e.pageY });
	}
	icon.unbind('hover');
	icon.hover(function (evObj){
			var floaterHtml;
			if (current > target){
				floaterHtml = '<h4>' + name + '</h4>' + 
					'<p>This alert has been triggered</p>' + 
					'<table>' + 
					'<tr><td class="alertStatusFloaterName">Current</td><td class="alertStatusFloaterValue">' + formatAmount(current) + '</td></tr>' + 
					'<tr><td class="alertStatusFloaterName">Exceeded by</td><td class="alertStatusFloaterValue">' + formatAmount(current - target) + '</td></tr>' + 
					'<tr><td class="alertStatusFloaterName">Limit</td><td class="alertStatusFloaterValue">' + formatAmount(target) + '</td></tr>' + 
					'<tr><td class="alertStatusFloaterName">Progress</td><td class="alertStatusFloaterValue">' + (100 * current/target).toFixed(2) + '%</td></tr>' + 
					'</table>';
				$('#floater').addClass('floaterErr');
				
			} else {
				floaterHtml = '<h4>' + name + '</h4>' + 
					'<table>' + 
					'<tr><td class="alertStatusFloaterName">Current</td><td class="alertStatusFloaterValue">' + formatAmount(current) + '</td></tr>' + 
					'<tr><td class="alertStatusFloaterName">Remaining</td><td class="alertStatusFloaterValue">' + formatAmount(target - current) + '</td></tr>' + 
					'<tr><td class="alertStatusFloaterName">Limit</td><td class="alertStatusFloaterValue">' + formatAmount(target) + '</td></tr>' + 
					'<tr><td class="alertStatusFloaterName">Progress</td><td class="alertStatusFloaterValue">' + (100 * current/target).toFixed(2) + '%</td></tr>' + 
					'</table>';
				$('#floater').removeClass('floaterErr');
			}
			$('#floater').html(floaterHtml);
			
		 // Display the floating window that appears when we hover over the bars
			var cOffset = $(this).offset();
			
			$('#floater').css({'left' : evObj.clientX - cOffset.left, 'top' : evObj.clientY - cOffset.top});
			$('#floater').fadeIn(200);
			
			$(this).bind('mousemove', floatMouseMoveHandler);
			    
			floaterVisible = true;
		}, function(){
		 // Hide the floating window
			floaterVisible = false;				
			$('#floater').fadeOut(200);
			
			$(this).css('cursor', '');
			$('#floater').removeClass('floaterErr');
			$(this).unbind('mousemove', floatMouseMoveHandler);
		}			
	);
	
}
function populateAlertsTable(alertsArray){
	if (alertsArray && alertsArray.length){
		$('#noAlerts').hide();
		$('#alertsDisplay').show();
		
		var tbody = $('#alertsTable tbody');
		tbody.html('');
		
		$.each(alertsArray, function(i,o){
			var tr = $('<tr id="alertRow' + o.id + '"><td class="alertName">' + o.name + '</td></tr>');
			
			var tdProgress = $('<td></td>');
			var progress = $('<div class="alertProgress" id="progress' + o.id + '"></div>');
			progress.progressbar();
			tdProgress.append(progress);
			tr.append(tdProgress);
			
			var tdStatus = $('<td class="alertStatus" id="status' + o.id + '"></td>');
			tdStatus.html('');
			tr.append(tdStatus);

			var tdStatusIcon = $('<td class="alertStatusIcon" id="statusIcon' + o.id + '"></td>');
			tr.append(tdStatusIcon);

			var tdLink = $('<td class="alertLink"></td>');
			var editLink = $('<a class="adminOnly">Edit</a>');
			editLink.click(function(){editAlert(o);});
			tdLink.append(editLink);
			tr.append(tdLink);
			
			var tdDelete = $('<td class="alertLink adminOnly"></td>');
			var deleteLink = $('<a  class="adminOnly">Delete</a>');
			deleteLink.click(function(){deleteAlert(o);});
			tdDelete.append(deleteLink);
			tr.append(tdDelete);

			tbody.append(tr);
		});
		updateAlertProgressBars();
		
	} else {
		$('#alertsDisplay').hide();
		$('#noAlerts').show();		
	}
}

function editAlert(alertObj){
	resetAlertView();
	createAlertModel.reset();
	
	var yr, mn, dy, hr, isRollingStart;
	
	createAlertModel.setId(alertObj.id);
	createAlertModel.setAmount(alertObj.amount);
	createAlertModel.setDirection(alertObj.direction);
	
	function isRolling(txt){
		return txt.indexOf('-') === 0;
	}
	
 // Set start date/time - we need to work out which of the 3 types of Start this represents
 	if (alertObj.bound[0] !== '*'){
 	 // The year is '*' so this must be a 'fixed' start date/time
 	 	createAlertModel.setStartType(START_TYPE_FIXED);
 	 	yr = Number(alertObj.bound[0]);
 	 	mn = Number(alertObj.bound[1]);
 	 	dy = Number(alertObj.bound[2]);
 	 	hr = Number(alertObj.bound[4]);
 	 	assert(!isNaN(yr) && !isNaN(mn) && !isNaN(dy) && !isNaN(hr));
 	 	var fixedDate = new Date(Date.UTC(yr, mn-1, dy, hr));
 	 	createAlertModel.setStartFixedDate(fixedDate);
 	 	createAlertModel.setStartFixedTime(fixedDate.getHours());
 	 	
 	} else {
 		var isRollingMonths = isRolling(alertObj.bound[1]);
 		var isRollingDays   = isRolling(alertObj.bound[2]);
 		var isRollingHours  = isRolling(alertObj.bound[4]);

 		if (isRollingMonths || isRollingDays || isRollingHours){
 		 // Either the month, day or hour value begins with a '-', so this must be a 'rolling' start date/time
 			assert(alertObj.bound[0] === '*' && alertObj.bound[3] === '*');
 			createAlertModel.setStartType(START_TYPE_ROLLING);
 			
 			if (isRollingMonths){
	 			createAlertModel.setStartRollingAmount(-Number(alertObj.bound[1]));
	 			createAlertModel.setStartRollingUnits(START_ROLLING_MONTHS);
	 			
 			} else if (isRollingDays){
	 			createAlertModel.setStartRollingAmount(-Number(alertObj.bound[2]));
	 			createAlertModel.setStartRollingUnits(START_ROLLING_DAYS);

	 		} else if (isRollingHours){
	 			createAlertModel.setStartRollingAmount(-Number(alertObj.bound[4]));
	 			createAlertModel.setStartRollingUnits(START_ROLLING_HOURS);
	 			
	 		} else {
	 			assert(false, "isRollingMonths/isRollingDays/isRollingHours");	
	 		}
 			
 		} else {
 		 // By process of elimination, this should be a 'repeating' start date/time - do some checks to make sure
 			assert(alertObj.bound[0] === '*' && alertObj.bound[1] === '*', "Repeating start, year/month not wildcards");
 			assert(!isNaN(alertObj.bound[4]), "Repeating start, hour not a number");
 			createAlertModel.setStartType(START_TYPE_REPEATING);
 			
 			var hasWeekday  = (alertObj.bound[3] !== '*');
 			var hasMonthDay = (alertObj.bound[2] !== '*');
 			if (hasWeekday){
 				assert(!hasMonthDay, "Repeating start, hasWeekday and hasMonthDay");
 				createAlertModel.setStartRepeatingType(START_REPEATING_WEEK);
 				createAlertModel.setStartRepeatingDay(alertObj.bound[3]);

 			} else if (hasMonthDay){
 				createAlertModel.setStartRepeatingType(START_REPEATING_MONTH);
 				createAlertModel.setStartRepeatingDate(alertObj.bound[2]);
 				
 			} else {
 				createAlertModel.setStartRepeatingType(START_REPEATING_DAY);
 				
 			}
 			createAlertModel.setStartRepeatingTime(alertObj.bound[4]);
 		}
 	}
	
 // Set the periods
 	if (alertObj.periods && alertObj.periods.length) {
 		createAlertModel.setTimesType(TIMES_SOME);
	 	$.each(alertObj.periods, function(i,o){
	 		assert((o[0] === '*') && (o[1] === '*') && (o[2] === '*'));
	 		var daysTxt = o[3].split(',');
	 		var days = [0,0,0,0,0,0,0];
	 		$.each(daysTxt, function(i,o){
	 			days[Number(o)] = 1;
	 		});
	 		var parts = o[4].split('-');
	 		var fromHours = Number(parts[0]);
	 		var toHours   = parts.length > 1 ? Number(parts[1]) : fromHours;
	 		createAlertModel.addPeriod(days, fromHours, toHours + 1); // Add 1 to the upper bound, '8-8' (or just '8') covers range 8am to 9am, so display it as 8-9
	 	});

 	} else {
 		createAlertModel.setTimesType(TIMES_ALL);
 	}
 	
	createAlertModel.setName(alertObj.name);
	showAlertEditor();
}

function deleteAlert(alert){
	showConfirm("Delete " + alert.name + " - are you sure?", function(){
		$.get('alert?action=delete&id=' + alert.id, tabShowAlerts);   
	});
}

function tabShowAlerts(){
	$.get('alert?action=list', function(jsonData){
			populateAlertsTable(jsonData);
		});
}

function updateAlertProgressBars(){
 // Loop once for each alert
	$.get('alert?action=status', function(jsonData){
		$.each(jsonData, function(i,o){
			var value = o.current / o.limit;
			var barValue, statusTxt = (Math.round(value * 100) + '%');
			if (value > 1){
				barValue  = 100;
			} else {
				barValue = 100 * value;
			}
			
			$('#status' + o.id).html(statusTxt);
			
			if (value > 1){
				$('#progress' + o.id).progressbar("destroy").addClass('alertProgressFull').addClass('roundedCorners');
				$('#statusIcon' + o.id).html('<span class="ui-icon ui-icon-notice">&nbsp;</span>');
				$('#alertRow' + o.id).addClass('ui-state-error');
			} else {
				$('#progress' + o.id).progressbar("option", "value", barValue);
				$('#statusIcon' + o.id).html('<span class="ui-icon ui-icon-info">&nbsp;</span>');
			}
			
			setHoverStatus($('#statusIcon' + o.id), o.name, o.limit, o.current);
		});
	});   	
	setAdminOnlyHandlers();
}

var START_TYPE_FIXED     = 1;
var START_TYPE_REPEATING = 2;
var START_TYPE_ROLLING   = 3;

var START_REPEATING_MONTH = 1;
var START_REPEATING_WEEK  = 2;
var START_REPEATING_DAY   = 3;

var START_ROLLING_HOURS  = 1;
var START_ROLLING_DAYS   = 2;
var START_ROLLING_MONTHS = 3;

var TIMES_ALL  = 1;
var TIMES_SOME = 2;

var ARR_ALL_DAYS = [1,1,1,1,1,1,1];
var ARR_WEEKENDS = [1,0,0,0,0,0,1];
var ARR_WEEKDAYS = [0,1,1,1,1,1,0];

var createAlertModel = (function(){
	var model = {};
	
	var onChangeCallback;
	model.setChangeHandler = function(fnCallback){
		onChangeCallback = fnCallback;
	};
	
	function onChange(){
		onChangeCallback(model);
	}
	
	model.reset = function(){
		id = name = amount = direction = startType = startFixedDate = startFixedTime = startRepeatingType = startRepeatingDate = startRepeatingDay = startRepeatingTime = startRollingUnits = startRollingAmount = timesType = null;
		periods = {};
		nextPeriodId = periodCount = 0;
	};
	
	var id;
	model.setId = function(i){
		id = i;
	};
	model.getId = function(){
		return id;
	};
	
	var amount = null;
	model.setAmount = function(amt){
		amount = amt;
		onChange();
	};
	model.getAmount = function(){
		return amount;
	};
	
	var direction;
	model.setDirection = function(dirn){
		direction = dirn;	
		onChange();
	}
	model.getDirection = function(){
		return direction;
	};
	
	var startType = null;
	model.setStartType = function(st){
		startType = Number(st);	
		onChange();
	}
	model.getStartType = function(){
		return startType;	
	}
	
	var startFixedDate = null;
	model.setStartFixedDate = function(dt){
		startFixedDate = new Date(dt.getTime());	
		(startFixedTime !== null) && startFixedDate.setHours(startFixedTime);
		onChange();
	}
	model.getStartFixedDate = function(){
		return startFixedDate ? new Date(startFixedDate.getTime()) : null;	
	}

	var startFixedTime = null;
	model.setStartFixedTime = function(tm){
		startFixedTime = tm;	
		(startFixedDate !== null) && startFixedDate.setHours(tm);
		onChange();
	}
	model.getStartFixedTime = function(){
		return (startFixedDate === null) ? startFixedTime : startFixedDate.getHours();	
	}
	
	var startRepeatingType = null;
	model.setStartRepeatingType = function(rt){
		startRepeatingType = Number(rt);	
		onChange();
	}
	model.getStartRepeatingType = function(){
		return startRepeatingType;	
	}
	
	var startRepeatingDate = null;
	model.setStartRepeatingDate = function(dt){
		startRepeatingDate = dt;	
		onChange();
	}
	model.getStartRepeatingDate = function(){
		return startRepeatingDate;	
	}
	
	var startRepeatingDay = null;
	model.setStartRepeatingDay = function(dy){
		startRepeatingDay = dy;	
		onChange();
	}
	model.getStartRepeatingDay = function(){
		return startRepeatingDay;	
	}
	
	var startRepeatingTime = null;
	model.setStartRepeatingTime = function(tm){
		startRepeatingTime = tm;	
		onChange();
	}
	model.getStartRepeatingTime = function(){
		return startRepeatingTime;
	}
	
	var startRollingUnits = null;
	model.setStartRollingUnits = function(ru){
		startRollingUnits = Number(ru);	
		onChange();
	}
	model.getStartRollingUnits = function(){
		return startRollingUnits;	
	}
	
	var startRollingAmount = '';
	model.setStartRollingAmount = function(ra){
		startRollingAmount = ra;	
		onChange();
	}
	model.getStartRollingAmount = function(){
		return startRollingAmount;
	}
	
	var timesType = null;
	model.setTimesType = function(tt){
		timesType = tt;	
		onChange();
	}
	model.getTimesType = function(){
		return timesType;	
	}
	
	var periods = {};
	var nextPeriodId = 0, periodCount = 0;
	model.addPeriod = function(arrDays, timeFrom, timeTo){
		var id = nextPeriodId++;
		periods[id] = [arrDays, timeFrom, timeTo];
		periodCount++;
		onChange();
	};
	model.removePeriod = function(id){
		delete periods[id];
		periodCount--;
		onChange();
	};
	model.removePeriods = function(){
		periods = {};
		periodCount = 0;
		onChange();
	};
	model.getPeriods = function(){
		return periods;	
	}
	
	model.isComplete = function(){
		return this.isDirectionComplete() && this.isAmountComplete() && this.isStartComplete() && this.isTimesComplete() && this.isNameComplete();
	};
	model.isDirectionComplete = function(){
		return !!direction;
	};
	model.isAmountComplete = function(){
		return !!amount;
	};
	model.isStartComplete = function(){
		if (startType === START_TYPE_FIXED){
			return startFixedDate !== null;
			
		} else if (startType === START_TYPE_REPEATING){
			if (startRepeatingType === START_REPEATING_MONTH){
				return (startRepeatingDate !== null) && (startRepeatingTime !== null);
				
			} else if (startRepeatingType === START_REPEATING_WEEK){
				return (startRepeatingDay !== null) && (startRepeatingTime !== null);
				
			} else if (startRepeatingType === START_REPEATING_DAY){
				return startRepeatingTime !== null;
				
			} else {
				assert(startRepeatingType === null);
			}
			
		} else if (startType === START_TYPE_ROLLING){
			return startRollingUnits && startRollingAmount && Number(startRollingAmount);
			
		} else {
			assert(startType === null);
		}
	};
	model.isTimesComplete = function(){
		if (timesType === TIMES_SOME){
			return periodCount > 0;
		} else {
			return (timesType !== null);
		}
	};
	
	var name;
	model.setName = function(nm){
		name = nm;	
		onChange();
	};
	model.getName = function(){
		return name;
	};
	
	model.isNameComplete = function(){
		return !!name;
	};

	return model;
}());

createAlertModel.setChangeHandler(function(){updateCreateAlertViewFromModel(true);});

function resetAlertView(){
	$('#createAlertBoxAccordion').accordion('destroy').accordion(accordionOptions);

 // Direction
	$('input:radio[name=createAlertDirn]').attr('checked','');
	
 // Amount
	$('#createAlertAmount').val('');
	$('#createAlertAmountDesc').html('');
	
 // Start
	$('input:radio[name=createAlertStart]').attr('checked','');
	$('#createAlertStartFixedDetails,#createAlertStartRepeatingDetails,#createAlertStartRollingDetails').hide();
	
 // Start - Fixed
	$('#createAlertStartFixedDate').val('');
	$('#createAlertStartFixedTime').val('');
	
 // Start - Repeating
	$('input:radio[name=createAlertStartRepeatingType]').attr('checked','');
	$('#createAlertStartRepeatingDateBox').hide();
	$('#createAlertStartRepeatingDayBox').hide();
	$('#createAlertStartRepeatingTimeBox').hide();
	$('#createAlertStartRepeatingDate').val('');
	$('#createAlertStartRepeatingDay').val('');
	$('#createAlertStartRepeatingTime').val('');
	
 // Start - Rolling
 	$('#createAlertStartRollingAmount').val('');
 	$('#createAlertStartRollingUnits').val('');
 	
 // Periods
 	$('input:radio[name=createAlertPeriod]').attr('checked','');
 	$('#createAlertWeekdays input:checkbox').attr('checked','');
 	$('#createAlertPeriodList tbody').html('<tr><td colspan="3">No periods added yet</td></tr>');	
	$('#createAlertPeriodTimes').slider('values', 0,  6);
	$('#createAlertPeriodTimes').slider('values', 1, 18);
	$('#createAlertPeriodSomeDetails').hide();
	
 // Name
 	$('#createAlertName').val('');
 	
}

function updateCreateAlertViewFromModel(isUserEdit){
 // If no direction has been selected then populate it
 	var dirn = createAlertModel.getDirection();
	if (dirn){//if (!$('input:radio[name=createAlertDirn]:checked').val() && dirn){
		$('input:radio[name=createAlertDirn][value=' + dirn + ']').attr('checked', 'checked');
	} else {//if (!dirn) {
		$('input:radio[name=createAlertDirn]').attr('checked', '');
	}

 // If no amount has been entered then enter one
 	var amt = createAlertModel.getAmount();
 	if (!isUserEdit && (amt !== null)){
 		$('#createAlertAmount').val(formatAmount(amt, true));
 	} else if (amt === null) {
 		$('#createAlertAmount').val('');
 	}
 
 // If no start type has been set then set one
 	var start = createAlertModel.getStartType();
	if (start){//!$('input:radio[name=createAlertStart]:checked').val() && start){
		$('input:radio[name=createAlertStart][value=' + start + ']').attr('checked', 'checked');
	} else {//if (!start) {
		$('input:radio[name=createAlertStart]').attr('checked', '');
		$('#createAlertStartFixedDate').datepicker('setDate', null);
		$('#createAlertStartFixedTime').val(null);
		$('input:radio[name=createAlertStartRepeatingType]').attr('checked', '');
		$('#createAlertStartRepeatingDate,#createAlertStartRepeatingDay,#createAlertStartRepeatingTime').val(null);
		$('#createAlertStartRepeatingDateBox,#createAlertStartRepeatingDayBox,#createAlertStartRepeatingTimeBox').hide();
		$('#createAlertStartRollingAmount').val('');
		$('#createAlertStartRollingUnits').val(null);
	}
	
 // Display the correct input fields and values according to which start type was selected
	$('.createAlertStartDetails').hide();
	switch (start){
		case START_TYPE_FIXED:
			$('#createAlertStartFixedDetails').show();
			
			$('#createAlertStartFixedDate').datepicker('setDate', createAlertModel.getStartFixedDate());
			$('#createAlertStartFixedTime').val(createAlertModel.getStartFixedTime());
			break;
			
		case START_TYPE_REPEATING:
			$('#createAlertStartRepeatingDetails').show();
			
			var repeatingStartType = createAlertModel.getStartRepeatingType();
			if (repeatingStartType){//(!$('input:radio[name=createAlertStartRepeatingType]:checked').val() && repeatingStartType){		
				$('input:radio[name=createAlertStartRepeatingType][value=' + repeatingStartType + ']').attr('checked', 'checked');
			}
			
			if (repeatingStartType){
				switch(repeatingStartType){
					case START_REPEATING_MONTH:
						$('#createAlertStartRepeatingDateBox,#createAlertStartRepeatingTimeBox').show();
						//$('#createAlertStartRepeatingDate,#createAlertStartRepeatingTime').change();
						$('#createAlertStartRepeatingDayBox').hide();
						break;
						
					case START_REPEATING_WEEK:
						$('#createAlertStartRepeatingDayBox,#createAlertStartRepeatingTimeBox').show();
						//$('#createAlertStartRepeatingDay,#createAlertStartRepeatingTime').change();
						$('#createAlertStartRepeatingDateBox').hide();
						break;
						
					case START_REPEATING_DAY:
						$('#createAlertStartRepeatingTimeBox').show();
						//$('#createAlertStartRepeatingTime').change();
						$('#createAlertStartRepeatingDayBox,#createAlertStartRepeatingDateBox').hide();
						break;
						
					default:
						assert(false, "Bad repeatingStartType: " + repeatingStartType);
						break;
				}
			}

			var startRepeatingDate = createAlertModel.getStartRepeatingDate();
			if (startRepeatingDate){//(!$('#createAlertStartRepeatingDate').val() && startRepeatingDate){
				$('#createAlertStartRepeatingDate').val(startRepeatingDate);
			}

			var startRepeatingDay  = createAlertModel.getStartRepeatingDay();
			if (startRepeatingDay){//(!$('#createAlertStartRepeatingDay').val() && startRepeatingDay){
				$('#createAlertStartRepeatingDay').val(startRepeatingDay);
			}

			var startRepeatingTime = createAlertModel.getStartRepeatingTime();
			if (startRepeatingTime){//(!$('#createAlertStartRepeatingTime').val() && (startRepeatingTime !== null)){
				$('#createAlertStartRepeatingTime').val(startRepeatingTime);
			}

			break;

		case START_TYPE_ROLLING:
			$('#createAlertStartRollingDetails').show();
			
			var startRollingAmt = createAlertModel.getStartRollingAmount();
			if (startRollingAmt){//(!$('#createAlertStartRollingAmount').val() && startRollingAmt){
				$('#createAlertStartRollingAmount').val(startRollingAmt);
			}
			
			var startRollingUnits = createAlertModel.getStartRollingUnits();
			
			if (startRollingUnits){//(!$('#createAlertStartRollingUnits').val() && startRollingUnits){
				$('#createAlertStartRollingUnits').val(startRollingUnits);
			}
			break;

		default:
			assert(start === null, "Bad start type: " + start);
			break;
	}
 	
 
 // If no name has been entered then enter one
 	var name = createAlertModel.getName();
 	if (name !== null){//!$('#createAlertName').val() && (name !== null)){
 		$('#createAlertName').val(name);
 	} else {//if (name === null){
 		$('#createAlertName').val('');
 	}

 // Set the Period values
 	var timesType = createAlertModel.getTimesType();
 	if (timesType){//!$('input:radio[name=createAlertPeriod]:checked').val() && timesType) {
	 	if (timesType === TIMES_ALL) {
			$('#createAlertPeriodAll').attr('checked', 'checked');	
		} else {
			assert (timesType == TIMES_SOME, "Bad periods type: " + timesType);
			$('#createAlertPeriodSome').attr('checked', 'checked');
		}
	}
	
	if (timesType){
	 	if (timesType === TIMES_ALL) {
			$('#createAlertPeriodSomeDetails').hide();
			
		} else {
			assert (timesType == TIMES_SOME, "Bad periods type: " + timesType);
			$('#createAlertPeriodSomeDetails').show();

			function describeDays(arrDays){
				if (arraysEqual(ARR_ALL_DAYS, arrDays)){
					return "All Days";	
				} else if (arraysEqual(ARR_WEEKDAYS, arrDays)){
					return "Weekdays";
				} else if (arraysEqual(ARR_WEEKENDS, arrDays)){
					return "Weekends";
				} else {
					var days = [];
					var dayNames = $.datepicker.regional[''].dayNamesMin;
					$.each(arrDays, function(i,o){
						o && days.push(dayNames[i]);
					});
					
					return days.join(',');	
				}
			}
			$('#createAlertPeriodList tbody').html('');
			var periodCount = 0;
			$.each(createAlertModel.getPeriods(), function(k,v){
				var tr = $('<tr></tr>');
				var td1 = $('<td>' + describeDays(v[0]) + '</td>');
				var td2 = $('<td>' + v[1] + '-' + v[2] + '</td>');
				var td3 = $('<td></td>');
				var rmLink = $('<a>Remove</a>');
				rmLink.click(function(){
					createAlertModel.removePeriod(k);
				});
				td3.append(rmLink);
				
				tr.append(td1);
				tr.append(td2);
				tr.append(td3);
				
				$('#createAlertPeriodList tbody').append(tr);
				periodCount++;
			});
			if (periodCount === 0){
				$('#createAlertPeriodList tbody').html('<tr><td colspan="3">No periods added yet</td></tr>');	
				$('#createAlertWeekdays input').attr('checked', '');
				$('#createAlertPeriodTimes').slider('values', 0,  6);
				$('#createAlertPeriodTimes').slider('values', 1, 18);
			}						
		}
	}
	updateCompletionStatus();
	
	$('#createAlertSave').unbind('click');
	if (createAlertModel.isComplete()){
		$('#createAlertSave').removeClass('saveDisabled');
		$('#createAlertSave').click(function(){
			function makePeriodTxt(yr, mn, dy, wk, hr){
				return "['" + yr + "','" + mn + "','" + dy + "','" + wk + "','" + hr + "']";	
			}
			function makeBoundParam(){
				var fixedDate, fixedTime;
				var repeatingDate, repeatingWeekday, repeatingHour;
				var yr, mn, dy, wk, hr;
				
				switch (createAlertModel.getStartType()) {
					case START_TYPE_FIXED:
						fixedDate = createAlertModel.getStartFixedDate();
						fixedTime = createAlertModel.getStartFixedTime();
						
						yr = fixedDate.getUTCFullYear();
						mn = fixedDate.getUTCMonth() + 1;
						dy = fixedDate.getUTCDate();
						wk = '*';
						hr = fixedDate.getUTCHours();
						
						break;
						
					case START_TYPE_REPEATING:
						yr = '*';
						mn = '*';
						hr = createAlertModel.getStartRepeatingTime();
						
						switch (createAlertModel.getStartRepeatingType()){
							case START_REPEATING_MONTH:
								dy = createAlertModel.getStartRepeatingDate();
								wk = '*';
								break;
								
							case START_REPEATING_WEEK:
								dy = '*';
								wk = createAlertModel.getStartRepeatingDay();
								break;
								
							case START_REPEATING_DAY:
								wk = '*';
								dy = '*';
								break;
								
							default:
								assert(false, "Unexpected getStartRepeatingType(): " + createAlertModel.getStartRepeatingType());
								break;
						}
						break;	
					
					case START_TYPE_ROLLING:
						yr = '*';
						wk = '*';
						
						switch (createAlertModel.getStartRollingUnits()){
							case START_ROLLING_MONTHS:
								mn = '-' + createAlertModel.getStartRollingAmount();
								dy = '-0';
								hr = '-0';
								break;
									
							case START_ROLLING_DAYS:
								mn = '*';
								dy = '-' + createAlertModel.getStartRollingAmount();
								hr = '-0';
								break;

							case START_ROLLING_HOURS:
								mn = '*';
								dy = '*';
								hr = '-' + createAlertModel.getStartRollingAmount();
								break;
								
							default:
								assert(false, 'Unexpected getStartRollingUnits(): ' + createAlertModel.getStartRollingUnits());
								break;
						}
						break;
					
					default:
						assert(false, "Unexpected getStartType(): " + createAlertModel.getStartType());
						break;
				}
				
				return makePeriodTxt(yr, mn, dy, wk, hr);
			}
			function makePeriodsParam(){
				var periods = [];
				if (createAlertModel.getTimesType() === TIMES_SOME){
					$.each(createAlertModel.getPeriods(), function(i,o){
						var wk = [];
						$.each(o[0], function(i,o){
							o && wk.push(i);
						});
						var hr = o[1] + '-' + (o[2] - 1); // Subtract 1 from upper bound, '8am until 9am' means '8:xx'
						
						periods.push(makePeriodTxt('*', '*', '*', wk.join(','), hr));
					});
				}
				return "[" + periods.join(",") + "]";
			}
			
			var url;
			if (createAlertModel.getId()){
				url = 'alert?action=update&id=' + createAlertModel.getId();	
			} else {
				url = 'alert?action=create';
			}
			
			
			url += '&name=' + createAlertModel.getName();
			url += '&active=1';
			url += '&direction=' + createAlertModel.getDirection();
			url += '&amount=' + createAlertModel.getAmount();
			url += '&bound=' + makeBoundParam();
			url += '&periods=' + makePeriodsParam();
			
			$.get(url, function(){
				hideCreateAlertBox();
				tabShowAlerts();
			});   
		});
	} else {
		$('#createAlertSave').addClass('saveDisabled');
	}
}

function updateCompletionStatus(){
	function setStatusIndicator(isComplete, id){
		var o = $('#' + id);
		//o.addClass(isComplete ? 'ui-icon-check' : 'ui-icon-close');
		//o.removeClass(isComplete ? 'ui-icon-close' : 'ui-icon-check');
		if (isComplete){
			o.find('a').css('color', '');
		} else {
			o.find('a').css('color', 'red');
		}
	}
	setStatusIndicator(createAlertModel.isDirectionComplete(), "dirnHeader"); //"createAlertStatusDir");
	setStatusIndicator(createAlertModel.isAmountComplete(),    "amtHeader"); //"createAlertStatusAmt");
	setStatusIndicator(createAlertModel.isStartComplete(),     "startHeader"); //"createAlertStatusStart");
	setStatusIndicator(createAlertModel.isTimesComplete(),     "timesHeader"); //"createAlertStatusTimes");
	setStatusIndicator(createAlertModel.isNameComplete(),      "nameHeader"); //"createAlertStatusName");
	
	var completePercentage = (createAlertModel.isDirectionComplete() ? 20 : 0) + 
		(createAlertModel.isAmountComplete() ? 20 : 0) + 
		(createAlertModel.isStartComplete() ? 20 : 0) + 
		(createAlertModel.isTimesComplete() ? 20 : 0) + 
		(createAlertModel.isNameComplete() ? 20 : 0);
	
	$('#createAlertPercentageComplete').html(completePercentage + '%')
	if (completePercentage === 100){
		$('#createAlertPercentageComplete').css('color', '');
	} else {
		$('#createAlertPercentageComplete').css('color', 'red');	
	}
	
}
function hideCreateAlertBox(){
	$("#createAlertBox").fadeOut();
	$("#modalBackground").hide();
}

function showAlertEditor(reset){
	reset && createAlertModel.reset();
	
	var bg = $("#modalBackground");
	bg.show();
	
	var box = $("#createAlertBox");
	box.fadeIn("slow");
	box.css("left", ($(window).width() - box.width() ) / 2 + $(window).scrollLeft() + "px");

	resetAlertView();
	updateCreateAlertViewFromModel(false);
}
var accordionOptions = {
	autoHeight : false, 
	clearStyle: true,
	change : function(){
		$('#createAlertBoxAccordion h3').each(function(o){
			updateCompletionStatus();
		});
	}
};

$(document).ready(function(){
 // Show the Help dialog box when the help link is clicked
	var alertHelpDialog = $('#alerts .dialog').dialog(dialogOpts);
	$('#alertHelpLink').click(function(){
			alertHelpDialog.dialog("open");
		});
   
	$('#createAnotherAlert, #createFirstAlert').click(function(){
		showAlertEditor(true);
	});
	$('#createAlertBoxAccordion').accordion(accordionOptions);
	$('#updateAlerts').click(updateAlertProgressBars);
   
   	$('#createAlertBoxAccordion input[name=createAlertDirn]').click(function(){
   		createAlertModel.setDirection($(this).val());
   	});
   	
   	$('#createAlertBoxAccordion input[name=createAlertStart]').click(function(e){
   		createAlertModel.setStartType($(this).val());
	})
	$('#createAlertStartFixed').click(function(){
		$('#createAlertStartFixedTime').change();
	});
	$('#createAlertStartRolling').click(function(){
		$('#createAlertStartRollingUnits').change();
	});
	
	$('#createAlertStartFixedDate').datepicker({'onClose' : function(txt,dp){
		var dt = $(this).datepicker('getDate');
		createAlertModel.setStartFixedDate(dt);
	}});
	$('#createAlertStartFixedTime').change(function(){
		createAlertModel.setStartFixedTime($(this).val());
	});
	
	
	$('#createAlertBoxAccordion input[name=createAlertStartRepeatingType]').click(function(){
		createAlertModel.setStartRepeatingType($(this).val());
	});
	
	$('#createAlertStartRepeatingDate').change(function(){
		createAlertModel.setStartRepeatingDate($(this).val());
	});
	$('#createAlertStartRepeatingDay').change(function(){
		createAlertModel.setStartRepeatingDay($(this).val());
	});
	$('#createAlertStartRepeatingTime').change(function(){
		createAlertModel.setStartRepeatingTime($(this).val());
	});
	
	$('#createAlertStartRollingAmount').keypress(makeKeyPressHandler(0,8,48,49,50,51,52,53,54,55,56,57));
	$('#createAlertStartRollingAmount').keyup(function(){
		createAlertModel.setStartRollingAmount($(this).val());
	});
	$('#createAlertStartRollingUnits').change(function(){
		createAlertModel.setStartRollingUnits($(this).val());
	});
	
	
	$('#createAlertPeriodAll').click(function(){
		createAlertModel.setTimesType(TIMES_ALL);
	});
	$('#createAlertPeriodSome').click(function(){
		createAlertModel.setTimesType(TIMES_SOME);
	});
	
	$('#createAlertWeekdaysLabels td').each(function(i,o){
		$(o).html($.datepicker.regional[''].dayNamesMin[i]);
	});
	function setCreateAlertDays(arrDays){
		$.each(arrDays, function(i,o){
			$('#createAlertPeriodWeekday' + i).attr('checked', o ? 'checked' : '');
		});	
	}
	function getCreateAlertDays(){
		var i, arr=[];
		for(i=0; i<7; i++){
			arr.push($('#createAlertPeriodWeekday' + i).attr('checked') ? 1 : 0);
		}
		return arr;
	}
	
	$('#createAlertPeriodDaysAll').click(function(){
		setCreateAlertDays(ARR_ALL_DAYS);
	});
	$('#createAlertPeriodDaysNone').click(function(){
		setCreateAlertDays([0,0,0,0,0,0,0]);
	});
	$('#createAlertPeriodDaysWeekends').click(function(){
		setCreateAlertDays(ARR_WEEKENDS);
	});
	$('#createAlertPeriodDaysWeekdays').click(function(){
		setCreateAlertDays(ARR_WEEKDAYS);
	});
	
	$('#createAlertPeriodAddBtn').click(function(){
		var arrDays  = getCreateAlertDays();
		var timeFrom = sliderDiv.slider('values', 0);
		var timeTo   = sliderDiv.slider('values', 1);
		
		if ($.inArray(1, arrDays) === -1){
			showError('You must select at least 1 day before adding this interval to the list');
		} else if (timeFrom >= timeTo){
			showError('You must select a time interval covering at least 1 hour before adding this interval to the list');
		} else {
			createAlertModel.addPeriod(arrDays, timeFrom, timeTo);
		}
	});
	
	function displaySliderTimes(t1,t2){
		$('#createAlertPeriodTimesTxt').html(zeroPad(t1) + ':00 - ' +zeroPad(t2) + ':00');
	}
	var sliderDiv = $('#createAlertPeriodTimes');
	sliderDiv.slider({
	    animate: true,
	    range: true,
	    values: [6,18],
	    orientation : 'horizontal',
	    min: 0,
	    max: 24,
	    change: function(event, ui) {
	            displaySliderTimes(ui.values[0], ui.values[1]);
	        },
	    slide: function(event, ui) {
	            displaySliderTimes(ui.values[0], ui.values[1]);
	        }
    });
    displaySliderTimes(sliderDiv.slider('values', 0), sliderDiv.slider('values', 1));
    
	
	$('#createAlertBoxAccordion select.timeList').each(function(i,o){
		var opts = [];
		var i;
		
		for(i=0; i<24; i++){
			opts.push('<option value="' + i + '">' + zeroPad(i) + ':00</option>');	
		}
		$(o).html(opts.join(''));
	});

	$('#createAlertBoxAccordion select.dateList').each(function(i,o){
		var opts = [];
		var i;
		
		for(i=1; i<32; i++){
			opts.push('<option value="' + i + '">' + i + '</option>');	
		}
		$(o).html(opts.join(''));
	});
	
	$('#createAlertBoxAccordion select.dayList').each(function(i,o){
		var opts = [];
		var days = $.datepicker.regional[''].dayNames;
		var i;
		
		for(i=0; i<7; i++){
			opts.push('<option value="' + i + '">' + days[i] + '</option>');	
		}
		$(o).html(opts.join(''));
	});

   	$('#createAlertAmount').keyup(function(){
   		var txtValue = $('#createAlertAmount').val();
   		var bytes = parseAmountValue(txtValue);
   		bytes = bytes || 0;
   		var amountTxt = formatAmount(bytes);
   		$('#createAlertAmountDesc').html(amountTxt);
   		createAlertModel.setAmount(bytes);
	});
	
	$('#createAlertName').keyup(function(){
		var name = $('#createAlertName').val();
   		createAlertModel.setName(name);
	});
   
    $('#createAlertCancel').click(hideCreateAlertBox);
    $('#createAlertSave').button();
	$('#createAlertCancel').button();

});