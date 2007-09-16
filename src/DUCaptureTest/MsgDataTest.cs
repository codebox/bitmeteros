using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace bitmeter.capture {
    [TestFixture]
    public class MsgDataTest {
        [Test]
        public void test(){
            const string IF_NAME = "interface name";
            const uint DL_VALUE = 1234;
            const uint UL_VALUE = 12345;
            const uint TS = 112233;
            const uint INTERVAL = 2;
            MsgData data = new MsgData(IF_NAME, TS, INTERVAL, DL_VALUE, UL_VALUE);

            Assert.AreEqual(IF_NAME, data.Interface);
            Assert.AreEqual(DL_VALUE, data.Dl);
            Assert.AreEqual(UL_VALUE, data.Ul);
            Assert.AreEqual(TS, data.Timestamp);
            Assert.AreEqual(INTERVAL, data.Interval);
        }

    }
}
