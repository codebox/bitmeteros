using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.ServiceProcess;
using System.Text;
using bitmeter.utils;

namespace bitmeter.capture {
    public partial class DUCaptureService : ServiceBase {
        private const string CONFIG_FILE = "config.txt";
        Controller controller;

        public DUCaptureService() {
            InitializeComponent();
        }

        protected override void OnStart(string[] args) {
            Config configFile = new Config(CONFIG_FILE);

            string dbHost = configFile.getValue("capture.du.db.host");
            int dbPort = configFile.getIntValue("capture.du.db.port");
            bool stayConnected = configFile.getBoolValue("capture.du.conn.persist");
            IMessageDispatcher messageDispatcher = new MessageDispatcher(dbHost, dbPort, stayConnected);
            
            IDUDataFactory dataSourceFactory = new DUDataFactory();
            IDictionary<string, string> excludedByMacAdapters = configFile.getValues("capture.du.exclude.mac.");
            foreach (string mac in excludedByMacAdapters.Values) {
                dataSourceFactory.setExclusionByMacAddress(mac);
            }

            IDictionary<string, string> excludedByDscAdapters = configFile.getValues("capture.du.exclude.desc.");
            foreach (string desc in excludedByDscAdapters.Values) {
                dataSourceFactory.setExclusionByName(desc);
            }

            ServerSocket socket = new ServerSocket((int)dbPort);
            socket.ConnectionReceived += new ConnectionListener(socket_ConnectionReceived);
            socket.DataReceived += new DataListener(socket_DataReceived);
            socket.Listen();

            uint tickInterval = configFile.getUIntValue("capture.du.timer.pollInterval");
            controller = new Controller(dataSourceFactory, messageDispatcher, tickInterval);
            controller.Start();
        }

        protected override void OnStop() {
            controller.Stop();
            controller.Dispose();
            controller = null;
        }

        void socket_DataReceived(System.Net.IPEndPoint endPoint, string data) {
            Log.info("DATA: " + data);
        }

        void socket_ConnectionReceived(System.Net.IPEndPoint endPoint) {
            Log.info("CONNECTION");
        }

    }
}
