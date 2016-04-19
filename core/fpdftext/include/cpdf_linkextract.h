// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFTEXT_INCLUDE_CPDF_LINKEXTRACT_H_
#define CORE_FPDFTEXT_INCLUDE_CPDF_LINKEXTRACT_H_

#include "core/fxcrt/include/fx_basic.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_TextPage;

class CPDF_LinkExt {
 public:
  CPDF_LinkExt() {}
  ~CPDF_LinkExt() {}

  int m_Start;
  int m_Count;
  CFX_WideString m_strUrl;
};

class CPDF_LinkExtract {
 public:
  CPDF_LinkExtract();
  ~CPDF_LinkExtract();

  FX_BOOL ExtractLinks(const CPDF_TextPage* pTextPage);
  int CountLinks() const;
  CFX_WideString GetURL(int index) const;
  void GetBoundedSegment(int index, int& start, int& count) const;
  void GetRects(int index, CFX_RectArray& rects) const;

  FX_BOOL IsExtract() const { return m_bIsParsed; }

 protected:
  void ParseLink();
  void DeleteLinkList();
  FX_BOOL CheckWebLink(CFX_WideString& strBeCheck);
  bool CheckMailLink(CFX_WideString& str);
  void AppendToLinkList(int start, int count, const CFX_WideString& strUrl);

 private:
  CFX_ArrayTemplate<CPDF_LinkExt*> m_LinkList;
  const CPDF_TextPage* m_pTextPage;
  CFX_WideString m_strPageText;
  bool m_bIsParsed;
};

#endif  // CORE_FPDFTEXT_INCLUDE_CPDF_LINKEXTRACT_H_
