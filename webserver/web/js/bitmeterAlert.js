/*global $,BITMETER,window,config*/
/*jslint sloppy: true, white: true, plusplus: true, unparam: true */

BITMETER.tabShowAlerts = function(){
    function deleteAlert(alert){
        BITMETER.confirmDialog.show("Delete " + alert.name + " - are you sure?", function(){
            $.get('alert?action=delete&id=' + alert.id, BITMETER.tabShowAlerts);
        });
    }

    function editAlert(alertObj){
        BITMETER.resetAlertView();
        BITMETER.createAlertModel.reset();

        var yr, mn, dy, hr, isRollingMonths, isRollingDays, isRollingHours,
            fixedDate, hasWeekday, hasMonthDay;

        BITMETER.createAlertModel.setId(alertObj.id);
        BITMETER.createAlertModel.setAmount(alertObj.amount);
        BITMETER.createAlertModel.setFilter(alertObj.filter);

        function isRolling(txt){
            return txt.indexOf('-') === 0;
        }

     // Set start date/time - we need to work out which of the 3 types of Start this represents
        if (alertObj.bound[0] !== '*'){
         // The year is '*' so this must be a 'fixed' start date/time
            BITMETER.createAlertModel.setStartType(BITMETER.consts.alerts.startTypeFixed);
            yr = Number(alertObj.bound[0]);
            mn = Number(alertObj.bound[1]);
            dy = Number(alertObj.bound[2]);
            hr = Number(alertObj.bound[4]);
            BITMETER.assert(!isNaN(yr) && !isNaN(mn) && !isNaN(dy) && !isNaN(hr));
            fixedDate = new Date(Date.UTC(yr, mn-1, dy, hr));
            BITMETER.createAlertModel.setStartFixedDate(fixedDate);
            BITMETER.createAlertModel.setStartFixedTime(fixedDate.getHours());

        } else {
            isRollingMonths = isRolling(alertObj.bound[1]);
            isRollingDays   = isRolling(alertObj.bound[2]);
            isRollingHours  = isRolling(alertObj.bound[4]);

            if (isRollingMonths || isRollingDays || isRollingHours){
             // Either the month, day or hour value begins with a '-', so this must be a 'rolling' start date/time
                BITMETER.assert(alertObj.bound[0] === '*' && alertObj.bound[3] === '*');
                BITMETER.createAlertModel.setStartType(BITMETER.consts.alerts.startTypeRolling);

                if (isRollingMonths){
                    BITMETER.createAlertModel.setStartRollingAmount(-Number(alertObj.bound[1]));
                    BITMETER.createAlertModel.setStartRollingUnits(BITMETER.consts.alerts.startRollingMonths);

                } else if (isRollingDays){
                    BITMETER.createAlertModel.setStartRollingAmount(-Number(alertObj.bound[2]));
                    BITMETER.createAlertModel.setStartRollingUnits(BITMETER.consts.alerts.startRollingDays);

                } else if (isRollingHours){
                    BITMETER.createAlertModel.setStartRollingAmount(-Number(alertObj.bound[4]));
                    BITMETER.createAlertModel.setStartRollingUnits(BITMETER.consts.alerts.startRollingHours);

                } else {
                    BITMETER.assert(false, "isRollingMonths/isRollingDays/isRollingHours");
                }

            } else {
             // By process of elimination, this should be a 'repeating' start date/time - do some checks to make sure
                BITMETER.assert(alertObj.bound[0] === '*' && alertObj.bound[1] === '*', "Repeating start, year/month not wildcards");
                BITMETER.assert(!isNaN(alertObj.bound[4]), "Repeating start, hour not a number");
                BITMETER.createAlertModel.setStartType(BITMETER.consts.alerts.startTypeRepeating);

                hasWeekday  = (alertObj.bound[3] !== '*');
                hasMonthDay = (alertObj.bound[2] !== '*');
                if (hasWeekday){
                    BITMETER.assert(!hasMonthDay, "Repeating start, hasWeekday and hasMonthDay");
                    BITMETER.createAlertModel.setStartRepeatingType(BITMETER.consts.alerts.startRepeatingWeek);
                    BITMETER.createAlertModel.setStartRepeatingDay(alertObj.bound[3]);

                } else if (hasMonthDay){
                    BITMETER.createAlertModel.setStartRepeatingType(BITMETER.consts.alerts.startRepeatingMonth);
                    BITMETER.createAlertModel.setStartRepeatingDate(alertObj.bound[2]);

                } else {
                    BITMETER.createAlertModel.setStartRepeatingType(BITMETER.consts.alerts.startRepeatingDay);

                }
                BITMETER.createAlertModel.setStartRepeatingTime(alertObj.bound[4]);
            }
        }

     // Set the periods
        if (alertObj.periods && alertObj.periods.length) {
            BITMETER.createAlertModel.setTimesType(BITMETER.consts.alerts.timesSome);
            $.each(alertObj.periods, function(i,o){
                BITMETER.assert((o[0] === '*') && (o[1] === '*') && (o[2] === '*'));
                var daysTxt = o[3].split(','),
                    days    = [0,0,0,0,0,0,0],
                    parts   = o[4].split('-'),
                    fromHours, toHours;

                fromHours = Number(parts[0]);
                toHours   = parts.length > 1 ? Number(parts[1]) : fromHours;

                $.each(daysTxt, function(i,o){
                    days[Number(o)] = 1;
                });

                BITMETER.createAlertModel.addPeriod(days, fromHours, toHours + 1); // Add 1 to the upper bound, '8-8' (or just '8') covers range 8am to 9am, so display it as 8-9
            });

        } else {
            BITMETER.createAlertModel.setTimesType(BITMETER.consts.alerts.timesAll);
        }

        BITMETER.createAlertModel.setName(alertObj.name);
        BITMETER.showAlertEditor();
    }

    $.get('alert?action=list', function(alertsArray){
        if (alertsArray && alertsArray.length){
            $('#noAlerts').hide();
            $('#alertsDisplay').show();

            var tbody = $('#alertsTable tbody');
            tbody.html('');

            $.each(alertsArray, function(i,o){
                var tr           = $('<tr id="alertRow' + o.id + '"><td class="alertName">' + o.name + '</td></tr>'),
                    tdProgress   = $('<td></td>'),
                    progress     = $('<div class="alertProgress" id="progress' + o.id + '"></div>'),
                    tdStatus     = $('<td class="alertStatus" id="status' + o.id + '"></td>'),
                    tdStatusIcon = $('<td class="alertStatusIcon" id="statusIcon' + o.id + '"></td>'),                    tdLink     = $('<td class="alertLink"></td>'),
                    editLink     = $('<a class="adminOnly">Edit</a>'),
                    tdDelete     = $('<td class="alertLink adminOnly"></td>'),
                    deleteLink   = $('<a class="adminOnly deleteAlertLink">Delete</a>');

                progress.progressbar();
                tdProgress.append(progress);
                tr.append(tdProgress);

                tdStatus.html('');
                tr.append(tdStatus);

                tr.append(tdStatusIcon);

                editLink.click(function(){editAlert(o);});
                tdLink.append(editLink);
                tr.append(tdLink);

                deleteLink.bind('click', function(){deleteAlert(o);});
                tdDelete.append(deleteLink);
                tr.append(tdDelete);

                tbody.append(tr);
            });
            BITMETER.updateAlertProgressBars();

        } else {
            $('#alertsDisplay').hide();
            $('#noAlerts').show();
        }
    });
    
    BITMETER.refreshTimer.set(BITMETER.updateAlertProgressBars, BITMETER.model.getAlertsRefresh());      
};

