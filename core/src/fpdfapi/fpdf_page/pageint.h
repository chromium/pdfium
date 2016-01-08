// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFAPI_FPDF_PAGE_PAGEINT_H_
#define CORE_SRC_FPDFAPI_FPDF_PAGE_PAGEINT_H_

#include <map>
#include <memory>
#include <vector>

#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fpdfapi/fpdf_pageobj.h"

class CPDF_AllStates;
class CPDF_ParseOptions;

#define PARSE_STEP_LIMIT 100

class CPDF_StreamParser {
 public:
  enum SyntaxType { EndOfData, Number, Keyword, Name, Others };

  CPDF_StreamParser(const uint8_t* pData, FX_DWORD dwSize);
  ~CPDF_StreamParser();

  CPDF_Stream* ReadInlineStream(CPDF_Document* pDoc,
                                CPDF_Dictionary* pDict,
                                CPDF_Object* pCSObj,
                                FX_BOOL bDecode);
  SyntaxType ParseNextElement();
  uint8_t* GetWordBuf() { return m_WordBuffer; }
  FX_DWORD GetWordSize() const { return m_WordSize; }
  CPDF_Object* GetObject() {
    CPDF_Object* pObj = m_pLastObj;
    m_pLastObj = NULL;
    return pObj;
  }
  FX_DWORD GetPos() const { return m_Pos; }
  void SetPos(FX_DWORD pos) { m_Pos = pos; }
  CPDF_Object* ReadNextObject(FX_BOOL bAllowNestedArray = FALSE,
                              FX_BOOL bInArray = FALSE);
  void SkipPathObject();

 protected:
  friend class fpdf_page_parser_old_ReadHexString_Test;

  void GetNextWord(FX_BOOL& bIsNumber);
  CFX_ByteString ReadString();
  CFX_ByteString ReadHexString();
  const uint8_t* m_pBuf;

  // Length in bytes of m_pBuf.
  FX_DWORD m_Size;

  // Current byte position within m_pBuf.
  FX_DWORD m_Pos;

  uint8_t m_WordBuffer[256];
  FX_DWORD m_WordSize;
  CPDF_Object* m_pLastObj;

 private:
  bool PositionIsInBounds() const;
};

#define PARAM_BUF_SIZE 16
struct ContentParam {
  int m_Type;
  union {
    struct {
      FX_BOOL m_bInteger;
      union {
        int m_Integer;
        FX_FLOAT m_Float;
      };
    } m_Number;
    CPDF_Object* m_pObject;
    struct {
      int m_Len;
      char m_Buffer[32];
    } m_Name;
  };
};
#define _FPDF_MAX_FORM_LEVEL_ 30
#define _FPDF_MAX_TYPE3_FORM_LEVEL_ 4
#define _FPDF_MAX_OBJECT_STACK_SIZE_ 512
class CPDF_StreamContentParser {
 public:
  CPDF_StreamContentParser(CPDF_Document* pDoc,
                           CPDF_Dictionary* pPageResources,
                           CPDF_Dictionary* pParentResources,
                           CFX_Matrix* pmtContentToUser,
                           CPDF_PageObjects* pObjList,
                           CPDF_Dictionary* pResources,
                           CFX_FloatRect* pBBox,
                           CPDF_ParseOptions* pOptions,
                           CPDF_AllStates* pAllStates,
                           int level);
  ~CPDF_StreamContentParser();

  CPDF_PageObjects* GetObjectList() const { return m_pObjectList; }
  CPDF_AllStates* GetCurStates() const { return m_pCurStates.get(); }
  FX_BOOL IsColored() const { return m_bColored; }
  const FX_FLOAT* GetType3Data() const { return m_Type3Data; }

  void AddNumberParam(const FX_CHAR* str, int len);
  void AddObjectParam(CPDF_Object* pObj);
  void AddNameParam(const FX_CHAR* name, int size);
  int GetNextParamPos();
  void ClearAllParams();
  CPDF_Object* GetObject(FX_DWORD index);
  CFX_ByteString GetString(FX_DWORD index);
  FX_FLOAT GetNumber(FX_DWORD index);
  FX_FLOAT GetNumber16(FX_DWORD index);
  int GetInteger(FX_DWORD index) { return (int32_t)(GetNumber(index)); }
  FX_BOOL OnOperator(const FX_CHAR* op);
  void BigCaseCaller(int index);
  FX_DWORD GetParsePos() { return m_pSyntax->GetPos(); }
  void AddTextObject(CFX_ByteString* pText,
                     FX_FLOAT fInitKerning,
                     FX_FLOAT* pKerning,
                     int count);

