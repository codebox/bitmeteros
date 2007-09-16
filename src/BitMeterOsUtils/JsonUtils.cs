using System;
using System.Collections.Generic;
using System.Text;
using System.Collections;
using Jayrock.Json;
using Jayrock.Json.Conversion;
using System.Diagnostics;

namespace bitmeter.utils {
    /*
     * Contains methods for creating and parsing JSON messages that get passed between the components 
     * of the application. Data messages are of the form:
     *  {
     *      'msgType' : 'exampleMessage',
     *      'msgId'   : '123',
     *      'data'    : {
     *          'dlTotal' : 10000,
     *          'ulTotal' : 5000
     *      }
     *  }
     * the 'msgId' value is optional.
     */

    public class JsonUtils {
        public class DataMessage {
            string msgId, msgType;
            List<IDictionary<string, object>> data;

            public DataMessage(string msgId, string msgType, IDictionary<string, object> data) {
                initMembers(msgId, msgType);

                if (data == null) {
                    throw new ArgumentException("A null data parameter is not allowed.");
                }

                this.data = new List<IDictionary<string, object>>();
                this.data.Add(data);
            }

            public DataMessage(string msgId, string msgType, List<IDictionary<string, object>> data) {
                initMembers(msgId, msgType);

                if (data == null) {
                    throw new ArgumentException("A null data parameter is not allowed.");
                }
                this.data = data;
            }

            private void initMembers(string msgId, string msgType) {
                this.msgId = msgId;

                if (msgType == null) {
                    throw new ArgumentException("A null msgType parameter is not allowed.");
                }
                this.msgType = msgType;
            }

            public string MsgId {
                get {
                    return this.msgId;
                }
            }

            public string MsgType {
                get {
                    return this.msgType;
                }
            }

            public List<IDictionary<string, object>> Data {
                get {
                    return this.data;   //TODO mutable
                }
            }

        }

        private const string KEY_MSG_TYPE = "msgType";
        private const string KEY_MSG_ID   = "msgId";
        private const string KEY_DATA     = "data";

        public static DataMessage parseDataMessage(string jsonMessage) {
            JsonObject jsonObject = (JsonObject)JsonConvert.Import(jsonMessage);

            DataMessage dataMessage = null;

            string msgType = (string)jsonObject[KEY_MSG_TYPE];
            string msgId   = (string)jsonObject[KEY_MSG_ID];

            if (jsonObject[KEY_DATA] is JsonArray) {
                JsonArray msgDataArray = (JsonArray)jsonObject[KEY_DATA];

                List<IDictionary<string, object>> dataList = new List<IDictionary<string, object>>();
                IDictionary<string, object> dataItem;
                foreach(JsonObject jsonObj in msgDataArray){
                    dataItem = buildDataDictionary(jsonObj);
                    dataList.Add(dataItem);
                }
                dataMessage = new DataMessage(msgId, msgType, dataList);

            } else if (jsonObject[KEY_DATA] is JsonObject) {
                JsonObject msgData = (JsonObject)jsonObject[KEY_DATA];

                IDictionary<string, object> data = buildDataDictionary(msgData);
                dataMessage = new DataMessage(msgId, msgType, data);

            } else {
                Debug.Assert(false, "Unexpected result type from JsonConvert.Import: " 
                    + ((jsonObject[KEY_DATA]==null) ? "null" : jsonObject[KEY_DATA].GetType().Name));
            }
            
            return dataMessage;
        }

        private static IDictionary<string, object> buildDataDictionary(JsonObject jsonData) {
            IDictionary<string, object> data = new Dictionary<string, object>();

            object value;
            foreach (string key in jsonData.Names) {
                value = jsonData[key];
                if (value is string) {
                    data[key] = value;

                } else if (value is JsonNumber) {
                    data[key] = ((JsonNumber)value).ToInt32();

                } else if (value is bool) {
                    data[key] = value;

                } else if (value == null) {
                    //

                } else {
                    throw new ArgumentException("Unable to process data key '" + value + "' because of the type of its value: " + data[key].GetType().Name);
                }
            }

            return data;
        }

		public static string makeDataMessage(string messageType, IList<IDictionary<string, object>> dataItemList) {
			return makeDataMessage(messageType, dataItemList, null);
		}
		
		public static string makeDataMessage(string messageType, IList<IDictionary<string, object>> dataItemList, string msgId) {
			string result;

            if (dataItemList == null || dataItemList.Count == 0) {
			 /* A message with no data items is legal, but there's no need to send it as an array - just 
			    delegate to the single-item version of the method with an empty Dictionary */
				result = makeDataMessage(messageType, new Dictionary<string, object>(), msgId);
				
			} else if (dataItemList.Count == 1) {
			 /* Since there's only 1 item in the List it doesn't really need to be a List, delegate
			    to the single-item version of the method */
				result = makeDataMessage(messageType, dataItemList[0], msgId);
				
			} else {
				JsonObject dataObject;
				JsonArray dataObjects = new JsonArray();
				foreach (IDictionary<string, object> dataItems in dataItemList){
					dataObject = makeDataObject(dataItems);	
					dataObjects.Add(dataObject);
				}
				
				result = composeJsonMessage(messageType, dataObjects, msgId);
			}
			
			return result;
			
		}
		
		public static string makeDataMessage(string messageType, IDictionary<string, object> dataItems) {
			return makeDataMessage(messageType, dataItems, null);
		}
		
        public static string makeDataMessage(string messageType, IDictionary<string, object> dataItems, string msgId) {
			JsonObject dataObject = makeDataObject(dataItems);
			return composeJsonMessage(messageType, dataObject, msgId);
        }
        
        private static string composeJsonMessage(string messageType, object dataObject, string msgId){
            if (messageType == null) {
                throw new NullReferenceException("Null 'messageType' parameter is not permitted.");
            }
            //TODO assert type of dataObject

         // This is the top-level object
            JsonObject messageObject = new JsonObject();

            messageObject.Put(KEY_MSG_TYPE, messageType);
            if (msgId != null) {
                messageObject.Put(KEY_MSG_ID, msgId);
            }
        	
            messageObject.Put(KEY_DATA, dataObject);

            string dataMessage = messageObject.ToString();

            Debug.Assert(dataMessage.IndexOf(messageType) >= 0, "Resulting message does not contain specified messageType value");
            Debug.Assert(msgId == null || dataMessage.IndexOf(msgId) >= 0, "Resulting message does not contain specified msgId value");

            return dataMessage;        	
        }
        
        private static JsonObject makeDataObject(IDictionary<string, object> dataItems) {
		 // This holds the data key/value pairs that form the contents of the message
            JsonObject dataObject = new JsonObject();
            if (dataItems != null) {
                foreach (KeyValuePair<string, object> entry in dataItems) {
                    dataObject.Put(entry.Key, entry.Value);
                }
            }        	
            return dataObject;
        }
    }
}
