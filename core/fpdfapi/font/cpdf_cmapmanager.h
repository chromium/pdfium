// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_CMAPMANAGER_H_
#define CORE_FPDFAPI_FONT_CPDF_CMAPMANAGER_H_

#include <map>
#include <memory>

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fxcrt/cfx_bytestring.h"
#include "core/fxcrt/cfx_retain_ptr.h"

class CPDF_CMapManager {
 public:
  CPDF_CMapManager();
  ~CPDF_CMapManager();

  CFX_RetainPtr<CPDF_CMap> GetPredefinedCMap(const CFX_ByteString& name,
                                             bool bPromptCJK);
  CPDF_CID2UnicodeMap* GetCID2UnicodeMap(CIDSet charset, bool bPromptCJK);

 private:
  CFX_RetainPtr<CPDF_CMap> LoadPredefinedCMap(const CFX_ByteString& name,
                                              bool bPromptCJK);
  std::unique_ptr<CPDF_CID2UnicodeMap> LoadCID2UnicodeMap(CIDSet charset,
                                                          bool bPromptCJK);

  std::map<CFX_ByteString, CFX_RetainPtr<CPDF_CMap>> m_CMaps;
  std::unique_ptr<CPDF_CID2UnicodeMap> m_CID2UnicodeMaps[6];
};

#endif  // CORE_FPDFAPI_FONT_CPDF_CMAPMANAGER_H_
