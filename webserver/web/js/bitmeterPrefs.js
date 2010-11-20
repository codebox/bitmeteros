/*global $,BITMETER,window,config*/
/*jslint onevar: true, undef: true, nomen: true, eqeqeq: true, bitwise: true, regexp: true, newcap: true, immed: true, strict: false */

/*
Contains the code for the Preferences page.
*/
BITMETER.tabShowPrefs = function(){
    // Nothing to do
};

$(function(){
    var colourPickerDialogObj = $('#colourPicker'),
        adapterTableBody      = $('#adapterTable'),
        colourPickerObj       = $.farbtastic('#colourPickerDiv', function(col){
            $('#colourPickerTxt').val(col);
        }),
        rowIndex=0,
        hosts = {},
        adapterCount = 0,
        HEX_COL = "hexcol",
        prefsDialog,
        dialogOpts = {
            autoOpen : false,
            minHeight: 300,
            minWidth: 440,
            height: 280,
            width: 440      
        };
                        
    $('#colourPickerDiv').farbtastic('#colourPickerTxt');
    $('#prefsTabs').tabs().addClass('ui-tabs-vertical ui-helper-clearfix'); 
    $("#prefsTabs li").removeClass('ui-corner-top').addClass('ui-corner-left'); 
    
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
    
    $('#colourPickerTxt').keyup(function(){
        colourPickerObj.setColor($('#colourPickerTxt').val());
    });
    
    colourPickerDialogObj.dialog(dialogOpts);
    $('#dlColourPicker').click(function(){
            showColourPicker('Download', $('#dlColourPicker').data(HEX_COL), function(value){
                    $('#dlColourPicker').css('background-color', value).data(HEX_COL, value);
                });
        });
    $('#ulColourPicker').click(function(){
            showColourPicker('Upload', $('#ulColourPicker').data(HEX_COL), function(value){
                    $('#ulColourPicker').css('background-color', value).data(HEX_COL, value);
                });
        });
        
    $('#colourPickerTxtDiv li').click(function(){
        colourPickerObj.setColor('#' + this.id.substr(3));
    });
    
    $('#dlColourPicker').css('background-color', BITMETER.model.getDownloadColour()).data(HEX_COL, BITMETER.model.getDownloadColour());
    $('#ulColourPicker').css('background-color', BITMETER.model.getUploadColour()).data(HEX_COL, BITMETER.model.getUploadColour());
    
    $('#rssCount').val(config.rssItems);
    $('#rssCount').keypress(BITMETER.makeKeyPressHandler(0,8,'0-9'));
    $('input[name="rssFreq"][value=' + config.rssFreq + ']').attr('checked', true);
    
    $('#rssHostname').val(BITMETER.model.rssHost);
    $('#rssHostname').keypress(BITMETER.makeKeyPressHandler(0,8,32,45,46,95,'a-z','A-Z','0-9'));
    
    $('#prefServerName').val(config.serverName);
    $('#prefServerName').keypress(BITMETER.makeKeyPressHandler(0,8,32,45,46,95,'a-z','A-Z','0-9'));
    
    $('#prefMonitorInterval').val(BITMETER.model.getMonitorRefresh()/1000);
    $('#prefHistoryInterval').val(BITMETER.model.getHistoryRefresh()/1000);
    $('#prefSummaryInterval').val(BITMETER.model.getSummaryRefresh()/1000);
    
    $('#prefMonitorInterval').keypress(BITMETER.makeKeyPressHandler(0,8,45,46,'0-9'));
    $('#prefHistoryInterval').keypress(BITMETER.makeKeyPressHandler(0,8,45,46,'0-9'));
    $('#prefSummaryInterval').keypress(BITMETER.makeKeyPressHandler(0,8,45,46,'0-9'));
    
 // Populate the Adapters table from the list of adapter data held in the config object
    adapterTableBody.html('');
    
    function describeHostAdapterCombination(val){
        BITMETER.assert(val);
        function describeHost(hs){
            return (hs === 'local') ? 'this host': ('the host <b>' + hs + '</b>.');
        }
        var parts = val.split(':', 2);
        if (parts.length === 1){
            return describeHost(val);
        } else {
            BITMETER.assert(parts.length === 2);
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
        if (show && BITMETER.model.getAdapters()){
            $('p.filterWarning').show();
            $('p.filterWarning').html('Data Filter is active, only displaying data from ' + describeHostAdapterCombination(BITMETER.model.getAdapters()));
        } else {
            $('p.filterWarning').hide();
        }
    }
    
    if (adapterCount > 1){
        addRow('', 'Display all data in the database.');

        $.each(hosts, function(hs, ad){
            addRow(hs);
        });
    
        $.each(config.adapters, function(i,o){
            if (hosts[o.hs] > 1){
                addRow(o.hs + ':' + o.ad);
            }
        });
    
        $('#adapterTable tbody input[value="' + BITMETER.model.getAdapters() + '"]').click();
        
    } else {
        adapterTableBody.append('<tr><td>Data from only 1 adapter was found in the database</td></tr>');
    }
    
    $('#chkShowFilterWarning').attr('checked', BITMETER.model.getShowFilterWarning());
    
    $('#prefsAdaptersSaveLocal').button();
    $('#prefsAdaptersSaveLocal').click(function(){
        var selectedAdapter = $('#adapterTable tbody input:checked'),
            showFilter      = $('#chkShowFilterWarning').attr('checked');
        
        if (selectedAdapter){
            BITMETER.model.setAdapters(selectedAdapter.val());
        }
        
        BITMETER.model.setShowFilterWarning(showFilter);
        
        toggleFilterWarning(selectedAdapter && showFilter);
    });
    
    
 // Set the correct display units type, and attach click handlers
    if (BITMETER.model.getBinaryUnits()){
        $('#binaryUnits').attr('checked', true);
    } else {
        $('#decimalUnits').attr('checked', true);
    }
    $('#prefsUnitsLocal').click(function(){
        var isBinary = $('#binaryUnits').attr('checked');
        BITMETER.model.setBinaryUnits(isBinary);
        window.location.reload();
    });
    $('#prefsUnitsLocal').button();

 // Handler for refresh page link
    function setRefreshHandlers(){
        $('a.refreshLink').click(function(){window.location.reload();});
    }
    setRefreshHandlers();
    
 // Handlers for the 2 Save... buttons on the Colours tab 
    function setColoursOnModel(){
        BITMETER.model.setDownloadColour($('#dlColourPicker').data(HEX_COL));
        BITMETER.model.setUploadColour($('#ulColourPicker').data(HEX_COL));
    }
    $('#prefsColoursRemote, #prefsColoursLocal').button();
    $('#prefsColoursRemote').click(function(){
        setColoursOnModel();
        $('#prefsSaveColoursStatus').html('Saving colour choices to the server... <img src="css/images/working.gif" /> ');
        var dlCol = BITMETER.model.getDownloadColour().substr(1),
            ulCol = BITMETER.model.getUploadColour().substr(1);
        $.ajax({
            url : 'config?web.colour_dl=' + dlCol + '&web.colour_ul=' + ulCol, 
            success : function(){
                $('#prefsSaveColoursStatus').html('Colour choices saved.');
                window.setTimeout(function(){
                        $('#prefsSaveColoursStatus').html('');
                    }, 3000);
            },
            error : function(){
                $('#prefsSaveColoursStatus').html('Failed to save colour choices.');
                window.setTimeout(function(){
                        $('#prefsSaveColoursStatus').html('');
                    }, 3000);
            }
        });
    });
    $('#prefsColoursLocal').click(function(){
        setColoursOnModel();
        $('#prefsSaveColoursStatus').html('Colour choices saved.');
        window.setTimeout(function(){
                $('#prefsSaveColoursStatus').html('');
            }, 3000);
    });

 // Handler for the Save... button on the RSS tab   
    $('#prefsRssSave').button().click(function(){
        var rssNumItems = $('#rssCount').val(),
            rssFreq     = $('input[name="rssFreq"]:checked').val(),
            rssHostname = $('#rssHostname').val();
        
        if (!rssNumItems){
            BITMETER.errorDialog.show("Please enter a numeric value for 'Number of Items'");    
        } else {
            $('#prefsRssSaveStatus').html('Saving RSS parameters... <img src="css/images/working.gif" />');
            $.ajax({
                url : 'config?web.rss.host=' + rssHostname + '&web.rss.freq=' + rssFreq + '&web.rss.items=' + rssNumItems, 
                success : function(){
                    $('#prefsRssSaveStatus').html('RSS parameters saved.');
                    window.setTimeout(function(){
                            $('#prefsRssSaveStatus').html('');
                        }, 3000);
                },
                error : function(){
                    $('#prefsRssSaveStatus').html('Failed to save RSS parameters.');
                    window.setTimeout(function(){
                            $('#prefsRssSaveStatus').html('');
                        }, 3000);
                }
            });
            
        }
    });
    
 // Handler for the Save... button on the Server Name tab       
    $('#prefsServerNameSave').button().click(function(){
        $('#prefsServerNameSaveStatus').html('Saving Server Name... <img src="css/images/working.gif" />');
        $.ajax({
            url : 'config?web.server_name=' + $('#prefServerName').val(), 
            success : function(){
				window.location.reload();
            },
            error : function(){
                $('#prefsServerNameSaveStatus').html('Failed to save Server Name');
                window.setTimeout(function(){
                        $('#prefsServerNameSaveStatus').html('');
                    }, 3000);
            }
        });         
    });
    
 // Handler for the 2 Save... buttons on the Refresh Intervals tab          
    function setIntervalsOnModelOk(){
        var refreshMonitor = Number($('#prefMonitorInterval').val()),
            refreshHistory = Number($('#prefHistoryInterval').val()),
            refreshSummary = Number($('#prefSummaryInterval').val()),
            validationOk = true,
            monitorMin, monitorMax, historyMin, historyMax, summaryMin, summaryMax;
        
     // Validate the monitor interval
        if (validationOk){
            monitorMin = config.monitorIntervalMin / 1000;
            monitorMax = config.monitorIntervalMax / 1000;
            
            if (!refreshMonitor || refreshMonitor < monitorMin || refreshMonitor > monitorMax){
                validationOk = false;
                BITMETER.errorDialog.show("Please enter a numeric value between " + monitorMin + 
                        " and " + monitorMax + " in the Monitor interval box.", 
                        function(){
                            $('#prefMonitorInterval').focus();  
                        });
            }
        }
        
     // Validate the history interval
        if (validationOk){
            historyMin = config.historyIntervalMin / 1000;
            historyMax = config.historyIntervalMax / 1000;
            
            if (!refreshHistory || refreshHistory < historyMin || refreshHistory > historyMax){
                validationOk = false;
                BITMETER.errorDialog.show("Please enter a numeric value between " + historyMin + 
                        " and " + historyMax + " in the History interval box.",
                        function(){
                            $('#prefHistoryInterval').focus();  
                        });
            }
        }
        
     // Validate the summary interval
        if (validationOk){
            summaryMin = config.summaryIntervalMin / 1000;
            summaryMax = config.summaryIntervalMax / 1000;
            
            if (!refreshSummary || refreshSummary < summaryMin || refreshSummary > summaryMax){
                validationOk = false;
                BITMETER.errorDialog.show("Please enter a numeric value between " + summaryMin + 
                        " and " + summaryMax + " in the Summary interval box.",
                        function(){
                            $('#prefSummaryInterval').focus();  
                        });
            }
        }

        if (validationOk){
            BITMETER.model.setMonitorRefresh(refreshMonitor * 1000);
            BITMETER.model.setHistoryRefresh(refreshHistory * 1000);
            BITMETER.model.setSummaryRefresh(refreshSummary * 1000);
        }
        return validationOk;
    }
    $('#prefsRefreshSaveRemote, #prefsRefreshSaveLocal').button();
    $('#prefsRefreshSaveRemote').click(function(){
        if (setIntervalsOnModelOk()){
            var url = 'config?web.monitor_interval=' + BITMETER.model.getMonitorRefresh() + 
                    '&web.history_interval=' + BITMETER.model.getHistoryRefresh() + 
                    '&web.summary_interval=' + BITMETER.model.getSummaryRefresh(); 
            $('#prefsRefreshSaveStatus').html('Saving Refresh Intervals... <img src="css/images/working.gif" />');
            $.ajax({
                url : url, 
                success : function(){
                    $('#prefsRefreshSaveStatus').html('Refresh Intervals saved.');
                    window.setTimeout(function(){
                            $('#prefsRefreshSaveStatus').html('');
                        }, 3000);
                },
                error : function(){
                    $('#prefsRefreshSaveStatus').html('Failed to save Refresh Intervals.');
                    window.setTimeout(function(){
                            $('#prefsRefreshSaveStatus').html('');
                        }, 3000);
                }
            });             
        }
    });
    $('#prefsRefreshSaveLocal').click(function(){
        setIntervalsOnModelOk();        
        $('#prefsRefreshSaveStatus').html('Refresh Intervals saved.');
        window.setTimeout(function(){
                $('#prefsRefreshSaveStatus').html('');
            }, 3000);
    });
    $(function() {
        $("#prefsTabs").tabs().addClass('ui-tabs-vertical ui-helper-clearfix');
        $("#prefsTabs li").removeClass('ui-corner-top').addClass('ui-corner-left');
    });

 // Show the Help dialog box when the help link is clicked
    prefsDialog = $('#prefsHelp').dialog(BITMETER.consts.dialogOpts);
    $('#prefsHelpLink').click(function(){
            prefsDialog.dialog("open");
        });
}); 
 
