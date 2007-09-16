using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;

namespace bitmeter.capture {
    [TestFixture]
    public class DUDataFactoryBaseTest {
        class TestFactory : DUDataFactoryBase {
            public override ICollection<DUData> getData() {
                return null;
            }
            public IList<string> getExclusionsByMacAddress() {
                return exclusionsByMacAddress;
            }
            public IList<string> getExclusionsByDescription() {
                return exclusionsByDescription;
            }
            public IList<string> getExternalDataSources() {
                return externalDataSources;
            }
        }

        const string MAC_1  = "11111111";
        const string MAC_2  = "22222222";
        const string DESC_1 = "Description 1";
        const string DESC_2 = "Description 2";
        const string EXT_1  = "External 1";
        const string EXT_2  = "External 2";

        [Test]
        public void testExclusions() {
            TestFactory factory = new TestFactory();

            Assert.AreEqual(0, factory.getExclusionsByMacAddress().Count);
            factory.setExclusionByMacAddress(MAC_1);
            Assert.AreEqual(1, factory.getExclusionsByMacAddress().Count);
            factory.setExclusionByMacAddress(MAC_2);
            Assert.AreEqual(2, factory.getExclusionsByMacAddress().Count);

            Assert.IsTrue(factory.getExclusionsByMacAddress().Contains(MAC_1));
            Assert.IsTrue(factory.getExclusionsByMacAddress().Contains(MAC_2));

            Assert.AreEqual(0, factory.getExclusionsByDescription().Count);
            factory.setExclusionByName(DESC_1);
            Assert.AreEqual(1, factory.getExclusionsByDescription().Count);
            factory.setExclusionByName(DESC_2);
            Assert.AreEqual(2, factory.getExclusionsByDescription().Count);

            Assert.IsTrue(factory.getExclusionsByDescription().Contains(DESC_1));
            Assert.IsTrue(factory.getExclusionsByDescription().Contains(DESC_2));

            factory.resetExclusions();

            Assert.AreEqual(0, factory.getExclusionsByDescription().Count);
            Assert.AreEqual(0, factory.getExclusionsByMacAddress().Count);
        }

        [Test]
        public void testExternal() {
            TestFactory factory = new TestFactory();

            Assert.AreEqual(0, factory.getExternalDataSources().Count);
            factory.setExternalDataSource(EXT_1);
            Assert.AreEqual(1, factory.getExternalDataSources().Count);
            factory.setExternalDataSource(EXT_2);
            Assert.AreEqual(2, factory.getExternalDataSources().Count);

            Assert.IsTrue(factory.getExternalDataSources().Contains(EXT_1));
            Assert.IsTrue(factory.getExternalDataSources().Contains(EXT_2));

            factory.resetExternalDataSources();

            Assert.AreEqual(0, factory.getExternalDataSources().Count);
        }


    }
}
