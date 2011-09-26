/*global $,BITMETER,window,config*/
/*jslint onevar: true, undef: true, nomen: true, eqeqeq: true, bitwise: true, regexp: true, newcap: true, immed: true, strict: false */

BITMETER.tabShowCalc = function(){
    function setSliderRange(min, max){
        var value = $('#calcSlider').slider('value');
        $('#calcSlider').slider('option', 'max', max);
        $('#calcSliderMax').html('- ' + BITMETER.formatAmount(max) + '/s');
        $('#calcSliderMin').html('- ' + BITMETER.formatAmount(min) + '/s');
        $('#calcSlider').slider('value', value); // need to reset the value so that the handle is positioned correctly
    }
    setSliderRange(0, BITMETER.model.getMonitorScale());
};

$(function(){
    var bytesPerK         = BITMETER.getBytesPerK(),
        sliderDiv         = $('#calcSlider'),
        calcSliderValue   = $('#calcSliderValue'),
        timeInput         = $('#calcHowMuchTimeInput'),
        howMuchSpeedInput = $('#calcHowMuchSpeedInput'),
        calcHowMuchResult = $('#calcHowMuchResult'),
        calcHowMuchDesc   = $('#calcHowMuchDesc'),
        amountInput       = $('#calcHowLongAmountInput'),
        howLongSpeedInput = $('#calcHowLongSpeedInput'),
        calcHowLongResult = $('#calcHowLongResult'),
        calcHowLongDesc   = $('#calcHowLongDesc'),
        parseTimeValue,
        useSliderForHowMuch = true,
        useSliderForHowLong = true;
    
    function parseSpeed(txt){
        var num = Number(txt);
        if (!isNaN(num)){
            return num * bytesPerK;   
        } else {
            return null;   
        }
    }

    parseTimeValue = (function(){
        var WHITESPACE_REGEX = /\s/g,
            DIGIT_REGEX      = /^\d$/,
            NUM_REGEX        = /^\d+$/;
        
        return function(txt){
            var num, totalInSeconds = 0, numBuffer = '', c, i, len, tmpTxt = txt.replace(WHITESPACE_REGEX, '').toLowerCase();
            
            if (NUM_REGEX.test(tmpTxt)){
             // Just numbers, so this is the number of seconds
                return Number(tmpTxt);
                
            } else {
                len = tmpTxt.length;
                for (i=0; i<len; i++){
                    c = tmpTxt[i];
                    if (DIGIT_REGEX.test(c)){
                        numBuffer += c;
                    } else {
                        if (!numBuffer){
                            return null;   
                        } else {
                            num = Number(numBuffer);
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
    }());
    
    function updateHowMuchResult(){
        var time, speed, result = '', desc = '',
            gotTime = !!timeInput.val(),
            gotSpeed  = useSliderForHowMuch || howMuchSpeedInput.val();
        
        if (gotTime && gotSpeed){
            time  = parseTimeValue(timeInput.val());
            speed = (useSliderForHowMuch ? sliderDiv.slider('value') : parseSpeed(howMuchSpeedInput.val()));
            
            if (time !== null && speed !== null){
                result = BITMETER.formatAmount(time * speed);
                desc   = 'Transferred in ' + BITMETER.formatInterval(time, BITMETER.formatInterval.LONG) + ' at ' + BITMETER.formatAmount(speed) + '/s';
            } else {
                result = '?';
                if (time === null){
                    desc = 'Did not understand the Time value. ';
                }
                if (speed === null){
                    desc = (desc ? desc + '<br>' : '');
                    desc += 'Did not understand the Speed value. ';
                }
            }
        }
        
        calcHowMuchResult.html(result);
        calcHowMuchDesc.html(desc);
    }
    
    function updateHowLongResult(){
        var amount, speed, result = '', desc = '', gotAmount = !!amountInput.val(),
            gotSpeed  = useSliderForHowLong || howLongSpeedInput.val();

        if (gotAmount && gotSpeed){
            amount = BITMETER.parseAmountValue(amountInput.val());
            speed = (useSliderForHowLong ? sliderDiv.slider('value') : parseSpeed(howLongSpeedInput.val()));
            
            if (amount !== null && speed !== null){
                if (speed === 0){
                    result = 'Never';    
                    desc   = 'Transfer will never complete when speed is 0';   
                } else {
                    result = BITMETER.formatInterval(amount/speed, BITMETER.formatInterval.SHORT);
                    desc   = 'To transfer ' + BITMETER.formatAmount(amount) + ' at ' + BITMETER.formatAmount(speed) + '/s';
                }    
            } else {
                result = '?';
                if (amount === null){
                    desc = 'Did not understand the Amount value. ';
                }
                if (speed === null){
                    desc = (desc ? desc + '<br>' : '');
                    desc += 'Did not understand the Speed value. ';
                }
            }
        }
        
        calcHowLongResult.html(result);
        calcHowLongDesc.html(desc);
    }
    
    function onSliderChange(value){
        calcSliderValue.html(BITMETER.formatAmount(value) + '/s');
        if (useSliderForHowMuch){
            updateHowMuchResult();
        }
        if (useSliderForHowLong){
            updateHowLongResult();
        }
    }

    sliderDiv.slider({
        animate: true,
        orientation : 'vertical',
        min: 0,
        max: BITMETER.model.getMonitorScale(),
        change: function(event, ui) {
                onSliderChange(ui.value);
            },
        slide: function(event, ui) {
                onSliderChange(ui.value);
            }
    }); 
    
    function getSliderValueInK(){
        return Math.round(sliderDiv.slider('value')/bytesPerK);
    }

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
    
    sliderDiv.slider('value', BITMETER.model.getMonitorScale()/2);
});
