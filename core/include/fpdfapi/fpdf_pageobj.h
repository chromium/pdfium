// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_PAGEOBJ_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_PAGEOBJ_H_

#include <vector>

#include "core/include/fpdfapi/fpdf_resource.h"
#include "core/include/fxge/fx_ge.h"

class CPDF_ClipPath;
class CPDF_ClipPathData;
class CPDF_ColorState;
class CPDF_ColorStateData;
class CPDF_ContentMark;
class CPDF_ContentMarkItem;
class CPDF_FormObject;
class CPDF_GeneralState;
class CPDF_GeneralStateData;
class CPDF_GraphicStates;
class CPDF_GraphState;
class CPDF_ImageObject;
class CPDF_PageObject;
class CPDF_Path;
class CPDF_PathObject;
class CPDF_ShadingObject;
class CPDF_TextObject;
class CPDF_TextState;
class CPDF_TextStateData;
class CPDF_TransferFunc;

class CPDF_Path : public CFX_CountRef<CFX_PathData> {
 public:
  int GetPointCount() const { return m_pObject->m_PointCount; }
  int GetFlag(int index) const { return m_pObject->m_pPoints[index].m_Flag; }
  FX_FLOAT GetPointX(int index) const {
    return m_pObject->m_pPoints[index].m_PointX;
  }
  FX_FLOAT GetPointY(int index) const {
    return m_pObject->m_pPoints[index].m_PointY;
  }
  FX_PATHPOINT* GetPoints() const { return m_pObject->m_pPoints; }
  CFX_FloatRect GetBoundingBox() const { return m_pObject->GetBoundingBox(); }
  CFX_FloatRect GetBoundingBox(FX_FLOAT line_width,
                               FX_FLOAT miter_limit) const {
    return m_pObject->GetBoundingBox(line_width, miter_limit);
  }
  FX_BOOL IsRect() const { return m_pObject->IsRect(); }

  void Transform(const CFX_Matrix* pMatrix) { GetModify()->Transform(pMatrix); }
  void Append(CPDF_Path src, const CFX_Matrix* pMatrix) {
    m_pObject->Append(src.m_pObject, pMatrix);
  }
  void AppendRect(FX_FLOAT left,
                  FX_FLOAT bottom,
                  FX_FLOAT right,
                  FX_FLOAT top) {
    m_pObject->AppendRect(left, bottom, right, top);
  }
};

class CPDF_ClipPathData {
 public:
  CPDF_ClipPathData();
  CPDF_ClipPathData(const CPDF_ClipPathData&);
  ~CPDF_ClipPathData();

  void SetCount(int path_count, int text_count);

  int m_PathCount;
  CPDF_Path* m_pPathList;
  uint8_t* m_pTypeList;
  int m_TextCount;
  CPDF_TextObject** m_pTextList;
};

class CPDF_ClipPath : public CFX_CountRef<CPDF_ClipPathData> {
 public:
  FX_DWORD GetPathCount() const { return m_pObject->m_PathCount; }
  CPDF_Path GetPath(int i) const { return m_pObject->m_pPathList[i]; }
  int GetClipType(int i) const { return m_pObject->m_pTypeList[i]; }
  FX_DWORD GetTextCount() const { return m_pObject->m_TextCount; }
  CPDF_TextObject* GetText(int i) const { return m_pObject->m_pTextList[i]; }
  CFX_FloatRect GetClipBox() const;
  void AppendPath(CPDF_Path path, int type, FX_BOOL bAutoMerge);
  void DeletePath(int layer_index);
  void AppendTexts(CPDF_TextObject** pTexts, int count);
  void Transform(const CFX_Matrix& matrix);
};

class CPDF_ColorStateData {
 public:
  CPDF_ColorStateData() : m_FillRGB(0), m_StrokeRGB(0) {}
  CPDF_ColorStateData(const CPDF_ColorStateData& src);

  void Default();

  CPDF_Color m_FillColor;
  FX_DWORD m_FillRGB;
  CPDF_Color m_StrokeColor;
  FX_DWORD m_StrokeRGB;
};

class CPDF_ColorState : public CFX_CountRef<CPDF_ColorStateData> {
 public:
  CPDF_Color* GetFillColor() const {
    return m_pObject ? &m_pObject->m_FillColor : nullptr;
  }

