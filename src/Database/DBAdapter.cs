using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using System.Collections;
using System.Diagnostics;
using bitmeter.utils;

namespace bitmeter.db {
    class DBAdapter : IDBAdapter{
        const int DB_WAIT_SLEEP_MS = 100;
        string dbPath;

        public DBAdapter(string dbPath) {
            this.dbPath = dbPath;
        }

        IntPtr connectionHandle = IntPtr.Zero;
        IntPtr prepareSQL(string sql) {
            if (connectionHandle == IntPtr.Zero) {
                int result = sqlite3open(dbPath, out connectionHandle);
                
                try{
                    checkReturnCode(result, "open");  
                  
                } catch (ApplicationException ex){
                    close();
                    connectionHandle = IntPtr.Zero;
                    throw ex;
                } 
            }

            IntPtr statementHandle, unusedsql;

            int prepres = sqlite3prepare(connectionHandle, sql, sql.Length, out statementHandle, out unusedsql);
            checkReturnCode(prepres, "prepare SQL '" + sql + "'");

            return statementHandle;
        }

        public void close() {
            if (connectionHandle != IntPtr.Zero) {
                int result = sqlite3close(connectionHandle);
                checkReturnCode(result, "close", false);
            } else {
                Debug.Assert(false);
            }
        }

        private void checkReturnCode(int returnCode) {
            checkReturnCode(returnCode, "<unspecified>");
        }

        private void checkReturnCode(int returnCode, string attemptedOp) {
            checkReturnCode(returnCode, attemptedOp, true);
        }

        private void checkReturnCode(int returnCode, string attemptedOp, bool throwEx){
            if (returnCode != SQLITE_OK) {
                string msg = "SQLite Error, operation was '" + attemptedOp + "', database was '" + this.dbPath + "' and error code was " +
                        returnCode + " [" + getMsgForCode(returnCode) + "]";
                if (throwEx) {
                    throw new ApplicationException(msg);
                } else {
                    Log.error(msg);
                }
                
            }
        }

        public void insert(string table, IDictionary<string, object> data) {
            string fieldNameList = makeCommaSeparatedList<string>(data.Keys, false);
            string fieldValueList = makeCommaSeparatedList<object>(data.Values, true);
            string sql = "INSERT INTO " + table + "(" + fieldNameList + ") VALUES (" + fieldValueList + ")";

            IntPtr statementhandle = prepareSQL(sql);
            
            int stepres;

            try {
                bool finished = false;
                do {
                    stepres = sqlite3step(statementhandle);

                    if (stepres == SQLITE_BUSY) {
                        System.Threading.Thread.Sleep(DB_WAIT_SLEEP_MS);
                    } else {
                        finished = true;
                    }

                } while (!finished);

                Debug.Assert(stepres == SQLITE_DONE);

            } finally {
                int finalres = sqlite3finalize(statementhandle);
                checkReturnCode(finalres, "Finalizing statement handle for sql " + sql, false);
            }
        }

        public IList<IList<string>> select(IList<string> fields, string table, string whereClause) {
            string sql = "SELECT " + makeCommaSeparatedList(fields, false) + " FROM " + table;
            if (whereClause != null && whereClause.Length > 0) {
                sql += " WHERE " + whereClause;
            }

            IntPtr statementhandle = prepareSQL(sql);

            IList<IList<string>> data = new List<IList<string>>();

            int stepres;
            IList<string> row;

            bool finished = false;

            try {
                do {
                    stepres = sqlite3step(statementhandle);

                    if (stepres == SQLITE_BUSY) {
                        System.Threading.Thread.Sleep(DB_WAIT_SLEEP_MS);

                    } else if (stepres == SQLITE_ROW) {
                        row = new List<string>();
                        for (int i = 0; i < fields.Count; i++) {
                            row.Add(sqlite3columntext(statementhandle, i));
                        }
                        data.Add(row);

                    } else {
                        finished = true;
                    }

                } while (!finished);
                Debug.Assert(stepres == SQLITE_DONE);

            } finally {
                int finalres = sqlite3finalize(statementhandle);
                checkReturnCode(finalres, "Finalizing statement handle for sql " + sql, false);
            }

            return data;
        }

        private string makeCommaSeparatedList<T>(ICollection<T> list, bool quoteStrings) {
            StringBuilder text = new StringBuilder();
            Boolean isFirst = true;
            foreach(T item in list){
                if (!isFirst) {
                    text.Append(", ");
                }
                if (item is string) {
                    if (quoteStrings) text.Append("'");
                    text.Append(item.ToString());
                    if (quoteStrings) text.Append("'");
                } else {
                    text.Append(item.ToString());
                } 
                isFirst = false;
            }
            return text.ToString();
        }

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_open")]
        public static extern int sqlite3open(string filename, out IntPtr dbhandle);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_close")]
        public static extern int sqlite3close(IntPtr dbhandle);

        //[DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_exec")]
        //public static extern int sqlite3exec(IntPtr dbhandle, string sql, ExecuteCallbackFunctionType callback, IntPtr callbackArg1, out IntPtr errorMessage);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_changes")]
        public static extern int sqlite3_changes(IntPtr dbhandle);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_total_changes")]
        public static extern int sqlite3totalchanges(IntPtr dbhandle);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_prepare")]
        public static extern int sqlite3prepare(IntPtr dbhandle, string sql, int stringlength, out IntPtr statementhandle, out IntPtr unusedsql);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_step")]
        public static extern int sqlite3step(IntPtr statementhandle);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_finalize")]
        public static extern int sqlite3finalize(IntPtr statementhandle);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_reset")]
        public static extern int sqlite3reset(IntPtr statementhandle);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_column_count")]
        public static extern int sqlite3columncount(IntPtr statementhandle);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_column_name")]
        public static extern string sqlite3columnname(IntPtr statementhandle, int columnindex);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_column_type")]
        public static extern int sqlite3columntype(IntPtr statementhandle, int columnindex);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_column_int")]
        public static extern int sqlite3columnint(IntPtr statementhandle, int columnindex);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_column_double")]
        public static extern double sqlite3columndouble(IntPtr statementhandle, int columnindex);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_column_text")]
        public static extern string sqlite3columntext(IntPtr statementhandle, int columnindex);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_column_blob")]
        public static extern IntPtr sqlite3columnblob(IntPtr statementhandle, int columnindex);

