// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_STREAMCONTENTPARSER_H_
#define CORE_FPDFAPI_PAGE_CPDF_STREAMCONTENTPARSER_H_

#include <map>
#include <memory>
#include <set>
#include <stack>
#include <vector>

#include "core/fpdfapi/page/cpdf_contentmarks.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_number.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_path.h"
#include "third_party/base/span.h"

class CPDF_AllStates;
class CPDF_ColorSpace;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Font;
class CPDF_Image;
class CPDF_ImageObject;
class CPDF_Object;
class CPDF_PageObject;
class CPDF_PageObjectHolder;
class CPDF_Pattern;
class CPDF_ShadingPattern;
class CPDF_Stream;
class CPDF_StreamParser;
class CPDF_TextObject;

class CPDF_StreamContentParser {
 public:
  CPDF_StreamContentParser(CPDF_Document* pDoc,
                           RetainPtr<CPDF_Dictionary> pPageResources,
                           RetainPtr<CPDF_Dictionary> pParentResources,
                           const CFX_Matrix* pmtContentToUser,
                           CPDF_PageObjectHolder* pObjHolder,
                           RetainPtr<CPDF_Dictionary> pResources,
                           const CFX_FloatRect& rcBBox,
                           const CPDF_AllStates* pStates,
                           std::set<const uint8_t*>* pParsedSet);
  ~CPDF_StreamContentParser();

  uint32_t Parse(pdfium::span<const uint8_t> pData,
                 uint32_t start_offset,
                 uint32_t max_cost,
                 const std::vector<uint32_t>& stream_start_offsets);
  CPDF_PageObjectHolder* GetPageObjectHolder() const { return m_pObjectHolder; }
  CPDF_AllStates* GetCurStates() const { return m_pCurStates.get(); }
  bool IsColored() const { return m_bColored; }
  pdfium::span<const float> GetType3Data() const { return m_Type3Data; }
  RetainPtr<CPDF_Font> FindFont(const ByteString& name);

  static ByteStringView FindKeyAbbreviationForTesting(ByteStringView abbr);
  static ByteStringView FindValueAbbreviationForTesting(ByteStringView abbr);

 private:
  enum class RenderType : bool { kFill = false, kStroke = true };

  struct ContentParam {
    enum class Type : uint8_t { kObject = 0, kNumber, kName };

    ContentParam();
    ~ContentParam();

    Type m_Type = Type::kObject;
    FX_Number m_Number;
    ByteString m_Name;
    RetainPtr<CPDF_Object> m_pObject;
  };

  static constexpr int kParamBufSize = 16;

  using OpCodes = std::map<uint32_t, void (CPDF_StreamContentParser::*)()>;
  static OpCodes InitializeOpCodes();

  void AddNameParam(ByteStringView bsName);
  void AddNumberParam(ByteStringView str);
  void AddObjectParam(RetainPtr<CPDF_Object> pObj);
  int GetNextParamPos();
  void ClearAllParams();
  RetainPtr<CPDF_Object> GetObject(uint32_t index);
  ByteString GetString(uint32_t index) const;
  float GetNumber(uint32_t index) const;
  // Calls GetNumber() |count| times and returns the values in reverse order.
  // e.g. for |count| = 3, returns [GetNumber(2), GetNumber(1), GetNumber(0)].
  std::vector<float> GetNumbers(size_t count) const;
  int GetInteger(uint32_t index) const {
    return static_cast<int>(GetNumber(index));
  }
  // Makes a point from {GetNumber(index + 1), GetNumber(index)}.
  CFX_PointF GetPoint(uint32_t index) const;
  // Makes a matrix from {GetNumber(5), ..., GetNumber(0)}.
  CFX_Matrix GetMatrix() const;
  void OnOperator(ByteStringView op);
  void AddTextObject(const ByteString* pStrs,
                     float fInitKerning,
                     const std::vector<float>& kernings,
                     size_t nSegs);
  float GetHorizontalTextSize(float fKerning) const;
  float GetVerticalTextSize(float fKerning) const;

  void OnChangeTextMatrix();
  void ParsePathObject();
  void AddPathPoint(const CFX_PointF& point, CFX_Path::Point::Type type);
  void AddPathPointAndClose(const CFX_PointF& point,
                            CFX_Path::Point::Type type);
  void AddPathRect(float x, float y, float w, float h);
  void AddPathObject(CFX_FillRenderOptions::FillType fill_type,
                     RenderType render_type);
  CPDF_ImageObject* AddImage(RetainPtr<CPDF_Stream> pStream);
  CPDF_ImageObject* AddImage(uint32_t streamObjNum);
  CPDF_ImageObject* AddImage(const RetainPtr<CPDF_Image>& pImage);

  void AddForm(RetainPtr<CPDF_Stream> pStream);
  void SetGraphicStates(CPDF_PageObject* pObj,
                        bool bColor,
                        bool bText,
                        bool bGraph);
  RetainPtr<CPDF_ColorSpace> FindColorSpace(const ByteString& name);
  RetainPtr<CPDF_Pattern> FindPattern(const ByteString& name);
  RetainPtr<CPDF_ShadingPattern> FindShading(const ByteString& name);
  RetainPtr<CPDF_Dictionary> FindResourceHolder(const ByteString& type);
  RetainPtr<CPDF_Object> FindResourceObj(const ByteString& type,
                                         const ByteString& name);

