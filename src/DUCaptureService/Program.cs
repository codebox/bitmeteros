using System.Collections.Generic;
using System.ServiceProcess;
using System.Text;

namespace bitmeter.capture {
    static class Program {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        static void Main() {
            ServiceBase[] ServicesToRun;

            // More than one user Service may run within the same process. To add
            // another service to this process, change the following line to
            // create a second service object. For example,
            //
            //   ServicesToRun = new ServiceBase[] {new Service1(), new MySecondUserService()};
            //
            ServicesToRun = new ServiceBase[] { new DUCaptureService() };

            ServiceBase.Run(ServicesToRun);
        }
    }
}