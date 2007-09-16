using System;
using System.Runtime.InteropServices;

namespace bitmeter.serviceinstaller {
    class ServiceInstaller {

        [DllImport("advapi32.dll")]
        public static extern IntPtr OpenSCManager(string lpMachineName, string lpSCDB, int scParameter);
        [DllImport("Advapi32.dll")]
        public static extern IntPtr CreateService(IntPtr SC_HANDLE, string lpSvcName, string lpDisplayName,
        int dwDesiredAccess, int dwServiceType, int dwStartType, int dwErrorControl, string lpPathName,
        string lpLoadOrderGroup, int lpdwTagId, string lpDependencies, string lpServiceStartName, string lpPassword);
        [DllImport("advapi32.dll")]
        public static extern void CloseServiceHandle(IntPtr SCHANDLE);
        [DllImport("advapi32.dll")]
        public static extern int StartService(IntPtr SVHANDLE, int dwNumServiceArgs, string lpServiceArgVectors);
        [DllImport("advapi32.dll", SetLastError = true)]
        public static extern IntPtr OpenService(IntPtr SCHANDLE, string lpSvcName, int dwNumServiceArgs);
        [DllImport("advapi32.dll")]
        public static extern int DeleteService(IntPtr SVHANDLE);
        [DllImport("kernel32.dll")]
        public static extern int GetLastError();

        private static string INSTALL = "INSTALL";
        private static string UNINSTALL = "UNINSTALL";

        private static string USAGE = "Usage:\n  ServiceInstaller.exe " +
                INSTALL + "   <Path to EXE> <Service Name> <Display Name>\n  ServiceInstaller.exe " +
                UNINSTALL + " <Service Name>";

        [STAThread]
        static void Main(string[] args) {
            if (args.Length == 0) {
                Console.WriteLine(USAGE);

            } else {
                if (INSTALL.Equals(args[0])) {
                    if (args.Length != 4) {
                        Console.WriteLine(USAGE);

                    } else {
                        string path = args[1];
                        string name = args[2];
                        string description = args[3];

                        ServiceInstaller c = new ServiceInstaller();
                        if (c.InstallService(path, name, description)) {
                            Console.WriteLine("Service '" + name + "' installed successfully");
                        } else {
                            Console.WriteLine("Unable to install the '" + name + "' service");
                        }

                    }

                } else if (UNINSTALL.Equals(args[0])) {
                    if (args.Length != 2) {
                        Console.WriteLine(USAGE);

                    } else {
                        string name = args[1];

                        ServiceInstaller c = new ServiceInstaller();
                        if (c.UnInstallService(name)) {
                            Console.WriteLine("Service '" + name + "' uninstalled successfully");
                        } else {
                            Console.WriteLine("Unable to uninstall the '" + name + "' service");
                        }

                    }
                }
            }
        }

        public bool InstallService(string svcPath, string svcName, string svcDispName) {
            int SC_MANAGER_CREATE_SERVICE = 0x0002;
            int SERVICE_WIN32_OWN_PROCESS = 0x00000010;
            int SERVICE_ERROR_NORMAL = 0x00000001;
            int STANDARD_RIGHTS_REQUIRED = 0xF0000;
            int SERVICE_QUERY_CONFIG = 0x0001;
            int SERVICE_CHANGE_CONFIG = 0x0002;
            int SERVICE_QUERY_STATUS = 0x0004;
            int SERVICE_ENUMERATE_DEPENDENTS = 0x0008;
            int SERVICE_START = 0x0010;
            int SERVICE_STOP = 0x0020;
            int SERVICE_PAUSE_CONTINUE = 0x0040;
            int SERVICE_INTERROGATE = 0x0080;
            int SERVICE_USER_DEFINED_CONTROL = 0x0100;
            int SERVICE_ALL_ACCESS = (STANDARD_RIGHTS_REQUIRED |
                    SERVICE_QUERY_CONFIG |
                    SERVICE_CHANGE_CONFIG |
                    SERVICE_QUERY_STATUS |
                    SERVICE_ENUMERATE_DEPENDENTS |
                    SERVICE_START |
                    SERVICE_STOP |
                    SERVICE_PAUSE_CONTINUE |
                    SERVICE_INTERROGATE |
                    SERVICE_USER_DEFINED_CONTROL);
            int SERVICE_AUTO_START = 0x00000002;

            IntPtr sc_handle = OpenSCManager(null, null, SC_MANAGER_CREATE_SERVICE);
            if (sc_handle.ToInt32() != 0) {
                IntPtr sv_handle = CreateService(sc_handle, svcName, svcDispName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, svcPath, null, 0, null, null, null);
                if (sv_handle.ToInt32() == 0) {
                    Console.WriteLine("Unable to create service");
                    CloseServiceHandle(sc_handle);
                    return false;

                } else {
                    int i = StartService(sv_handle, 0, null);
                    // If the value i is zero, then there was an error starting the service.
                    // note: error may arise if the service is already running or some other problem.
                    if (i == 0) {
                        Console.WriteLine("Service installed ok, but unable to start");
                        return false;
                    }
                    CloseServiceHandle(sc_handle);
                    return true;
                }
            } else {
                Console.WriteLine("Unable to open Service Control Manager");
                return false;
            }
        }

        public bool UnInstallService(string svcName) {
            int GENERIC_WRITE = 0x40000000;
            IntPtr sc_hndl = OpenSCManager(null, null, GENERIC_WRITE);
            if (sc_hndl.ToInt32() != 0) {
                int DELETE = 0x10000;
                IntPtr svc_hndl = OpenService(sc_hndl, svcName, DELETE);

                if (svc_hndl.ToInt32() != 0) {
                    int i = DeleteService(svc_hndl);
                    if (i != 0) {
                        CloseServiceHandle(sc_hndl);
                        return true;
                    } else {
                        Console.WriteLine("Unable to delete the service");
                        CloseServiceHandle(sc_hndl);
                        return false;
                    }
                } else {
                    Console.WriteLine("Unable to open the service");
                    return false;
                }
            } else {
                Console.WriteLine("Unable to open Service Control Manager");
                return false;

            }
        }
    }
}