  // Takes ownership of |pImageObj|, returns unowned pointer to it.
  CPDF_ImageObject* AddImageObject(std::unique_ptr<CPDF_ImageObject> pImageObj);

  std::vector<float> GetColors() const;
  std::vector<float> GetNamedColors() const;
  int32_t GetCurrentStreamIndex();

  void Handle_CloseFillStrokePath();
  void Handle_FillStrokePath();
  void Handle_CloseEOFillStrokePath();
  void Handle_EOFillStrokePath();
  void Handle_BeginMarkedContent_Dictionary();
  void Handle_BeginImage();
  void Handle_BeginMarkedContent();
  void Handle_BeginText();
  void Handle_CurveTo_123();
  void Handle_ConcatMatrix();
  void Handle_SetColorSpace_Fill();
  void Handle_SetColorSpace_Stroke();
  void Handle_SetDash();
  void Handle_SetCharWidth();
  void Handle_SetCachedDevice();
  void Handle_ExecuteXObject();
  void Handle_MarkPlace_Dictionary();
  void Handle_EndImage();
  void Handle_EndMarkedContent();
  void Handle_EndText();
  void Handle_FillPath();
  void Handle_FillPathOld();
  void Handle_EOFillPath();
  void Handle_SetGray_Fill();
  void Handle_SetGray_Stroke();
  void Handle_SetExtendGraphState();
  void Handle_ClosePath();
  void Handle_SetFlat();
  void Handle_BeginImageData();
  void Handle_SetLineJoin();
  void Handle_SetLineCap();
  void Handle_SetCMYKColor_Fill();
  void Handle_SetCMYKColor_Stroke();
  void Handle_LineTo();
  void Handle_MoveTo();
  void Handle_SetMiterLimit();
  void Handle_MarkPlace();
  void Handle_EndPath();
  void Handle_SaveGraphState();
  void Handle_RestoreGraphState();
  void Handle_Rectangle();
  void Handle_SetRGBColor_Fill();
  void Handle_SetRGBColor_Stroke();
  void Handle_SetRenderIntent();
  void Handle_CloseStrokePath();
  void Handle_StrokePath();
  void Handle_SetColor_Fill();
  void Handle_SetColor_Stroke();
  void Handle_SetColorPS_Fill();
  void Handle_SetColorPS_Stroke();
  void Handle_ShadeFill();
  void Handle_SetCharSpace();
  void Handle_MoveTextPoint();
  void Handle_MoveTextPoint_SetLeading();
  void Handle_SetFont();
  void Handle_ShowText();
  void Handle_ShowText_Positioning();
  void Handle_SetTextLeading();
  void Handle_SetTextMatrix();
  void Handle_SetTextRenderMode();
  void Handle_SetTextRise();
  void Handle_SetWordSpace();
  void Handle_SetHorzScale();
  void Handle_MoveToNextLine();
  void Handle_CurveTo_23();
  void Handle_SetLineWidth();
  void Handle_Clip();
  void Handle_EOClip();
  void Handle_CurveTo_13();
  void Handle_NextLineShowText();
  void Handle_NextLineShowText_Space();
  void Handle_Invalid();

  UnownedPtr<CPDF_Document> const m_pDocument;
  RetainPtr<CPDF_Dictionary> const m_pPageResources;
  RetainPtr<CPDF_Dictionary> const m_pParentResources;
  RetainPtr<CPDF_Dictionary> const m_pResources;
  UnownedPtr<CPDF_PageObjectHolder> const m_pObjectHolder;
  UnownedPtr<std::set<const uint8_t*>> const m_ParsedSet;
  CFX_Matrix m_mtContentToUser;
  const CFX_FloatRect m_BBox;
  uint32_t m_ParamStartPos = 0;
  uint32_t m_ParamCount = 0;
  std::unique_ptr<CPDF_StreamParser> m_pSyntax;
  std::unique_ptr<CPDF_AllStates> m_pCurStates;
  std::stack<std::unique_ptr<CPDF_ContentMarks>> m_ContentMarksStack;
  std::vector<std::unique_ptr<CPDF_TextObject>> m_ClipTextList;
  std::vector<CFX_Path::Point> m_PathPoints;
  CFX_PointF m_PathStart;
  CFX_PointF m_PathCurrent;
  CFX_FillRenderOptions::FillType m_PathClipType =
      CFX_FillRenderOptions::FillType::kNoFill;
  ByteString m_LastImageName;
  RetainPtr<CPDF_Image> m_pLastImage;
  bool m_bColored = false;
  std::vector<std::unique_ptr<CPDF_AllStates>> m_StateStack;
  float m_Type3Data[6] = {0.0f};
  ContentParam m_ParamBuf[kParamBufSize];

  // The merged stream offsets at which a content stream ends and another
  // begins.
  std::vector<uint32_t> m_StreamStartOffsets;

  // The merged stream offset at which the last |m_pSyntax| started parsing.
  uint32_t m_StartParseOffset = 0;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_STREAMCONTENTPARSER_H_
