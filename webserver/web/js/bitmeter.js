// The 'model' contains the application state
var model = buildModel();

// Manages the automatic screen refreshes
var refreshTimer;

// Gets called to signal to the current tab that it is being closed
var onTabHide;

var errorDialog;
var WEEKDAYS, MONTHS;

// Prevent AJAX GET requests from being cached on IE
$.ajaxSetup({
    cache: false,
    timeout: 30000,
    error : function (xhrObj, errType) {
    		var msg;
    		if (errType === "error"){
    			if (xhrObj.status === 0){
	    			msg = "Unable to contact the server, is it still running?";
    			} else {
    				msg = "An HTTP error occurred: " + xhrObj.status + " " + xhrObj.statusText ;
    			}
    			
    		} else if (errType === "timeout"){
	    		msg = "The server is taking a long time to respond - this may be caused by a slow network (if you are monitoring remotely) or very high CPU load.";
	    		
	    	} else if (errType === "parsererror"){
		    	msg = "Unable to parse the server response";
		    	
    		} else {
			    msg = "An error occurred while attempting to communicate with the server: " + errType;
    		}
			showError(msg);
		},
	dataFilter : function(data, dataType){
		hideError();
		return data;
	}
});

// Help dialogs use these config values
var dialogOpts = {
	autoOpen : false,
	buttons :  { "OK" : function() { $(this).dialog("close"); }},
	height: 250,
	width: 400
};

// Used to parse server responses into JS objects
function doEval(objTxt){
	try{
		return eval('(' + objTxt + ')');
	} catch (e){
		return {};
	}
}

// Helpful when developing
var assert = (function(){
	var DO_ASSERT = true;
	return function(expr, msg){
		if (DO_ASSERT && !expr){
			msg = msg || 'Assertion Error';
			alert(msg);
			throw msg;	
		}
	};
})();

// Get current time in seconds
function getTime(){
	return Math.floor((new Date()).getTime() / 1000);
}

// Add leading zero to values less than 10
function zeroPad(val){
    if (val === 0){
        return '00';
	} else if (val < 10){
		return '0' + val;
	} else {
		return '' + val;
	}
}

function addAdaptersToRequest(req){
	if (model.getAdapters()){
		if (req.indexOf('?') < 0){
			req += '?';
		} else {
			req += '&';
		}
		req += ('ha=' + model.getAdapters());
	}
	return req;
}

var errDialogVisible = false;
function showError(msg){
	var bg = $("#modalBackground");
	bg.show();
	
	var box = $("#errorDialog");
	box.fadeIn("slow");
	box.css("left", ( $(window).width() - box.width() ) / 2 + $(window).scrollLeft() + "px");
	$("#errorDialog #msg").html(msg);	
	errDialogVisible = true;
}
function hideError(){
	if (errDialogVisible){
		errDialogVisible = false;
		$("#errorDialog").fadeOut("slow");
		$("#modalBackground").hide();
	}
}

function getBytesPerK(){
    return model.getBinaryUnits() ? 1024 : 1000;   
}
var FORMAT_INTERVAL_TINY  = 0;
var FORMAT_INTERVAL_SHORT = 1;
var FORMAT_INTERVAL_LONG  = 2;

var formatInterval = (function(){
    var SECS_PER_MINUTE = 60;
    var SECS_PER_HOUR   = 60 * 60;
    
    return function(intervalInSec, compactness){
        var rem = intervalInSec;
        
        var sec  = intervalInSec % 60;
        rem -= sec;
        
        var min  = (rem / 60) % 60;
        rem -= min * 60;
        
        var hour = (rem / 3600) % 24;
        rem -= hour * 3600;
        
        var day  = (rem / 86400);

        if (compactness === FORMAT_INTERVAL_TINY){
            return zeroPad(hour) + ':' + zeroPad(min) + ':' + zeroPad(Math.round(sec));
            
        } else if (compactness === FORMAT_INTERVAL_SHORT){
            if (intervalInSec === 0){
                return '0 sec';   
                
            } else if (intervalInSec < 1){
                return '<1 sec';   
                
            } else {
                var result = '';
                if (day > 0){
                    result += day + 'd ';
                }
                if (hour > 0){
                    result += hour + 'h ';
                }
                if (min > 0){
                    result += min + 'm ';
                }
                if (sec > 0){
                    result += Math.round(sec) + 's';
                }                     
                return result;           
            }
            
		} else if (compactness === FORMAT_INTERVAL_LONG){
            if (intervalInSec === 0){
                return '0 sec';   
            } else if (intervalInSec < 1){
                return '<1 sec';   
            } else {
                var result = '';
                if (day > 0){
                    result += (day + ' day' + (day===1 ? '' : 's') + ' ');
                }
                if (hour > 0){
                    result += (hour + ' hour' + (hour===1 ? '' : 's') + ' ');
                }
                if (min > 0){
                    result += (min + ' minute' + (min===1 ? '' : 's') + ' ');
                }
                if (sec > 0){
                	var roundedSec = Math.round(sec); 
                    result += (roundedSec + ' second' + (roundedSec===1 ? '' : 's'));
                }                     
                return result;           
            }            
        } else {
        	assert(false, 'Bad compactness value: ' + compactness);
        }
    };
})();

