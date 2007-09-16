using System;
using System.Collections.Generic;
using System.Text;
using NUnit.Framework;
using System.IO;

namespace bitmeter.utils {
    [TestFixture]
    public class ConfigTest {
        [TestFixtureSetUp]
        public void setUp() {
            File.Copy(@"..\..\resources\testconfig.bak.txt", @"..\..\resources\testconfig.txt", true);
        }

        [TestFixtureTearDown]
        public void tearDown() {
            File.Copy(@"..\..\resources\testconfig.bak.txt", @"..\..\resources\testconfig.txt", true);
        }

        [Test]
        public void testGetMethods() {
            Config config = new Config(@"..\..\resources\testconfig.txt");
            Assert.AreEqual("hello world", config.getValue("string.value.ok"));
            Assert.AreEqual("hello world", config.getValue("string.value.spaces"));
            Assert.AreEqual("", config.getValue("string.value.empty"));
            Assert.AreEqual(123, config.getIntValue("int.value.ok"));
            Assert.AreEqual(234, config.getIntValue("uint.value.ok"));

            try {
                config.getUIntValue("uint.value.bad");
                Assert.Fail("Expected a FormatException");
            } catch (FormatException) {
                // ok
            }

            try {
                config.getIntValue("int.value.bad");
                Assert.Fail("Expected a FormatException");
            } catch (FormatException) {
                // ok
            }

            Assert.AreEqual(true, config.getBoolValue("bool.value.ok1"));
            Assert.AreEqual(true, config.getBoolValue("bool.value.ok2"));
            Assert.AreEqual(false, config.getBoolValue("bool.value.ok3"));
            Assert.AreEqual(false, config.getBoolValue("bool.value.ok4"));

            try {
                config.getBoolValue("bool.value.bad");
                Assert.Fail("Expected a FormatException");
            } catch (FormatException) {
                // ok
            }

            try {
                config.getValue("non.existant.value");
                Assert.Fail("Expected an ApplicationException");
            } catch (ApplicationException) {
                // ok
            }
        }

        [Test]
        public void testGetMultiple() {
            Config config = new Config(@"..\..\resources\testconfig.txt");
            IDictionary<string, string> matches = config.getValues("bool.");
            Assert.AreEqual(5, matches.Keys.Count);

            matches = config.getValues("nomatchesforthis");
            Assert.AreEqual(0, matches.Keys.Count);

            matches = config.getValues("");
            Assert.AreEqual(12, matches.Keys.Count);
        }

        [Test]
        public void testSetMethods() {
            const string KEY_NAME = "new.value";
            const string KEY_VALUE = "test value";

            Config config = new Config(@"..\..\resources\testconfig.txt");

         // Test with a completely new value
            Assert.IsFalse(config.hasValue("new.value"));
            config.setValue(KEY_NAME, KEY_VALUE);
            Assert.IsTrue(config.hasValue("new.value"));
            Assert.AreEqual(KEY_VALUE, config.getValue(KEY_NAME));

         // Test with a pre-existing value
            const string NEW_VALUE = "a new value";
            config.setValue(KEY_NAME, NEW_VALUE);
            Assert.AreEqual(NEW_VALUE, config.getValue(KEY_NAME));

         // Test boolean set
            const bool BOOL_VALUE = true;
            config.setValue(KEY_NAME, BOOL_VALUE);
            Assert.AreEqual(BOOL_VALUE, config.getBoolValue(KEY_NAME));

         // Test boolean set
            const uint UINT_VALUE = 123456;
            config.setValue(KEY_NAME, UINT_VALUE);
            Assert.AreEqual(UINT_VALUE, config.getUIntValue(KEY_NAME));

        }

        [Test]
        public void testBadFile() {
            try {
                new Config(@"..\..\resources\notthere.txt");
            } catch (FileNotFoundException) {
                // ok
            }
        }
    }
}
