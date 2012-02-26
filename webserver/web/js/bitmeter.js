/*global $,BITMETER,window,config*/
/*jslint onevar: true, undef: true, nomen: true, eqeqeq: true, bitwise: true, regexp: true, newcap: true, immed: true, strict: false */

$.ajaxSetup({
    cache: false,
    timeout: 30000,
    error : function (xhrObj, errType) {
            var msg;
            if (errType === "error") {
                if (xhrObj.status === 0) {
                    msg = "Unable to contact the server, is it still running?";
                } else {
                    msg = "An HTTP error occurred: " + xhrObj.status + " " + xhrObj.statusText;
                }

            } else if (errType === "timeout") {
                msg = "The server is taking a long time to respond - this may be caused by a slow network (if you are monitoring remotely) or very high CPU load.";

            } else if (errType === "parsererror") {
                msg = "Unable to parse the server response";

            } else {
                msg = "An error occurred while attempting to communicate with the server: " + errType;
            }
            BITMETER.errorDialog.show(msg);
        },
    dataFilter : function (data, dataType) {
        if (dataType !== 'script' && !data){
         // If the server goes down we get an empty 'data' parameter
            throw "No data returned";	
        }
        BITMETER.errorDialog.hide();
        return data;
    }
});

BITMETER.refreshTimer = (function(){
 // Manages the automatic screen refreshes
    var timer = {}, interval;

    timer.clear = function(){
        if (interval){
            window.clearInterval(interval);
        }
        interval = null;
    };

    timer.set = function(fnDoThis, periodInMillis){
        this.clear();
        interval = window.setInterval(fnDoThis, periodInMillis);
    };

    return timer;
}());

// Gets called to signal to the current tab that it is being closed
BITMETER.onTabHide = (function(){
    var onTabHide = {}, fnDoThis;

    onTabHide.set = function(newFn){
        fnDoThis = newFn;
    };

    onTabHide.run = function(){
     // Run the handler and then remove it, only want these to get executed once
        if (fnDoThis){
            fnDoThis();
        }
        this.set(null);
    };

    return onTabHide;
}());

BITMETER.infoFloat = (function(){
 /* Returns an object representing the small hovering box that appears near the mouse
    pointer in certain areas of the page (eg when hovering over the bars on the History
    page). */
    var float = {}, guiBox, floaterVisible, hoverElement;

    float.setHTML = function(html){
        guiBox.html(html);
    };

    function mouseMoveHandler(e){
        guiBox.css({'left' : e.pageX + 15, 'top' : e.pageY });
    }

    float.show = function(el, evObj, html){
        if (!floaterVisible){
            hoverElement = el;
            if (html){
                this.setHTML(html);
            }

            var cOffset = hoverElement.offset();

            this.getBox(); // make sure we have a reference to the DOM element
            guiBox.css({'left' : evObj.clientX - cOffset.left, 'top' : evObj.clientY - cOffset.top});
            guiBox.fadeIn(200);

            hoverElement.bind('mousemove', mouseMoveHandler);

            floaterVisible = true;
        }
    };

    float.hide = function(){
        if (floaterVisible){
            guiBox.fadeOut(200);

            hoverElement.unbind('mousemove', mouseMoveHandler);

            floaterVisible = false;
            hoverElement = null;
        }
    };

    float.isDisplayed = function(){
        return floaterVisible;
    };

    float.getBox = function(){
        return guiBox ? guiBox : (guiBox = $('#floater'));
    };

    return float;
}());

// Help dialogs use these config values
BITMETER.consts.dialogOpts = {
    autoOpen : false,
    buttons :  { "OK" : function() { $(this).dialog("close"); }},
    height: 250,
    width: 400
};

// Helpful when developing
BITMETER.assert = (function(){
    var DO_ASSERT = true;
    return function(expr, msg){
        if (DO_ASSERT && !expr) {
            msg = msg || 'Assertion Error';
            window.alert(msg);
            throw msg;
        }
    };
}());

BITMETER.getTime = function(){
 // Get current time in seconds
    return Math.floor((new Date()).getTime() / 1000);
};

BITMETER.makeKeyPressHandler = function(){
    var allowed = [];
    $.each(arguments, function(n,o){
        if (typeof o === 'number') {
            allowed.push(o);
            
        } else if (typeof o === 'string') {
            if ((o.length === 3) && (o.charAt(1) === '-')) {
                var i, from = o.charCodeAt(0), to = o.charCodeAt(2);
                BITMETER.assert(from<=to, 'makeKeyPressHandler from=' + from + ' to=' + to);

                for (i=from; i<=to; i++) {
                    allowed.push(i);
                }
                
            } else {
                BITMETER.assert(false, 'makeKeyPressHandler arg=' + o);
            }
        }
    });

    return function(evt){
        if ($.inArray(evt.which, allowed) < 0) {
            evt.preventDefault();
        }
    };
};

