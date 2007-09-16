using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.capture {
    public abstract class DUDataFactoryBase : IDUDataFactory {
        protected IList<string> exclusionsByMacAddress = new List<string>();
        protected IList<string> exclusionsByDescription = new List<string>();
        protected IList<string> externalDataSources = new List<string>();

        public void setExclusionByMacAddress(string macAddress) {
            exclusionsByMacAddress.Add(macAddress);
        }

        public void setExclusionByName(string name) {
            exclusionsByDescription.Add(name);
        }

        public int resetExclusions() {
            int exclusionCountBeforeReset = exclusionsByMacAddress.Count + exclusionsByDescription.Count;

            exclusionsByMacAddress = new List<string>();
            exclusionsByDescription = new List<string>();

            return exclusionCountBeforeReset;
        }

        public void setExternalDataSource(string externalDataSource) {
            externalDataSources.Add(externalDataSource);
        }
        public int resetExternalDataSources() {
            int externalCountBeforeReset = externalDataSources.Count;

            externalDataSources = new List<string>();

            return externalCountBeforeReset;
        }

        public abstract ICollection<DUData> getData();
    }
}
