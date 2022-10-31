// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_TIMEZONEPROVIDER_H_
#define XFA_FXFA_PARSER_CXFA_TIMEZONEPROVIDER_H_

class CXFA_TimeZoneProvider {
 public:
  CXFA_TimeZoneProvider();
  ~CXFA_TimeZoneProvider();

  int GetTimeZoneInMinutes() const { return tz_minutes_; }

 private:
  int tz_minutes_;
};

#endif  // XFA_FXFA_PARSER_CXFA_TIMEZONEPROVIDER_H_