// Add leading zero to values less than 10
BITMETER.zeroPad = function(val){
    if (val === 0) {
        return '00';
    } else if (val < 10) {
        return '0' + val;
    } else {
        return '' + val;
    }
};

BITMETER.arraysEqual = function(arr1, arr2) {
    if (arr1.length !== arr2.length) {
        return false;
    } else {
        var i, l=arr1.length;
        for (i=0; i<l; i++) {
            if (arr1[i] !== arr2[i]) {
                return false;
            }
        }
        return true;
    }
};

BITMETER.addAdaptersToRequest = function(req){
    if (BITMETER.model.getAdapters()) {
        if (req.indexOf('?') < 0){
            req += '?';
        } else {
            req += '&';
        }
        req += ('ha=' + BITMETER.model.getAdapters());
    }
    return req;
};

BITMETER.confirmDialog = (function(){
    var dialog = {};
    
    dialog.show = function(msg, fnOnYes, fnOnNo){
        var bg = $("#dialogModalBackground"), box = $("#confirmDialog");
        function hideDialog(){
            $('#confirmYes, #confirmNo').unbind('click');
            bg.hide();
            box.fadeOut();
        }

        bg.show();

        box.fadeIn("slow");
        box.css("left", ($(window).width() - box.width() ) / 2 + $(window).scrollLeft() + "px");

        $('#confirmMsg').html(msg);

        $('#confirmYes').click(function(){
            hideDialog();
            if (fnOnYes) {
                fnOnYes();
            }
        });
        $('#confirmNo').click(function(){
            hideDialog();
            if (fnOnNo) {
                fnOnNo();
            }
        });
    };

    return dialog;
}());

BITMETER.errorDialog = (function(){
    var errDialogVisible, dialog = {}, fnOnClose;
    
    dialog.show = function(msg, onClose){
        if (!errDialogVisible){
            errDialogVisible = true;
            
            var bg = $("#dialogModalBackground"), box = $("#errorDialog");
            bg.show();
            box.fadeIn("slow");
            box.css("left", ( $(window).width() - box.width() ) / 2 + $(window).scrollLeft() + "px");

            $("#errorDialog button").click(dialog.hide);
        }
        
     // Calling 'show' when the dialog is already displayed will update the message
        $("#errorDialog #msg").html(msg);
        fnOnClose = onClose;
    };

    dialog.hide = function(){
        if (errDialogVisible){
            $("#errorDialog").fadeOut("slow");
            $("#dialogModalBackground").hide();
            errDialogVisible = false;
            
            if (fnOnClose){
                fnOnClose();
            }
            fnOnClose = null;
        }
    };

    return dialog;
}());

BITMETER.parseAmountValue = (function(){
    var WHITESPACE_REGEX = /\s/g,
        NUM_REGEX        = /^\d[\.\d]*$/,
        WITH_UNITS_REGEX = /^\d[\.\d]*[kmgt]b$/;
        
    return function(txt){
        var bytesPerK = BITMETER.getBytesPerK(), 
            tmpTxt = txt.replace(WHITESPACE_REGEX, '').toLowerCase(),
            num, factor, unitsPart, numPart;
            
        if (NUM_REGEX.test(tmpTxt)) {
         // Just numbers, so this is a byte value
            num = Number(tmpTxt);
            return isNaN(num) ? null : num;

        } else if (WITH_UNITS_REGEX.test(tmpTxt)) {
            numPart = Number(tmpTxt.substring(0, tmpTxt.length-2));
            if (isNaN(numPart)){
                return null;
            }
            unitsPart = tmpTxt.substring(tmpTxt.length-2);

            if (unitsPart === 'kb') {
                factor = bytesPerK;
            } else if (unitsPart === 'mb') {
                factor = bytesPerK * bytesPerK;
            } else if (unitsPart === 'gb') {
                factor = bytesPerK * bytesPerK * bytesPerK;
            } else if (unitsPart === 'tb') {
                factor = bytesPerK * bytesPerK * bytesPerK * bytesPerK;
            } else {
                BITMETER.assert(false, 'In parseAmountValue(), value was ' + txt);
            }
            return numPart * factor;

        } else {
            return null;
        }
    };
}());
BITMETER.getBytesPerK = function (){
    return BITMETER.model.getBinaryUnits() ? 1024 : 1000;
};

