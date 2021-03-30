// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"

#include <stdlib.h>
#include <time.h>

#include "build/build_config.h"

static bool g_bProviderTimeZoneSet = false;

#if defined(OS_WIN)
#define TIMEZONE _timezone
#define TZSET _tzset
#else
#define TZSET tzset
#define TIMEZONE timezone
#endif

CXFA_TimeZoneProvider::CXFA_TimeZoneProvider() {
  if (!g_bProviderTimeZoneSet) {
    g_bProviderTimeZoneSet = true;
    TZSET();
  }
  m_tz.tzHour = static_cast<int8_t>(TIMEZONE / -3600);
  m_tz.tzMinute =
      static_cast<int8_t>((abs(static_cast<int>(TIMEZONE)) % 3600) / 60);
}

CXFA_TimeZoneProvider::~CXFA_TimeZoneProvider() = default;
