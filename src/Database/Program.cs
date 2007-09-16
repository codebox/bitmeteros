using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.db {
    class Program {
        static void Main(string[] args) {
            IDBAdapter db = new DBAdapter(@"C:\code\bitmeterOS\src\Database\bitmeter.db");

            IDictionary<string, object> data = new Dictionary<string, object>();
            data["name"] = "bob";
            data["age"] = 23;

            db.insert("testtab", data);
            IList<string> fields = new List<string>();
            fields.Add("name");
            fields.Add("age");

            db.select(fields, "testtab", "name='bob'");
            db.close();
        }
    }
}
