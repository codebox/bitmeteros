using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
using bitmeter.utils;
using System.Threading;

namespace bitmeter.capture {
    [TestFixture]
    public class MessageDispatcherTest {
        [Test]
        public void test() {
            const string HOST = "localhost";
            const int PORT = 25672;
            const string MSG = "test message";

            Log.LogEvent += new Log.LogEventHandler(Log_LogEvent);
            MessageDispatcher dispatcher = new MessageDispatcher(HOST, PORT, true);

            ServerSocket server = new ServerSocket((int)PORT);
            server.Listen();

            dispatcher.send(MSG);
            waitForLog();

            Assert.AreEqual(MSG, latestMessage);
        }

        object logSyncObject = new object();
        string latestMessage = null;

        void waitForLog() {
            lock (logSyncObject) {
                if (latestMessage != null) {
                    return;
                } else {
                    bool gotLock = Monitor.Wait(logSyncObject, 3000);
                    if (!gotLock) {
                        Assert.Fail("waitForLog wait timeout expired");
                    }
                }
            }
        }

        void Log_LogEvent(Log.LogEventType eventType, string msg) {
            lock (logSyncObject) {
                latestMessage = msg;
                Monitor.PulseAll(logSyncObject);
            }
        }

        
    }
}
