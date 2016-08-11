// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_DATETIME_H_
#define FPDFSDK_INCLUDE_CPDFSDK_DATETIME_H_

#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
#else
#include <ctime>
#endif

#include "fpdfsdk/cfx_systemhandler.h"

class CPDFSDK_DateTime {
 public:
  CPDFSDK_DateTime();
  explicit CPDFSDK_DateTime(const CFX_ByteString& dtStr);
  explicit CPDFSDK_DateTime(const FX_SYSTEMTIME& st);
  CPDFSDK_DateTime(const CPDFSDK_DateTime& datetime);

  bool operator==(const CPDFSDK_DateTime& datetime) const;
  bool operator!=(const CPDFSDK_DateTime& datetime) const;

  CPDFSDK_DateTime& FromPDFDateTimeString(const CFX_ByteString& dtStr);
  CFX_ByteString ToCommonDateTimeString();
  CFX_ByteString ToPDFDateTimeString();
  void ToSystemTime(FX_SYSTEMTIME& st);
  time_t ToTime_t() const;
  CPDFSDK_DateTime ToGMT() const;
  CPDFSDK_DateTime& AddDays(short days);
  CPDFSDK_DateTime& AddSeconds(int seconds);

  void ResetDateTime();

 private:
  struct FX_DATETIME {
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    int8_t tzHour;
    uint8_t tzMinute;
  } dt;
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_DATETIME_H_
