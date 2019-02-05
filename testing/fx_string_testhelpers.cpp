// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fx_string_testhelpers.h"

#include <iomanip>
#include <ios>
#include <string>

std::ostream& operator<<(std::ostream& os, const CFX_DateTime& dt) {
  os << dt.GetYear() << "-" << std::to_string(dt.GetMonth()) << "-"
     << std::to_string(dt.GetDay()) << " " << std::to_string(dt.GetHour())
     << ":" << std::to_string(dt.GetMinute()) << ":"
     << std::to_string(dt.GetSecond()) << "."
     << std::to_string(dt.GetMillisecond());
  return os;
}