        [DllImport("Sqlite3.Dll", EntryPoint = "sqlite3_column_bytes")]
        public static extern int sqlite3columnbytes(IntPtr statementhandle, int columnindex);

        private string getMsgForCode(int code) {
            string msg;

            switch (code) {
                case SQLITE_OK:
                    msg = "Successful result ";
                    break;
                case SQLITE_ERROR:
                    msg = " SQL error or missing database ";
                    break;
                case SQLITE_INTERNAL:
                    msg = " An internal logic error in SQLite ";
                    break;
                case SQLITE_PERM:
                    msg = " Access permission denied ";
                    break;
                case SQLITE_ABORT:
                    msg = " Callback routine requested an abort ";
                    break;
                case SQLITE_BUSY:
                    msg = " The database file is locked ";
                    break;
                case SQLITE_LOCKED:
                    msg = " A table in the database is locked ";
                    break;
                case SQLITE_NOMEM:
                    msg = " A malloc() failed ";
                    break;
                case SQLITE_READONLY:
                    msg = " Attempt to write a readonly database ";
                    break;
                case SQLITE_INTERRUPT:
                    msg = " Operation terminated by sqlite_interrupt() ";
                    break;
                case SQLITE_IOERR:
                    msg = " Some kind of disk I/O error occurred ";
                    break;
                case SQLITE_CORRUPT:
                    msg = " The database disk image is malformed ";
                    break;
                case SQLITE_NOTFOUND:
                    msg = " (Internal Only) Table or record not found ";
                    break;
                case SQLITE_FULL:
                    msg = " Insertion failed because database is full ";
                    break;
                case SQLITE_CANTOPEN:
                    msg = " Unable to open the database file ";
                    break;
                case SQLITE_PROTOCOL:
                    msg = " Database lock protocol error ";
                    break;
                case SQLITE_EMPTY:
                    msg = " (Internal Only) Database table is empty ";
                    break;
                case SQLITE_SCHEMA:
                    msg = " The database schema changed ";
                    break;
                case SQLITE_TOOBIG:
                    msg = " Too much data for one row of a table ";
                    break;
                case SQLITE_CONSTRAINT:
                    msg = " Abort due to contraint violation ";
                    break;
                case SQLITE_MISMATCH:
                    msg = " Data type mismatch ";
                    break;
                case SQLITE_MISUSE:
                    msg = " Library used incorrectly ";
                    break;
                case SQLITE_NOLFS:
                    msg = " Uses OS features not supported on host ";
                    break;
                case SQLITE_AUTH:
                    msg = " Authorization denied ";
                    break;
                case SQLITE_ROW:
                    msg = " sqlite_step() has another row ready ";
                    break;
                case SQLITE_DONE:
                    msg = "sqlite_step() has finished executing";
                    break;
                default:
                    msg = "<Unknown code - no mesaage available>";
                    Debug.Assert(false);
                    break;
            }
            return msg;
        }

        const int SQLITE_OK = 0;   //Successful result 
        const int SQLITE_ERROR = 1;   // SQL error or missing database 
        const int SQLITE_INTERNAL = 2;   // An internal logic error in SQLite 
        const int SQLITE_PERM = 3;   // Access permission denied 
        const int SQLITE_ABORT = 4;   // Callback routine requested an abort 
        const int SQLITE_BUSY = 5;   // The database file is locked 
        const int SQLITE_LOCKED = 6;   // A table in the database is locked 
        const int SQLITE_NOMEM = 7;   // A malloc() failed 
        const int SQLITE_READONLY = 8;   // Attempt to write a readonly database 
        const int SQLITE_INTERRUPT = 9;   // Operation terminated by sqlite_interrupt() 
        const int SQLITE_IOERR = 10;   // Some kind of disk I/O error occurred 
        const int SQLITE_CORRUPT = 11;   // The database disk image is malformed 
        const int SQLITE_NOTFOUND = 12;   // (Internal Only) Table or record not found 
        const int SQLITE_FULL = 13;   // Insertion failed because database is full 
        const int SQLITE_CANTOPEN = 14;   // Unable to open the database file 
        const int SQLITE_PROTOCOL = 15;   // Database lock protocol error 
        const int SQLITE_EMPTY = 16;   // (Internal Only) Database table is empty 
        const int SQLITE_SCHEMA = 17;   // The database schema changed 
        const int SQLITE_TOOBIG = 18;   // Too much data for one row of a table 
        const int SQLITE_CONSTRAINT = 19;   // Abort due to contraint violation 
        const int SQLITE_MISMATCH = 20;   // Data type mismatch 
        const int SQLITE_MISUSE = 21;   // Library used incorrectly 
        const int SQLITE_NOLFS = 22;   // Uses OS features not supported on host 
        const int SQLITE_AUTH = 23;   // Authorization denied 
        const int SQLITE_ROW = 100;   // sqlite_step() has another row ready 
        const int SQLITE_DONE = 101;   // sqlite_step() has finished executing

        public void Dispose() {
            close();
            GC.SuppressFinalize(this);
        }

        ~DBAdapter() {
            Dispose();
        }

    }
}