  CPDF_Color* GetStrokeColor() const {
    return m_pObject ? &m_pObject->m_StrokeColor : nullptr;
  }

  void SetFillColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues);
  void SetStrokeColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues);
  void SetFillPattern(CPDF_Pattern* pattern, FX_FLOAT* pValue, int nValues);
  void SetStrokePattern(CPDF_Pattern* pattern, FX_FLOAT* pValue, int nValues);

 private:
  void SetColor(CPDF_Color& color,
                FX_DWORD& rgb,
                CPDF_ColorSpace* pCS,
                FX_FLOAT* pValue,
                int nValues);
};

class CPDF_GraphState : public CFX_CountRef<CFX_GraphStateData> {
};

class CPDF_TextStateData {
 public:
  CPDF_TextStateData();
  CPDF_TextStateData(const CPDF_TextStateData& src);
  ~CPDF_TextStateData();

  CPDF_Font* m_pFont;
  CPDF_Document* m_pDocument;
  FX_FLOAT m_FontSize;
  FX_FLOAT m_CharSpace;
  FX_FLOAT m_WordSpace;
  FX_FLOAT m_Matrix[4];
  int m_TextMode;
  FX_FLOAT m_CTM[4];
};

class CPDF_TextState : public CFX_CountRef<CPDF_TextStateData> {
 public:
  CPDF_Font* GetFont() const { return m_pObject->m_pFont; }
  void SetFont(CPDF_Font* pFont);
  FX_FLOAT GetFontSize() const { return m_pObject->m_FontSize; }
  FX_FLOAT* GetMatrix() const { return m_pObject->m_Matrix; }
  FX_FLOAT GetFontSizeV() const;
  FX_FLOAT GetFontSizeH() const;
  FX_FLOAT GetBaselineAngle() const;
  FX_FLOAT GetShearAngle() const;
};

class CPDF_GeneralStateData {
 public:
  CPDF_GeneralStateData();
  CPDF_GeneralStateData(const CPDF_GeneralStateData& src);
  ~CPDF_GeneralStateData();

  void SetBlendMode(const CFX_ByteStringC& blend_mode);

  char m_BlendMode[16];
  int m_BlendType;
  CPDF_Object* m_pSoftMask;
  FX_FLOAT m_SMaskMatrix[6];
  FX_FLOAT m_StrokeAlpha;
  FX_FLOAT m_FillAlpha;
  CPDF_Object* m_pTR;
  CPDF_TransferFunc* m_pTransferFunc;
  CFX_Matrix m_Matrix;
  int m_RenderIntent;
  FX_BOOL m_StrokeAdjust;
  FX_BOOL m_AlphaSource;
  FX_BOOL m_TextKnockout;
  FX_BOOL m_StrokeOP;
  FX_BOOL m_FillOP;
  int m_OPMode;
  CPDF_Object* m_pBG;
  CPDF_Object* m_pUCR;
  CPDF_Object* m_pHT;
  FX_FLOAT m_Flatness;
  FX_FLOAT m_Smoothness;
};

class CPDF_GeneralState : public CFX_CountRef<CPDF_GeneralStateData> {
 public:
  void SetRenderIntent(const CFX_ByteString& ri);

  int GetBlendType() const {
    return m_pObject ? m_pObject->m_BlendType : FXDIB_BLEND_NORMAL;
  }

  int GetAlpha(FX_BOOL bStroke) const {
    return m_pObject ? FXSYS_round((bStroke ? m_pObject->m_StrokeAlpha
                                            : m_pObject->m_FillAlpha) *
                                   255)
                     : 255;
  }
};

class CPDF_ContentMarkItem {
 public:
  enum ParamType { None, PropertiesDict, DirectDict };

  CPDF_ContentMarkItem();
  CPDF_ContentMarkItem(const CPDF_ContentMarkItem& src);
  ~CPDF_ContentMarkItem();

  const CFX_ByteString& GetName() const { return m_MarkName; }
  ParamType GetParamType() const { return m_ParamType; }
  CPDF_Dictionary* GetParam() const { return m_pParam; }
  FX_BOOL HasMCID() const;
  void SetName(const CFX_ByteString& name) { m_MarkName = name; }
  void SetParam(ParamType type, CPDF_Dictionary* param) {
    m_ParamType = type;
    m_pParam = param;
  }