BITMETER.formatInterval = function(intervalInSec, compactness){
    var sec, min, hour, day, result, rem = intervalInSec, roundedSec;

    sec  = intervalInSec % 60;
    rem -= sec;

    min  = (rem / 60) % 60;
    rem -= min * 60;

    hour = (rem / 3600) % 24;
    rem -= hour * 3600;

    day  = (rem / 86400);

    if (compactness === BITMETER.formatInterval.TINY) {
        return BITMETER.zeroPad(hour) + ':' + BITMETER.zeroPad(min) + ':' + BITMETER.zeroPad(Math.round(sec));
        
    } else if (compactness === BITMETER.formatInterval.SHORT) {
        if (intervalInSec === 0) {
            return '0 sec';
            
        } else if (intervalInSec < 1) {
            return '<1 sec';
            
        } else {
            result = '';
            if (day > 0) {
                result += day + 'd ';
            }
            if (hour > 0) {
                result += hour + 'h ';
            }
            if (min > 0) {
                result += min + 'm ';
            }
            if (sec > 0) {
                result += Math.round(sec) + 's';
            }
            return result;
        }
        
    } else if (compactness === BITMETER.formatInterval.LONG){
        if (intervalInSec === 0) {
            return '0 sec';
            
        } else if (intervalInSec < 1) {
            return '<1 sec';
            
        } else {
            if (day > 0) {
                result += (day + ' day' + (day===1 ? '' : 's') + ' ');
            }
            if (hour > 0) {
                result += (hour + ' hour' + (hour===1 ? '' : 's') + ' ');
            }
            if (min > 0) {
                result += (min + ' minute' + (min===1 ? '' : 's') + ' ');
            }
            if (sec > 0) {
                roundedSec = Math.round(sec);
                result += (roundedSec + ' second' + (roundedSec===1 ? '' : 's'));
            }
            return result;
        }
    } else {
        BITMETER.assert(false, 'Bad compactness value: ' + compactness);
    }
};

BITMETER.formatInterval.TINY  = 0;
BITMETER.formatInterval.SHORT = 1;
BITMETER.formatInterval.LONG  = 2;

// Convert an integer UL/DL value into a 2-dp floating point value with 2 letter abbreviation
BITMETER.formatAmount = (function(){
    var K = BITMETER.getBytesPerK(),
        KB_MIN = K,
        MB_MIN = KB_MIN * K,
        GB_MIN = MB_MIN * K,
        TB_MIN = GB_MIN * K,
        PB_MIN = TB_MIN * K;

    return function (amt, hideDp){
        var numAmt, units, dp = hideDp ? 0 : 2;
        if (amt < KB_MIN) {
            numAmt = amt.toFixed(dp);
            units = 'B';
        } else if (amt < MB_MIN) {
            numAmt = (amt/KB_MIN).toFixed(dp);
            units = 'kB';
        } else if (amt < GB_MIN) {
            numAmt = (amt/MB_MIN).toFixed(dp);
            units = 'MB';
        } else if (amt < TB_MIN) {
            numAmt = (amt/GB_MIN).toFixed(dp);
            units = 'GB';
        } else if (amt < PB_MIN) {
            numAmt = (amt/TB_MIN).toFixed(dp);
            units = 'TB';
        } else {
            numAmt = (amt/PB_MIN).toFixed(dp);
            units = 'PB';
        }
        return numAmt + ' ' + units;
    };
}());

// Adjust the vertical scale of a graph
BITMETER.applyScale = (function(){
    var MIN_SCALE = 4;
    return function(graph, newScale){
        if (newScale >= MIN_SCALE) {
            graph.getAxes().yaxis.options.max = newScale;
            graph.setupGrid();
            graph.draw();
            return true;
        } else {
            return false;
        }
    };
}());

BITMETER.makeYAxisIntervalFn = function(targetTickCount){
 /* Build a function that can be used to calculate vertical scale values that should be used on the graphs,
    we want the values to be nicely rounded (eg 350.00 kb/s not 357.23 kb/s) and to be able to vary how many
    we get for each graph, also needs to work at any scale. */
    return function(axis){
        var K = 1024, max = axis.max, exactGap, exactGapUnits, exactGapInUnits, modValue, ticks, nextTick, roundedGap;

     // If we divide the scale up into exactly the requested number of parts, this is the interval we would get
        exactGap = max / targetTickCount;

     /* Since we are displaying the values using abbreviations (eg 12kB not 12582912) we convert the exactGap value into
        an appropriate unit and then have a look at it. exactGapInUnits value will hold the exactGap in whatever units are
        appropriate (if the value is in the gigabyte range, it will be expressed in gigabytes)*/
        exactGapUnits = Math.floor(Math.log(exactGap)/Math.log(K));
        exactGapInUnits = exactGap / (Math.pow(K,exactGapUnits));

     /* Now we need to find a value near to exactGapInUnits, but rounded acceptably. modValue is how much we subtract from
        the exact value to get the rounded value. */
        if (exactGapInUnits < 1) {
            modValue = 0;
        } else if (exactGapInUnits < 10) {
            modValue = exactGapInUnits % 1;
        } else {
            modValue = exactGapInUnits % 10;
        }

     // This is how big the vertical scale interval will be, in the units we picked earlier
        roundedGap = exactGapInUnits - modValue;

     // This is how big the vertical scale interval will be, in bytes
        roundedGap *= Math.floor(Math.pow(K,exactGapUnits));
        if (roundedGap === 0) {
            roundedGap = 1;
        }

     // We need to return an array containing the scale values, so just keep adding roundedGap until we pass the graph maximum
        ticks = [];
        nextTick = 0;
        while (nextTick < max) {
            ticks.push(nextTick);
            nextTick += roundedGap;
        }

        return ticks;
    };
};

