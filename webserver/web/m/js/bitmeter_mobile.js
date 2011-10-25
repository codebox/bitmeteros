var formatAmount = (function(){
	var K = 1024;
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
function showError(msg){
	//TODO
	alert(msg);
}
function buildAjaxRequest(){
	var request = {}, onSuccess, onError;

	function makeXHRObject(){
		var xHRObject;
		
        if (window.XMLHttpRequest) {
        	try {
    			xHRObject = new XMLHttpRequest();
            } catch(e) {
    			xHRObject = null;
            }
            
        } else if (window.ActiveXObject) {
           	try {
            	xHRObject = new ActiveXObject("Msxml2.XMLHTTP");
          	} catch(e) {
            	try {
              		xHRObject = new ActiveXObject("Microsoft.XMLHTTP");
            	} catch(e) {
              		xHRObject = null;
            	}
    		}
        }
	        
	    return xHRObject;
	};	
	
	function buildResponseHandler( xHRObject ){
	 // Form a closure containing the objects we need to deal with the response
		return function(){
			if ( xHRObject.readyState == 4 ) {
	            if ( xHRObject.status >= 200 && xHRObject.status <= 299 ) {
	            	if (onSuccess){
		            	onSuccess( xHRObject.responseText );
		            }
	            
	            } else {
	             // There was an HTTP error
	             	if (onError) {
		                onError( 'HTTP ERROR: The response code was ' + xHRObject.status );
		            }
	            }
	        } 
		}; 	
	}
	
	request.setSuccessHandler = function(handler){
		onSuccess = handler;
	};
	request.setErrorHandler = function(handler){
		onError = handler;
	};
	
	request.send = function(url){
		var xHRObject = makeXHRObject();
		if (xHRObject === null){
			showError();
		} else {
			xHRObject.onreadystatechange = buildResponseHandler( xHRObject );
			
			xHRObject.open('GET', url, true );
			xHRObject.send(null);
		}
	};
	
	return request;
};