 private:
  CFX_ByteString m_MarkName;
  ParamType m_ParamType;
  CPDF_Dictionary* m_pParam;
};

class CPDF_ContentMarkData {
 public:
  CPDF_ContentMarkData() {}
  CPDF_ContentMarkData(const CPDF_ContentMarkData& src);

  int CountItems() const;
  CPDF_ContentMarkItem& GetItem(int index) { return m_Marks[index]; }
  const CPDF_ContentMarkItem& GetItem(int index) const {
    return m_Marks[index];
  }

  int GetMCID() const;
  void AddMark(const CFX_ByteString& name,
               CPDF_Dictionary* pDict,
               FX_BOOL bDictNeedClone);
  void DeleteLastMark();

 private:
  std::vector<CPDF_ContentMarkItem> m_Marks;
};

class CPDF_ContentMark : public CFX_CountRef<CPDF_ContentMarkData> {
 public:
  int GetMCID() const { return m_pObject ? m_pObject->GetMCID() : -1; }

  FX_BOOL HasMark(const CFX_ByteStringC& mark) const;

  FX_BOOL LookupMark(const CFX_ByteStringC& mark,
                     CPDF_Dictionary*& pDict) const;
};

class CPDF_GraphicStates {
 public:
  void CopyStates(const CPDF_GraphicStates& src);

  void DefaultStates();

  CPDF_ClipPath m_ClipPath;

  CPDF_GraphState m_GraphState;

  CPDF_ColorState m_ColorState;

  CPDF_TextState m_TextState;

  CPDF_GeneralState m_GeneralState;
};

class CPDF_PageObject : public CPDF_GraphicStates {
 public:
  enum Type {
    TEXT = 1,
    PATH,
    IMAGE,
    SHADING,
    FORM,
  };

  CPDF_PageObject();
  virtual ~CPDF_PageObject();

  virtual CPDF_PageObject* Clone() const = 0;
  virtual Type GetType() const = 0;
  virtual void Transform(const CFX_Matrix& matrix) = 0;
  virtual bool IsText() const { return false; }
  virtual bool IsPath() const { return false; }
  virtual bool IsImage() const { return false; }
  virtual bool IsShading() const { return false; }
  virtual bool IsForm() const { return false; }
  virtual CPDF_TextObject* AsText() { return nullptr; }
  virtual const CPDF_TextObject* AsText() const { return nullptr; }
  virtual CPDF_PathObject* AsPath() { return nullptr; }
  virtual const CPDF_PathObject* AsPath() const { return nullptr; }
  virtual CPDF_ImageObject* AsImage() { return nullptr; }
  virtual const CPDF_ImageObject* AsImage() const { return nullptr; }
  virtual CPDF_ShadingObject* AsShading() { return nullptr; }
  virtual const CPDF_ShadingObject* AsShading() const { return nullptr; }
  virtual CPDF_FormObject* AsForm() { return nullptr; }
  virtual const CPDF_FormObject* AsForm() const { return nullptr; }

  void TransformClipPath(CFX_Matrix& matrix);
  void TransformGeneralState(CFX_Matrix& matrix);
  FX_RECT GetBBox(const CFX_Matrix* pMatrix) const;

  FX_FLOAT m_Left;
  FX_FLOAT m_Right;
  FX_FLOAT m_Top;
  FX_FLOAT m_Bottom;
  CPDF_ContentMark m_ContentMark;

 protected:
  void CopyData(const CPDF_PageObject* pSrcObject);

 private:
  CPDF_PageObject(const CPDF_PageObject& src) = delete;
  void operator=(const CPDF_PageObject& src) = delete;
};

struct CPDF_TextObjectItem {
  FX_DWORD m_CharCode;
  FX_FLOAT m_OriginX;
  FX_FLOAT m_OriginY;
};

class CPDF_TextObject : public CPDF_PageObject {
 public:
  CPDF_TextObject();
  ~CPDF_TextObject() override;

  // CPDF_PageObject:
  CPDF_TextObject* Clone() const override;
  Type GetType() const override { return TEXT; };
  void Transform(const CFX_Matrix& matrix) override;
  bool IsText() const override { return true; };
  CPDF_TextObject* AsText() override { return this; };
  const CPDF_TextObject* AsText() const override { return this; };

