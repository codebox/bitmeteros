using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
using System.Net.Sockets;
using System.Net;
using System.Threading;

namespace bitmeter.utils {
    [TestFixture]
    public class SocketTest {
        const string HOST_OK = "127.0.0.1";
        const string HOST_ERR = "no.such.host.anywhere";
        const int PORT_OK   = 21123;
        const int PORT_ERR = -21123;
        const int WAIT_TIMEOUT_MILLIS = 5000;
        string latestMessage = null;
        string allMessages = null;
        IPEndPoint latestConnectClient = null;
        IPEndPoint latestSendClient    = null;

        [Test]
        public void testManyClients() {
            testManyClients(10, true);
            testManyClients(10, false);
        }

        private void testManyClients(int clientCount, bool persistent) {
            ServerSocket serverSocket = buildNewListeningServerSocket();
            try {
                ClientSocket[] clients = new ClientSocket[clientCount];
                for (int i = 0; i < clientCount; i++) {
                    clients[i] = new ClientSocket(HOST_OK, PORT_OK, persistent);
                }

                const string CLIENT_MSG_BASE = "testManyClients test data ";
                string msg;
                for (int i = 0; i < clientCount; i++) {
                    msg = CLIENT_MSG_BASE + i;
                    resetLatest();
                    clients[i].Send(msg);
                    waitForData();
                    Assert.AreEqual(msg, latestMessage);

                    msg = CLIENT_MSG_BASE + i + " again";
                    resetLatest();
                    clients[i].Send(msg);
                    waitForData();
                    Assert.AreEqual(msg, latestMessage);
                }

            } finally {
                serverSocket.StopListening(true);
            }
        }

        [Test]
        public void testServerFailureForPersistentClient() {
         // Tests that a client with 'stayConnected=true' can correctly handle a server failure and recovery
            ServerSocket serverSocket = buildNewListeningServerSocket();
            try {
                const string CLIENT_MSG = "testServerFailureForPersistentClient test data";
                ClientSocket clientSocket = new ClientSocket(HOST_OK, PORT_OK, true);
                clientSocket.Send(CLIENT_MSG);

                waitForData();
                Assert.AreEqual(CLIENT_MSG, latestMessage);
                resetLatest();

             // close the socket and re-open it without an intervening send
                serverSocket.StopListening(true);
                serverSocket.Listen();

                clientSocket.Send(CLIENT_MSG);

                waitForData();
                Assert.AreEqual(CLIENT_MSG, latestMessage);
                resetLatest();

             // close the socket and re-open it with an intervening send
                serverSocket.StopListening(true);

                try {
                    clientSocket.Send(CLIENT_MSG);
                    Assert.Fail("Expected SocketException");
                } catch (SocketException ex) {
                    // ok
                }
                serverSocket.Listen();

                clientSocket.Send(CLIENT_MSG);
                waitForData();
                Assert.AreEqual(CLIENT_MSG, latestMessage);
                resetLatest();


            } finally {
                serverSocket.StopListening(true);
            }


        }

        [Test]
        public void testServerEvents() {
            ServerSocket serverSocket = buildNewListeningServerSocket();

            try {
                const string CLIENT_MSG = "testServerEvents test data";
             // Try with a client socket that keeps its connection between sends
                ClientSocket clientSocketPersistent = new ClientSocket(HOST_OK, PORT_OK, true);
                clientSocketPersistent.Send(CLIENT_MSG);
                waitForData();

                Assert.AreEqual(CLIENT_MSG, latestMessage);
                Assert.AreEqual(IPAddress.Parse(HOST_OK), latestConnectClient.Address);
                Assert.AreEqual(IPAddress.Parse(HOST_OK), latestSendClient.Address);
                resetLatest();

                clientSocketPersistent.Send(CLIENT_MSG);
                waitForData();

                Assert.AreEqual(CLIENT_MSG, latestMessage);
                Assert.AreEqual(IPAddress.Parse(HOST_OK), latestSendClient.Address);
                resetLatest();

             // Try with a client socket that doesn't keep its connection between sends
                ClientSocket clientSocketNonPersistent = new ClientSocket(HOST_OK, PORT_OK, false);
                clientSocketNonPersistent.Send(CLIENT_MSG);
                waitForData();

                //Log.info("char1=" + latestMessage[0]);
                Assert.AreEqual(CLIENT_MSG, latestMessage);
                Assert.AreEqual(IPAddress.Parse(HOST_OK), latestConnectClient.Address);
                Assert.AreEqual(IPAddress.Parse(HOST_OK), latestSendClient.Address);
                resetLatest();

                clientSocketNonPersistent.Send(CLIENT_MSG);
                waitForData();

                Assert.AreEqual(CLIENT_MSG, latestMessage);
                Assert.AreEqual(IPAddress.Parse(HOST_OK), latestConnectClient.Address);
                Assert.AreEqual(IPAddress.Parse(HOST_OK), latestSendClient.Address);
                resetLatest();

            } finally {
                serverSocket.StopListening(true);
            }
        }

        [Test]
        public void testClientConnectFailure() {
            try {
                ClientSocket clientSocket = new ClientSocket(HOST_OK, PORT_OK, true);
                clientSocket.Send("some data");
                Assert.Fail("Expected SocketException");
            } catch (SocketException) {
                // ok
            }
        }

        [Test]
        public void testBadClientParams() {
            try{
                ClientSocket clientSocket = new ClientSocket(HOST_OK, PORT_ERR, true);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException){
                // ok
            }

            try {
                ClientSocket clientSocket = new ClientSocket(HOST_ERR, PORT_OK, true);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // ok
            }
        }

        [Test]
        public void testBadServerParams() {
            try {
                ServerSocket serverSocket = new ServerSocket(PORT_ERR);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // ok
            }
        }

        object dataSyncObject = new object();
        private void waitForData() {
            lock (dataSyncObject) {
                if (latestMessage != null) {
                    return;
                } else {
                    bool gotLock = Monitor.Wait(dataSyncObject, WAIT_TIMEOUT_MILLIS);
                    if (!gotLock) {
                        Assert.Fail("waitForData wait timeout expired");
                    }
                }
            }
        }
        public void onData(IPEndPoint endPoint, string data) {
            lock (dataSyncObject) {
                latestSendClient = endPoint;
                latestMessage = data;
                allMessages += (data + "\n");
                Monitor.PulseAll(dataSyncObject);
            }
        }

        object connectionSyncObject = new object();
        private void waitForConnection() {
            lock (connectionSyncObject) {
                if (latestConnectClient != null) {
                    return;
                } else {
                    bool gotLock = Monitor.Wait(connectionSyncObject, WAIT_TIMEOUT_MILLIS);
                    if (!gotLock) {
                        Assert.Fail("waitForConnection wait timeout expired");
                    }
                }
            }
        }
        public void onConnection(IPEndPoint endPoint) {
            lock (connectionSyncObject) {
                latestConnectClient = endPoint;
                Monitor.PulseAll(connectionSyncObject);
            }
        }

        private ServerSocket buildNewListeningServerSocket() {
            ServerSocket serverSocket = new ServerSocket(PORT_OK);
            serverSocket.ConnectionReceived += new ConnectionListener(onConnection);
            serverSocket.DataReceived += new DataListener(onData);
            resetLatest();
            serverSocket.Listen();

            return serverSocket;
        }

        private void resetLatest() {
            allMessages = null;
            latestMessage = null;
            latestConnectClient = null;
            latestSendClient = null;
        }

    }
}
