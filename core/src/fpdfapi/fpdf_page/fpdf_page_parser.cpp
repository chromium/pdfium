// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/src/fpdfapi/fpdf_page/pageint.h"

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fpdfapi/fpdf_serial.h"

namespace {

struct _FX_BSTR {
  const FX_CHAR* m_Ptr;
  int m_Size;
};
#define _FX_BSTRC(str) \
  { str, sizeof(str) - 1 }

struct PDF_AbbrPairs {
  _FX_BSTR full_name;
  _FX_BSTR abbr;
};

const PDF_AbbrPairs PDF_InlineKeyAbbr[] = {
    {_FX_BSTRC("BitsPerComponent"), _FX_BSTRC("BPC")},
    {_FX_BSTRC("ColorSpace"), _FX_BSTRC("CS")},
    {_FX_BSTRC("Decode"), _FX_BSTRC("D")},
    {_FX_BSTRC("DecodeParms"), _FX_BSTRC("DP")},
    {_FX_BSTRC("Filter"), _FX_BSTRC("F")},
    {_FX_BSTRC("Height"), _FX_BSTRC("H")},
    {_FX_BSTRC("ImageMask"), _FX_BSTRC("IM")},
    {_FX_BSTRC("Interpolate"), _FX_BSTRC("I")},
    {_FX_BSTRC("Width"), _FX_BSTRC("W")},
};

const PDF_AbbrPairs PDF_InlineValueAbbr[] = {
    {_FX_BSTRC("DeviceGray"), _FX_BSTRC("G")},
    {_FX_BSTRC("DeviceRGB"), _FX_BSTRC("RGB")},
    {_FX_BSTRC("DeviceCMYK"), _FX_BSTRC("CMYK")},
    {_FX_BSTRC("Indexed"), _FX_BSTRC("I")},
    {_FX_BSTRC("ASCIIHexDecode"), _FX_BSTRC("AHx")},
    {_FX_BSTRC("ASCII85Decode"), _FX_BSTRC("A85")},
    {_FX_BSTRC("LZWDecode"), _FX_BSTRC("LZW")},
    {_FX_BSTRC("FlateDecode"), _FX_BSTRC("Fl")},
    {_FX_BSTRC("RunLengthDecode"), _FX_BSTRC("RL")},
    {_FX_BSTRC("CCITTFaxDecode"), _FX_BSTRC("CCF")},
    {_FX_BSTRC("DCTDecode"), _FX_BSTRC("DCT")},
};

struct AbbrReplacementOp {
  bool is_replace_key;
  CFX_ByteString key;
  CFX_ByteStringC replacement;
};

CFX_ByteStringC PDF_FindFullName(const PDF_AbbrPairs* table,
                                 size_t count,
                                 const CFX_ByteStringC& abbr) {
  for (size_t i = 0; i < count; ++i) {
    if (abbr.GetLength() != table[i].abbr.m_Size)
      continue;
    if (memcmp(abbr.GetPtr(), table[i].abbr.m_Ptr, abbr.GetLength()))
      continue;
    return CFX_ByteStringC(table[i].full_name.m_Ptr, table[i].full_name.m_Size);
  }
  return CFX_ByteStringC();
}

}  // namespace

CPDF_StreamContentParser::CPDF_StreamContentParser(
    CPDF_Document* pDocument,
    CPDF_Dictionary* pPageResources,
    CPDF_Dictionary* pParentResources,
    CFX_Matrix* pmtContentToUser,
    CPDF_PageObjects* pObjList,
    CPDF_Dictionary* pResources,
    CPDF_Rect* pBBox,
    CPDF_ParseOptions* pOptions,
    CPDF_AllStates* pStates,
    int level)
    : m_pDocument(pDocument),
      m_pPageResources(pPageResources),
      m_pParentResources(pParentResources),
      m_pResources(pResources),
      m_pObjectList(pObjList),
      m_Level(level),
      m_ParamStartPos(0),
      m_ParamCount(0),
      m_pCurStates(new CPDF_AllStates),
      m_pLastTextObject(nullptr),
      m_DefFontSize(0),
      m_pPathPoints(nullptr),
      m_PathPointCount(0),
      m_PathAllocSize(0),
      m_PathCurrentX(0.0f),
      m_PathCurrentY(0.0f),
      m_PathClipType(0),
      m_pLastImage(nullptr),
      m_pLastImageDict(nullptr),
      m_pLastCloneImageDict(nullptr),
      m_bReleaseLastDict(TRUE),
      m_bColored(FALSE),
      m_bResourceMissing(FALSE) {
  if (pmtContentToUser) {
    m_mtContentToUser = *pmtContentToUser;
  }
  if (pOptions) {
    m_Options = *pOptions;
  }
  if (!m_pResources) {
    m_pResources = m_pParentResources;
  }
  if (!m_pResources) {
    m_pResources = m_pPageResources;
  }
  if (pBBox) {
    m_BBox = *pBBox;
  }
  if (pStates) {
    m_pCurStates->Copy(*pStates);
  } else {
    m_pCurStates->m_GeneralState.New();
    m_pCurStates->m_GraphState.New();
    m_pCurStates->m_TextState.New();
    m_pCurStates->m_ColorState.New();
  }
  for (size_t i = 0; i < FX_ArraySize(m_Type3Data); ++i) {
    m_Type3Data[i] = 0.0;
  }
}

