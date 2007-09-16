using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.capture {
    class DummyMessageDispatcher : IMessageDispatcher{
        private string latestMessage = null;
        private int msgCount = 0;
        private bool barfOnSend = false;

        void IMessageDispatcher.send(string message) {
            if (barfOnSend) {
                throw new ApplicationException("barfing as instructed");
            }
            lock(this){
                latestMessage = message;
                msgCount++;
            }
        }

        public string LatestMessage{
            get {
                return latestMessage;
            }
            set {
                this.latestMessage = value;
            }
        }

        public int MsgCount {
            get {
                return msgCount;
            }
        }

        public bool BarfOnSend {
            get {
                return barfOnSend;
            }
            set {
                barfOnSend = value;
            }
        }

        public void Dispose() { }
    }
}
