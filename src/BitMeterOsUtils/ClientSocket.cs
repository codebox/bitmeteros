using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using System.Net;

namespace bitmeter.utils {
    public class ClientSocket : IDisposable {
        string serverHost;
        int serverPort;
        bool stayConnected;
        Socket socket;
        IPEndPoint connDestination;

        public ClientSocket(string serverHost, int serverPort, bool stayConnected) {
            this.serverHost = serverHost;
            IPAddress ipAddress = SocketUtils.resolveHostNameToIpAddress(serverHost);
            connDestination = new IPEndPoint(ipAddress, serverPort);

            SocketUtils.validatePort(serverPort);
            this.serverPort = serverPort;

            this.stayConnected = stayConnected;
        }


        public void Send(string data) {
         // performs a blocking send 
            lock (this) {
                if (socket == null ){ //|| !socket.Poll(-1, SelectMode.SelectWrite)) {
                    Connect();
                }
                byte[] dataToSend = System.Text.Encoding.ASCII.GetBytes(data);
                try {
                    socket.Send(dataToSend);
                } catch (SocketException) {
                    //TODO better
                    Connect();
                    socket.Send(dataToSend);
                }
                
                //send
                if (!stayConnected) {
                    Disconnect();
                }
            }
        }

        private void Connect() {
            lock (this) {
                socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                socket.Connect(connDestination);
            }
        }
        private void Disconnect() {
            lock (this) {
                socket.Shutdown(SocketShutdown.Both);
                socket.Close();
                socket = null;
            }
        }

        public void Dispose() {
            if (socket != null){
                socket.Shutdown(SocketShutdown.Both);
                socket.Close();
                socket = null;
            }
            GC.SuppressFinalize(this);
        }

        ~ClientSocket() {
            Dispose();
        }
    }
}