CPDF_StreamContentParser::~CPDF_StreamContentParser() {
  ClearAllParams();
  FX_Free(m_pPathPoints);
  if (m_pLastImageDict) {
    m_pLastImageDict->Release();
  }
  if (m_pLastCloneImageDict) {
    m_pLastCloneImageDict->Release();
  }
}
int CPDF_StreamContentParser::GetNextParamPos() {
  if (m_ParamCount == PARAM_BUF_SIZE) {
    m_ParamStartPos++;
    if (m_ParamStartPos == PARAM_BUF_SIZE) {
      m_ParamStartPos = 0;
    }
    if (m_ParamBuf1[m_ParamStartPos].m_Type == 0) {
      if (CPDF_Object* pObject = m_ParamBuf1[m_ParamStartPos].m_pObject)
        pObject->Release();
    }
    return m_ParamStartPos;
  }
  int index = m_ParamStartPos + m_ParamCount;
  if (index >= PARAM_BUF_SIZE) {
    index -= PARAM_BUF_SIZE;
  }
  m_ParamCount++;
  return index;
}
void CPDF_StreamContentParser::AddNameParam(const FX_CHAR* name, int len) {
  int index = GetNextParamPos();
  if (len > 32) {
    m_ParamBuf1[index].m_Type = 0;
    m_ParamBuf1[index].m_pObject =
        new CPDF_Name(PDF_NameDecode(CFX_ByteStringC(name, len)));
  } else {
    m_ParamBuf1[index].m_Type = PDFOBJ_NAME;
    if (!FXSYS_memchr(name, '#', len)) {
      FXSYS_memcpy(m_ParamBuf1[index].m_Name.m_Buffer, name, len);
      m_ParamBuf1[index].m_Name.m_Len = len;
    } else {
      CFX_ByteString str = PDF_NameDecode(CFX_ByteStringC(name, len));
      FXSYS_memcpy(m_ParamBuf1[index].m_Name.m_Buffer, str.c_str(),
                   str.GetLength());
      m_ParamBuf1[index].m_Name.m_Len = str.GetLength();
    }
  }
}
void CPDF_StreamContentParser::AddNumberParam(const FX_CHAR* str, int len) {
  int index = GetNextParamPos();
  m_ParamBuf1[index].m_Type = PDFOBJ_NUMBER;
  FX_atonum(CFX_ByteStringC(str, len), m_ParamBuf1[index].m_Number.m_bInteger,
            &m_ParamBuf1[index].m_Number.m_Integer);
}
void CPDF_StreamContentParser::AddObjectParam(CPDF_Object* pObj) {
  int index = GetNextParamPos();
  m_ParamBuf1[index].m_Type = 0;
  m_ParamBuf1[index].m_pObject = pObj;
}
void CPDF_StreamContentParser::ClearAllParams() {
  FX_DWORD index = m_ParamStartPos;
  for (FX_DWORD i = 0; i < m_ParamCount; i++) {
    if (m_ParamBuf1[index].m_Type == 0) {
      if (CPDF_Object* pObject = m_ParamBuf1[index].m_pObject)
        pObject->Release();
    }
    index++;
    if (index == PARAM_BUF_SIZE) {
      index = 0;
    }
  }
  m_ParamStartPos = 0;
  m_ParamCount = 0;
}
CPDF_Object* CPDF_StreamContentParser::GetObject(FX_DWORD index) {
  if (index >= m_ParamCount) {
    return NULL;
  }
  int real_index = m_ParamStartPos + m_ParamCount - index - 1;
  if (real_index >= PARAM_BUF_SIZE) {
    real_index -= PARAM_BUF_SIZE;
  }
  ContentParam& param = m_ParamBuf1[real_index];
  if (param.m_Type == PDFOBJ_NUMBER) {
    CPDF_Number* pNumber = param.m_Number.m_bInteger
                               ? new CPDF_Number(param.m_Number.m_Integer)
                               : new CPDF_Number(param.m_Number.m_Float);

    param.m_Type = 0;
    param.m_pObject = pNumber;
    return pNumber;
  }
  if (param.m_Type == PDFOBJ_NAME) {
    CPDF_Name* pName = new CPDF_Name(
        CFX_ByteString(param.m_Name.m_Buffer, param.m_Name.m_Len));
    param.m_Type = 0;
    param.m_pObject = pName;
    return pName;
  }
  if (param.m_Type == 0) {
    return param.m_pObject;
  }
  ASSERT(FALSE);
  return NULL;
}
CFX_ByteString CPDF_StreamContentParser::GetString(FX_DWORD index) {
  if (index >= m_ParamCount) {
    return CFX_ByteString();
  }
  int real_index = m_ParamStartPos + m_ParamCount - index - 1;
  if (real_index >= PARAM_BUF_SIZE) {
    real_index -= PARAM_BUF_SIZE;
  }
  ContentParam& param = m_ParamBuf1[real_index];
  if (param.m_Type == PDFOBJ_NAME) {
    return CFX_ByteString(param.m_Name.m_Buffer, param.m_Name.m_Len);
  }
  if (param.m_Type == 0 && param.m_pObject) {
    return param.m_pObject->GetString();
  }
  return CFX_ByteString();
}
FX_FLOAT CPDF_StreamContentParser::GetNumber(FX_DWORD index) {
  if (index >= m_ParamCount) {
    return 0;
  }
  int real_index = m_ParamStartPos + m_ParamCount - index - 1;
  if (real_index >= PARAM_BUF_SIZE) {
    real_index -= PARAM_BUF_SIZE;
  }
  ContentParam& param = m_ParamBuf1[real_index];
  if (param.m_Type == PDFOBJ_NUMBER) {
    return param.m_Number.m_bInteger ? (FX_FLOAT)param.m_Number.m_Integer
                                     : param.m_Number.m_Float;
  }
  if (param.m_Type == 0 && param.m_pObject) {
    return param.m_pObject->GetNumber();
  }
  return 0;
}
FX_FLOAT CPDF_StreamContentParser::GetNumber16(FX_DWORD index) {
  return GetNumber(index);
}
void CPDF_StreamContentParser::SetGraphicStates(CPDF_PageObject* pObj,
                                                FX_BOOL bColor,
                                                FX_BOOL bText,
                                                FX_BOOL bGraph) {
  pObj->m_GeneralState = m_pCurStates->m_GeneralState;
  pObj->m_ClipPath = m_pCurStates->m_ClipPath;
  pObj->m_ContentMark = m_CurContentMark;
  if (bColor) {
    pObj->m_ColorState = m_pCurStates->m_ColorState;
  }
  if (bGraph) {
    pObj->m_GraphState = m_pCurStates->m_GraphState;
  }
  if (bText) {
    pObj->m_TextState = m_pCurStates->m_TextState;
  }
}

