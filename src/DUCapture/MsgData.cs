using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.capture
{
    public struct MsgData {
        private uint dl, ul, timestamp, interval;
        private string interfaceName;

        public MsgData(string interfaceName, uint timestamp, uint interval, uint dl, uint ul) {
            this.interfaceName = interfaceName;
            this.timestamp = timestamp;
            this.interval = interval;
            this.dl = dl;
            this.ul = ul;
        }

        public string Interface{
            get {
                return interfaceName;
            }
        }

        public uint Timestamp{
            get {
                return timestamp;
            }
        }

        public uint Interval{
            get {
                return interval;
            }
        }

        public uint Dl{
            get {
                return dl;
            }
        }

        public uint Ul{
            get {
                return ul;
            }
        }
    }
}
