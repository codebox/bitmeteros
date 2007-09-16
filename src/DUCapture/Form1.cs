using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using bitmeter.utils;

namespace bitmeter.capture {
    public partial class Form1 : Form {
        public Form1() {
            InitializeComponent();
        }
        private const string CONFIG_FILE = "config.txt";
        static Controller controller;
        static Form1 form1;
        [STAThread]
        static void Main() {
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
            form1 = new Form1();
            Application.Run(form1);
        }

        static void socket_DataReceived(System.Net.IPEndPoint endPoint, string data) {
            Log.info("DATA: " + data);
        }

        static void socket_ConnectionReceived(System.Net.IPEndPoint endPoint) {
            Log.info("CONNECTION");
        }

        private void button1_Click(object sender, EventArgs e) {
            controller.Start();
        }

        private void button2_Click(object sender, EventArgs e) {
            controller.Stop();
        }
    }
}