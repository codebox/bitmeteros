using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace bitmeter.utils {
    public class Config {
        private const string COMMENT = "#";
        private char[] DELIMITERS = new char[] { '=' };

        private string file;
        private IDictionary<string, string> configData = new Dictionary<string, string>();

        public Config(string file) {
            this.file = file;
            TextReader reader = new StreamReader(file);

            string thisLine;
            string[] parts;

            while ((thisLine = reader.ReadLine()) != null) {
                thisLine = thisLine.Trim();
                if (thisLine.Length > 0 && ! thisLine.StartsWith(COMMENT)) {
                    parts = thisLine.Split(DELIMITERS, 2);
                    if (parts.Length < 2) {
                        Log.warn("Bad line found in config file, no delimiter: '" + thisLine + "'");
                    } else {
                        configData.Add(parts[0].Trim(), parts[1].Trim());
                    }
                }
            }
            reader.Close();

        }

        public bool hasValue(string key) {
            return configData.ContainsKey(key);
        }

        public IDictionary<string, string> getValues(string keyStartsWith) {
            IDictionary<string, string> matches = new Dictionary<string, string>();
            lock (configData) {
                foreach (string key in configData.Keys) {
                    if (key.StartsWith(keyStartsWith)) {
                        matches.Add(key, configData[key]);
                    }
                }
            }
            return matches;
        }

        public string getValue(string key) {
            lock (configData) {
                if (configData.ContainsKey(key)) {
                    return configData[key];
                } else {
                    throw new ApplicationException("No config value with a key of '" + key + "' could be found in the file '" + this.file + "'");
                }
            }
        }

        public int getIntValue(string key) {
            return int.Parse(getValue(key));
        }

        public uint getUIntValue(string key) {
            return uint.Parse(getValue(key));
        }

        public bool getBoolValue(string key) {
            return bool.Parse(getValue(key));
        }

        public void setValue(string key, string value) {
            lock (configData) {
                bool dataHasChanged = false;
                if (configData.ContainsKey(key)) {
                    if (configData[key] != value) {
                        configData[key] = value;
                        dataHasChanged = true;
                    }
                } else {
                    configData.Add(key, value);
                    dataHasChanged = true;
                }
                if (dataHasChanged) {
                    doSave();
                }
            }
        }

        private void doSave() {
            TextWriter writer = new StreamWriter(this.file);
            foreach (string key in configData.Keys) {
                writer.WriteLine(key + DELIMITERS[0] + configData[key]);
            }
            writer.Close();
        }

        public void setValue(string key, uint value) {
            setValue(key, value.ToString());
        }
        public void setValue(string key, bool value) {
            setValue(key, value.ToString());
        }

    }
}
