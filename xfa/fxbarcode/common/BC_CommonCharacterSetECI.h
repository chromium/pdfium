// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_COMMON_BC_COMMONCHARACTERSETECI_H_
#define XFA_FXBARCODE_COMMON_BC_COMMONCHARACTERSETECI_H_

#include "core/fxcrt/include/fx_string.h"
#include "xfa/fxbarcode/common/BC_CommonECI.h"

class CBC_CommonCharacterSetECI : public CBC_CommonECI {
 public:
  CBC_CommonCharacterSetECI(int32_t value, CFX_ByteString encodingName);
  virtual ~CBC_CommonCharacterSetECI();
  CFX_ByteString GetEncodingName();
  static void AddCharacterSet(int32_t value, CFX_ByteString encodingName);
  int32_t GetValue();
  static CBC_CommonCharacterSetECI* GetCharacterSetECIByValue(int32_t value);
  static CBC_CommonCharacterSetECI* GetCharacterSetECIByName(
      const CFX_ByteString& name);

 private:
  CFX_ByteString m_encodingName;
  static void initialize();
};

#endif  // XFA_FXBARCODE_COMMON_BC_COMMONCHARACTERSETECI_H_
