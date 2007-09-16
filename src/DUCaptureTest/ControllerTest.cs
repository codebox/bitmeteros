using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
using bitmeter.utils;
using System.Timers;
using System.Threading;

namespace bitmeter.capture {
    [TestFixture]
    public class ControllerTest {
        private DummyMessageDispatcher messageDispatcher;
        private DummyDUDataFactory dataFactory;
        private Controller controller;
        private const int TICK_INTERVAL = 100;  // in millieconds

        const string DS_1 = "data source 1";
        const string DS_2 = "data source 2";
        const string DS_3 = "data source 3";

        private void reset() {
            messageDispatcher = new DummyMessageDispatcher();
            dataFactory = new DummyDUDataFactory();
            controller = new Controller(dataFactory, messageDispatcher, 1);
            controller.TickFactor = TICK_INTERVAL;
        }

        [Test]
        public void testNoDataSources() {
            reset();

            controller.onTick();
            Assert.IsNull(messageDispatcher.LatestMessage);

            controller.onTick();
            Assert.IsNull(messageDispatcher.LatestMessage);
        }

        [Test]
        public void test1DataSource() {
            reset();
            dataFactory.setDataValues(DS_1, DS_1, 0, 0, 0);

            controller.onTick();
            Assert.IsNull(messageDispatcher.LatestMessage);

            controller.onTick();
            JsonUtils.parseDataMessage(messageDispatcher.LatestMessage);

            string msg = messageDispatcher.LatestMessage;
            JsonUtils.DataMessage dataMsg = JsonUtils.parseDataMessage(msg);
            Assert.AreEqual(1, dataMsg.Data.Count);
        }

        [Test]
        public void test3DataSources() {
            reset();
            dataFactory.setDataValues(DS_1, DS_1, 0, 0, 0);
            dataFactory.setDataValues(DS_2, DS_2, 0, 0, 0);
            dataFactory.setDataValues(DS_3, DS_3, 0, 0, 0);

            controller.onTick();
            controller.onTick();

            JsonUtils.parseDataMessage(messageDispatcher.LatestMessage);

            string msg = messageDispatcher.LatestMessage;
            JsonUtils.DataMessage dataMsg = JsonUtils.parseDataMessage(msg);
            Assert.AreEqual(3, dataMsg.Data.Count);
        }

        [Test]
        public void testConcurrency() {
            reset();
            dataFactory.setDataValues(DS_1, DS_1, 0, 0, 0);
            dataFactory.Delay = 3 * TICK_INTERVAL;

            Job job1 = new Job(controller, 0);
            Thread t1 = new Thread(new ThreadStart(job1.run));

            Job job2 = new Job(controller, TICK_INTERVAL);
         // This thread should abort since the previous one will still be running
            Thread t2 = new Thread(new ThreadStart(job2.run));

            Job job3 = new Job(controller, TICK_INTERVAL * 5);
            Thread t3 = new Thread(new ThreadStart(job3.run));

            t1.Start();
            t2.Start();
            t3.Start();

            t1.Join();
            t2.Join();
            t3.Join();

            Assert.AreEqual(1, messageDispatcher.MsgCount);
        }

