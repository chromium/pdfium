// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_GRID_H
#define _FWL_GRID_H
class IFWL_Widget;
class IFWL_Content;
#define FWL_CLASS_Grid L"FWL_GRID"
#define FWL_CLASSHASH_Grid 3150298670
#define FWL_GRIDSTYLEEXT_ShowGridLines (1L << 0)
struct FWL_LAYOUTDATA {
  FX_FLOAT fWidth;
  FX_FLOAT fHeight;
};
enum FWL_GRIDUNIT {
  FWL_GRIDUNIT_Auto = 0,
  FWL_GRIDUNIT_Fixed,
  FWL_GRIDUNIT_Scaled,
  FWL_GRIDUNIT_Infinity,
};
enum FWL_GRIDMARGIN {
  FWL_GRIDMARGIN_Left = 0,
  FWL_GRIDMARGIN_Top,
  FWL_GRIDMARGIN_Right,
  FWL_GRIDMARGIN_Bottom,
};
enum FWL_GRIDSIZE {
  FWL_GRIDSIZE_Width = 0,
  FWL_GRIDSIZE_Height,
  FWL_GRIDSIZE_MinWidth,
  FWL_GRIDSIZE_MinHeight,
  FWL_GRIDSIZE_MaxWidth,
  FWL_GRIDSIZE_MaxHeight,
};
typedef struct _FWL_HGRIDCOLROW { void* pData; } * FWL_HGRIDCOLROW;
class IFWL_Grid : public IFWL_Content {
 public:
  static IFWL_Grid* Create(const CFWL_WidgetImpProperties& properties);

  FWL_HGRIDCOLROW InsertColRow(FX_BOOL bColumn, int32_t nIndex = -1);
  int32_t CountColRows(FX_BOOL bColumn);
  FWL_HGRIDCOLROW GetColRow(FX_BOOL bColumn, int32_t nIndex);
  int32_t GetIndex(FWL_HGRIDCOLROW hColRow);
  FX_FLOAT GetSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit);
  FWL_ERR SetSize(FWL_HGRIDCOLROW hColRow, FX_FLOAT fSize, FWL_GRIDUNIT eUnit);
  FX_FLOAT GetMinSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit);
  FWL_ERR SetMinSize(FWL_HGRIDCOLROW hColRow,
                     FX_FLOAT fSize,
                     FWL_GRIDUNIT eUnit);
  FX_FLOAT GetMaxSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit);
  FWL_ERR SetMaxSize(FWL_HGRIDCOLROW hColRow,
                     FX_FLOAT fSize,
                     FWL_GRIDUNIT eUnit);
  FX_BOOL DeleteColRow(FWL_HGRIDCOLROW hColRow);
  FX_BOOL IsColumn(FWL_HGRIDCOLROW hColRow);
  int32_t GetWidgetPos(IFWL_Widget* pWidget, FX_BOOL bColumn);
  FWL_ERR SetWidgetPos(IFWL_Widget* pWidget, int32_t iPos, FX_BOOL bColumn);
  int32_t GetWidgetSpan(IFWL_Widget* pWidget, FX_BOOL bColumn);
  FWL_ERR SetWidgetSpan(IFWL_Widget* pWidget, int32_t iSpan, FX_BOOL bColumn);
  FX_FLOAT GetWidgetSize(IFWL_Widget* pWidget,
                         FWL_GRIDSIZE eSize,
                         FWL_GRIDUNIT& eUnit);
  FWL_ERR SetWidgetSize(IFWL_Widget* pWidget,
                        FWL_GRIDSIZE eSize,
                        FX_FLOAT fSize,
                        FWL_GRIDUNIT eUit);
  FX_BOOL GetWidgetMargin(IFWL_Widget* pWidget,
                          FWL_GRIDMARGIN eMargin,
                          FX_FLOAT& fMargin);
  FWL_ERR SetWidgetMargin(IFWL_Widget* pWidget,
                          FWL_GRIDMARGIN eMargin,
                          FX_FLOAT fMargin);
  FWL_ERR RemoveWidgetMargin(IFWL_Widget* pWidget, FWL_GRIDMARGIN eMargin);
  FX_FLOAT GetGridSize(FWL_GRIDSIZE eSize, FWL_GRIDUNIT& eUnit);
  FWL_ERR SetGridSize(FWL_GRIDSIZE eSize, FX_FLOAT fSize, FWL_GRIDUNIT eUit);

 protected:
  IFWL_Grid();
};
#endif
