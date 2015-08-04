// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_PARSER
#define _FDE_PARSER
enum FDE_VISUALOBJTYPE {
  FDE_VISUALOBJ_Canvas = 0x00,
  FDE_VISUALOBJ_Text = 0x01,
  FDE_VISUALOBJ_Image = 0x02,
  FDE_VISUALOBJ_Path = 0x04,
  FDE_VISUALOBJ_Widget = 0x08,
};
typedef struct _FDE_HVISUALOBJ { void* pData; } const* FDE_HVISUALOBJ;
class IFDE_VisualSet {
 public:
  virtual ~IFDE_VisualSet() {}
  virtual FDE_VISUALOBJTYPE GetType() = 0;
  virtual FX_BOOL GetBBox(FDE_HVISUALOBJ hVisualObj, CFX_RectF& bbox) = 0;
  virtual FX_BOOL GetMatrix(FDE_HVISUALOBJ hVisualObj, CFX_Matrix& matrix) = 0;
  virtual FX_BOOL GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) = 0;
  virtual FX_BOOL GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) = 0;
};
class IFDE_CanvasSet : public IFDE_VisualSet {
 public:
  virtual FX_POSITION GetFirstPosition(FDE_HVISUALOBJ hCanvas) = 0;
  virtual FDE_HVISUALOBJ GetNext(FDE_HVISUALOBJ hCanvas,
                                 FX_POSITION& pos,
                                 IFDE_VisualSet*& pVisualSet) = 0;
  virtual FDE_HVISUALOBJ GetParentCanvas(FDE_HVISUALOBJ hCanvas,
                                         IFDE_VisualSet*& pVisualSet) = 0;
};
class IFDE_TextSet : public IFDE_VisualSet {
 public:
  virtual int32_t GetString(FDE_HVISUALOBJ hText, CFX_WideString& wsText) = 0;
  virtual IFX_Font* GetFont(FDE_HVISUALOBJ hText) = 0;
  virtual FX_FLOAT GetFontSize(FDE_HVISUALOBJ hText) = 0;
  virtual FX_ARGB GetFontColor(FDE_HVISUALOBJ hText) = 0;
  virtual int32_t GetDisplayPos(FDE_HVISUALOBJ hText,
                                FXTEXT_CHARPOS* pCharPos,
                                FX_BOOL bCharCode = FALSE,
                                CFX_WideString* pWSForms = NULL) = 0;
  virtual int32_t GetCharRects(FDE_HVISUALOBJ hText,
                               CFX_RectFArray& rtArray) = 0;
};
class IFDE_ImageSet : public IFDE_VisualSet {
 public:
  virtual IFDE_Image* GetImage(FDE_HVISUALOBJ hImage) = 0;
  virtual FX_POSITION GetFirstFilterPosition(FDE_HVISUALOBJ hImage) = 0;
  virtual FDE_LPCIMAGEFILTERPARAMS GetNextFilter(FDE_HVISUALOBJ hImage,
                                                 FX_POSITION& pos) = 0;
};
#define FDE_FILLMODE_Alternate 1
#define FDE_FILLMODE_Winding 2
#define FDE_PATHRENDER_Stroke 1
#define FDE_PATHRENDER_Fill 2
#define FDE_PATHRENDER_FillStroke 3
class IFDE_PathSet : public IFDE_VisualSet {
 public:
  virtual IFDE_Path* GetPath(FDE_HVISUALOBJ hPath) = 0;
  virtual int32_t GetFillMode(FDE_HVISUALOBJ hPath) = 0;
  virtual int32_t GetRenderMode(FDE_HVISUALOBJ hPath) = 0;
  virtual IFDE_Pen* GetPen(FDE_HVISUALOBJ hPath) = 0;
  virtual FX_FLOAT GetPenWidth(FDE_HVISUALOBJ hPath) = 0;
  virtual IFDE_Brush* GetBrush(FDE_HVISUALOBJ hPath) = 0;
};
enum FDE_WIDGETOBJ {
  FDE_WIDGETOBJ_Unknown = 0x0000,
  FDE_WIDGETOBJ_Anchor = 0x0100,
  FDE_WIDGETOBJ_NamedDest = 0x0200,
  FDE_WIDGETOBJ_HyperLink = 0x0400,
};
#define FDE_WIDGETPARAM_Uri 1
#define FDE_WIDGETPARAM_Rects 2
class IFDE_WidgetSet : public IFDE_VisualSet {
 public:
  virtual FDE_WIDGETOBJ GetWidgetType(FDE_HVISUALOBJ hWidget) = 0;
  virtual FX_FLOAT GetFloat(FDE_HVISUALOBJ hWidget,
                            int32_t iParameter,
                            FX_FLOAT fDefVal = 0.0f) = 0;
  virtual int32_t GetInteger(FDE_HVISUALOBJ hWidget,
                             int32_t iParameter,
                             int32_t iDefVal = 0) = 0;
  virtual FX_BOOL GetString(FDE_HVISUALOBJ hWidget,
                            int32_t iParameter,
                            CFX_WideString& wsValue) = 0;
  virtual FX_BOOL GetRects(FDE_HVISUALOBJ hWidget,
                           int32_t iParameter,
                           CFX_RectFArray& rects) = 0;
};
class IFDE_VisualSetIterator {
 public:
  static IFDE_VisualSetIterator* Create();
  virtual ~IFDE_VisualSetIterator() {}
  virtual void Release() = 0;
  virtual FX_BOOL AttachCanvas(IFDE_CanvasSet* pCanvas) = 0;
  virtual FX_BOOL FilterObjects(FX_DWORD dwObjects = 0xFFFFFFFF) = 0;
  virtual void Reset() = 0;
  virtual FDE_HVISUALOBJ GetNext(IFDE_VisualSet*& pVisualSet,
                                 FDE_HVISUALOBJ* phCanvasObj = NULL,
                                 IFDE_CanvasSet** ppCanvasSet = NULL) = 0;
};
#endif
