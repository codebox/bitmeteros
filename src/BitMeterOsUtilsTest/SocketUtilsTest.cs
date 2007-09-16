using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
using System.Net;

namespace bitmeter.utils {
    [TestFixture]
    public class SocketUtilsTest {
        [Test]
        public void testPortNumbers() {
            SocketUtils.validatePort(0);
            SocketUtils.validatePort(80);
            SocketUtils.validatePort(65535);

            try {
                SocketUtils.validatePort(-1);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // ok
            }

            try {
                SocketUtils.validatePort(-1000000);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // ok
            }

            try {
                SocketUtils.validatePort(65536);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // ok
            }

            try {
                SocketUtils.validatePort(1000000);
                Assert.Fail("Expected ArgumentException");
            } catch (ArgumentException) {
                // ok
            }
        }

        [Test]
        public void testDns() {
            string ipString = SocketUtils.resolveHostNameToIpString("localhost");
            Assert.AreEqual("127.0.0.1", ipString);

            IPAddress ipAddress = SocketUtils.resolveHostNameToIpAddress("localhost");
            Assert.AreEqual(IPAddress.Parse("127.0.0.1"), ipAddress);
        }
    }
}
