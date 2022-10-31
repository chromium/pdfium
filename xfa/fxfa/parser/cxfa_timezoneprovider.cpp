// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_timezoneprovider.h"

#include <stdlib.h>
#include <time.h>

#include "build/build_config.h"

static bool g_bProviderTimeZoneSet = false;

#if BUILDFLAG(IS_WIN)
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
  tz_minutes_ = TIMEZONE / -60;
}

CXFA_TimeZoneProvider::~CXFA_TimeZoneProvider() = default;
