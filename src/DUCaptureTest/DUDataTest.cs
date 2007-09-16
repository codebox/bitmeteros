using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace bitmeter.capture {
    [TestFixture]
    public class DUDataTest {
        [Test]
        public void test() {
            const string ID = "ID";
            const string DESC = "DESC";
            const uint DL = 123;
            const uint UL = 345;
            const uint TS = 1234;

            DUData duData = new DUData(ID, DESC, DL, UL, TS);
            Assert.AreEqual(ID, duData.Id);
            Assert.AreEqual(DESC, duData.DescriptiveName);
            Assert.AreEqual(DL, duData.DlTotal);
            Assert.AreEqual(UL, duData.UlTotal);
            Assert.AreEqual(TS, duData.Ts);
        }
    }
}
