using System;
using System.Collections.Generic;
using System.Text;
using bitmeter.utils;
using System.Net.NetworkInformation;
using System.Net;

namespace bitmeter.capture {
    public class DUDataFactory : DUDataFactoryBase {
        
        public override ICollection<DUData> getData() {
            List<DUData> dataList = new List<DUData>();
            DUData data;
            foreach (NetworkInterface networkInterface in NetworkInterface.GetAllNetworkInterfaces()) {
                if (isIncluded(networkInterface)) {
                    data = makeDataFromNetworkInterface(networkInterface);
                    dataList.Add(data);
                }
            }

            return dataList;
        }

        private DUData makeDataFromNetworkInterface(NetworkInterface networkInterface) {
            IPv4InterfaceStatistics stats = networkInterface.GetIPv4Statistics();
            string description = networkInterface.Description;
            string id = networkInterface.GetPhysicalAddress().ToString();
            uint dl = (uint) stats.BytesReceived;
            uint ul = (uint) stats.BytesSent;
            uint ts = TimeUtils.getTimeValue();

            DUData data = new DUData(id, description, dl, ul, ts);
            return data;
        }

        private bool isIncluded(NetworkInterface networkInterface) {
            if (networkInterface.NetworkInterfaceType == NetworkInterfaceType.Loopback) {
                return false;

            } else {
                string macAddress = networkInterface.GetPhysicalAddress().ToString();
                if (exclusionsByMacAddress.Contains(macAddress)){
                    return false;
                }

                string description = networkInterface.Description;
                if (exclusionsByDescription.Contains(description)){
                    return false;
                }
                
                return true;
            }
        }



    }
}