const CPDF_StreamContentParser::OpCode CPDF_StreamContentParser::g_OpCodes[] = {
    {FXBSTR_ID('"', 0, 0, 0),
     &CPDF_StreamContentParser::Handle_NextLineShowText_Space},
    {FXBSTR_ID('\'', 0, 0, 0),
     &CPDF_StreamContentParser::Handle_NextLineShowText},
    {FXBSTR_ID('B', 0, 0, 0), &CPDF_StreamContentParser::Handle_FillStrokePath},
    {FXBSTR_ID('B', '*', 0, 0),
     &CPDF_StreamContentParser::Handle_EOFillStrokePath},
    {FXBSTR_ID('B', 'D', 'C', 0),
     &CPDF_StreamContentParser::Handle_BeginMarkedContent_Dictionary},
    {FXBSTR_ID('B', 'I', 0, 0), &CPDF_StreamContentParser::Handle_BeginImage},
    {FXBSTR_ID('B', 'M', 'C', 0),
     &CPDF_StreamContentParser::Handle_BeginMarkedContent},
    {FXBSTR_ID('B', 'T', 0, 0), &CPDF_StreamContentParser::Handle_BeginText},
    {FXBSTR_ID('B', 'X', 0, 0),
     &CPDF_StreamContentParser::Handle_BeginSectionUndefined},
    {FXBSTR_ID('C', 'S', 0, 0),
     &CPDF_StreamContentParser::Handle_SetColorSpace_Stroke},
    {FXBSTR_ID('D', 'P', 0, 0),
     &CPDF_StreamContentParser::Handle_MarkPlace_Dictionary},
    {FXBSTR_ID('D', 'o', 0, 0),
     &CPDF_StreamContentParser::Handle_ExecuteXObject},
    {FXBSTR_ID('E', 'I', 0, 0), &CPDF_StreamContentParser::Handle_EndImage},
    {FXBSTR_ID('E', 'M', 'C', 0),
     &CPDF_StreamContentParser::Handle_EndMarkedContent},
    {FXBSTR_ID('E', 'T', 0, 0), &CPDF_StreamContentParser::Handle_EndText},
    {FXBSTR_ID('E', 'X', 0, 0),
     &CPDF_StreamContentParser::Handle_EndSectionUndefined},
    {FXBSTR_ID('F', 0, 0, 0), &CPDF_StreamContentParser::Handle_FillPathOld},
    {FXBSTR_ID('G', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetGray_Stroke},
    {FXBSTR_ID('I', 'D', 0, 0),
     &CPDF_StreamContentParser::Handle_BeginImageData},
    {FXBSTR_ID('J', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetLineCap},
    {FXBSTR_ID('K', 0, 0, 0),
     &CPDF_StreamContentParser::Handle_SetCMYKColor_Stroke},
    {FXBSTR_ID('M', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetMiterLimit},
    {FXBSTR_ID('M', 'P', 0, 0), &CPDF_StreamContentParser::Handle_MarkPlace},
    {FXBSTR_ID('Q', 0, 0, 0),
     &CPDF_StreamContentParser::Handle_RestoreGraphState},
    {FXBSTR_ID('R', 'G', 0, 0),
     &CPDF_StreamContentParser::Handle_SetRGBColor_Stroke},
    {FXBSTR_ID('S', 0, 0, 0), &CPDF_StreamContentParser::Handle_StrokePath},
    {FXBSTR_ID('S', 'C', 0, 0),
     &CPDF_StreamContentParser::Handle_SetColor_Stroke},
    {FXBSTR_ID('S', 'C', 'N', 0),
     &CPDF_StreamContentParser::Handle_SetColorPS_Stroke},
    {FXBSTR_ID('T', '*', 0, 0),
     &CPDF_StreamContentParser::Handle_MoveToNextLine},
    {FXBSTR_ID('T', 'D', 0, 0),
     &CPDF_StreamContentParser::Handle_MoveTextPoint_SetLeading},
    {FXBSTR_ID('T', 'J', 0, 0),
     &CPDF_StreamContentParser::Handle_ShowText_Positioning},
    {FXBSTR_ID('T', 'L', 0, 0),
     &CPDF_StreamContentParser::Handle_SetTextLeading},
    {FXBSTR_ID('T', 'c', 0, 0), &CPDF_StreamContentParser::Handle_SetCharSpace},
    {FXBSTR_ID('T', 'd', 0, 0),
     &CPDF_StreamContentParser::Handle_MoveTextPoint},
    {FXBSTR_ID('T', 'f', 0, 0), &CPDF_StreamContentParser::Handle_SetFont},
    {FXBSTR_ID('T', 'j', 0, 0), &CPDF_StreamContentParser::Handle_ShowText},
    {FXBSTR_ID('T', 'm', 0, 0),
     &CPDF_StreamContentParser::Handle_SetTextMatrix},
    {FXBSTR_ID('T', 'r', 0, 0),
     &CPDF_StreamContentParser::Handle_SetTextRenderMode},
    {FXBSTR_ID('T', 's', 0, 0), &CPDF_StreamContentParser::Handle_SetTextRise},
    {FXBSTR_ID('T', 'w', 0, 0), &CPDF_StreamContentParser::Handle_SetWordSpace},
    {FXBSTR_ID('T', 'z', 0, 0), &CPDF_StreamContentParser::Handle_SetHorzScale},
    {FXBSTR_ID('W', 0, 0, 0), &CPDF_StreamContentParser::Handle_Clip},
    {FXBSTR_ID('W', '*', 0, 0), &CPDF_StreamContentParser::Handle_EOClip},
    {FXBSTR_ID('b', 0, 0, 0),
     &CPDF_StreamContentParser::Handle_CloseFillStrokePath},
    {FXBSTR_ID('b', '*', 0, 0),
     &CPDF_StreamContentParser::Handle_CloseEOFillStrokePath},
    {FXBSTR_ID('c', 0, 0, 0), &CPDF_StreamContentParser::Handle_CurveTo_123},
    {FXBSTR_ID('c', 'm', 0, 0), &CPDF_StreamContentParser::Handle_ConcatMatrix},
    {FXBSTR_ID('c', 's', 0, 0),
     &CPDF_StreamContentParser::Handle_SetColorSpace_Fill},
    {FXBSTR_ID('d', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetDash},
    {FXBSTR_ID('d', '0', 0, 0), &CPDF_StreamContentParser::Handle_SetCharWidth},
    {FXBSTR_ID('d', '1', 0, 0),
     &CPDF_StreamContentParser::Handle_SetCachedDevice},
    {FXBSTR_ID('f', 0, 0, 0), &CPDF_StreamContentParser::Handle_FillPath},
    {FXBSTR_ID('f', '*', 0, 0), &CPDF_StreamContentParser::Handle_EOFillPath},
    {FXBSTR_ID('g', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetGray_Fill},
    {FXBSTR_ID('g', 's', 0, 0),
     &CPDF_StreamContentParser::Handle_SetExtendGraphState},
    {FXBSTR_ID('h', 0, 0, 0), &CPDF_StreamContentParser::Handle_ClosePath},
    {FXBSTR_ID('i', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetFlat},
    {FXBSTR_ID('j', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetLineJoin},
    {FXBSTR_ID('k', 0, 0, 0),
     &CPDF_StreamContentParser::Handle_SetCMYKColor_Fill},
    {FXBSTR_ID('l', 0, 0, 0), &CPDF_StreamContentParser::Handle_LineTo},
    {FXBSTR_ID('m', 0, 0, 0), &CPDF_StreamContentParser::Handle_MoveTo},
    {FXBSTR_ID('n', 0, 0, 0), &CPDF_StreamContentParser::Handle_EndPath},
    {FXBSTR_ID('q', 0, 0, 0), &CPDF_StreamContentParser::Handle_SaveGraphState},
    {FXBSTR_ID('r', 'e', 0, 0), &CPDF_StreamContentParser::Handle_Rectangle},
    {FXBSTR_ID('r', 'g', 0, 0),
     &CPDF_StreamContentParser::Handle_SetRGBColor_Fill},
    {FXBSTR_ID('r', 'i', 0, 0),
     &CPDF_StreamContentParser::Handle_SetRenderIntent},
    {FXBSTR_ID('s', 0, 0, 0),
     &CPDF_StreamContentParser::Handle_CloseStrokePath},
    {FXBSTR_ID('s', 'c', 0, 0),
     &CPDF_StreamContentParser::Handle_SetColor_Fill},
    {FXBSTR_ID('s', 'c', 'n', 0),
     &CPDF_StreamContentParser::Handle_SetColorPS_Fill},
    {FXBSTR_ID('s', 'h', 0, 0), &CPDF_StreamContentParser::Handle_ShadeFill},
    {FXBSTR_ID('v', 0, 0, 0), &CPDF_StreamContentParser::Handle_CurveTo_23},
    {FXBSTR_ID('w', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetLineWidth},
    {FXBSTR_ID('y', 0, 0, 0), &CPDF_StreamContentParser::Handle_CurveTo_13},
};
FX_BOOL CPDF_StreamContentParser::OnOperator(const FX_CHAR* op) {
  int i = 0;
  FX_DWORD opid = 0;
  while (i < 4 && op[i]) {
    opid = (opid << 8) + op[i];
    i++;
  }
  while (i < 4) {
    opid <<= 8;
    i++;
  }
  int low = 0, high = sizeof g_OpCodes / sizeof(OpCode) - 1;
  while (low <= high) {
    int middle = (low + high) / 2;
    int compare = opid - g_OpCodes[middle].m_OpId;
    if (compare == 0) {
      (this->*g_OpCodes[middle].m_OpHandler)();
      return TRUE;
    }
    if (compare < 0) {
      high = middle - 1;
    } else {
      low = middle + 1;
    }
  }
  return m_CompatCount != 0;
}
void CPDF_StreamContentParser::Handle_CloseFillStrokePath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  Handle_ClosePath();
  AddPathObject(FXFILL_WINDING, TRUE);
}
void CPDF_StreamContentParser::Handle_FillStrokePath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathObject(FXFILL_WINDING, TRUE);
}
void CPDF_StreamContentParser::Handle_CloseEOFillStrokePath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathPoint(m_PathStartX, m_PathStartY, FXPT_LINETO | FXPT_CLOSEFIGURE);
  AddPathObject(FXFILL_ALTERNATE, TRUE);
}
void CPDF_StreamContentParser::Handle_EOFillStrokePath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathObject(FXFILL_ALTERNATE, TRUE);
}
void CPDF_StreamContentParser::Handle_BeginMarkedContent_Dictionary() {
  if (!m_Options.m_bMarkedContent) {
    return;
  }
  CFX_ByteString tag = GetString(1);
  CPDF_Object* pProperty = GetObject(0);
  if (!pProperty) {
    return;
  }
  FX_BOOL bDirect = TRUE;
  if (pProperty->IsName()) {
    pProperty = FindResourceObj("Properties", pProperty->GetString());
    if (!pProperty)
      return;
    bDirect = FALSE;
  }
  if (CPDF_Dictionary* pDict = pProperty->AsDictionary()) {
    m_CurContentMark.GetModify()->AddMark(tag, pDict, bDirect);
  }
}
void CPDF_StreamContentParser::Handle_BeginMarkedContent() {
  if (!m_Options.m_bMarkedContent) {
    return;
  }
  CFX_ByteString tag = GetString(0);
  m_CurContentMark.GetModify()->AddMark(tag, NULL, FALSE);
}

void PDF_ReplaceAbbr(CPDF_Object* pObj) {
  switch (pObj->GetType()) {
    case PDFOBJ_DICTIONARY: {
      CPDF_Dictionary* pDict = pObj->AsDictionary();
      std::vector<AbbrReplacementOp> replacements;
      for (const auto& it : *pDict) {
        CFX_ByteString key = it.first;
        CPDF_Object* value = it.second;
        CFX_ByteStringC fullname = PDF_FindFullName(
            PDF_InlineKeyAbbr, FX_ArraySize(PDF_InlineKeyAbbr), key);
        if (!fullname.IsEmpty()) {
          AbbrReplacementOp op;
          op.is_replace_key = true;
          op.key = key;
          op.replacement = fullname;
          replacements.push_back(op);
          key = fullname;
        }

        if (value->IsName()) {
          CFX_ByteString name = value->GetString();
          fullname = PDF_FindFullName(PDF_InlineValueAbbr,
                                      FX_ArraySize(PDF_InlineValueAbbr), name);
          if (!fullname.IsEmpty()) {
            AbbrReplacementOp op;
            op.is_replace_key = false;
            op.key = key;
            op.replacement = fullname;
            replacements.push_back(op);
          }
        } else {
          PDF_ReplaceAbbr(value);
        }
      }
      for (const auto& op : replacements) {
        if (op.is_replace_key)
          pDict->ReplaceKey(op.key, op.replacement);
        else
          pDict->SetAtName(op.key, op.replacement);
      }
      break;
    }
    case PDFOBJ_ARRAY: {
      CPDF_Array* pArray = pObj->AsArray();
      for (FX_DWORD i = 0; i < pArray->GetCount(); i++) {
        CPDF_Object* pElement = pArray->GetElement(i);
        if (pElement->IsName()) {
          CFX_ByteString name = pElement->GetString();
          CFX_ByteStringC fullname = PDF_FindFullName(
              PDF_InlineValueAbbr, FX_ArraySize(PDF_InlineValueAbbr), name);
          if (!fullname.IsEmpty()) {
            pArray->SetAt(i, new CPDF_Name(fullname));
          }
        } else {
          PDF_ReplaceAbbr(pElement);
        }
      }
      break;
    }
  }
}

void CPDF_StreamContentParser::Handle_BeginText() {
  m_pCurStates->m_TextMatrix.Set(1.0f, 0, 0, 1.0f, 0, 0);
  OnChangeTextMatrix();
  m_pCurStates->m_TextX = 0;
  m_pCurStates->m_TextY = 0;
  m_pCurStates->m_TextLineX = 0;
  m_pCurStates->m_TextLineY = 0;
}
void CPDF_StreamContentParser::Handle_BeginSectionUndefined() {
  m_CompatCount++;
}
void CPDF_StreamContentParser::Handle_CurveTo_123() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathPoint(GetNumber(5), GetNumber(4), FXPT_BEZIERTO);
  AddPathPoint(GetNumber(3), GetNumber(2), FXPT_BEZIERTO);
  AddPathPoint(GetNumber(1), GetNumber(0), FXPT_BEZIERTO);
}
void CPDF_StreamContentParser::Handle_ConcatMatrix() {
  FX_FLOAT a2 = GetNumber16(5), b2 = GetNumber16(4), c2 = GetNumber16(3),
           d2 = GetNumber16(2);
  FX_FLOAT e2 = GetNumber(1), f2 = GetNumber(0);
  CFX_Matrix new_matrix(a2, b2, c2, d2, e2, f2);
  new_matrix.Concat(m_pCurStates->m_CTM);
  m_pCurStates->m_CTM = new_matrix;
  OnChangeTextMatrix();
}
void CPDF_StreamContentParser::Handle_SetColorSpace_Fill() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  CFX_ByteString csname = GetString(0);
  CPDF_ColorSpace* pCS = FindColorSpace(csname);
  if (!pCS) {
    return;
  }
  m_pCurStates->m_ColorState.GetModify()->m_FillColor.SetColorSpace(pCS);
}
void CPDF_StreamContentParser::Handle_SetColorSpace_Stroke() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  CFX_ByteString csname = GetString(0);
  CPDF_ColorSpace* pCS = FindColorSpace(csname);
  if (!pCS) {
    return;
  }
  m_pCurStates->m_ColorState.GetModify()->m_StrokeColor.SetColorSpace(pCS);
}
void CPDF_StreamContentParser::Handle_SetDash() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  CPDF_Array* pArray = GetObject(1) ? GetObject(1)->GetArray() : NULL;
  if (!pArray) {
    return;
  }
  m_pCurStates->SetLineDash(pArray, GetNumber(0), 1.0f);
}
void CPDF_StreamContentParser::Handle_SetCharWidth() {
  m_Type3Data[0] = GetNumber(1);
  m_Type3Data[1] = GetNumber(0);
  m_bColored = TRUE;
}
void CPDF_StreamContentParser::Handle_SetCachedDevice() {
  for (int i = 0; i < 6; i++) {
    m_Type3Data[i] = GetNumber(5 - i);
  }
  m_bColored = FALSE;
}
void CPDF_StreamContentParser::Handle_ExecuteXObject() {
  CFX_ByteString name = GetString(0);
  if (name == m_LastImageName && m_pLastImage && m_pLastImage->GetStream() &&
      m_pLastImage->GetStream()->GetObjNum()) {
    AddImage(nullptr, m_pLastImage, FALSE);
    return;
  }

  if (m_Options.m_bTextOnly) {
    if (!m_pResources)
      return;

    CPDF_Dictionary* pList = m_pResources->GetDict("XObject");
    if (!pList && m_pPageResources && m_pResources != m_pPageResources)
      pList = m_pPageResources->GetDict("XObject");
    if (!pList)
      return;
    CPDF_Reference* pRes = ToReference(pList->GetElement(name));
    if (!pRes)
      return;

    FX_BOOL bForm;
    if (m_pDocument->IsFormStream(pRes->GetRefObjNum(), bForm) && !bForm)
      return;
  }

  CPDF_Stream* pXObject = ToStream(FindResourceObj("XObject", name));
  if (!pXObject) {
    m_bResourceMissing = TRUE;
    return;
  }

  CFX_ByteStringC type = pXObject->GetDict()
                             ? pXObject->GetDict()->GetConstString("Subtype")
                             : CFX_ByteStringC();
  if (type == "Image") {
    if (m_Options.m_bTextOnly) {
      return;
    }
    CPDF_ImageObject* pObj = AddImage(pXObject, NULL, FALSE);
    m_LastImageName = name;
    m_pLastImage = pObj->m_pImage;
    if (!m_pObjectList->m_bHasImageMask)
      m_pObjectList->m_bHasImageMask = m_pLastImage->IsMask();
  } else if (type == "Form") {
    AddForm(pXObject);
  } else {
    return;
  }
}
void CPDF_StreamContentParser::AddForm(CPDF_Stream* pStream) {
  if (!m_Options.m_bSeparateForm) {
    CPDF_Dictionary* pResources = pStream->GetDict()->GetDict("Resources");
    CFX_Matrix form_matrix = pStream->GetDict()->GetMatrix("Matrix");
    form_matrix.Concat(m_pCurStates->m_CTM);
    CPDF_Array* pBBox = pStream->GetDict()->GetArray("BBox");
    CFX_FloatRect form_bbox;
    CPDF_Path ClipPath;
    if (pBBox) {
      form_bbox = pStream->GetDict()->GetRect("BBox");
      ClipPath.New();
      ClipPath.AppendRect(form_bbox.left, form_bbox.bottom, form_bbox.right,
                          form_bbox.top);
      ClipPath.Transform(&form_matrix);
      form_bbox.Transform(&form_matrix);
    }
    CPDF_StreamContentParser parser(m_pDocument, m_pPageResources, m_pResources,
                                    &m_mtContentToUser, m_pObjectList,
                                    pResources, &form_bbox, &m_Options,
                                    m_pCurStates.get(), m_Level + 1);
    parser.m_pCurStates->m_CTM = form_matrix;
    if (ClipPath.NotNull()) {
      parser.m_pCurStates->m_ClipPath.AppendPath(ClipPath, FXFILL_WINDING,
                                                 TRUE);
    }
    CPDF_StreamAcc stream;
    stream.LoadAllData(pStream, FALSE);
    if (stream.GetSize() == 0) {
      return;
    }
    parser.Parse(stream.GetData(), stream.GetSize(), 0);
    return;
  }
  CPDF_FormObject* pFormObj = new CPDF_FormObject;
  pFormObj->m_pForm =
      new CPDF_Form(m_pDocument, m_pPageResources, pStream, m_pResources);
  pFormObj->m_FormMatrix = m_pCurStates->m_CTM;
  pFormObj->m_FormMatrix.Concat(m_mtContentToUser);
  CPDF_AllStates status;
  status.m_GeneralState = m_pCurStates->m_GeneralState;
  status.m_GraphState = m_pCurStates->m_GraphState;
  status.m_ColorState = m_pCurStates->m_ColorState;
  status.m_TextState = m_pCurStates->m_TextState;
  pFormObj->m_pForm->ParseContent(&status, NULL, NULL, &m_Options, m_Level + 1);
  if (!m_pObjectList->m_bBackgroundAlphaNeeded &&
      pFormObj->m_pForm->m_bBackgroundAlphaNeeded) {
    m_pObjectList->m_bBackgroundAlphaNeeded = TRUE;
  }
  pFormObj->CalcBoundingBox();
  SetGraphicStates(pFormObj, TRUE, TRUE, TRUE);
  m_pObjectList->m_ObjectList.AddTail(pFormObj);
}
CPDF_ImageObject* CPDF_StreamContentParser::AddImage(CPDF_Stream* pStream,
                                                     CPDF_Image* pImage,
                                                     FX_BOOL bInline) {
  if (!pStream && !pImage) {
    return NULL;
  }
  CFX_Matrix ImageMatrix;
  ImageMatrix.Copy(m_pCurStates->m_CTM);
  ImageMatrix.Concat(m_mtContentToUser);
  CPDF_ImageObject* pImageObj = new CPDF_ImageObject;
  if (pImage) {
    pImageObj->m_pImage =
        m_pDocument->GetPageData()->GetImage(pImage->GetStream());
  } else if (pStream->GetObjNum()) {
    pImageObj->m_pImage = m_pDocument->LoadImageF(pStream);
  } else {
    pImageObj->m_pImage = new CPDF_Image(m_pDocument);
    pImageObj->m_pImage->LoadImageF(pStream, bInline);
  }
  SetGraphicStates(pImageObj, pImageObj->m_pImage->IsMask(), FALSE, FALSE);
  pImageObj->m_Matrix = ImageMatrix;
  pImageObj->CalcBoundingBox();
  m_pObjectList->m_ObjectList.AddTail(pImageObj);
  return pImageObj;
}
void CPDF_StreamContentParser::Handle_MarkPlace_Dictionary() {}
void CPDF_StreamContentParser::Handle_EndImage() {}
void CPDF_StreamContentParser::Handle_EndMarkedContent() {
  if (!m_Options.m_bMarkedContent) {
    return;
  }
  if (m_CurContentMark.IsNull()) {
    return;
  }
  int count = m_CurContentMark.GetObject()->CountItems();
  if (count == 1) {
    m_CurContentMark.SetNull();
    return;
  }
  m_CurContentMark.GetModify()->DeleteLastMark();
}
void CPDF_StreamContentParser::Handle_EndText() {
  int count = m_ClipTextList.GetSize();
  if (count == 0) {
    return;
  }
  if (m_pCurStates->m_TextState.GetObject()->m_TextMode < 4) {
    for (int i = 0; i < count; i++) {
      delete m_ClipTextList.GetAt(i);
    }
  } else {
    m_pCurStates->m_ClipPath.AppendTexts(m_ClipTextList.GetData(), count);
  }
  m_ClipTextList.RemoveAll();
}
void CPDF_StreamContentParser::Handle_EndSectionUndefined() {
  if (m_CompatCount) {
    m_CompatCount--;
  }
}
void CPDF_StreamContentParser::Handle_FillPath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathObject(FXFILL_WINDING, FALSE);
}
void CPDF_StreamContentParser::Handle_FillPathOld() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathObject(FXFILL_WINDING, FALSE);
}
void CPDF_StreamContentParser::Handle_EOFillPath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathObject(FXFILL_ALTERNATE, FALSE);
}
void CPDF_StreamContentParser::Handle_SetGray_Fill() {
  FX_FLOAT value = GetNumber(0);
  CPDF_ColorSpace* pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY);
  m_pCurStates->m_ColorState.SetFillColor(pCS, &value, 1);
}
void CPDF_StreamContentParser::Handle_SetGray_Stroke() {
  FX_FLOAT value = GetNumber(0);
  CPDF_ColorSpace* pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY);
  m_pCurStates->m_ColorState.SetStrokeColor(pCS, &value, 1);
}
void CPDF_StreamContentParser::Handle_SetExtendGraphState() {
  CFX_ByteString name = GetString(0);
  CPDF_Dictionary* pGS = ToDictionary(FindResourceObj("ExtGState", name));
  if (!pGS) {
    m_bResourceMissing = TRUE;
    return;
  }
  m_pCurStates->ProcessExtGS(pGS, this);
}
void CPDF_StreamContentParser::Handle_ClosePath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  if (m_PathPointCount == 0) {
    return;
  }
  if (m_PathStartX != m_PathCurrentX || m_PathStartY != m_PathCurrentY) {
    AddPathPoint(m_PathStartX, m_PathStartY, FXPT_LINETO | FXPT_CLOSEFIGURE);
  } else if (m_pPathPoints[m_PathPointCount - 1].m_Flag != FXPT_MOVETO) {
    m_pPathPoints[m_PathPointCount - 1].m_Flag |= FXPT_CLOSEFIGURE;
  }
}
void CPDF_StreamContentParser::Handle_SetFlat() {
  m_pCurStates->m_GeneralState.GetModify()->m_Flatness = GetNumber(0);
}
void CPDF_StreamContentParser::Handle_BeginImageData() {}
void CPDF_StreamContentParser::Handle_SetLineJoin() {
  m_pCurStates->m_GraphState.GetModify()->m_LineJoin =
      (CFX_GraphStateData::LineJoin)GetInteger(0);
}
void CPDF_StreamContentParser::Handle_SetLineCap() {
  m_pCurStates->m_GraphState.GetModify()->m_LineCap =
      (CFX_GraphStateData::LineCap)GetInteger(0);
}
void CPDF_StreamContentParser::Handle_SetCMYKColor_Fill() {
  if (m_ParamCount != 4)
    return;

  FX_FLOAT values[4];
  for (int i = 0; i < 4; i++) {
    values[i] = GetNumber(3 - i);
  }
  CPDF_ColorSpace* pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICECMYK);
  m_pCurStates->m_ColorState.SetFillColor(pCS, values, 4);
}
void CPDF_StreamContentParser::Handle_SetCMYKColor_Stroke() {
  if (m_ParamCount != 4)
    return;

  FX_FLOAT values[4];
  for (int i = 0; i < 4; i++) {
    values[i] = GetNumber(3 - i);
  }
  CPDF_ColorSpace* pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICECMYK);
  m_pCurStates->m_ColorState.SetStrokeColor(pCS, values, 4);
}
void CPDF_StreamContentParser::Handle_LineTo() {
  if (m_ParamCount != 2)
    return;

  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathPoint(GetNumber(1), GetNumber(0), FXPT_LINETO);
}
void CPDF_StreamContentParser::Handle_MoveTo() {
  if (m_ParamCount != 2)
    return;

  if (m_Options.m_bTextOnly) {
    m_pSyntax->SkipPathObject();
    return;
  }
  AddPathPoint(GetNumber(1), GetNumber(0), FXPT_MOVETO);
  ParsePathObject();
}
void CPDF_StreamContentParser::Handle_SetMiterLimit() {
  m_pCurStates->m_GraphState.GetModify()->m_MiterLimit = GetNumber(0);
}
void CPDF_StreamContentParser::Handle_MarkPlace() {}
void CPDF_StreamContentParser::Handle_EndPath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathObject(0, FALSE);
}
void CPDF_StreamContentParser::Handle_SaveGraphState() {
  std::unique_ptr<CPDF_AllStates> pStates(new CPDF_AllStates);
  pStates->Copy(*m_pCurStates);
  m_StateStack.push_back(std::move(pStates));
}
void CPDF_StreamContentParser::Handle_RestoreGraphState() {
  if (m_StateStack.empty())
    return;
  std::unique_ptr<CPDF_AllStates> pStates = std::move(m_StateStack.back());
  m_StateStack.pop_back();
  m_pCurStates->Copy(*pStates);
}
void CPDF_StreamContentParser::Handle_Rectangle() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  FX_FLOAT x = GetNumber(3), y = GetNumber(2);
  FX_FLOAT w = GetNumber(1), h = GetNumber(0);
  AddPathRect(x, y, w, h);
}
void CPDF_StreamContentParser::AddPathRect(FX_FLOAT x,
                                           FX_FLOAT y,
                                           FX_FLOAT w,
                                           FX_FLOAT h) {
  AddPathPoint(x, y, FXPT_MOVETO);
  AddPathPoint(x + w, y, FXPT_LINETO);
  AddPathPoint(x + w, y + h, FXPT_LINETO);
  AddPathPoint(x, y + h, FXPT_LINETO);
  AddPathPoint(x, y, FXPT_LINETO | FXPT_CLOSEFIGURE);
}
void CPDF_StreamContentParser::Handle_SetRGBColor_Fill() {
  if (m_ParamCount != 3)
    return;

  FX_FLOAT values[3];
  for (int i = 0; i < 3; i++) {
    values[i] = GetNumber(2 - i);
  }
  CPDF_ColorSpace* pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
  m_pCurStates->m_ColorState.SetFillColor(pCS, values, 3);
}
void CPDF_StreamContentParser::Handle_SetRGBColor_Stroke() {
  if (m_ParamCount != 3)
    return;

  FX_FLOAT values[3];
  for (int i = 0; i < 3; i++) {
    values[i] = GetNumber(2 - i);
  }
  CPDF_ColorSpace* pCS = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
  m_pCurStates->m_ColorState.SetStrokeColor(pCS, values, 3);
}
void CPDF_StreamContentParser::Handle_SetRenderIntent() {}
void CPDF_StreamContentParser::Handle_CloseStrokePath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  Handle_ClosePath();
  AddPathObject(0, TRUE);
}
void CPDF_StreamContentParser::Handle_StrokePath() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathObject(0, TRUE);
}
void CPDF_StreamContentParser::Handle_SetColor_Fill() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  FX_FLOAT values[4];
  int nargs = m_ParamCount;
  if (nargs > 4) {
    nargs = 4;
  }
  for (int i = 0; i < nargs; i++) {
    values[i] = GetNumber(nargs - i - 1);
  }
  m_pCurStates->m_ColorState.SetFillColor(NULL, values, nargs);
}
void CPDF_StreamContentParser::Handle_SetColor_Stroke() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  FX_FLOAT values[4];
  int nargs = m_ParamCount;
  if (nargs > 4) {
    nargs = 4;
  }
  for (int i = 0; i < nargs; i++) {
    values[i] = GetNumber(nargs - i - 1);
  }
  m_pCurStates->m_ColorState.SetStrokeColor(NULL, values, nargs);
}
void CPDF_StreamContentParser::Handle_SetColorPS_Fill() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  CPDF_Object* pLastParam = GetObject(0);
  if (!pLastParam) {
    return;
  }
  int nargs = m_ParamCount;
  int nvalues = nargs;
  if (pLastParam->IsName()) {
    nvalues--;
  }
  FX_FLOAT* values = NULL;
  if (nvalues) {
    values = FX_Alloc(FX_FLOAT, nvalues);
    for (int i = 0; i < nvalues; i++) {
      values[i] = GetNumber(nargs - i - 1);
    }
  }
  if (nvalues != nargs) {
    CPDF_Pattern* pPattern = FindPattern(GetString(0), FALSE);
    if (pPattern) {
      m_pCurStates->m_ColorState.SetFillPattern(pPattern, values, nvalues);
    }
  } else {
    m_pCurStates->m_ColorState.SetFillColor(NULL, values, nvalues);
  }
  FX_Free(values);
}
void CPDF_StreamContentParser::Handle_SetColorPS_Stroke() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  CPDF_Object* pLastParam = GetObject(0);
  if (!pLastParam) {
    return;
  }
  int nargs = m_ParamCount;
  int nvalues = nargs;
  if (pLastParam->IsName())
    nvalues--;

  FX_FLOAT* values = NULL;
  if (nvalues) {
    values = FX_Alloc(FX_FLOAT, nvalues);
    for (int i = 0; i < nvalues; i++) {
      values[i] = GetNumber(nargs - i - 1);
    }
  }
  if (nvalues != nargs) {
    CPDF_Pattern* pPattern = FindPattern(GetString(0), FALSE);
    if (pPattern) {
      m_pCurStates->m_ColorState.SetStrokePattern(pPattern, values, nvalues);
    }
  } else {
    m_pCurStates->m_ColorState.SetStrokeColor(NULL, values, nvalues);
  }
  FX_Free(values);
}
CFX_FloatRect GetShadingBBox(CPDF_Stream* pStream,
                             ShadingType type,
                             const CFX_Matrix* pMatrix,
                             CPDF_Function** pFuncs,
                             int nFuncs,
                             CPDF_ColorSpace* pCS);