  int CountItems() const { return m_nChars; }
  void GetItemInfo(int index, CPDF_TextObjectItem* pInfo) const;
  int CountChars() const;
  void GetCharInfo(int index, FX_DWORD& charcode, FX_FLOAT& kerning) const;
  void GetCharInfo(int index, CPDF_TextObjectItem* pInfo) const;
  FX_FLOAT GetCharWidth(FX_DWORD charcode) const;
  FX_FLOAT GetPosX() const { return m_PosX; }
  FX_FLOAT GetPosY() const { return m_PosY; }
  void GetTextMatrix(CFX_Matrix* pMatrix) const;
  CPDF_Font* GetFont() const { return m_TextState.GetFont(); }
  FX_FLOAT GetFontSize() const { return m_TextState.GetFontSize(); }

  void SetText(const CFX_ByteString& text);
  void SetPosition(FX_FLOAT x, FX_FLOAT y);

  void RecalcPositionData() { CalcPositionData(nullptr, nullptr, 1); }

 protected:
  friend class CPDF_RenderStatus;
  friend class CPDF_StreamContentParser;
  friend class CPDF_TextRenderer;

  void SetSegments(const CFX_ByteString* pStrs, FX_FLOAT* pKerning, int nSegs);

  void CalcPositionData(FX_FLOAT* pTextAdvanceX,
                        FX_FLOAT* pTextAdvanceY,
                        FX_FLOAT horz_scale,
                        int level = 0);

  FX_FLOAT m_PosX;
  FX_FLOAT m_PosY;
  int m_nChars;
  FX_DWORD* m_pCharCodes;
  FX_FLOAT* m_pCharPos;
};

class CPDF_PathObject : public CPDF_PageObject {
 public:
  CPDF_PathObject();
  ~CPDF_PathObject() override;

  // CPDF_PageObject:
  CPDF_PathObject* Clone() const override;
  Type GetType() const override { return PATH; };
  void Transform(const CFX_Matrix& maxtrix) override;
  bool IsPath() const override { return true; };
  CPDF_PathObject* AsPath() override { return this; };
  const CPDF_PathObject* AsPath() const override { return this; };

  void CalcBoundingBox();

  CPDF_Path m_Path;
  int m_FillType;
  FX_BOOL m_bStroke;
  CFX_Matrix m_Matrix;
};

class CPDF_ImageObject : public CPDF_PageObject {
 public:
  CPDF_ImageObject();
  ~CPDF_ImageObject() override;

  // CPDF_PageObject:
  CPDF_ImageObject* Clone() const override;
  Type GetType() const override { return IMAGE; };
  void Transform(const CFX_Matrix& matrix) override;
  bool IsImage() const override { return true; };
  CPDF_ImageObject* AsImage() override { return this; };
  const CPDF_ImageObject* AsImage() const override { return this; };

  void CalcBoundingBox();

  CPDF_Image* m_pImage;
  CFX_Matrix m_Matrix;
};

class CPDF_ShadingObject : public CPDF_PageObject {
 public:
  CPDF_ShadingObject();
  ~CPDF_ShadingObject() override;

  // CPDF_PageObject:
  CPDF_ShadingObject* Clone() const override;
  Type GetType() const override { return SHADING; };
  void Transform(const CFX_Matrix& matrix) override;
  bool IsShading() const override { return true; };
  CPDF_ShadingObject* AsShading() override { return this; };
  const CPDF_ShadingObject* AsShading() const override { return this; };

  void CalcBoundingBox();

  CPDF_ShadingPattern* m_pShading;
  CFX_Matrix m_Matrix;
};

class CPDF_FormObject : public CPDF_PageObject {
 public:
  CPDF_FormObject();
  ~CPDF_FormObject() override;

  // CPDF_PageObject:
  CPDF_FormObject* Clone() const override;
  Type GetType() const override { return FORM; };
  void Transform(const CFX_Matrix& matrix) override;
  bool IsForm() const override { return true; };
  CPDF_FormObject* AsForm() override { return this; };
  const CPDF_FormObject* AsForm() const override { return this; };

  void CalcBoundingBox();

  CPDF_Form* m_pForm;
  CFX_Matrix m_FormMatrix;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_PAGEOBJ_H_
