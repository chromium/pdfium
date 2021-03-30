// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/crt/fx_timezone.h"

int FX_TimeZoneOffsetInMinutes(const FX_TIMEZONE& tz) {
  return tz.tzHour * 60 + tz.tzMinute;
}
