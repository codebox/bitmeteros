using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.capture {
    public interface IMessageDispatcher : IDisposable {
        void send(string message);
    }
}