  void ConvertUserSpace(FX_FLOAT& x, FX_FLOAT& y);
  void ConvertTextSpace(FX_FLOAT& x, FX_FLOAT& y);
  void OnChangeTextMatrix();
  FX_DWORD Parse(const uint8_t* pData, FX_DWORD dwSize, FX_DWORD max_cost);
  void ParsePathObject();
  void AddPathPoint(FX_FLOAT x, FX_FLOAT y, int flag);
  void AddPathRect(FX_FLOAT x, FX_FLOAT y, FX_FLOAT w, FX_FLOAT h);
  void AddPathObject(int FillType, FX_BOOL bStroke);
  CPDF_ImageObject* AddImage(CPDF_Stream* pStream,
                             CPDF_Image* pImage,
                             FX_BOOL bInline);
  void AddDuplicateImage();
  void AddForm(CPDF_Stream*);
  void SetGraphicStates(CPDF_PageObject* pObj,
                        FX_BOOL bColor,
                        FX_BOOL bText,
                        FX_BOOL bGraph);
  void SaveStates(CPDF_AllStates*);
  void RestoreStates(CPDF_AllStates*);
  CPDF_Font* FindFont(const CFX_ByteString& name);
  CPDF_ColorSpace* FindColorSpace(const CFX_ByteString& name);
  CPDF_Pattern* FindPattern(const CFX_ByteString& name, FX_BOOL bShading);
  CPDF_Object* FindResourceObj(const CFX_ByteStringC& type,
                               const CFX_ByteString& name);

 protected:
  struct OpCode {
    FX_DWORD m_OpId;
    void (CPDF_StreamContentParser::*m_OpHandler)();
  };
  static const OpCode g_OpCodes[];

  void Handle_CloseFillStrokePath();
  void Handle_FillStrokePath();
  void Handle_CloseEOFillStrokePath();
  void Handle_EOFillStrokePath();
  void Handle_BeginMarkedContent_Dictionary();
  void Handle_BeginImage();
  void Handle_BeginMarkedContent();
  void Handle_BeginText();
  void Handle_BeginSectionUndefined();
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
  void Handle_EndSectionUndefined();
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

  CPDF_Document* const m_pDocument;
  CPDF_Dictionary* m_pPageResources;
  CPDF_Dictionary* m_pParentResources;
  CPDF_Dictionary* m_pResources;
  CPDF_PageObjects* m_pObjectList;
  int m_Level;
  CFX_Matrix m_mtContentToUser;
  CFX_FloatRect m_BBox;
  CPDF_ParseOptions m_Options;
  ContentParam m_ParamBuf1[PARAM_BUF_SIZE];
  FX_DWORD m_ParamStartPos;
  FX_DWORD m_ParamCount;
  CPDF_StreamParser* m_pSyntax;
  std::unique_ptr<CPDF_AllStates> m_pCurStates;
  CPDF_ContentMark m_CurContentMark;
  CFX_ArrayTemplate<CPDF_TextObject*> m_ClipTextList;
  CPDF_TextObject* m_pLastTextObject;
  FX_FLOAT m_DefFontSize;
  int m_CompatCount;
  FX_PATHPOINT* m_pPathPoints;
  int m_PathPointCount;
  int m_PathAllocSize;
  FX_FLOAT m_PathStartX;
  FX_FLOAT m_PathStartY;
  FX_FLOAT m_PathCurrentX;
  FX_FLOAT m_PathCurrentY;
  int m_PathClipType;
  CFX_ByteString m_LastImageName;
  CPDF_Image* m_pLastImage;
  CFX_BinaryBuf m_LastImageDict;
  CFX_BinaryBuf m_LastImageData;
  CPDF_Dictionary* m_pLastImageDict;
  CPDF_Dictionary* m_pLastCloneImageDict;
  FX_BOOL m_bReleaseLastDict;
  FX_BOOL m_bSameLastDict;
  FX_BOOL m_bColored;
  FX_FLOAT m_Type3Data[6];
  FX_BOOL m_bResourceMissing;
  std::vector<std::unique_ptr<CPDF_AllStates>> m_StateStack;
};
class CPDF_ContentParser {
 public:
  enum ParseStatus { Ready, ToBeContinued, Done };

