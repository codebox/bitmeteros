using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace bitmeter.capture {
    [TestFixture]
    public class DUDataFactoryTest {
        [Test]
        public void test() {
            DUDataFactory factory = new DUDataFactory();
            IList<DUData> data = (IList<DUData>) factory.getData();

            int initialCount = data.Count;
            if (initialCount > 0) {
                factory.setExclusionByMacAddress(data[0].Id);
                data = (IList<DUData>) factory.getData();
                Assert.IsTrue(data.Count == initialCount - 1);

                factory.resetExclusions();
                data = (IList<DUData>)factory.getData();
                Assert.IsTrue(data.Count == initialCount);

                factory.setExclusionByName(data[0].DescriptiveName);
                data = (IList<DUData>)factory.getData();
                Assert.IsTrue(data.Count == initialCount - 1);

            }

        }
    }
}
