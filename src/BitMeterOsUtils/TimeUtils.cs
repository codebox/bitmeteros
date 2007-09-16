using System;
using System.Collections.Generic;
using System.Text;

namespace bitmeter.utils {
    public class TimeUtils {
        static long TIME_BASE_TICKS = new DateTime(2000, 1, 1).Ticks;

        public static uint getTimeValue(){
            return getTimeValue(DateTime.Now);
        }
        
        public static uint getTimeValue(DateTime dateTime){
            long ticksNow = dateTime.Ticks;
            long ticksSinceTimeBase = ticksNow - TIME_BASE_TICKS;

            long secondsSinceTimeBase = ticksSinceTimeBase / TimeSpan.TicksPerSecond;

            return (uint) secondsSinceTimeBase;        	
        }
        
        public static DateTime getDateFromTimeValue(uint timeValue){
        	long timeValueTicks = TIME_BASE_TICKS + (timeValue * TimeSpan.TicksPerSecond);
        	DateTime date = new DateTime(timeValueTicks);
        	return date;
        }
        
    }
}
