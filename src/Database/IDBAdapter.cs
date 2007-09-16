using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.db {
    interface IDBAdapter : IDisposable {
        void insert(string table, IDictionary<string, object> data);
        IList<IList<string>> select(IList<string> fields, string table, string whereClause);
        void close();
    }
}
