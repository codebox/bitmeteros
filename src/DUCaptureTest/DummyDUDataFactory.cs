using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace bitmeter.capture {
    class DummyDUDataFactory : DUDataFactoryBase {
        IDictionary<string, DUData> dataMap = new Dictionary<string,DUData>();
        int delay = 0;

        public override ICollection<DUData> getData() {
            if (delay > 0) {
                Thread.Sleep(delay);
            }
            return dataMap.Values;
        }

        internal DUData getData(string name) {
            if (delay > 0) {
                Thread.Sleep(delay);
            }
            return dataMap[name];
        }
        
        internal void setDataValues(string id, string name, uint dl, uint ul, uint ts) {
            dataMap[name] = new DUData(id, name, dl, ul, ts);
        }

        internal void removeDataValues(string id){
            dataMap.Remove(id);
        }

        internal int Delay {
            get {
                return delay;
            }
            set {
                delay = value;
            }
        }
    }
}
