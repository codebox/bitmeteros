using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using bitmeter.utils;
using System.Timers;

namespace bitmeter.capture {
    public class Controller : IDisposable {
        private IDUDataFactory dataFactory;
        private IMessageDispatcher dispatcher;
        private uint tickInterval;
        private bool isRunning = false;
        private Timer timer;
        private uint tickFactor = 1000;

        public Controller(IDUDataFactory dataFactory, IMessageDispatcher dispatcher, uint tickInterval) {
            this.dataFactory  = dataFactory;
            this.dispatcher   = dispatcher;
            this.tickInterval = tickInterval;
            timer = new Timer(tickInterval * tickFactor);
            timer.Elapsed += new ElapsedEventHandler(timer_Elapsed);
        }

        public void Start() {
            lock (timer) {
                if (!isRunning) {
                    timer.Start();
                    isRunning = true;
                }
            }
        }
        public void Stop() {
            lock (timer) {
                if (isRunning) {
                    timer.Stop();
                    isRunning = false;
                }
            }
        }

     // Better not change this value - used to speed up unit testing
        public uint TickFactor {
            get {
                return tickFactor;
            }
            set {
                tickFactor = value;
                timer.Interval = tickInterval * tickFactor;
            }
        }

        bool processingNow = false;
        
        IDictionary<string, DUData> previousTotals = new Dictionary<string, DUData>();
        IDictionary<string, DUData> currentTotals;

        void timer_Elapsed(object sender, ElapsedEventArgs e) {
            onTick();
        }

        public void onTick(){
            lock (this) {
                if (processingNow) {
                 // Just bail out if we are still processing the last one
                    Log.warn("onTick aborting, processingNow flag is set");
                    return;
                } else {
                    Log.info("onTick proceeding");
                    processingNow = true;
                }
            }

            try {
                //uint timeNow = TimeUtils.getTimeValue();
                currentTotals = new Dictionary<string, DUData>();

                string interfaceName = null;

                ICollection<DUData> dataList = dataFactory.getData();

             // This loop populates the 'currentTotals' Dictionary
                foreach (DUData data in dataList) {
                    try {
                        interfaceName = null;
                        interfaceName = data.Id;

                        Debug.Assert(!currentTotals.ContainsKey(interfaceName)); // Shouldn't be any duplicate interface names
                        currentTotals[interfaceName] = data;

                    } catch (ApplicationException ex) {
                        Log.warn("Error while querying interface" + (interfaceName == null ? "" : " '" + interfaceName + "'") + ", the error was: " + ex.ToString());
                    }
                }
                
			 // List of data that will get sent to the server this time
                List<MsgData> msgDataList = new List<MsgData>(); 
                
             // List of data that we can't use just yet because we have no previous readings to compare them to
                //List<DUData> dataFromNewInterfaces = new List<DUData>();

                DUData previousDuData, currentDuData;
                uint dlDiff, ulDiff;
                MsgData msgData;

             // Look at all the data we got this time, and check to see if we have anything to compare it to
                foreach (string ifName in currentTotals.Keys) {
                    if (previousTotals.ContainsKey(ifName)) {
                        // We have a previous value for this interface
                        previousDuData = previousTotals[ifName];

                        currentDuData = currentTotals[ifName];

                        // Calculate the deltas - we are interested in the differences since last time, not the actual values
                        dlDiff = currentDuData.DlTotal - previousDuData.DlTotal;
                        ulDiff = currentDuData.UlTotal - previousDuData.UlTotal;

                        msgData = new MsgData(ifName, currentDuData.Ts, currentDuData.Ts - previousDuData.Ts, dlDiff, ulDiff);
                        msgDataList.Add(msgData);
                    }
                }

                if (msgDataList.Count > 0) {
                    string msgToSend = MessageBuilder.buildMessage(msgDataList);

                    // This is a blocking send
                    dispatcher.send(msgToSend);
                }

             // If we have got to here then we can assume the send succeded, and we adjust the state of the current object
                foreach (string ifName in currentTotals.Keys) {
                    previousTotals[ifName] = currentTotals[ifName];
                }
                
            } catch (Exception ex){
                // TODO notify user here?
                Log.error("Error in main processing routine: " + ex.ToString());

            } finally {
                lock (this) {
                    processingNow = false;
                }
            }

        } //internal void onTick(){

        public void Dispose() {
            if (dispatcher != null){
                dispatcher.Dispose();
                dispatcher = null;
            }
            GC.SuppressFinalize(this);
        }

        ~Controller() {
            Dispose();
        }
    }
}
