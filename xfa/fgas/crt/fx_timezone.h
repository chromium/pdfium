// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_FX_TIMEZONE_H_
#define XFA_FGAS_CRT_FX_TIMEZONE_H_

#include <stdint.h>

struct FX_TIMEZONE {
  int8_t tzHour;
  uint8_t tzMinute;
};

int FX_TimeZoneOffsetInMinutes(const FX_TIMEZONE& tz);

#endif  // XFA_FGAS_CRT_FX_TIMEZONE_H_
