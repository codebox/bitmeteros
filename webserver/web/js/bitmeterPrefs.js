/*
Contains the code for the Preferences page.
*/
function tabShowPrefs(){

};

$(document).ready(function(){
	$('#colourPickerDiv').farbtastic('#colourPickerTxt');
	var colourPickerObj = $.farbtastic('#colourPickerDiv');
	
	var dialogOpts = {
		autoOpen : false,
		minHeight: 280,
		minWidth: 440,
		height: 280,
		width: 440		
	};
	
	function showColourPicker(valueDesc, initValue, onClose){
		$('#colourPickerTxtDiv h4').html(valueDesc + ' Colour');
		colourPickerObj.setColor(initValue);
		$('#colourPickerTxt').val(initValue);
		colourPickerDialogObj.dialog('option', 'buttons', { "OK" : function() { 
			onClose(colourPickerObj.color);
			$(this).dialog("close"); 
		}});
		colourPickerDialogObj.dialog("open");
	}
	
	var colourPickerDialogObj = $('#prefs .dialog');
	colourPickerDialogObj.dialog(dialogOpts);
	$('#dlColourPicker').click(function(){
			showColourPicker('Download', model.getDownloadColour(), function(value){
					model.setDownloadColour(value);
					$('#dlColourPicker').css('background-color', model.getDownloadColour());
				});
		});
	$('#ulColourPicker').click(function(){
			showColourPicker('Upload', model.getUploadColour(), function(value){
					model.setUploadColour(value);
					$('#ulColourPicker').css('background-color', model.getUploadColour());
				});
		});
		
	$('#colourPickerTxtDiv li').click(function(){
		colourPickerObj.setColor('#' + this.id);
	});
	
	$('#dlColourPicker').css('background-color', model.getDownloadColour());
	$('#ulColourPicker').css('background-color', model.getUploadColour());
}); 
 