  CPDF_ContentParser();
  ~CPDF_ContentParser();

  ParseStatus GetStatus() const { return m_Status; }
  void Start(CPDF_Page* pPage, CPDF_ParseOptions* pOptions);
  void Start(CPDF_Form* pForm,
             CPDF_AllStates* pGraphicStates,
             CFX_Matrix* pParentMatrix,
             CPDF_Type3Char* pType3Char,
             CPDF_ParseOptions* pOptions,
             int level);
  void Continue(IFX_Pause* pPause);

 private:
  enum InternalStage {
    STAGE_GETCONTENT = 1,
    STAGE_PARSE,
    STAGE_CHECKCLIP,
  };

  ParseStatus m_Status;
  InternalStage m_InternalStage;
  CPDF_PageObjects* m_pObjects;
  FX_BOOL m_bForm;
  CPDF_ParseOptions m_Options;
  CPDF_Type3Char* m_pType3Char;
  FX_DWORD m_nStreams;
  std::unique_ptr<CPDF_StreamAcc> m_pSingleStream;
  std::vector<std::unique_ptr<CPDF_StreamAcc>> m_StreamArray;
  uint8_t* m_pData;
  FX_DWORD m_Size;
  FX_DWORD m_CurrentOffset;
  std::unique_ptr<CPDF_StreamContentParser> m_pParser;
};
class CPDF_AllStates : public CPDF_GraphicStates {
 public:
  CPDF_AllStates();
  ~CPDF_AllStates();
  void Copy(const CPDF_AllStates& src);
  void ProcessExtGS(CPDF_Dictionary* pGS, CPDF_StreamContentParser* pParser);
  void SetLineDash(CPDF_Array*, FX_FLOAT, FX_FLOAT scale);
  CFX_Matrix m_TextMatrix, m_CTM, m_ParentMatrix;
  FX_FLOAT m_TextX, m_TextY, m_TextLineX, m_TextLineY;
  FX_FLOAT m_TextLeading, m_TextRise, m_TextHorzScale;
};

class CPDF_DocPageData {
 public:
  explicit CPDF_DocPageData(CPDF_Document* pPDFDoc);
  ~CPDF_DocPageData();

  void Clear(FX_BOOL bRelease = FALSE);
  CPDF_Font* GetFont(CPDF_Dictionary* pFontDict, FX_BOOL findOnly);
  CPDF_Font* GetStandardFont(const CFX_ByteStringC& fontName,
                             CPDF_FontEncoding* pEncoding);
  void ReleaseFont(CPDF_Dictionary* pFontDict);
  CPDF_ColorSpace* GetColorSpace(CPDF_Object* pCSObj,
                                 const CPDF_Dictionary* pResources);
  CPDF_ColorSpace* GetCopiedColorSpace(CPDF_Object* pCSObj);
  void ReleaseColorSpace(CPDF_Object* pColorSpace);
  CPDF_Pattern* GetPattern(CPDF_Object* pPatternObj,
                           FX_BOOL bShading,
                           const CFX_Matrix* matrix);
  void ReleasePattern(CPDF_Object* pPatternObj);
  CPDF_Image* GetImage(CPDF_Object* pImageStream);
  void ReleaseImage(CPDF_Object* pImageStream);
  CPDF_IccProfile* GetIccProfile(CPDF_Stream* pIccProfileStream);
  void ReleaseIccProfile(CPDF_IccProfile* pIccProfile);
  CPDF_StreamAcc* GetFontFileStreamAcc(CPDF_Stream* pFontStream);
  void ReleaseFontFileStreamAcc(CPDF_Stream* pFontStream,
                                FX_BOOL bForce = FALSE);
  FX_BOOL IsForceClear() const { return m_bForceClear; }
  CPDF_CountedColorSpace* FindColorSpacePtr(CPDF_Object* pCSObj) const;
  CPDF_CountedPattern* FindPatternPtr(CPDF_Object* pPatternObj) const;

 private:
  using CPDF_CountedFont = CPDF_CountedObject<CPDF_Font>;
  using CPDF_CountedIccProfile = CPDF_CountedObject<CPDF_IccProfile>;
  using CPDF_CountedImage = CPDF_CountedObject<CPDF_Image>;
  using CPDF_CountedStreamAcc = CPDF_CountedObject<CPDF_StreamAcc>;