BITMETER.updateAlertProgressBars = function(){
    function setHoverStatus(icon, name, target, current){
        icon.unbind('hover');
        icon.hover(function (evObj){
                var floaterHtml;
                if (current > target){
                    floaterHtml = '<h4>' + name + '</h4>' +
                        '<p>This alert has been triggered</p>' +
                        '<table>' +
                        '<tr><td class="alertStatusFloaterName">Current</td><td class="alertStatusFloaterValue">' + BITMETER.formatAmount(current) + '</td></tr>' +
                        '<tr><td class="alertStatusFloaterName">Exceeded by</td><td class="alertStatusFloaterValue">' + BITMETER.formatAmount(current - target) + '</td></tr>' +
                        '<tr><td class="alertStatusFloaterName">Limit</td><td class="alertStatusFloaterValue">' + BITMETER.formatAmount(target) + '</td></tr>' +
                        '<tr><td class="alertStatusFloaterName">Progress</td><td class="alertStatusFloaterValue">' + (100 * current/target).toFixed(2) + '%</td></tr>' +
                        '</table>';
                    BITMETER.infoFloat.getBox().addClass('floaterErr');

                } else {
                    floaterHtml = '<h4>' + name + '</h4>' +
                        '<table>' +
                        '<tr><td class="alertStatusFloaterName">Current</td><td class="alertStatusFloaterValue">' + BITMETER.formatAmount(current) + '</td></tr>' +
                        '<tr><td class="alertStatusFloaterName">Remaining</td><td class="alertStatusFloaterValue">' + BITMETER.formatAmount(target - current) + '</td></tr>' +
                        '<tr><td class="alertStatusFloaterName">Limit</td><td class="alertStatusFloaterValue">' + BITMETER.formatAmount(target) + '</td></tr>' +
                        '<tr><td class="alertStatusFloaterName">Progress</td><td class="alertStatusFloaterValue">' + (100 * current/target).toFixed(2) + '%</td></tr>' +
                        '</table>';
                    BITMETER.infoFloat.getBox().removeClass('floaterErr');
                }
                BITMETER.infoFloat.show(icon, evObj, floaterHtml);

            }, function(){
                BITMETER.infoFloat.getBox().removeClass('floaterErr');
                BITMETER.infoFloat.hide();
            }
        );
    }

 // Loop once for each alert
    $.get('alert?action=status', function(jsonData){
        $.each(jsonData, function(i,o){
            var value = o.current / o.limit,
                barValue, statusTxt = (Math.round(value * 100) + '%');

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
    BITMETER.setAdminOnlyHandlers();
};

BITMETER.consts = {};
BITMETER.consts.alerts = {};

BITMETER.consts.alerts.startTypeFixed      = 1;
BITMETER.consts.alerts.startTypeRepeating  = 2;
BITMETER.consts.alerts.startTypeRolling    = 3;

BITMETER.consts.alerts.startRepeatingMonth = 1;
BITMETER.consts.alerts.startRepeatingWeek  = 2;
BITMETER.consts.alerts.startRepeatingDay   = 3;

BITMETER.consts.alerts.startRollingHours   = 1;
BITMETER.consts.alerts.startRollingDays    = 2;
BITMETER.consts.alerts.startRollingMonths  = 3;

BITMETER.consts.alerts.timesAll  = 1;
BITMETER.consts.alerts.timesSome = 2;

BITMETER.consts.alerts.arrAllDays  = [1,1,1,1,1,1,1];
BITMETER.consts.alerts.arrWeekends = [1,0,0,0,0,0,1];
BITMETER.consts.alerts.arrWeekdays = [0,1,1,1,1,1,0];

BITMETER.createAlertModel = (function(){
    var model = {}, onChangeCallback, id, amount = null, filter, startType = null, startFixedDate = null,
            startFixedTime = null, startRepeatingType = null, startRepeatingDate = null,
            startRepeatingDay = null, startRepeatingTime = null, startRollingUnits = null,
            startRollingAmount = '', timesType = null, periods = {}, nextPeriodId = 0, periodCount = 0, name;

    model.setChangeHandler = function(fnCallback){
        onChangeCallback = fnCallback;
    };

    function onChange(){
        onChangeCallback(model);
    }

    model.reset = function(){
        id = name = amount = filter = startType = startFixedDate = startFixedTime = startRepeatingType = startRepeatingDate = startRepeatingDay = startRepeatingTime = startRollingUnits = startRollingAmount = timesType = null;
        periods = {};
        nextPeriodId = periodCount = 0;
    };

    model.setId = function(i){
        id = i;
    };
    model.getId = function(){
        return id;
    };

    model.setAmount = function(amt){
        amount = amt;
        onChange();
    };
    model.getAmount = function(){
        return amount;
    };

    model.setFilter = function(fltr){
        filter = fltr;
        onChange();
    };
    model.getFilter = function(){
        return filter;
    };

    model.setStartType = function(st){
        startType = Number(st);
        onChange();
    };
    model.getStartType = function(){
        return startType;
    };

    model.setStartFixedDate = function(dt){
        startFixedDate = new Date(dt.getTime());
        if (startFixedTime !== null) {
            startFixedDate.setHours(startFixedTime);
        }
        onChange();
    };
    model.getStartFixedDate = function(){
        return startFixedDate ? new Date(startFixedDate.getTime()) : null;
    };

    model.setStartFixedTime = function(tm){
        startFixedTime = tm;
        if (startFixedDate !== null) {
            startFixedDate.setHours(tm);
        }
        onChange();
    };
    model.getStartFixedTime = function(){
        return (startFixedDate === null) ? startFixedTime : startFixedDate.getHours();
    };

    model.setStartRepeatingType = function(rt){
        startRepeatingType = Number(rt);
        onChange();
    };
    model.getStartRepeatingType = function(){
        return startRepeatingType;
    };

    model.setStartRepeatingDate = function(dt){
        startRepeatingDate = dt;
        onChange();
    };
    model.getStartRepeatingDate = function(){
        return startRepeatingDate;
    };

    model.setStartRepeatingDay = function(dy){
        startRepeatingDay = dy;
        onChange();
    };
    model.getStartRepeatingDay = function(){
        return startRepeatingDay;
    };

    model.setStartRepeatingTime = function(tm){
        startRepeatingTime = tm;
        onChange();
    };
    model.getStartRepeatingTime = function(){
        return startRepeatingTime;
    };

    model.setStartRollingUnits = function(ru){
        startRollingUnits = Number(ru);
        onChange();
    };
    model.getStartRollingUnits = function(){
        return startRollingUnits;
    };

    model.setStartRollingAmount = function(ra){
        startRollingAmount = ra;
        onChange();
    };
    model.getStartRollingAmount = function(){
        return startRollingAmount;
    };

    model.setTimesType = function(tt){
        timesType = tt;
        onChange();
    };
    model.getTimesType = function(){
        return timesType;
    };

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
    };

    model.isComplete = function(){
        return this.isFilterComplete() && this.isAmountComplete() && this.isStartComplete() && this.isTimesComplete() && this.isNameComplete();
    };
    model.isFilterComplete = function(){
        return !!filter;
    };
    model.isAmountComplete = function(){
        return !!amount;
    };
    model.isStartComplete = function(){
        if (startType === BITMETER.consts.alerts.startTypeFixed){
            return startFixedDate !== null;

        } else if (startType === BITMETER.consts.alerts.startTypeRepeating){
            if (startRepeatingType === BITMETER.consts.alerts.startRepeatingMonth){
                return (startRepeatingDate !== null) && (startRepeatingTime !== null);

            } else if (startRepeatingType === BITMETER.consts.alerts.startRepeatingWeek){
                return (startRepeatingDay !== null) && (startRepeatingTime !== null);

            } else if (startRepeatingType === BITMETER.consts.alerts.startRepeatingDay){
                return startRepeatingTime !== null;

            } else {
                BITMETER.assert(startRepeatingType === null);
            }

        } else if (startType === BITMETER.consts.alerts.startTypeRolling){
            return startRollingUnits && startRollingAmount && Number(startRollingAmount);

        } else {
            BITMETER.assert(startType === null);
        }
    };
    model.isTimesComplete = function(){
        if (timesType === BITMETER.consts.alerts.timesSome){
            return periodCount > 0;
        } else {
            return (timesType !== null);
        }
    };

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

BITMETER.createAlertModel.setChangeHandler(function(){BITMETER.updateCreateAlertViewFromModel(true);});

BITMETER.resetAlertView = function(){
    $('#createAlertBoxAccordion').accordion('destroy').accordion(BITMETER.accordionOptions);

 // Filter
    $('input:radio[name=createAlertFilter]').attr('checked','');

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

    $('#createAlertBox select').attr('selectedIndex', -1);
};

BITMETER.updateCreateAlertViewFromModel = function(isUserEdit){
    var fltr, amt, start, repeatingStartType, startRepeatingDate, startRepeatingDay, startRepeatingTime,
            startRollingAmt, startRollingUnits, name, timesType, periodCount;

 // If no filter has been selected then populate it
    fltr = BITMETER.createAlertModel.getFilter();
    if (fltr){
        $('input:radio[name=createAlertFltr][value=' + fltr + ']').attr('checked', 'checked');
    } else {
        $('input:radio[name=createAlertFltr]').attr('checked', '');
    }

 // If no amount has been entered then enter one
    amt = BITMETER.createAlertModel.getAmount();
    if (!isUserEdit && (amt !== null)){
        $('#createAlertAmount').val(BITMETER.formatAmount(amt, true));
    } else if (amt === null) {
        $('#createAlertAmount').val('');
    }

 // If no start type has been set then set one
    start = BITMETER.createAlertModel.getStartType();
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
        case BITMETER.consts.alerts.startTypeFixed:
            $('#createAlertStartFixedDetails').show();

            $('#createAlertStartFixedDate').datepicker('setDate', BITMETER.createAlertModel.getStartFixedDate());
            $('#createAlertStartFixedTime').val(BITMETER.createAlertModel.getStartFixedTime());
            break;

        case BITMETER.consts.alerts.startTypeRepeating:
            $('#createAlertStartRepeatingDetails').show();

            repeatingStartType = BITMETER.createAlertModel.getStartRepeatingType();
            if (repeatingStartType){//(!$('input:radio[name=createAlertStartRepeatingType]:checked').val() && repeatingStartType){
                $('input:radio[name=createAlertStartRepeatingType][value=' + repeatingStartType + ']').attr('checked', 'checked');
            }

            if (repeatingStartType){
                switch(repeatingStartType){
                    case BITMETER.consts.alerts.startRepeatingMonth:
                        $('#createAlertStartRepeatingDateBox,#createAlertStartRepeatingTimeBox').show();
                        //$('#createAlertStartRepeatingDate,#createAlertStartRepeatingTime').change();
                        $('#createAlertStartRepeatingDayBox').hide();
                        break;

                    case BITMETER.consts.alerts.startRepeatingWeek:
                        $('#createAlertStartRepeatingDayBox,#createAlertStartRepeatingTimeBox').show();
                        //$('#createAlertStartRepeatingDay,#createAlertStartRepeatingTime').change();
                        $('#createAlertStartRepeatingDateBox').hide();
                        break;

                    case BITMETER.consts.alerts.startRepeatingDay:
                        $('#createAlertStartRepeatingTimeBox').show();
                        //$('#createAlertStartRepeatingTime').change();
                        $('#createAlertStartRepeatingDayBox,#createAlertStartRepeatingDateBox').hide();
                        break;

                    default:
                        BITMETER.assert(false, "Bad repeatingStartType: " + repeatingStartType);
                        break;
                }
            }

            startRepeatingDate = BITMETER.createAlertModel.getStartRepeatingDate();
            if (startRepeatingDate){//(!$('#createAlertStartRepeatingDate').val() && startRepeatingDate){
                $('#createAlertStartRepeatingDate').val(startRepeatingDate);
            }

            startRepeatingDay  = BITMETER.createAlertModel.getStartRepeatingDay();
            if (startRepeatingDay){//(!$('#createAlertStartRepeatingDay').val() && startRepeatingDay){
                $('#createAlertStartRepeatingDay').val(startRepeatingDay);
            }

            startRepeatingTime = BITMETER.createAlertModel.getStartRepeatingTime();
            if (startRepeatingTime){//(!$('#createAlertStartRepeatingTime').val() && (startRepeatingTime !== null)){
                $('#createAlertStartRepeatingTime').val(startRepeatingTime);
            }

            break;

        case BITMETER.consts.alerts.startTypeRolling:
            $('#createAlertStartRollingDetails').show();

            startRollingAmt = BITMETER.createAlertModel.getStartRollingAmount();
            if (startRollingAmt){//(!$('#createAlertStartRollingAmount').val() && startRollingAmt){
                $('#createAlertStartRollingAmount').val(startRollingAmt);
            }

            startRollingUnits = BITMETER.createAlertModel.getStartRollingUnits();

            if (startRollingUnits){//(!$('#createAlertStartRollingUnits').val() && startRollingUnits){
                $('#createAlertStartRollingUnits').val(startRollingUnits);
            }
            break;

        default:
            BITMETER.assert(start === null, "Bad start type: " + start);
            break;
    }


 // If no name has been entered then enter one
    name = BITMETER.createAlertModel.getName();
    if (name !== null){//!$('#createAlertName').val() && (name !== null)){
        $('#createAlertName').val(name);
    } else {//if (name === null){
        $('#createAlertName').val('');
    }

 // Set the Period values
    timesType = BITMETER.createAlertModel.getTimesType();
    if (timesType){//!$('input:radio[name=createAlertPeriod]:checked').val() && timesType) {
        if (timesType === BITMETER.consts.alerts.timesAll) {
            $('#createAlertPeriodAll').attr('checked', 'checked');
        } else {
            BITMETER.assert (timesType === BITMETER.consts.alerts.timesSome, "Bad periods type: " + timesType);
            $('#createAlertPeriodSome').attr('checked', 'checked');
        }
    }

    function describeDays(arrDays){
        if (BITMETER.arraysEqual(BITMETER.consts.alerts.arrAllDays, arrDays)){
            return "All Days";
        } else if (BITMETER.arraysEqual(BITMETER.consts.alerts.arrWeekdays, arrDays)){
            return "Weekdays";
        } else if (BITMETER.arraysEqual(BITMETER.consts.alerts.arrWeekends, arrDays)){
            return "Weekends";
        } else {
            var days = [], dayNames = $.datepicker.regional[''].dayNamesMin;
            $.each(arrDays, function(i,o){
                if (o){
                    days.push(dayNames[i]);
                }
            });

            return days.join(',');
        }
    }

    if (timesType){
        if (timesType === BITMETER.consts.alerts.timesAll) {
            $('#createAlertPeriodSomeDetails').hide();

        } else {
            BITMETER.assert (timesType === BITMETER.consts.alerts.timesSome, "Bad periods type: " + timesType);
            $('#createAlertPeriodSomeDetails').show();

            $('#createAlertPeriodList tbody').html('');
            periodCount = 0;
            $.each(BITMETER.createAlertModel.getPeriods(), function(k,v){
                var tr = $('<tr></tr>'),
                    td1 = $('<td>' + describeDays(v[0]) + '</td>'),
                    td2 = $('<td>' + v[1] + '-' + v[2] + '</td>'),
                    td3 = $('<td></td>'),
                    rmLink = $('<a>Remove</a>');

                rmLink.click(function(){
                    BITMETER.createAlertModel.removePeriod(k);
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
    BITMETER.updateAlertCompletionStatus();

    $('#createAlertSave').unbind('click.save');
    if (BITMETER.createAlertModel.isComplete()){
        $('#createAlertSave').removeClass('saveDisabled');
        $('#createAlertSave').bind('click.save', function(){
            var url;
            
            function makePeriodTxt(yr, mn, dy, wk, hr){
                return "['" + yr + "','" + mn + "','" + dy + "','" + wk + "','" + hr + "']";
            }
            function makeBoundParam(){
                var fixedDate, fixedTime, yr, mn, dy, wk, hr;

                switch (BITMETER.createAlertModel.getStartType()) {
                    case BITMETER.consts.alerts.startTypeFixed:
                        fixedDate = BITMETER.createAlertModel.getStartFixedDate();
                        fixedTime = BITMETER.createAlertModel.getStartFixedTime();

                        yr = fixedDate.getUTCFullYear();
                        mn = fixedDate.getUTCMonth() + 1;
                        dy = fixedDate.getUTCDate();
                        wk = '*';
                        hr = fixedDate.getUTCHours();

                        break;

                    case BITMETER.consts.alerts.startTypeRepeating:
                        yr = '*';
                        mn = '*';
                        hr = BITMETER.createAlertModel.getStartRepeatingTime();

                        switch (BITMETER.createAlertModel.getStartRepeatingType()){
                            case BITMETER.consts.alerts.startRepeatingMonth:
                                dy = BITMETER.createAlertModel.getStartRepeatingDate();
                                wk = '*';
                                break;

                            case BITMETER.consts.alerts.startRepeatingWeek:
                                dy = '*';
                                wk = BITMETER.createAlertModel.getStartRepeatingDay();
                                break;

                            case BITMETER.consts.alerts.startRepeatingDay:
                                wk = '*';
                                dy = '*';
                                break;

                            default:
                                BITMETER.assert(false, "Unexpected getStartRepeatingType(): " + BITMETER.createAlertModel.getStartRepeatingType());
                                break;
                        }
                        break;

                    case BITMETER.consts.alerts.startTypeRolling:
                        yr = '*';
                        wk = '*';

                        switch (BITMETER.createAlertModel.getStartRollingUnits()){
                            case BITMETER.consts.alerts.startRollingMonths:
                                mn = '-' + BITMETER.createAlertModel.getStartRollingAmount();
                                dy = '-0';
                                hr = '-0';
                                break;

                            case BITMETER.consts.alerts.startRollingDays:
                                mn = '*';
                                dy = '-' + BITMETER.createAlertModel.getStartRollingAmount();
                                hr = '-0';
                                break;

                            case BITMETER.consts.alerts.startRollingHours:
                                mn = '*';
                                dy = '*';
                                hr = '-' + BITMETER.createAlertModel.getStartRollingAmount();
                                break;

                            default:
                                BITMETER.assert(false, 'Unexpected getStartRollingUnits(): ' + BITMETER.createAlertModel.getStartRollingUnits());
                                break;
                        }
                        break;

                    default:
                        BITMETER.assert(false, "Unexpected getStartType(): " + BITMETER.createAlertModel.getStartType());
                        break;
                }

                return makePeriodTxt(yr, mn, dy, wk, hr);
            }
            function makePeriodsParam(){
                var periods = [];
                if (BITMETER.createAlertModel.getTimesType() === BITMETER.consts.alerts.timesSome){
                    $.each(BITMETER.createAlertModel.getPeriods(), function(i,o){
                        var wk = [], hr;
                        $.each(o[0], function(i,o){
                            if (o){
                                wk.push(i);
                            }
                        });
                        hr = o[1] + '-' + (o[2] - 1); // Subtract 1 from upper bound, '8am until 9am' means '8:xx'

                        periods.push(makePeriodTxt('*', '*', '*', wk.join(','), hr));
                    });
                }
                return "[" + periods.join(",") + "]";
            }

            if (BITMETER.createAlertModel.getId()){
                url = 'alert?action=update&id=' + BITMETER.createAlertModel.getId();
            } else {
                url = 'alert?action=create';
            }


            url += '&name=' + BITMETER.createAlertModel.getName();
            url += '&active=1';
            url += '&filter=' + BITMETER.createAlertModel.getFilter();
            url += '&amount=' + BITMETER.createAlertModel.getAmount();
            url += '&bound=' + makeBoundParam();
            url += '&periods=' + makePeriodsParam();

            $.get(url, function(){
                BITMETER.hideCreateAlertBox();
                BITMETER.tabShowAlerts();
            });
        });
    } else {
        $('#createAlertSave').addClass('saveDisabled');
    }
};

BITMETER.updateAlertCompletionStatus = function(){
    function setStatusIndicator(isComplete, id){
        var o = $('#' + id);
        if (isComplete){
            o.find('a').css('color', '');
        } else {
            o.find('a').css('color', 'red');
        }
    }
    setStatusIndicator(BITMETER.createAlertModel.isFilterComplete(),    "fltrHeader"); //"createAlertStatusDir");
    setStatusIndicator(BITMETER.createAlertModel.isAmountComplete(),    "amtHeader"); //"createAlertStatusAmt");
    setStatusIndicator(BITMETER.createAlertModel.isStartComplete(),     "startHeader"); //"createAlertStatusStart");
    setStatusIndicator(BITMETER.createAlertModel.isTimesComplete(),     "timesHeader"); //"createAlertStatusTimes");
    setStatusIndicator(BITMETER.createAlertModel.isNameComplete(),      "nameHeader"); //"createAlertStatusName");

    var completePercentage = (BITMETER.createAlertModel.isFilterComplete() ? 20 : 0) +
        (BITMETER.createAlertModel.isAmountComplete() ? 20 : 0) +
        (BITMETER.createAlertModel.isStartComplete() ? 20 : 0) +
        (BITMETER.createAlertModel.isTimesComplete() ? 20 : 0) +
        (BITMETER.createAlertModel.isNameComplete() ? 20 : 0);

    $('#createAlertPercentageComplete').html(completePercentage + '%');
    if (completePercentage === 100){
        $('#createAlertPercentageComplete').css('color', '');
    } else {
        $('#createAlertPercentageComplete').css('color', 'red');
    }
};

BITMETER.hideCreateAlertBox = function(){
    $("#createAlertBox").fadeOut();
    $("#modalBackground").hide();
};

BITMETER.showAlertEditor = function(reset){
    if (reset){
        BITMETER.createAlertModel.reset();
    }   

    var bg = $("#modalBackground"),
        box = $("#createAlertBox");
        
    bg.show();

    box.fadeIn("slow");
    box.css("left", ($(window).width() - box.width() ) / 2 + $(window).scrollLeft() + "px");

    BITMETER.resetAlertView();
    BITMETER.updateCreateAlertViewFromModel(false);
};

BITMETER.accordionOptions = {
    autoHeight : false,
    clearStyle: true,
    change : function(){
        $('#createAlertBoxAccordion h3').each(function(o){
            BITMETER.updateAlertCompletionStatus();
        });
    }
};

$(function(){
 // Show the Help dialog box when the help link is clicked
    var alertHelpDialog = $('#alerts .dialog').dialog(BITMETER.consts.dialogOpts),
        sliderDiv = $('#createAlertPeriodTimes'), filterList;
        
    $('#alertHelpLink').click(function(){
            alertHelpDialog.dialog("open");
        });

    $('#createAnotherAlert, #createFirstAlert').click(function(){
        BITMETER.showAlertEditor(true);
    });
    
    filterList = $('#createAlertBoxFilterList');
    BITMETER.forEachFilter(function(filter){
        var chkBoxId, chkBox, filterLabel;
        chkBoxId = "alertFilterChk" + filter.id;
        chkBox = $('<input id="' + chkBoxId + '" name="createAlertFltr" value="' + filter.id + '" type="radio"></input>');
        filterLabel = $('<label for="' + chkBoxId + '">' + filter.desc + '</label>');

        chkBox.click(function(){
            BITMETER.createAlertModel.setFilter($(this).val());         
        });

        filterList.append(chkBox);
        filterList.append(filterLabel);
        filterList.append('<br>');
    });
    
    $('#createAlertBoxAccordion').accordion(BITMETER.accordionOptions);
    $('#updateAlerts').click(BITMETER.updateAlertProgressBars);

    $('#createAlertBoxAccordion input[name=createAlertStart]').click(function(e){
        BITMETER.createAlertModel.setStartType($(this).val());
    });
    $('#createAlertStartFixed').click(function(){
        $('#createAlertStartFixedTime').change();
    });
    $('#createAlertStartRolling').click(function(){
        $('#createAlertStartRollingUnits').change();
    });

    $('#createAlertStartFixedDate').datepicker({'onClose' : function(txt,dp){
        var dt = $(this).datepicker('getDate');
        BITMETER.createAlertModel.setStartFixedDate(dt);
    }});
    $('#createAlertStartFixedTime').change(function(){
        BITMETER.createAlertModel.setStartFixedTime($(this).val());
    });


    $('#createAlertBoxAccordion input[name=createAlertStartRepeatingType]').click(function(){
        BITMETER.createAlertModel.setStartRepeatingType($(this).val());
    });

    $('#createAlertStartRepeatingDate').change(function(){
        BITMETER.createAlertModel.setStartRepeatingDate($(this).val());
    });
    $('#createAlertStartRepeatingDay').change(function(){
        BITMETER.createAlertModel.setStartRepeatingDay($(this).val());
    });
    $('#createAlertStartRepeatingTime').change(function(){
        BITMETER.createAlertModel.setStartRepeatingTime($(this).val());
    });

    $('#createAlertStartRollingAmount').keypress(BITMETER.makeKeyPressHandler(0,8,'0-9'));
    $('#createAlertStartRollingAmount').keyup(function(){
        BITMETER.createAlertModel.setStartRollingAmount($(this).val());
    });
    $('#createAlertStartRollingUnits').change(function(){
        BITMETER.createAlertModel.setStartRollingUnits($(this).val());
    });


    $('#createAlertPeriodAll').click(function(){
        BITMETER.createAlertModel.setTimesType(BITMETER.consts.alerts.timesAll);
    });
    $('#createAlertPeriodSome').click(function(){
        BITMETER.createAlertModel.setTimesType(BITMETER.consts.alerts.timesSome);
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
        setCreateAlertDays(BITMETER.consts.alerts.arrAllDays);
    });
    $('#createAlertPeriodDaysNone').click(function(){
        setCreateAlertDays([0,0,0,0,0,0,0]);
    });
    $('#createAlertPeriodDaysWeekends').click(function(){
        setCreateAlertDays(BITMETER.consts.alerts.arrWeekends);
    });
    $('#createAlertPeriodDaysWeekdays').click(function(){
        setCreateAlertDays(BITMETER.consts.alerts.arrWeekdays);
    });

    $('#createAlertPeriodAddBtn').click(function(){
        var arrDays  = getCreateAlertDays(),
            timeFrom = sliderDiv.slider('values', 0),
            timeTo   = sliderDiv.slider('values', 1);

        if ($.inArray(1, arrDays) === -1){
            BITMETER.errorDialog.show('You must select at least 1 day before adding this interval to the list');
        } else if (timeFrom >= timeTo){
            BITMETER.errorDialog.show('You must select a time interval covering at least 1 hour before adding this interval to the list');
        } else {
            BITMETER.createAlertModel.addPeriod(arrDays, timeFrom, timeTo);
        }
    });

    function displaySliderTimes(t1,t2){
        $('#createAlertPeriodTimesTxt').html(BITMETER.zeroPad(t1) + ':00 - ' +BITMETER.zeroPad(t2) + ':00');
    }
    
 // Browser detection workaround below for http://bugs.jqueryui.com/ticket/6750   
    sliderDiv.slider({
        animate: !$.browser.msie,
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


    $('#createAlertBoxAccordion select.timeList').each(function(n,o){
        var i, opts = [];

        for(i=0; i<24; i++){
            opts.push('<option value="' + i + '">' + BITMETER.zeroPad(i) + ':00</option>');
        }
        $(o).html(opts.join(''));
    });

    $('#createAlertBoxAccordion select.dateList').each(function(n,o){
        var i, opts = [];

        for(i=1; i<32; i++){
            opts.push('<option value="' + i + '">' + i + '</option>');
        }
        $(o).html(opts.join(''));
    });

    $('#createAlertBoxAccordion select.dayList').each(function(n,o){
        var i, opts = [], days = $.datepicker.regional[''].dayNames;

        for(i=0; i<7; i++){
            opts.push('<option value="' + i + '">' + days[i] + '</option>');
        }
        $(o).html(opts.join(''));
    });

    $('#createAlertAmount').keyup(function(){
        var bytes, amountTxt, txtValue = $('#createAlertAmount').val();
        bytes = BITMETER.parseAmountValue(txtValue);
        bytes = bytes || 0;
        bytes = Math.round(bytes);
        amountTxt = BITMETER.formatAmount(bytes);
        $('#createAlertAmountDesc').html(amountTxt);
        BITMETER.createAlertModel.setAmount(bytes);
    });

    $('#createAlertName').keyup(function(){
        var name = $('#createAlertName').val();
        BITMETER.createAlertModel.setName(name);
    });

    $('#createAlertCancel').click(BITMETER.hideCreateAlertBox);
    $('#createAlertSave').button();
    $('#createAlertCancel').button();
});