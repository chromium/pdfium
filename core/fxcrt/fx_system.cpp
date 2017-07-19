// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/fx_system.h"

#include <limits>

extern "C" int FXSYS_round(float d) {
  if (d < static_cast<float>(std::numeric_limits<int>::min()))
    return std::numeric_limits<int>::min();
  if (d > static_cast<float>(std::numeric_limits<int>::max()))
    return std::numeric_limits<int>::max();
  return static_cast<int>(round(d));
}

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

size_t FXSYS_wcsftime(wchar_t* strDest,
                      size_t maxsize,
                      const wchar_t* format,
                      const struct tm* timeptr) {
  // Avoid tripping an invalid parameter handler and crashing process.
  // Note: leap seconds may cause tm_sec == 60.
  if (timeptr->tm_year < -1900 || timeptr->tm_year > 8099 ||
      timeptr->tm_mon < 0 || timeptr->tm_mon > 11 || timeptr->tm_mday < 1 ||
      timeptr->tm_mday > 31 || timeptr->tm_hour < 0 || timeptr->tm_hour > 23 ||
      timeptr->tm_min < 0 || timeptr->tm_min > 59 || timeptr->tm_sec < 0 ||
      timeptr->tm_sec > 60 || timeptr->tm_wday < 0 || timeptr->tm_wday > 6 ||
      timeptr->tm_yday < 0 || timeptr->tm_yday > 365) {
    strDest[0] = L'\0';
    return 0;
  }
  return wcsftime(strDest, maxsize, format, timeptr);
}

#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