  using CPDF_ColorSpaceMap = std::map<CPDF_Object*, CPDF_CountedColorSpace*>;
  using CPDF_FontFileMap = std::map<CPDF_Stream*, CPDF_CountedStreamAcc*>;
  using CPDF_FontMap = std::map<CPDF_Dictionary*, CPDF_CountedFont*>;
  using CPDF_IccProfileMap = std::map<CPDF_Stream*, CPDF_CountedIccProfile*>;
  using CPDF_ImageMap = std::map<FX_DWORD, CPDF_CountedImage*>;
  using CPDF_PatternMap = std::map<CPDF_Object*, CPDF_CountedPattern*>;

  CPDF_Document* const m_pPDFDoc;
  FX_BOOL m_bForceClear;
  std::map<CFX_ByteString, CPDF_Stream*> m_HashProfileMap;
  CPDF_ColorSpaceMap m_ColorSpaceMap;
  CPDF_FontFileMap m_FontFileMap;
  CPDF_FontMap m_FontMap;
  CPDF_IccProfileMap m_IccProfileMap;
  CPDF_ImageMap m_ImageMap;
  CPDF_PatternMap m_PatternMap;
};

class CPDF_Function {
 public:
  static CPDF_Function* Load(CPDF_Object* pFuncObj);
  virtual ~CPDF_Function();
  FX_BOOL Call(FX_FLOAT* inputs,
               int ninputs,
               FX_FLOAT* results,
               int& nresults) const;
  int CountInputs() { return m_nInputs; }
  int CountOutputs() { return m_nOutputs; }

 protected:
  CPDF_Function();
  int m_nInputs, m_nOutputs;
  FX_FLOAT* m_pDomains;
  FX_FLOAT* m_pRanges;
  FX_BOOL Init(CPDF_Object* pObj);
  virtual FX_BOOL v_Init(CPDF_Object* pObj) = 0;
  virtual FX_BOOL v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const = 0;
};
class CPDF_IccProfile {
 public:
  CPDF_IccProfile(const uint8_t* pData, FX_DWORD dwSize);
  ~CPDF_IccProfile();
  int32_t GetComponents() const { return m_nSrcComponents; }
  FX_BOOL m_bsRGB;
  void* m_pTransform;

 private:
  int32_t m_nSrcComponents;
};

class CPDF_DeviceCS : public CPDF_ColorSpace {
 public:
  CPDF_DeviceCS(CPDF_Document* pDoc, int family);

  FX_BOOL GetRGB(FX_FLOAT* pBuf,
                 FX_FLOAT& R,
                 FX_FLOAT& G,
                 FX_FLOAT& B) const override;
  FX_BOOL SetRGB(FX_FLOAT* pBuf,
                 FX_FLOAT R,
                 FX_FLOAT G,
                 FX_FLOAT B) const override;
  FX_BOOL v_GetCMYK(FX_FLOAT* pBuf,
                    FX_FLOAT& c,
                    FX_FLOAT& m,
                    FX_FLOAT& y,
                    FX_FLOAT& k) const override;
  FX_BOOL v_SetCMYK(FX_FLOAT* pBuf,
                    FX_FLOAT c,
                    FX_FLOAT m,
                    FX_FLOAT y,
                    FX_FLOAT k) const override;
  void TranslateImageLine(uint8_t* pDestBuf,
                          const uint8_t* pSrcBuf,
                          int pixels,
                          int image_width,
                          int image_height,
                          FX_BOOL bTransMask = FALSE) const override;
};

class CPDF_PatternCS : public CPDF_ColorSpace {
 public:
  explicit CPDF_PatternCS(CPDF_Document* pDoc)
      : CPDF_ColorSpace(pDoc, PDFCS_PATTERN, 1),
        m_pBaseCS(nullptr),
        m_pCountedBaseCS(nullptr) {}
  ~CPDF_PatternCS() override;
  FX_BOOL v_Load(CPDF_Document* pDoc, CPDF_Array* pArray) override;
  FX_BOOL GetRGB(FX_FLOAT* pBuf,
                 FX_FLOAT& R,
                 FX_FLOAT& G,
                 FX_FLOAT& B) const override;
  CPDF_ColorSpace* GetBaseCS() const override;

 private:
  CPDF_ColorSpace* m_pBaseCS;
  CPDF_CountedColorSpace* m_pCountedBaseCS;
};

void PDF_ReplaceAbbr(CPDF_Object* pObj);

#endif  // CORE_SRC_FPDFAPI_FPDF_PAGE_PAGEINT_H_