        [Test]
        public void testAmounts() {
         // Tests that the basic logic that calculates ul/dl deltas and intervals is working
            reset();
            dataFactory.setDataValues(DS_1, DS_1, 0, 0, 0);

            const uint DL_INIT = 10000;
            const uint UL_INIT = 20000;
            const uint DL_DELTA = 1;
            const uint UL_DELTA = 2;
            const uint TS_INIT = 100;
            const uint TS_DELTA = 1;

            dataFactory.setDataValues(DS_1, DS_1, DL_INIT, UL_INIT, TS_INIT);
            controller.onTick();
            Assert.IsNull(messageDispatcher.LatestMessage);

            dataFactory.setDataValues(DS_1, DS_1, DL_INIT + DL_DELTA, UL_INIT + UL_DELTA, TS_INIT + TS_DELTA);
            controller.onTick();

            string msg = messageDispatcher.LatestMessage;
            JsonUtils.DataMessage dataMsg = JsonUtils.parseDataMessage(msg);
            Assert.AreEqual(1, dataMsg.Data.Count);
            Assert.AreEqual("" + DL_DELTA, dataMsg.Data[0]["dl"]);
            Assert.AreEqual("" + UL_DELTA, dataMsg.Data[0]["ul"]);
            Assert.AreEqual("" + TS_DELTA, dataMsg.Data[0]["in"]);
            Assert.AreEqual(DS_1, dataMsg.Data[0]["if"]);

            dataFactory.setDataValues(DS_1, DS_1, DL_INIT + 3 * DL_DELTA, UL_INIT + 3 * UL_DELTA, TS_INIT + 3 * TS_DELTA);
            controller.onTick();

            msg = messageDispatcher.LatestMessage;
            dataMsg = JsonUtils.parseDataMessage(msg);
            Assert.AreEqual(1, dataMsg.Data.Count);
            Assert.AreEqual("" + 2 * DL_DELTA, dataMsg.Data[0]["dl"]);
            Assert.AreEqual("" + 2 * UL_DELTA, dataMsg.Data[0]["ul"]);
            Assert.AreEqual("" + 2 * TS_DELTA, dataMsg.Data[0]["in"]);
            Assert.AreEqual(DS_1, dataMsg.Data[0]["if"]);
        }

        [Test]
        public void testTempDs() {
         // Tests that the controller behaves correctly when a interface temporarily goes down
            reset();
            dataFactory.setDataValues(DS_1, DS_1, 0, 0, 0);
            dataFactory.setDataValues(DS_2, DS_2, 0, 0, 0);

            const uint DS1_DL_INIT = 10000;
            const uint DS1_UL_INIT = 20000;
            const uint DS1_DL_DELTA = 1;
            const uint DS1_UL_DELTA = 2;

            const uint DS2_DL_INIT = 30000;
            const uint DS2_UL_INIT = 10000;
            const uint DS2_DL_DELTA = 4;
            const uint DS2_UL_DELTA = 2;

            const uint TS = 100;

            dataFactory.setDataValues(DS_1, DS_1, DS1_DL_INIT, DS1_UL_INIT, TS);
            dataFactory.setDataValues(DS_2, DS_2, DS2_DL_INIT, DS2_UL_INIT, TS);
            controller.onTick();
            Assert.IsNull(messageDispatcher.LatestMessage);

         // Remove DS_2 - should only get results from DS_1
            dataFactory.setDataValues(DS_1, DS_1, DS1_DL_INIT + DS1_DL_DELTA, DS1_UL_INIT + DS1_UL_DELTA, TS);
            dataFactory.removeDataValues(DS_2);
            controller.onTick();
            string msg = messageDispatcher.LatestMessage;
            JsonUtils.DataMessage dataMsg = JsonUtils.parseDataMessage(msg);
            Assert.AreEqual(1, dataMsg.Data.Count);
            Assert.AreEqual("" + DS1_DL_DELTA, dataMsg.Data[0]["dl"]);
            Assert.AreEqual("" + DS1_UL_DELTA, dataMsg.Data[0]["ul"]);
            Assert.AreEqual(DS_1, dataMsg.Data[0]["if"]);

         // Restore DS_2 - should get results from both, DS_2s amounts based on the values set before the interface went down
            dataFactory.setDataValues(DS_1, DS_1, DS1_DL_INIT + 2 * DS1_DL_DELTA, DS1_UL_INIT + 2 * DS1_UL_DELTA, TS);
            dataFactory.setDataValues(DS_2, DS_2, DS2_DL_INIT + 2 * DS2_DL_DELTA, DS2_UL_INIT + 2 * DS2_UL_DELTA, TS);

            controller.onTick();
            msg = messageDispatcher.LatestMessage;
            dataMsg = JsonUtils.parseDataMessage(msg);
            Assert.AreEqual(2, dataMsg.Data.Count);
            Assert.AreEqual("" + DS1_DL_DELTA, dataMsg.Data[0]["dl"]);
            Assert.AreEqual("" + DS1_UL_DELTA, dataMsg.Data[0]["ul"]);
            Assert.AreEqual(DS_1, dataMsg.Data[0]["if"]);

            Assert.AreEqual("" + 2 * DS2_DL_DELTA, dataMsg.Data[1]["dl"]);
            Assert.AreEqual("" + 2 * DS2_UL_DELTA, dataMsg.Data[1]["ul"]);
            Assert.AreEqual(DS_2, dataMsg.Data[1]["if"]);
        }

