function tabShowCalc(){
	setSliderRange(0, model.getMonitorScale());
}

function setSliderRange(min, max){
    var value = $('#calcSlider').slider('value');
    $('#calcSlider').slider('option', 'max', max);
    $('#calcSliderMax').html('- ' + formatAmount(max) + '/s');
    $('#calcSliderMin').html('- ' + formatAmount(min) + '/s');
    $('#calcSlider').slider('value', value); // need to reset the value so that the handle is positioned correctly
}

$(document).ready(function(){
    var bytesPerK = getBytesPerK();

    var sliderDiv = $('#calcSlider');
    sliderDiv.slider({
	    animate: true,
	    orientation : 'vertical',
	    min: 0,
	    max: model.getMonitorScale(),
	    change: function(event, ui) {
	            onSliderChange(ui.value);
	        },
	    slide: function(event, ui) {
	            onSliderChange(ui.value);
	        }
    }); 
    
    var calcSliderValue = $('#calcSliderValue');
    function onSliderChange(value){
        calcSliderValue.html(formatAmount(value) + '/s');
        useSliderForHowMuch && updateHowMuchResult();
        useSliderForHowLong && updateHowLongResult();
    }
    
    function getSliderValueInK(){
        return Math.round(sliderDiv.slider('value')/bytesPerK);
    }
    
    function parseSpeed(txt){
        var num = Number(txt);
        if (!isNaN(num)){
            return num * bytesPerK;   
        } else {
            return null;   
        }
    }

    var timeInput = $('#calcHowMuchTimeInput');
    var parseTimeValue = (function(){
        var WHITESPACE_REGEX = /\s/g;
        var DIGIT_REGEX      = /^\d$/;
        var NUM_REGEX        = /^\d+$/;
        
        return function(txt){
            var tmpTxt = txt.replace(WHITESPACE_REGEX, '').toLowerCase();
            
            if (NUM_REGEX.test(tmpTxt)){
             // Just numbers, so this is the number of seconds
                return Number(tmpTxt);
                
            } else {
                var c, i, len = tmpTxt.length;
                var numBuffer = '';
                var totalInSeconds = 0;
                for (i=0; i<len; i++){
                    c = tmpTxt[i];
                    if (DIGIT_REGEX.test(c)){
                        numBuffer += c;
                    } else {
                        if (!numBuffer){
                            return null;   
                        } else {
                            var num = Number(numBuffer);
                            if (c === 'd') {
                                totalInSeconds += (num * 3600 * 24);
                            } else if (c === 'h') {
                                totalInSeconds += (num * 3600);
                            } else if (c === 'm') {
                                totalInSeconds += (num * 60);
                            } else if (c === 's') {
                                totalInSeconds += num;
                            }                            
                            numBuffer = '';
                        }
                    }
                }
                return totalInSeconds;
            }
        };
    })();

    var howMuchSpeedInput = $('#calcHowMuchSpeedInput');
    var calcHowMuchResult = $('#calcHowMuchResult');
    var calcHowMuchDesc   = $('#calcHowMuchDesc');
    function updateHowMuchResult(){
        var gotTime = !!timeInput.val();
        var gotSpeed  = useSliderForHowMuch || howMuchSpeedInput.val();
        
        var result = '', desc = '';
        if (gotTime && gotSpeed){
            var time  = parseTimeValue(timeInput.val());
            var speed = (useSliderForHowMuch ? sliderDiv.slider('value') : parseSpeed(howMuchSpeedInput.val()));
            
            if (time !== null && speed !== null){
                result = formatAmount(time * speed);
                desc   = 'Transferred in ' + formatInterval(time, FORMAT_INTERVAL_LONG) + ' at ' + formatAmount(speed) + '/s';
            } else {
                result = '?';
                if (time === null){
                    desc = 'Did not understand the Time value. '
                }
                if (speed === null){
                    desc = (desc ? desc + '<br>' : '');
                    desc += 'Did not understand the Speed value. '
                }
            }
        }
        
        calcHowMuchResult.html(result);
        calcHowMuchDesc.html(desc);
    }
    
    var amountInput = $('#calcHowLongAmountInput');
    var parseAmountValue = (function(){
        var WHITESPACE_REGEX = /\s/g;
        var NUM_REGEX        = /^\d[\.\d]*$/;
        var WITH_UNITS_REGEX = /^\d[\.\d]*[kmgt]b$/;
        return function(txt){
            var tmpTxt = txt.replace(WHITESPACE_REGEX, '').toLowerCase();
            if (NUM_REGEX.test(tmpTxt)){
             // Just numbers, so this is a byte value
                var num = Number(tmpTxt);
                return isNaN(num) ? null : num;

            } else if (WITH_UNITS_REGEX.test(tmpTxt)) {
                var numPart   = Number(tmpTxt.substring(0, tmpTxt.length-2));
                if (isNaN(numPart)){
                    return null;    
                }
                var unitsPart = tmpTxt.substring(tmpTxt.length-2);
                var factor;
                if (unitsPart === 'kb'){
                    factor = bytesPerK;
                } else if (unitsPart === 'mb'){
                    factor = bytesPerK * bytesPerK;
                } else if (unitsPart === 'gb'){
                    factor = bytesPerK * bytesPerK * bytesPerK;
                } else if (unitsPart === 'tb'){
                    factor = bytesPerK * bytesPerK * bytesPerK * bytesPerK;
                } else {
                    assert(false, 'In parseAmountValue(), value was ' + txt);
                }
                return numPart * factor;
            } else {
                return null;   
            }
        };        
    })();
    var howLongSpeedInput = $('#calcHowLongSpeedInput');
    var calcHowLongResult = $('#calcHowLongResult');
    var calcHowLongDesc   = $('#calcHowLongDesc');
    function updateHowLongResult(){
        var gotAmount = !!amountInput.val();
        var gotSpeed  = useSliderForHowLong || howLongSpeedInput.val();

        var result = '', desc = '';        
        if (gotAmount && gotSpeed){
            var amount = parseAmountValue(amountInput.val());
            var speed = (useSliderForHowLong ? sliderDiv.slider('value') : parseSpeed(howLongSpeedInput.val()));
            
            if (amount !== null && speed !== null){
                if (speed === 0){
                    result = 'Never';    
                    desc   = 'Transfer will never complete when speed is 0';   
                } else {
                    result = formatInterval(amount/speed, FORMAT_INTERVAL_SHORT);
                    desc   = 'To transfer ' + formatAmount(amount) + ' at ' + formatAmount(speed) + '/s';
                }    
            } else {
                result = '?';
                if (amount === null){
                    desc = 'Did not understand the Amount value. '
                }
                if (speed === null){
                    desc = (desc ? desc + '<br>' : '');
                    desc += 'Did not understand the Speed value. '
                }
            }
        }
        
        calcHowLongResult.html(result);
        calcHowLongDesc.html(desc);
    }
    
    var useSliderForHowMuch = true;
    var useSliderForHowLong = true;
    
    $('a#calcShowHowMuchSpeedLink').click(function(){
        $('span#calcHowMuchSpeedInputSpan').show();
        $('span#calcHowMuchSpeedLinkSpan').hide();
        $('span#calcHowMuchSpeedInputSpan input').val(getSliderValueInK()).focus().select();
        useSliderForHowMuch = false;
    });
    $('a#calcShowHowLongSpeedLink').click(function(){
        $('span#calcHowLongSpeedInputSpan').show();
        $('span#calcHowLongSpeedLinkSpan').hide();
        $('span#calcHowLongSpeedInputSpan input').val(getSliderValueInK()).focus().select();
        useSliderForHowLong = false;
    });
    $('a#calcHideHowMuchSpeedLink').click(function(){
        $('span#calcHowMuchSpeedLinkSpan').show();
        $('span#calcHowMuchSpeedInputSpan').hide();
        useSliderForHowMuch = true;
        updateHowMuchResult();
    });
    $('a#calcHideHowLongSpeedLink').click(function(){
        $('span#calcHowLongSpeedLinkSpan').show();
        $('span#calcHowLongSpeedInputSpan').hide();
        useSliderForHowLong = true;
        updateHowLongResult();
    });

    $('#calcHowLongSpeedInput').keyup(function(e){
        updateHowLongResult();
    });
    $('#calcHowMuchSpeedInput').keyup(function(e){
        updateHowMuchResult();
    });
    amountInput.keyup(function(e){
        updateHowLongResult();
    });
    timeInput.keyup(function(e){
        updateHowMuchResult();
    });
    

    sliderDiv.slider('value', model.getMonitorScale()/2);
    
});