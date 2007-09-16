using System;
using System.Collections.Generic;
using System.Text;
using System.Net.Sockets;
using System.Net;

namespace bitmeter.utils {
    public delegate void DataListener(IPEndPoint endPoint, string data);
    public delegate void ConnectionListener(IPEndPoint endPoint);

    public class ServerSocket : IDisposable {
        public event DataListener DataReceived;
        public event ConnectionListener ConnectionReceived;
        Socket listenerSocket;
        IPEndPoint localPort;
        const int LISTEN_QUEUE_SIZE = 16;
        const int RECEIVE_BUFFER_SIZE = 1024;
        IList<WorkerSocket> workerSockets;
        bool acceptConnections = false;

        class WorkerSocket {
            Socket socket;
            byte[] dataBuffer;

            public WorkerSocket(Socket socket) {
                this.socket = socket;
                this.dataBuffer = new byte[RECEIVE_BUFFER_SIZE];
            }

            public byte[] DataBuffer {
                get {
                    return this.dataBuffer;
                }
            }

            public Socket Socket {
                get {
                    return this.socket;
                }
            }
        }

        int portNumber;
        public ServerSocket(int portNumber) {
            this.portNumber = portNumber;
            localPort = new IPEndPoint(IPAddress.Any, portNumber);
            workerSockets = new List<WorkerSocket>();
        }

        public void Listen() {
            if (listenerSocket == null) {
                listenerSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                listenerSocket.Bind(localPort);
                listenerSocket.Listen(LISTEN_QUEUE_SIZE);
                listenerSocket.BeginAccept(new AsyncCallback(OnConnect), null);
                acceptConnections = true;
            }
        }

        private void OnConnect(IAsyncResult asyncResult) {
            if (acceptConnections) {
                Socket socket = listenerSocket.EndAccept(asyncResult);

                Log.info("Server socket connected");
                if (ConnectionReceived != null) {
                    ConnectionReceived((IPEndPoint)socket.RemoteEndPoint);
                }

                WorkerSocket workerSocket = new WorkerSocket(socket);
                workerSockets.Add(workerSocket);
                socket.BeginReceive(workerSocket.DataBuffer, 0, workerSocket.DataBuffer.Length, SocketFlags.None, new AsyncCallback(OnData), workerSocket);
                listenerSocket.BeginAccept(new AsyncCallback(OnConnect), null);
            } 
        }

        private void OnData(IAsyncResult asyncResult) {
            if (acceptConnections) {
                WorkerSocket workerSocket = (WorkerSocket)asyncResult.AsyncState;
                int bytesReceived = workerSocket.Socket.EndReceive(asyncResult);
                if (bytesReceived > 0) {
                    char[] charData = new char[bytesReceived];

                    System.Text.Decoder decoder = System.Text.Encoding.UTF8.GetDecoder();
                    int charLen = decoder.GetChars(workerSocket.DataBuffer, 0, bytesReceived, charData, 0);
                    String stringData = new String(charData);

                    Log.info("Server socket received: " + stringData + " " + bytesReceived + " " + charLen);
                    if (DataReceived != null) {
                        DataReceived((IPEndPoint)workerSocket.Socket.RemoteEndPoint, stringData);
                    }

                    workerSocket.Socket.BeginReceive(workerSocket.DataBuffer, 0, workerSocket.DataBuffer.Length, SocketFlags.None, new AsyncCallback(OnData), workerSocket);
                }
            }
        }

        public void StopListening(bool closeWorkers) {
            acceptConnections = false;
            if (listenerSocket != null) {
                //listenerSocket.Shutdown(SocketShutdown.Both);
                listenerSocket.Close();
                listenerSocket = null;
            }

            if (closeWorkers) {
                if (workerSockets != null) {
                    foreach (WorkerSocket workerSocket in workerSockets) {
                        //workerSocket.Socket.Shutdown(SocketShutdown.Both);
                        workerSocket.Socket.Close();
                    }
                }
            }
        }

        public void Dispose() {
            StopListening(true);
            GC.SuppressFinalize(this);
        }

        ~ServerSocket() {
            Dispose();
        }
    }
}
