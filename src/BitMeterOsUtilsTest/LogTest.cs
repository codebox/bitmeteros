using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace bitmeter.utils {
    [TestFixture]
    public class LogTest {
        private Log.LogEventType lastEventType;
        private string lastLogMsg = null;

        [Test]
        public void test() {
            Log.LogEvent += new Log.LogEventHandler(Log_LogEvent);

            const string DEBUG_MSG = "debug";
            const string INFO_MSG = "info";
            const string WARN_MSG = "warn";
            const string ERR_MSG = "error";

            Log.debug(DEBUG_MSG);
            Assert.AreEqual(lastEventType, Log.LogEventType.DEBUG);
            Assert.AreEqual(lastLogMsg, DEBUG_MSG);

            Log.info(INFO_MSG);
            Assert.AreEqual(lastEventType, Log.LogEventType.INFO);
            Assert.AreEqual(lastLogMsg, INFO_MSG);

            Log.warn(WARN_MSG);
            Assert.AreEqual(lastEventType, Log.LogEventType.WARN);
            Assert.AreEqual(lastLogMsg, WARN_MSG);

            Log.error(ERR_MSG);
            Assert.AreEqual(lastEventType, Log.LogEventType.ERROR);
            Assert.AreEqual(lastLogMsg, ERR_MSG);
        }

        void Log_LogEvent(Log.LogEventType eventType, string msg) {
            this.lastEventType = eventType;
            this.lastLogMsg = msg;
        }
    }
}
