// Copyright 2009 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


// This files includes functions to support time related operations.
// It defines common functions, which are only available on platforms.
// It also provides functions to parse RFC times.

#ifndef COMMON_TIMESUPPORT_H__
#define COMMON_TIMESUPPORT_H__

#include <time.h>

#ifdef WIN32
// Convert a string representation time to a time tm structure.
// It is the conversion function of strftime().
// Linux provides this function.
char *strptime(const char *buf, const char *fmt, struct tm *tm);
#endif

#endif  // COMMON_TIMESUPPORT_H__
