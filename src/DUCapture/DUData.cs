using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.capture
{
    public class DUData {
        private uint dl, ul, ts;
        private string descriptiveName, id;

        public DUData(string id, string descriptiveName, uint dlTotal, uint ulTotal, uint ts) {
            this.id = id;
            this.descriptiveName = descriptiveName;
            this.dl = dlTotal;
            this.ul = ulTotal;
            this.ts = ts;
        }

        public uint DlTotal {
            get {
                return dl;
            }
        }

        public uint UlTotal {
            get {
                return ul;
            }
        }

        public uint Ts {
            get {
                return ts;
            }
        }

        public string DescriptiveName {
            get {
                return descriptiveName;
            }
        }

        public string Id {
            get {
                return id;
            }
        }
    }
}