// Convert an integer UL/DL value into a 2-dp floating point value with 2 letter abbreviation
var formatAmount = (function(){
	var K = getBytesPerK();
	var KB_MIN = K;
	var MB_MIN = KB_MIN * K;
	var GB_MIN = MB_MIN * K;
	var TB_MIN = GB_MIN * K;
	var PB_MIN = TB_MIN * K;
	var EB_MIN = PB_MIN * K;

	return function (amt){
		var numAmt, units;
		if (amt < KB_MIN){
			numAmt = amt.toFixed(2);
			units = 'B';
		} else if (amt < MB_MIN){
			numAmt = (amt/KB_MIN).toFixed(2);
			units = 'kB';
		} else if (amt < GB_MIN){
			numAmt = (amt/MB_MIN).toFixed(2);
			units = 'MB';
		} else if (amt < TB_MIN){
			numAmt = (amt/GB_MIN).toFixed(2);
			units = 'GB';
		} else if (amt < PB_MIN){
			numAmt = (amt/TB_MIN).toFixed(2);
			units = 'TB';
		} else {
			numAmt = (amt/PB_MIN).toFixed(2);
			units = 'PB';
		}
		return numAmt + ' ' + units;
	};
})();

// Adjust the vertical scale of a graph
var applyScale = (function(){
	var MIN_SCALE = 4;
	return function(graph, newScale){
		if (newScale >= MIN_SCALE){
			graph.getOptions().yaxis.max = newScale;
			graph.setupGrid();
			graph.draw();
			return true;
		} else {
			return false;
		}
	};
})();

var makeYAxisIntervalFn = (function(targetTickCount){
 /* Build a function that can be used to calculate vertical scale values that should be used on the graphs,
    we want the values to be nicely rounded (eg 350.00 kb/s not 357.23 kb/s) and to be able to vary how many
    we get for each graph, also needs to work at any scale. */
	return function(axis){
		var K = 1024;
		var max = axis.max;
		
	 // If we divide the scale up into exactly the requested number of parts, this is the interval we would get
		var exactGap = max / targetTickCount; 
		
	 /* Since we are displaying the values using abbreviations (eg 12kB not 12582912) we convert the exactGap value into 
	    an appropriate unit and then have a look at it. exactGapInUnits value will hold the exactGap in whatever units are 
	    appropriate (if the value is in the gigabyte range, it will be expressed in gigabytes)*/
		var exactGapUnits = Math.floor(Math.log(exactGap)/Math.log(K));
		var exactGapInUnits = exactGap / (Math.pow(K,exactGapUnits));
		
	 /* Now we need to find a value near to exactGapInUnits, but rounded acceptably. modValue is how much we subtract from 
	    the exact value to get the rounded value. */
		var modValue;
		if (exactGapInUnits < 1){
			modValue = 0;
		} else if (exactGapInUnits < 10){
			modValue = exactGapInUnits % 1;	
		} else {
			modValue = exactGapInUnits % 10;
		}
		
	 // This is how big the vertical scale interval will be, in the units we picked earlier
		roundedGap = exactGapInUnits - modValue;
		
	 // This is how big the vertical scale interval will be, in bytes
		roundedGap *= Math.floor(Math.pow(K,exactGapUnits));
		if (roundedGap == 0){
			roundedGap = 1;	
		}
		
	 // We need to return an array containing the scale values, so just keep adding roundedGap until we pass the graph maximum
		var ticks = [];
		var nextTick = 0;
		while (nextTick < max){
			ticks.push(nextTick);
			nextTick += roundedGap	
		}
		
		return ticks;
	};
});

$(document).ready(function(){
	 // Set up the event handlers for the tabs across the top of the screen
		$("#tabs").tabs({
				show: function(event, ui) {
					 // If there was a refresh timer active for the previous tab then clear it
						refreshTimer && clearInterval(refreshTimer);
						
					 // If there is any code that the current tab should run before it gets hidden then do it now
						onTabHide && onTabHide();
						onTabHide = null;
						
						var tabIndex = ui.index;
						if (tabIndex === 0){
							tabShowMonitor();
						} else if (tabIndex === 1){
							tabShowHistory();
						} else if (tabIndex === 2){
							tabShowSummary();
						} else if (tabIndex === 3){
							tabShowQuery();
						} else if (tabIndex === 4){
							tabShowCalc();
						} else if (tabIndex === 5){
							tabShowPrefs();
						}
					},
				cookie: { expires: 30 }
			});

	 // Display the app version on the About screen
		$('#version').html(config.version);
		
	 // If the host serving this page has a name then display it in the appropriate places
	 	if (config.serverName){
	 		var serverNameSuffix = ' - ' + config.serverName;
		 	$('h1').append(serverNameSuffix);
		 	document.title += serverNameSuffix;
		}
		
	 // External links open new windows
		$("a[href^='http'], #donateForm").attr('target','_blank');

		$("noscript").hide(); // Needed for IE8
		$("#errorDialog, #modalBackground").hide();
		
		$.getJSON("http://updates.codebox.org.uk/version/bitmeteros/version.js?callback=?", showVersion);

	});

function showVersion(data){
	function isNewVersion(currentVersion, newVersion){
		var currentVersionParts = currentVersion.split('.');
		var newVersionParts     = newVersion.split('.');
		if (currentVersionParts.length === 3 && newVersionParts.length === 3){
			function makeVersionPartsNum(parts){
				return (parts[0] * 100 * 100) + (parts[1] * 100) + parts[2];
			}
			var diff = makeVersionPartsNum(newVersionParts) - makeVersionPartsNum(currentVersionParts);
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
}


