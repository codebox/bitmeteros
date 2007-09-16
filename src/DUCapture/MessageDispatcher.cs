using System;
using System.Collections.Generic;
using System.Text;
using bitmeter.utils;

namespace bitmeter.capture {
    public class MessageDispatcher : IMessageDispatcher {
        string dbHost;
        int dbPort;
        bool stayConnected;
        ClientSocket socket;

        public MessageDispatcher(string dbHost, int dbPort, bool stayConnected) {
            this.dbHost = dbHost;
            this.dbPort = dbPort;
            this.stayConnected = stayConnected;

            socket = new ClientSocket(dbHost, dbPort, stayConnected);
        }

        #region IMessageDispatcher Members

        public void send(string message) {
            Log.info(message);
            socket.Send(message);
        }

        #endregion

        public void Dispose() {
            if (socket != null){
                socket.Dispose();
                socket = null;
            }
            GC.SuppressFinalize(this);
        }

        ~MessageDispatcher() {
            Dispose();
        }
    }
}
