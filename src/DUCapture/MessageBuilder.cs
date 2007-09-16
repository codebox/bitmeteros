using System;
using System.Collections.Generic;
using System.Text;
using bitmeter.utils;

namespace bitmeter.capture {
    internal class MessageBuilder {
    	private const string MSG_TYPE = "duTotals";
    	
    	private const string INTERFACE_KEY = "if";
    	private const string TIMESTAMP_KEY = "ts";
    	private const string INTERVAL_KEY  = "in";
    	private const string DL_VALUE_KEY  = "dl";
    	private const string UL_VALUE_KEY  = "ul";

        internal static string buildMessage(List<MsgData> msgDataList){
        	string result;
            if (msgDataList.Count == 0) {
            	result = null;
            	
            } else {
            	IDictionary<string, object> msgDataDictionary;
            	List<IDictionary<string, object>> dataItemList = new List<IDictionary<string, object>>();
            	
            	foreach (MsgData msgData in msgDataList) {
            		msgDataDictionary = buildDictionaryFromMsgData(msgData);	
            		dataItemList.Add(msgDataDictionary);
            	}
            	
            	result = JsonUtils.makeDataMessage(MSG_TYPE, dataItemList);
            }

            return result;
        }
        
        private static IDictionary<string, object> buildDictionaryFromMsgData(MsgData msgData) {
        	IDictionary<string, object> result = new Dictionary<string, object>();
        	
        	result[INTERFACE_KEY] = msgData.Interface;
        	result[TIMESTAMP_KEY] = msgData.Timestamp;
        	result[INTERVAL_KEY] = msgData.Interval;
        	result[DL_VALUE_KEY] = msgData.Dl;
        	result[UL_VALUE_KEY] = msgData.Ul;
        	
        	return result;
        }
    }
}
