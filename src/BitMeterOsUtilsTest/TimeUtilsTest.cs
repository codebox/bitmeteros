using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
using System.Threading;

namespace bitmeter.utils {
    [TestFixture]
    public class TimeUtilsTest {
        [Test]
        public void testGetTimeValue() {
            uint time1 = TimeUtils.getTimeValue();
            Thread.Sleep(1000);
            uint time2 = TimeUtils.getTimeValue();
            Assert.IsTrue(time2 - time1 > 0);
        }

        [Test]
        public void testGetDateFromTimeValue() {
            DateTime date = TimeUtils.getDateFromTimeValue(0);
            Assert.AreEqual(DateTime.Parse("1 Jan 2000 00:00:00"), date);

            date = TimeUtils.getDateFromTimeValue(100000000);
            Assert.AreEqual(DateTime.Parse("3 Mar 2003 09:46:40"), date);

            date = TimeUtils.getDateFromTimeValue(1000000000);
            Assert.AreEqual(DateTime.Parse("9 Sep 2031 01:46:40"), date);

        }
    }
}