void CPDF_StreamContentParser::Handle_ShadeFill() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  CPDF_Pattern* pPattern = FindPattern(GetString(0), TRUE);
  if (!pPattern) {
    return;
  }
  if (pPattern->m_PatternType != CPDF_Pattern::SHADING) {
    return;
  }
  CPDF_ShadingPattern* pShading = static_cast<CPDF_ShadingPattern*>(pPattern);
  if (!pShading->m_bShadingObj) {
    return;
  }
  if (!pShading->Load()) {
    return;
  }
  CPDF_ShadingObject* pObj = new CPDF_ShadingObject;
  pObj->m_pShading = pShading;
  SetGraphicStates(pObj, FALSE, FALSE, FALSE);
  pObj->m_Matrix = m_pCurStates->m_CTM;
  pObj->m_Matrix.Concat(m_mtContentToUser);
  CFX_FloatRect bbox;
  if (!pObj->m_ClipPath.IsNull()) {
    bbox = pObj->m_ClipPath.GetClipBox();
  } else {
    bbox = m_BBox;
  }
  if (pShading->IsMeshShading()) {
    bbox.Intersect(GetShadingBBox(ToStream(pShading->m_pShadingObj),
                                  pShading->m_ShadingType, &pObj->m_Matrix,
                                  pShading->m_pFunctions, pShading->m_nFuncs,
                                  pShading->m_pCS));
  }
  pObj->m_Left = bbox.left;
  pObj->m_Right = bbox.right;
  pObj->m_Top = bbox.top;
  pObj->m_Bottom = bbox.bottom;
  m_pObjectList->m_ObjectList.AddTail(pObj);
}
void CPDF_StreamContentParser::Handle_SetCharSpace() {
  m_pCurStates->m_TextState.GetModify()->m_CharSpace = GetNumber(0);
}
void CPDF_StreamContentParser::Handle_MoveTextPoint() {
  m_pCurStates->m_TextLineX += GetNumber(1);
  m_pCurStates->m_TextLineY += GetNumber(0);
  m_pCurStates->m_TextX = m_pCurStates->m_TextLineX;
  m_pCurStates->m_TextY = m_pCurStates->m_TextLineY;
}
void CPDF_StreamContentParser::Handle_MoveTextPoint_SetLeading() {
  Handle_MoveTextPoint();
  m_pCurStates->m_TextLeading = -GetNumber(0);
}
void CPDF_StreamContentParser::Handle_SetFont() {
  FX_FLOAT fs = GetNumber(0);
  if (fs == 0) {
    fs = m_DefFontSize;
  }
  m_pCurStates->m_TextState.GetModify()->m_FontSize = fs;
  CPDF_Font* pFont = FindFont(GetString(1));
  if (pFont) {
    m_pCurStates->m_TextState.SetFont(pFont);
  }
}
CPDF_Object* CPDF_StreamContentParser::FindResourceObj(
    const CFX_ByteStringC& type,
    const CFX_ByteString& name) {
  if (!m_pResources) {
    return NULL;
  }
  if (m_pResources == m_pPageResources) {
    CPDF_Dictionary* pList = m_pResources->GetDict(type);
    if (!pList) {
      return NULL;
    }
    CPDF_Object* pRes = pList->GetElementValue(name);
    return pRes;
  }
  CPDF_Dictionary* pList = m_pResources->GetDict(type);
  if (!pList) {
    if (!m_pPageResources) {
      return NULL;
    }
    CPDF_Dictionary* pList = m_pPageResources->GetDict(type);
    if (!pList) {
      return NULL;
    }
    CPDF_Object* pRes = pList->GetElementValue(name);
    return pRes;
  }
  CPDF_Object* pRes = pList->GetElementValue(name);
  return pRes;
}
CPDF_Font* CPDF_StreamContentParser::FindFont(const CFX_ByteString& name) {
  CPDF_Dictionary* pFontDict = ToDictionary(FindResourceObj("Font", name));
  if (!pFontDict) {
    m_bResourceMissing = TRUE;
    return CPDF_Font::GetStockFont(m_pDocument, "Helvetica");
  }

  CPDF_Font* pFont = m_pDocument->LoadFont(pFontDict);
  if (pFont && pFont->GetType3Font()) {
    pFont->GetType3Font()->SetPageResources(m_pResources);
    pFont->GetType3Font()->CheckType3FontMetrics();
  }
  return pFont;
}
CPDF_ColorSpace* CPDF_StreamContentParser::FindColorSpace(
    const CFX_ByteString& name) {
  if (name == "Pattern") {
    return CPDF_ColorSpace::GetStockCS(PDFCS_PATTERN);
  }
  if (name == "DeviceGray" || name == "DeviceCMYK" || name == "DeviceRGB") {
    CFX_ByteString defname = "Default";
    defname += name.Mid(7);
    CPDF_Object* pDefObj = FindResourceObj("ColorSpace", defname);
    if (!pDefObj) {
      if (name == "DeviceGray") {
        return CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY);
      }
      if (name == "DeviceRGB") {
        return CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
      }
      return CPDF_ColorSpace::GetStockCS(PDFCS_DEVICECMYK);
    }
    return m_pDocument->LoadColorSpace(pDefObj);
  }
  CPDF_Object* pCSObj = FindResourceObj("ColorSpace", name);
  if (!pCSObj) {
    m_bResourceMissing = TRUE;
    return NULL;
  }
  return m_pDocument->LoadColorSpace(pCSObj);
}
CPDF_Pattern* CPDF_StreamContentParser::FindPattern(const CFX_ByteString& name,
                                                    FX_BOOL bShading) {
  CPDF_Object* pPattern =
      FindResourceObj(bShading ? "Shading" : "Pattern", name);
  if (!pPattern || (!pPattern->IsDictionary() && !pPattern->IsStream())) {
    m_bResourceMissing = TRUE;
    return NULL;
  }
  return m_pDocument->LoadPattern(pPattern, bShading,
                                  &m_pCurStates->m_ParentMatrix);
}
void CPDF_StreamContentParser::ConvertTextSpace(FX_FLOAT& x, FX_FLOAT& y) {
  m_pCurStates->m_TextMatrix.Transform(x, y, x, y);
  ConvertUserSpace(x, y);
}
void CPDF_StreamContentParser::ConvertUserSpace(FX_FLOAT& x, FX_FLOAT& y) {
  m_pCurStates->m_CTM.Transform(x, y, x, y);
  m_mtContentToUser.Transform(x, y, x, y);
}
void CPDF_StreamContentParser::AddTextObject(CFX_ByteString* pStrs,
                                             FX_FLOAT fInitKerning,
                                             FX_FLOAT* pKerning,
                                             int nsegs) {
  CPDF_Font* pFont = m_pCurStates->m_TextState.GetFont();
  if (!pFont) {
    return;
  }
  if (fInitKerning != 0) {
    if (!pFont->IsVertWriting()) {
      m_pCurStates->m_TextX -=
          FXSYS_Mul(fInitKerning, m_pCurStates->m_TextState.GetFontSize()) /
          1000;
    } else {
      m_pCurStates->m_TextY -=
          FXSYS_Mul(fInitKerning, m_pCurStates->m_TextState.GetFontSize()) /
          1000;
    }
  }
  if (nsegs == 0) {
    return;
  }
  int textmode;
  if (pFont->GetFontType() == PDFFONT_TYPE3) {
    textmode = 0;
  } else {
    textmode = m_pCurStates->m_TextState.GetObject()->m_TextMode;
  }
  CPDF_TextObject* pText = new CPDF_TextObject;
  m_pLastTextObject = pText;
  SetGraphicStates(pText, TRUE, TRUE, TRUE);
  if (textmode && textmode != 3 && textmode != 4 && textmode != 7) {
    FX_FLOAT* pCTM = pText->m_TextState.GetModify()->m_CTM;
    pCTM[0] = m_pCurStates->m_CTM.a;
    pCTM[1] = m_pCurStates->m_CTM.c;
    pCTM[2] = m_pCurStates->m_CTM.b;
    pCTM[3] = m_pCurStates->m_CTM.d;
  }
  pText->SetSegments(pStrs, pKerning, nsegs);
  pText->m_PosX = m_pCurStates->m_TextX;
  pText->m_PosY = m_pCurStates->m_TextY + m_pCurStates->m_TextRise;
  ConvertTextSpace(pText->m_PosX, pText->m_PosY);
  FX_FLOAT x_advance, y_advance;
  pText->CalcPositionData(&x_advance, &y_advance, m_pCurStates->m_TextHorzScale,
                          m_Level);
  m_pCurStates->m_TextX += x_advance;
  m_pCurStates->m_TextY += y_advance;
  if (textmode > 3) {
    CPDF_TextObject* pCopy = new CPDF_TextObject;
    pCopy->Copy(pText);
    m_ClipTextList.Add(pCopy);
  }
  m_pObjectList->m_ObjectList.AddTail(pText);
  if (pKerning && pKerning[nsegs - 1] != 0) {
    if (!pFont->IsVertWriting()) {
      m_pCurStates->m_TextX -=
          FXSYS_Mul(pKerning[nsegs - 1],
                    m_pCurStates->m_TextState.GetFontSize()) /
          1000;
    } else {
      m_pCurStates->m_TextY -=
          FXSYS_Mul(pKerning[nsegs - 1],
                    m_pCurStates->m_TextState.GetFontSize()) /
          1000;
    }
  }
}
void CPDF_StreamContentParser::Handle_ShowText() {
  CFX_ByteString str = GetString(0);
  if (str.IsEmpty()) {
    return;
  }
  AddTextObject(&str, 0, NULL, 1);
}
void CPDF_StreamContentParser::Handle_ShowText_Positioning() {
  CPDF_Array* pArray = GetObject(0) ? GetObject(0)->GetArray() : NULL;
  if (!pArray) {
    return;
  }
  int n = pArray->GetCount();
  int nsegs = 0;
  for (int i = 0; i < n; i++) {
    if (pArray->GetElementValue(i)->IsString())
      nsegs++;
  }
  if (nsegs == 0) {
    for (int i = 0; i < n; i++) {
      m_pCurStates->m_TextX -=
          FXSYS_Mul(pArray->GetNumber(i),
                    m_pCurStates->m_TextState.GetFontSize()) /
          1000;
    }
    return;
  }
  CFX_ByteString* pStrs = new CFX_ByteString[nsegs];
  FX_FLOAT* pKerning = FX_Alloc(FX_FLOAT, nsegs);
  int iSegment = 0;
  FX_FLOAT fInitKerning = 0;
  for (int i = 0; i < n; i++) {
    CPDF_Object* pObj = pArray->GetElementValue(i);
    if (pObj->IsString()) {
      CFX_ByteString str = pObj->GetString();
      if (str.IsEmpty()) {
        continue;
      }
      pStrs[iSegment] = str;
      pKerning[iSegment++] = 0;
    } else {
      FX_FLOAT num = pObj ? pObj->GetNumber() : 0;
      if (iSegment == 0) {
        fInitKerning += num;
      } else {
        pKerning[iSegment - 1] += num;
      }
    }
  }
  AddTextObject(pStrs, fInitKerning, pKerning, iSegment);
  delete[] pStrs;
  FX_Free(pKerning);
}
void CPDF_StreamContentParser::Handle_SetTextLeading() {
  m_pCurStates->m_TextLeading = GetNumber(0);
}
void CPDF_StreamContentParser::Handle_SetTextMatrix() {
  m_pCurStates->m_TextMatrix.Set(GetNumber16(5), GetNumber16(4), GetNumber16(3),
                                 GetNumber16(2), GetNumber(1), GetNumber(0));
  OnChangeTextMatrix();
  m_pCurStates->m_TextX = 0;
  m_pCurStates->m_TextY = 0;
  m_pCurStates->m_TextLineX = 0;
  m_pCurStates->m_TextLineY = 0;
}
void CPDF_StreamContentParser::OnChangeTextMatrix() {
  CFX_Matrix text_matrix(m_pCurStates->m_TextHorzScale, 0.0f, 0.0f, 1.0f, 0.0f,
                         0.0f);
  text_matrix.Concat(m_pCurStates->m_TextMatrix);
  text_matrix.Concat(m_pCurStates->m_CTM);
  text_matrix.Concat(m_mtContentToUser);
  FX_FLOAT* pTextMatrix = m_pCurStates->m_TextState.GetModify()->m_Matrix;
  pTextMatrix[0] = text_matrix.a;
  pTextMatrix[1] = text_matrix.c;
  pTextMatrix[2] = text_matrix.b;
  pTextMatrix[3] = text_matrix.d;
}
void CPDF_StreamContentParser::Handle_SetTextRenderMode() {
  int mode = GetInteger(0);
  if (mode < 0 || mode > 7) {
    return;
  }
  m_pCurStates->m_TextState.GetModify()->m_TextMode = mode;
}
void CPDF_StreamContentParser::Handle_SetTextRise() {
  m_pCurStates->m_TextRise = GetNumber(0);
}
void CPDF_StreamContentParser::Handle_SetWordSpace() {
  m_pCurStates->m_TextState.GetModify()->m_WordSpace = GetNumber(0);
}
void CPDF_StreamContentParser::Handle_SetHorzScale() {
  if (m_ParamCount != 1) {
    return;
  }
  m_pCurStates->m_TextHorzScale = GetNumber(0) / 100;
  OnChangeTextMatrix();
}
void CPDF_StreamContentParser::Handle_MoveToNextLine() {
  m_pCurStates->m_TextLineY -= m_pCurStates->m_TextLeading;
  m_pCurStates->m_TextX = m_pCurStates->m_TextLineX;
  m_pCurStates->m_TextY = m_pCurStates->m_TextLineY;
}
void CPDF_StreamContentParser::Handle_CurveTo_23() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathPoint(m_PathCurrentX, m_PathCurrentY, FXPT_BEZIERTO);
  AddPathPoint(GetNumber(3), GetNumber(2), FXPT_BEZIERTO);
  AddPathPoint(GetNumber(1), GetNumber(0), FXPT_BEZIERTO);
}
void CPDF_StreamContentParser::Handle_SetLineWidth() {
  FX_FLOAT width = GetNumber(0);
  m_pCurStates->m_GraphState.GetModify()->m_LineWidth = width;
}
void CPDF_StreamContentParser::Handle_Clip() {
  m_PathClipType = FXFILL_WINDING;
}
void CPDF_StreamContentParser::Handle_EOClip() {
  m_PathClipType = FXFILL_ALTERNATE;
}
void CPDF_StreamContentParser::Handle_CurveTo_13() {
  if (m_Options.m_bTextOnly) {
    return;
  }
  AddPathPoint(GetNumber(3), GetNumber(2), FXPT_BEZIERTO);
  AddPathPoint(GetNumber(1), GetNumber(0), FXPT_BEZIERTO);
  AddPathPoint(GetNumber(1), GetNumber(0), FXPT_BEZIERTO);
}
void CPDF_StreamContentParser::Handle_NextLineShowText() {
  Handle_MoveToNextLine();
  Handle_ShowText();
}
void CPDF_StreamContentParser::Handle_NextLineShowText_Space() {
  m_pCurStates->m_TextState.GetModify()->m_WordSpace = GetNumber(2);
  m_pCurStates->m_TextState.GetModify()->m_CharSpace = GetNumber(1);
  Handle_NextLineShowText();
}
void CPDF_StreamContentParser::Handle_Invalid() {}
void CPDF_StreamContentParser::AddPathPoint(FX_FLOAT x, FX_FLOAT y, int flag) {
  m_PathCurrentX = x;
  m_PathCurrentY = y;
  if (flag == FXPT_MOVETO) {
    m_PathStartX = x;
    m_PathStartY = y;
    if (m_PathPointCount &&
        m_pPathPoints[m_PathPointCount - 1].m_Flag == FXPT_MOVETO) {
      m_pPathPoints[m_PathPointCount - 1].m_PointX = x;
      m_pPathPoints[m_PathPointCount - 1].m_PointY = y;
      return;
    }
  } else if (m_PathPointCount == 0) {
    return;
  }
  m_PathPointCount++;
  if (m_PathPointCount > m_PathAllocSize) {
    int newsize = m_PathPointCount + 256;
    FX_PATHPOINT* pNewPoints = FX_Alloc(FX_PATHPOINT, newsize);
    if (m_PathAllocSize) {
      FXSYS_memcpy(pNewPoints, m_pPathPoints,
                   m_PathAllocSize * sizeof(FX_PATHPOINT));
      FX_Free(m_pPathPoints);
    }
    m_pPathPoints = pNewPoints;
    m_PathAllocSize = newsize;
  }
  m_pPathPoints[m_PathPointCount - 1].m_Flag = flag;
  m_pPathPoints[m_PathPointCount - 1].m_PointX = x;
  m_pPathPoints[m_PathPointCount - 1].m_PointY = y;
}
void CPDF_StreamContentParser::AddPathObject(int FillType, FX_BOOL bStroke) {
  int PathPointCount = m_PathPointCount, PathClipType = m_PathClipType;
  m_PathPointCount = 0;
  m_PathClipType = 0;
  if (PathPointCount <= 1) {
    if (PathPointCount && PathClipType) {
      CPDF_Path path;
      path.New()->AppendRect(0, 0, 0, 0);
      m_pCurStates->m_ClipPath.AppendPath(path, FXFILL_WINDING, TRUE);
    }
    return;
  }
  if (PathPointCount &&
      m_pPathPoints[PathPointCount - 1].m_Flag == FXPT_MOVETO) {
    PathPointCount--;
  }
  CPDF_Path Path;
  CFX_PathData* pPathData = Path.New();
  pPathData->SetPointCount(PathPointCount);
  FXSYS_memcpy(pPathData->GetPoints(), m_pPathPoints,
               sizeof(FX_PATHPOINT) * PathPointCount);
  CFX_Matrix matrix = m_pCurStates->m_CTM;
  matrix.Concat(m_mtContentToUser);
  if (bStroke || FillType) {
    CPDF_PathObject* pPathObj = new CPDF_PathObject;
    pPathObj->m_bStroke = bStroke;
    pPathObj->m_FillType = FillType;
    pPathObj->m_Path = Path;
    pPathObj->m_Matrix = matrix;
    SetGraphicStates(pPathObj, TRUE, FALSE, TRUE);
    pPathObj->CalcBoundingBox();
    m_pObjectList->m_ObjectList.AddTail(pPathObj);
  }
  if (PathClipType) {
    if (!matrix.IsIdentity()) {
      Path.Transform(&matrix);
      matrix.SetIdentity();
    }
    m_pCurStates->m_ClipPath.AppendPath(Path, PathClipType, TRUE);
  }
}
