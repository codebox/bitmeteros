using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.capture {
    public interface IDUDataFactory {
        ICollection<DUData> getData();
        void setExclusionByMacAddress(string macAddress);
        void setExclusionByName(string name);
        int resetExclusions();
        void setExternalDataSource(string external);
    }
}
