using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;

namespace bitmeter.utils {
    public class SocketUtils {
        const int MAX_PORT = 65535;

        public static void validatePort(int port) {
            if (port < 0) {
                throw new ArgumentException("Invalid port number '" + port + "' - cannot be less than 0");
            } else {
                validatePort((uint)port);
            }
        }
        public static void validatePort(uint port) {
            if (port > MAX_PORT) {
                throw new ArgumentException("Invalid port number '" + port + "' - cannot be greater than " + MAX_PORT);
            }
        }

        public static string resolveHostNameToIpString(string hostName) {
            return resolveHostNameToIpAddress(hostName).ToString();
        }
        public static IPAddress resolveHostNameToIpAddress(string hostName) {
            try {
                return Dns.GetHostEntry(hostName).AddressList[0];
            } catch (SocketException ex) {
                throw new ArgumentException("Unable to resolve the host name '" + hostName + "'", ex);
            }
        }
    }
}
