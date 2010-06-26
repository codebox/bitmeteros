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
		minHeight: 300,
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
	
 // Populate the Adapters table from the list of adapter data held in the config object
	var adapterTableBody = $('#adapterTable');
	adapterTableBody.html('');
	
	var rowIndex=0;
	function describeHostAdapterCombination(val){
		assert(val);
		function describeHost(hs){
			return (hs === 'local') ? 'this host': ('the host <b>' + hs + '</b>.');
		}
		var parts = val.split(':', 2);
		if (parts.length === 1){
			return describeHost(val);
		} else {
			assert(parts.length === 2);
			return 'the adapter <b>' + parts[1] + '</b> on ' + describeHost(parts[0]);
		}		
	}
	function addRow(value, desc){
		var html;
		html =  '<tr><td><input type="radio" class="prefRadio" name="adapter" id="a' + rowIndex + '" value="' + value + '" /></td>';
		html += '<td><label for="a' + rowIndex + '">';
		if (desc){
			html += desc;
		} else {
			html += 'Display only data from ' + describeHostAdapterCombination(value);
		}
		html += '</label></td></tr>';
		adapterTableBody.append(html);
		rowIndex++;
	}
	
	var hosts = {};
	var adapterCount = 0;
	$.each(config.adapters, function(i,o){
		var hostName = o.hs;
		if (hosts[hostName]){
			hosts[hostName]++;
		} else {
			hosts[hostName] = 1;
		}
		adapterCount++;
	});
	
	function toggleFilterWarning(show){
		if (show){
			$('p.filterWarning').show();
			$('p.filterWarning').html('Data Filter is active, only displaying data from ' + describeHostAdapterCombination(model.getAdapters()));
		} else {
			$('p.filterWarning').hide();
		}
	}
	
	if (adapterCount > 1){
		addRow('', 'Display all data in the database.');

		$.each(hosts, function(hs, ad){
			addRow(hs);
		})
	
		$.each(config.adapters, function(i,o){
			if (hosts[o.hs] > 1){
				addRow(o.hs + ':' + o.ad);
			}
		});
	
		$('#adapterTable tbody input').click(function(e){
			model.setAdapters(e.currentTarget.value);
			toggleFilterWarning(model.getAdapters() && model.getShowFilterWarning());
		});
	
		$('#adapterTable tbody input[value="' + model.getAdapters() + '"]').click();
		
	} else {
		adapterTableBody.append('<tr><td>Data from only 1 adapter was found in the database</td></tr>');
	}
	
	$('#chkShowFilterWarning').click(function(e){
		model.setShowFilterWarning(e.currentTarget.checked);
		toggleFilterWarning(model.getAdapters() && model.getShowFilterWarning());		
	});
	$('#chkShowFilterWarning').attr('checked', model.getShowFilterWarning());
	
	
 // Set the correct display units type, and attach click handlers
 	if (model.getBinaryUnits()){
 		$('#binaryUnits').attr('checked', true);
 	} else {
	 	$('#decimalUnits').attr('checked', true);
 	}
 	$('#binaryUnits').click(function(){model.setBinaryUnits(true)});
 	$('#decimalUnits').click(function(){model.setBinaryUnits(false)});

 // Handler for refresh page link
 	$('a.refreshLink').click(function(){location.reload();});
}); 
 
