using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.utils {
    public class Log {
        public enum LogEventType{
            DEBUG, INFO, WARN, ERROR
        }

        public delegate void LogEventHandler(LogEventType eventType, string msg);
        public static event LogEventHandler LogEvent;

        public static void debug(string msg){
            print(LogEventType.DEBUG + ": " + msg);
            doEvent(LogEventType.DEBUG, msg);
        }
        public static void info(string msg) {
            print(LogEventType.INFO + ":  " + msg);
            doEvent(LogEventType.INFO, msg);
        }
        public static void warn(string msg) {
            print(LogEventType.WARN + ":  " + msg);
            doEvent(LogEventType.WARN, msg);
        }
        public static void error(string msg) {
            print(LogEventType.ERROR + ": " + msg);
            doEvent(LogEventType.ERROR, msg);
        }

        private static void doEvent(LogEventType eventType, string msg){
            if (LogEvent != null) {
                LogEvent(eventType, msg);
            }
        }
        
        private static void print(string msg){
            //TODO
            Console.WriteLine(msg);
        }
    }
}
