// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECTHOLDER_H_
#define CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECTHOLDER_H_

#include <deque>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "core/fpdfapi/page/cpdf_transparency.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/fx_dib.h"

class CPDF_ContentParser;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_PageObject;
class CPDF_Stream;
class PauseIndicatorIface;

// These structs are used to keep track of resources that have already been
// generated in the page object holder.
struct GraphicsData {
  float fillAlpha;
  float strokeAlpha;
  BlendMode blendType;

  bool operator<(const GraphicsData& other) const;
};

struct FontData {
  ByteString baseFont;
  ByteString type;

  bool operator<(const FontData& other) const;
};

class CPDF_PageObjectHolder {
 public:
  enum class ParseState : uint8_t { kNotParsed, kParsing, kParsed };

  using iterator = std::deque<std::unique_ptr<CPDF_PageObject>>::iterator;
  using const_iterator =
      std::deque<std::unique_ptr<CPDF_PageObject>>::const_iterator;

  CPDF_PageObjectHolder(CPDF_Document* pDoc,
                        CPDF_Dictionary* pDict,
                        CPDF_Dictionary* pPageResources,
                        CPDF_Dictionary* pResources);
  virtual ~CPDF_PageObjectHolder();

  virtual bool IsPage() const;

  void StartParse(std::unique_ptr<CPDF_ContentParser> pParser);
  void ContinueParse(PauseIndicatorIface* pPause);
  ParseState GetParseState() const { return m_ParseState; }

  CPDF_Document* GetDocument() const { return m_pDocument.Get(); }

  CPDF_Dictionary* GetDict() const { return m_pDict.Get(); }
  size_t GetPageObjectCount() const { return m_PageObjectList.size(); }
  CPDF_PageObject* GetPageObjectByIndex(size_t index) const;
  void AppendPageObject(std::unique_ptr<CPDF_PageObject> pPageObj);
  bool RemovePageObject(CPDF_PageObject* pPageObj);
  bool ErasePageObjectAtIndex(size_t index);

  iterator begin() { return m_PageObjectList.begin(); }
  const_iterator begin() const { return m_PageObjectList.begin(); }

  iterator end() { return m_PageObjectList.end(); }
  const_iterator end() const { return m_PageObjectList.end(); }

  const CFX_Matrix& GetLastCTM() const { return m_LastCTM; }
  const CFX_FloatRect& GetBBox() const { return m_BBox; }

  const CPDF_Transparency& GetTransparency() const { return m_Transparency; }
  bool BackgroundAlphaNeeded() const { return m_bBackgroundAlphaNeeded; }
  void SetBackgroundAlphaNeeded(bool needed) {
    m_bBackgroundAlphaNeeded = needed;
  }

  bool HasImageMask() const { return !m_MaskBoundingBoxes.empty(); }
  const std::vector<CFX_FloatRect>& GetMaskBoundingBoxes() const {
    return m_MaskBoundingBoxes;
  }
  void AddImageMaskBoundingBox(const CFX_FloatRect& box);
  void Transform(const CFX_Matrix& matrix);
  bool HasDirtyStreams() const { return !m_DirtyStreams.empty(); }
  std::set<int32_t> TakeDirtyStreams();

  RetainPtr<CPDF_Dictionary> m_pPageResources;
  RetainPtr<CPDF_Dictionary> m_pResources;
  std::map<GraphicsData, ByteString> m_GraphicsMap;
  std::map<FontData, ByteString> m_FontsMap;

 protected:
  void LoadTransInfo();

  CFX_FloatRect m_BBox;
  CPDF_Transparency m_Transparency;

 private:
  bool m_bBackgroundAlphaNeeded = false;
  ParseState m_ParseState = ParseState::kNotParsed;
  RetainPtr<CPDF_Dictionary> const m_pDict;
  UnownedPtr<CPDF_Document> m_pDocument;
  std::vector<CFX_FloatRect> m_MaskBoundingBoxes;
  std::unique_ptr<CPDF_ContentParser> m_pParser;
  std::deque<std::unique_ptr<CPDF_PageObject>> m_PageObjectList;
  CFX_Matrix m_LastCTM;

  // The indexes of Content streams that are dirty and need to be regenerated.
  std::set<int32_t> m_DirtyStreams;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_PAGEOBJECTHOLDER_H_
