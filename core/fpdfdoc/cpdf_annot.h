// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_ANNOT_H_
#define CORE_FPDFDOC_CPDF_ANNOT_H_

#include <map>
#include <memory>

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

class CFX_RenderDevice;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Form;
class CPDF_Page;
class CPDF_RenderContext;
class CPDF_RenderOptions;
class CPDF_Stream;

#define ANNOTFLAG_INVISIBLE 0x0001
#define ANNOTFLAG_HIDDEN 0x0002
#define ANNOTFLAG_PRINT 0x0004
#define ANNOTFLAG_NOZOOM 0x0008
#define ANNOTFLAG_NOROTATE 0x0010
#define ANNOTFLAG_NOVIEW 0x0020
#define ANNOTFLAG_READONLY 0x0040
#define ANNOTFLAG_LOCKED 0x0080
#define ANNOTFLAG_TOGGLENOVIEW 0x0100

class CPDF_Annot {
 public:
  enum AppearanceMode { Normal, Rollover, Down };

  CPDF_Annot(CPDF_Dictionary* pDict, CPDF_Document* pDocument);
  ~CPDF_Annot();

  CFX_ByteString GetSubType() const;
  uint32_t GetFlags() const;
  void GetRect(CFX_FloatRect& rect) const;
  const CPDF_Dictionary* GetAnnotDict() const { return m_pAnnotDict; }
  CPDF_Dictionary* GetAnnotDict() { return m_pAnnotDict; }
  FX_BOOL DrawAppearance(CPDF_Page* pPage,
                         CFX_RenderDevice* pDevice,
                         const CFX_Matrix* pUser2Device,
                         AppearanceMode mode,
                         const CPDF_RenderOptions* pOptions);
  FX_BOOL DrawInContext(const CPDF_Page* pPage,
                        CPDF_RenderContext* pContext,
                        const CFX_Matrix* pUser2Device,
                        AppearanceMode mode);
  void ClearCachedAP();
  void DrawBorder(CFX_RenderDevice* pDevice,
                  const CFX_Matrix* pUser2Device,
                  const CPDF_RenderOptions* pOptions);
  CPDF_Form* GetAPForm(const CPDF_Page* pPage, AppearanceMode mode);

 private:
  CPDF_Dictionary* const m_pAnnotDict;
  CPDF_Document* const m_pDocument;
  const CFX_ByteString m_sSubtype;
  std::map<CPDF_Stream*, CPDF_Form*> m_APMap;
};

#endif  // CORE_FPDFDOC_CPDF_ANNOT_H_