$(function(){
 // Set up the event handlers for the tabs across the top of the screen
    $("#tabs").tabs({
            show: function(event, ui) {
                 // If there was a refresh timer active for the previous tab then clear it
                    BITMETER.refreshTimer.clear();

                 // If there is any code that the current tab should run before it gets hidden then do it now
                    BITMETER.onTabHide.run();
                    
                    var tabIndex = ui.index;
                    if (tabIndex === 0) {
                        BITMETER.tabShowMonitor();
                    } else if (tabIndex === 1) {
                        BITMETER.tabShowHistory();
                    } else if (tabIndex === 2) {
                        BITMETER.tabShowSummary();
                    } else if (tabIndex === 3) {
                        BITMETER.tabShowQuery();
                    } else if (tabIndex === 4) {
                        BITMETER.tabShowAlerts();
                    } else if (tabIndex === 5) {
                        BITMETER.tabShowCalc();
                    } else if (tabIndex === 6) {
                       BITMETER.tabShowPrefs();
                    }
                },
            cookie: { expires: 30 }
        });

 // Display the app version on the About screen
    $('#version').html(config.version);

    BITMETER.setAdminOnlyHandlers();

 // If the host serving this page has a name then display it in the appropriate places
    if (config.serverName) {
        var serverNameSuffix = ' - ' + config.serverName;
        $('h1').append(serverNameSuffix);
        window.document.title += serverNameSuffix;
    }

	BITMETER.restoreDocTitle = (function(title){
		 // Create a function that knows what the original title of the page was, and will restore it when called
		    return function(){window.document.title = title;};
		}(window.document.title));

 // External links open new windows
    $("a[href^='http'], #donateForm, .popup").attr('target','_blank');

    $("noscript").hide(); // Needed for IE8
    $("#errorDialog, #dialogModalBackground").hide();
    
    var datePickerFormat = $('#createAlertStartFixedDate').datepicker("option", "dateFormat");
    $('#createAlertStartDetailsFormat').html("(" + datePickerFormat + ")");
    
    $.ajax({
        url : "http://updates.codebox.org.uk/version/bitmeteros/version2.js", 
        dataType : 'script'
    });
});

BITMETER.showVersion = function(data){
    function isNewVersion(currentVersion, newVersion){
        var currentVersionParts = currentVersion.split('.'),
            newVersionParts     = newVersion.split('.'),
            diff;
        function makeVersionPartsNum(parts){
            return (parts[0] * 100 * 100) + (parts[1] * 100) + parts[2];
        }
            
        if (currentVersionParts.length === 3 && newVersionParts.length === 3) {
            diff = makeVersionPartsNum(newVersionParts) - makeVersionPartsNum(currentVersionParts);
            return diff > 0;

        } else {
            return false;
        }
    }
    if (isNewVersion(config.version, data.number)){
        $('#newVersion h2').append(data.number);
        $('#newVersionDownload').attr('href', data.url);
        $("#tabs").tabs("add", "#newVersion", "New Version");
        $('#newVersionInfo').html(data.description);
        $('#version').show();
        $('#newVersion').show();

        $('a[href=#newVersion]').effect("pulsate", { times: 10 }, 4000);
    }
};

BITMETER.setAdminOnlyHandlers = function(){
 // Disable admin functions if this client is not allowed to perform them (don't worry, they get blocked server-side as well)
    if (!config.allowAdmin) {
        $('.adminOnly').addClass('forbidden');
        $('a.adminOnly, button.adminOnly').unbind('click').click(function(e){
            BITMETER.errorDialog.show('This feature cannot be used when accessing the web interface remotely.<br><br> ' +
                'To use this feature you must either access the interface locally (using <em>localhost</em> ' +
                'as the host name in the url), or change the access permissions to allow remote ' +
                'administrative access.<br><br>');
        });
        $('button.adminOnly').button('option', 'disabled', true);
    }
};
BITMETER.playAudio = (function(){
    if (BITMETER.model.getAudioSupport()){
        var wavCache = {};
        return function(id){
            if (!wavCache[id]) {
                wavCache[id] = {'play':$.noop};
                wavCache[id] = new Audio("audio/" + id + ".wav");
            }
            wavCache[id].play();
        };
    } else {
        return $.noop;
    }
})();
