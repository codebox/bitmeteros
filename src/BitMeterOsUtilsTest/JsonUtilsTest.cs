using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace bitmeter.utils {
    [TestFixture]
    public class JsonUtilsTest {
        const string MSG_TYPE = "message type";
        const string MSG_ID   = "id";

        const string DATA_KEY_STR = "stringKey";
        const string DATA_VAL_STR = "string value";

        const string DATA_KEY_BOOL = "boolKey";
        const bool   DATA_VAL_BOOL = true;

        const string DATA_KEY_NUM = "numKey";
        const int    DATA_VAL_NUM = 123;

        const string DATA_KEY_NULL = "nullKey";

        [Test]
        public void dataMessageClass() {
            IDictionary<string, object> data = makeDataDictionary(true, true, true);
            JsonUtils.DataMessage msgObj1 = new JsonUtils.DataMessage(MSG_ID, MSG_TYPE, data);

            Assert.AreEqual(MSG_TYPE, msgObj1.MsgType);
            Assert.AreEqual(MSG_ID, msgObj1.MsgId);
            Assert.AreEqual(data, msgObj1.Data[0]);

            JsonUtils.DataMessage msgObj2 = new JsonUtils.DataMessage(null, MSG_TYPE, data);
            Assert.IsNull(msgObj2.MsgId);

            try {
                new JsonUtils.DataMessage(MSG_ID, null, data);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // expected
            }

            try {
                new JsonUtils.DataMessage(MSG_ID, MSG_TYPE, (List<IDictionary<string, object>>) null);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // expected
            }

            try {
                new JsonUtils.DataMessage(MSG_ID, MSG_TYPE, (IDictionary<string, object>)null);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // expected
            }
        }

        [Test]
        public void makeSingleDataMessage() {
            string msgText1 = JsonUtils.makeDataMessage(MSG_TYPE, makeDataDictionary(false, false, false), MSG_ID);
            JsonUtils.DataMessage msgObj1 = JsonUtils.parseDataMessage(msgText1);

            Assert.AreEqual(MSG_TYPE, msgObj1.MsgType);
            Assert.AreEqual(MSG_ID, msgObj1.MsgId);
            Assert.AreEqual(1, msgObj1.Data.Count);
            Assert.AreEqual(0, msgObj1.Data[0].Count);

            IDictionary<string, object> data = makeDataDictionary(true, true, true);
            data[DATA_KEY_NULL] = null;

            string msgText2 = JsonUtils.makeDataMessage(MSG_TYPE, data, MSG_ID);
            JsonUtils.DataMessage msgObj2 = JsonUtils.parseDataMessage(msgText2);

            Assert.AreEqual(DATA_VAL_STR, msgObj2.Data[0][DATA_KEY_STR]);
            Assert.AreEqual(DATA_VAL_NUM, msgObj2.Data[0][DATA_KEY_NUM]);
            Assert.AreEqual(DATA_VAL_BOOL, msgObj2.Data[0][DATA_KEY_BOOL]);
            Assert.IsFalse(msgObj2.Data[0].ContainsKey(DATA_KEY_NULL));

         // missing msgId
            JsonUtils.DataMessage msgObj3 = JsonUtils.parseDataMessage(JsonUtils.makeDataMessage(MSG_TYPE, (IDictionary<string, object>)null));
            Assert.IsNull(msgObj3.MsgId);

         // null msgType
            try {
                JsonUtils.makeDataMessage(null, data, MSG_ID);
                Assert.Fail("Expected NullReferenceException");
            } catch (NullReferenceException) {
                //expected
            }
        }

        [Test]
        public void makeMultipleDataMessage() {
            List<IDictionary<string, object>> dataItemList = new List<IDictionary<string, object>>();

         // Zero items in the list
            string msgText0 = JsonUtils.makeDataMessage(MSG_TYPE, dataItemList, MSG_ID);
            JsonUtils.DataMessage msgObj0 = JsonUtils.parseDataMessage(msgText0);

            Assert.AreEqual(MSG_TYPE, msgObj0.MsgType);
            Assert.AreEqual(MSG_ID, msgObj0.MsgId);
            Assert.AreEqual(1, msgObj0.Data.Count);
            Assert.AreEqual(0, msgObj0.Data[0].Count);
            
         // 1 item in the list
            dataItemList.Add(makeDataDictionary(true, true, true));
            string msgText1 = JsonUtils.makeDataMessage(MSG_TYPE, dataItemList, MSG_ID);
            JsonUtils.DataMessage msgObj1 = JsonUtils.parseDataMessage(msgText1);
            Assert.AreEqual(1, msgObj1.Data.Count);

         // 3 items in the list
            dataItemList = new List<IDictionary<string, object>>();
            dataItemList.Add(makeDataDictionary(true, false, true));
            dataItemList.Add(makeDataDictionary(true, true, false));
            dataItemList.Add(makeDataDictionary(false, false, true));
            string msgText2 = JsonUtils.makeDataMessage(MSG_TYPE, dataItemList, MSG_ID);
            JsonUtils.DataMessage msgObj2 = JsonUtils.parseDataMessage(msgText2);
            Assert.AreEqual(3, msgObj2.Data.Count);

         // missing msgId
            JsonUtils.DataMessage msgObj3 = JsonUtils.parseDataMessage(JsonUtils.makeDataMessage(MSG_TYPE, (List<IDictionary<string, object>>)null));
            Assert.IsNull(msgObj3.MsgId);

         // null msgType
            try {
                JsonUtils.makeDataMessage(null, dataItemList, MSG_ID);
                Assert.Fail("Expected NullReferenceException");
            } catch (NullReferenceException) {
                //expected
            }


        }

        private IDictionary<string, object> makeDataDictionary(bool hasString, bool hasBool, bool hasNum) {
            IDictionary<string, object> data = new Dictionary<string, object>();

            if (hasString) {
                data[DATA_KEY_STR] = DATA_VAL_STR;
            }
            if (hasBool) {
                data[DATA_KEY_BOOL] = DATA_VAL_BOOL;
            }
            if (hasNum) {
                data[DATA_KEY_NUM] = DATA_VAL_NUM;
            }

            return data;
        }
    }
}