        [Test]
        public void testErrOnSend() {
         // Tests that the controller behaves correctly when unable to send data to the server
            reset();
            dataFactory.setDataValues(DS_1, DS_1, 0, 0, 0);

            const uint DL_INIT = 10000;
            const uint UL_INIT = 20000;
            const uint DL_DELTA = 1;
            const uint UL_DELTA = 2;
            const uint TS = 1;

            dataFactory.setDataValues(DS_1, DS_1, DL_INIT, UL_INIT, TS);
            controller.onTick();
            Assert.IsNull(messageDispatcher.LatestMessage);

            dataFactory.setDataValues(DS_1, DS_1, DL_INIT + DL_DELTA, UL_INIT + UL_DELTA, TS);
            messageDispatcher.BarfOnSend = true;
            controller.onTick();
            Assert.IsNull(messageDispatcher.LatestMessage);

            dataFactory.setDataValues(DS_1, DS_1, DL_INIT + 3 * DL_DELTA, UL_INIT + 3 * UL_DELTA, TS);
            messageDispatcher.BarfOnSend = false;
            controller.onTick();

            string msg = messageDispatcher.LatestMessage;
            JsonUtils.DataMessage dataMsg = JsonUtils.parseDataMessage(msg);
            Assert.AreEqual(1, dataMsg.Data.Count);
            Assert.AreEqual("" + 3 * DL_DELTA, dataMsg.Data[0]["dl"]);
            Assert.AreEqual("" + 3 * UL_DELTA, dataMsg.Data[0]["ul"]);
        }

        [Test]
        public void testStartStop() {
         // Tests that the Start/Stop methods work correctly
            reset();
            dataFactory.setDataValues(DS_1, DS_1, 0, 0, 0);

         // Start the controller, wait 2 ticks, then check we have messages
            Assert.IsNull(messageDispatcher.LatestMessage);
            controller.Start();
            Thread.Sleep(TICK_INTERVAL * 3);
            Assert.IsNotNull(messageDispatcher.LatestMessage);

         // wait another 2 ticks while the controller is stopped, check we don't have new messages
            controller.Stop();
            messageDispatcher.LatestMessage = null;
            Thread.Sleep(TICK_INTERVAL * 3);
            Assert.IsNull(messageDispatcher.LatestMessage);

         // stop the controller again without starting it, wait 2 ticks, check no new messages have appeared
            controller.Stop();
            Thread.Sleep(TICK_INTERVAL * 3);
            Assert.IsNull(messageDispatcher.LatestMessage);

         // start the controller without stopping it, check we are still getting new messages
            controller.Start();
            Thread.Sleep(TICK_INTERVAL * 3);
            Assert.IsNotNull(messageDispatcher.LatestMessage);
            messageDispatcher.LatestMessage = null;
            controller.Start();
            Thread.Sleep(TICK_INTERVAL * 3);
            Assert.IsNotNull(messageDispatcher.LatestMessage);
        }

        private class Job {
            Controller controller;
            int delay;
            public Job(Controller controller, int delay) {
                this.controller = controller;
                this.delay = delay;
            }
            
            public void run() {
                if (delay > 0) {
                    Thread.Sleep(delay);
                }
                this.controller.onTick();
            }
        }


    }
}
