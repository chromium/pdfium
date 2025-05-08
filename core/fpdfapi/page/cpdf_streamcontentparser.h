// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_STREAMCONTENTPARSER_H_
#define CORE_FPDFAPI_PAGE_CPDF_STREAMCONTENTPARSER_H_

#include <array>
#include <memory>
#include <stack>
#include <variant>
#include <vector>

#include "core/fpdfapi/page/cpdf_contentmarks.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_number.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_path.h"

class CPDF_AllStates;
class CPDF_ColorSpace;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Font;
class CPDF_Image;
class CPDF_ImageObject;
class CPDF_Object;
class CPDF_PageObject;
class CPDF_Pattern;
class CPDF_ShadingPattern;
class CPDF_Stream;
class CPDF_StreamParser;
class CPDF_TextObject;

class CPDF_StreamContentParser {
 public:
  static void InitializeGlobals();
  static void DestroyGlobals();

  CPDF_StreamContentParser(CPDF_Document* pDoc,
                           RetainPtr<CPDF_Dictionary> pPageResources,
                           RetainPtr<CPDF_Dictionary> pParentResources,
                           const CFX_Matrix* pmtContentToUser,
                           CPDF_PageObjectHolder* pObjHolder,
                           RetainPtr<CPDF_Dictionary> pResources,
                           const CFX_FloatRect& rcBBox,
                           const CPDF_AllStates* pStates,
                           CPDF_Form::RecursionState* parse_state);
  ~CPDF_StreamContentParser();

  uint32_t Parse(pdfium::span<const uint8_t> pData,
                 uint32_t start_offset,
                 uint32_t max_cost,
                 const std::vector<uint32_t>& stream_start_offsets);
  CPDF_PageObjectHolder* GetPageObjectHolder() const { return object_holder_; }
  CPDF_AllStates* GetCurStates() const { return cur_states_.get(); }
  bool IsColored() const { return colored_; }
  pdfium::span<const float> GetType3Data() const { return type3_data_; }
  RetainPtr<CPDF_Font> FindFont(const ByteString& name);
  CPDF_PageObjectHolder::CTMMap TakeAllCTMs();

  static ByteStringView FindKeyAbbreviationForTesting(ByteStringView abbr);
  static ByteStringView FindValueAbbreviationForTesting(ByteStringView abbr);

 private:
  enum class RenderType : bool { kFill = false, kStroke = true };

  using ContentParam =
      std::variant<RetainPtr<CPDF_Object>, FX_Number, ByteString>;

  static constexpr int kParamBufSize = 16;

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
  void AddTextObject(pdfium::span<const ByteString> strings,
                     pdfium::span<const float> kernings,
                     float initial_kerning);
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
  CPDF_ImageObject* AddImageFromStream(RetainPtr<CPDF_Stream> pStream,
                                       const ByteString& name);
  CPDF_ImageObject* AddImageFromStreamObjNum(uint32_t stream_obj_num,
                                             const ByteString& name);
  CPDF_ImageObject* AddLastImage();

  void AddForm(RetainPtr<CPDF_Stream> pStream, const ByteString& name);
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

  UnownedPtr<CPDF_Document> const document_;
  RetainPtr<CPDF_Dictionary> const page_resources_;
  RetainPtr<CPDF_Dictionary> const parent_resources_;
  RetainPtr<CPDF_Dictionary> const resources_;
  UnownedPtr<CPDF_PageObjectHolder> const object_holder_;
  UnownedPtr<CPDF_Form::RecursionState> const recursion_state_;
  CFX_Matrix mt_content_to_user_;
  const CFX_FloatRect bbox_;
  uint32_t param_start_pos_ = 0;
  uint32_t param_count_ = 0;
  std::unique_ptr<CPDF_StreamParser> syntax_;
  std::unique_ptr<CPDF_AllStates> cur_states_;
  std::stack<std::unique_ptr<CPDF_ContentMarks>> content_marks_stack_;
  std::vector<std::unique_ptr<CPDF_TextObject>> clip_text_list_;
  std::vector<CFX_Path::Point> path_points_;
  CFX_PointF path_start_;
  CFX_PointF path_current_;
  CFX_FillRenderOptions::FillType path_clip_type_ =
      CFX_FillRenderOptions::FillType::kNoFill;
  ByteString last_image_name_;
  RetainPtr<CPDF_Image> last_image_;
  bool colored_ = false;
  std::vector<std::unique_ptr<CPDF_AllStates>> state_stack_;
  std::array<float, 6> type3_data_ = {};
  std::array<ContentParam, kParamBufSize> param_buf_;
  CPDF_PageObjectHolder::CTMMap all_ctms_;

  // The merged stream offsets at which a content stream ends and another
  // begins.
  std::vector<uint32_t> stream_start_offsets_;

  // The merged stream offset at which the last |syntax_| started parsing.
  uint32_t start_parse_offset_ = 0;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_STREAMCONTENTPARSER_H_
