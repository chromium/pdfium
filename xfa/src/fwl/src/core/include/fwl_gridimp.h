// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_GRID_IMP_H
#define _FWL_GRID_IMP_H
class CFWL_Content;
class IFWL_Widget;
class CFWL_GridLength;
class CFWL_GridColRow;
class CFWL_GridWidgetInfo;
class CFWL_GridImp;
class CFWL_GridLength {
 public:
  CFWL_GridLength() : fLength(0), eUnit(FWL_GRIDUNIT_Fixed) {}
  CFWL_GridLength(FX_FLOAT fValue, FWL_GRIDUNIT e)
      : fLength(fValue), eUnit(e) {}
  FX_FLOAT fLength;
  FWL_GRIDUNIT eUnit;
};
class CFWL_GridColRow {
 public:
  CFWL_GridColRow()
      : m_Size(1, FWL_GRIDUNIT_Scaled),
        m_MinSize(0, FWL_GRIDUNIT_Fixed),
        m_MaxSize(0, FWL_GRIDUNIT_Infinity),
        m_fActualSize(0),
        m_fActualPos(0) {}
  CFWL_GridLength m_Size;
  CFWL_GridLength m_MinSize;
  CFWL_GridLength m_MaxSize;
  FX_FLOAT m_fActualSize;
  FX_FLOAT m_fActualPos;
};
class CFWL_GridWidgetInfo {
 public:
  CFWL_GridWidgetInfo()
      : m_iColumn(0),
        m_iColumnSpan(1),
        m_iRow(0),
        m_iRowSpan(1),
        m_dwMarginFlag(0),
        m_fActualWidth(0),
        m_fActualHeight(0) {
    m_Size[FWL_GRIDSIZE_Width].eUnit = FWL_GRIDUNIT_Auto;
    m_Size[FWL_GRIDSIZE_Width].fLength = 0;
    m_Size[FWL_GRIDSIZE_Height].eUnit = FWL_GRIDUNIT_Auto;
    m_Size[FWL_GRIDSIZE_Height].fLength = 0;
    m_Size[FWL_GRIDSIZE_MinWidth].eUnit = FWL_GRIDUNIT_Fixed;
    m_Size[FWL_GRIDSIZE_MinWidth].fLength = 0;
    m_Size[FWL_GRIDSIZE_MaxWidth].eUnit = FWL_GRIDUNIT_Infinity;
    m_Size[FWL_GRIDSIZE_MaxWidth].fLength = 0;
    m_Size[FWL_GRIDSIZE_MinHeight].eUnit = FWL_GRIDUNIT_Fixed;
    m_Size[FWL_GRIDSIZE_MinHeight].fLength = 0;
    m_Size[FWL_GRIDSIZE_MaxHeight].eUnit = FWL_GRIDUNIT_Infinity;
    m_Size[FWL_GRIDSIZE_MaxHeight].fLength = 0;
    m_Margin[0] = m_Margin[1] = m_Margin[2] = m_Margin[3] = 0;
  }
  int32_t m_iColumn;
  int32_t m_iColumnSpan;
  int32_t m_iRow;
  int32_t m_iRowSpan;
  CFWL_GridLength m_Size[6];
  FX_DWORD m_dwMarginFlag;
  FX_FLOAT m_Margin[4];
  FX_FLOAT m_fActualWidth;
  FX_FLOAT m_fActualHeight;
};
class CFWL_GridImp : public CFWL_ContentImp {
 public:
  CFWL_GridImp(const CFWL_WidgetImpProperties& properties, IFWL_Widget* pOuter);
  virtual ~CFWL_GridImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR SetWidgetRect(const CFX_RectF& rect);
  virtual FWL_ERR Update();
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);

  virtual FWL_ERR InsertWidget(IFWL_Widget* pChild, int32_t nIndex = -1);
  virtual FWL_ERR RemoveWidget(IFWL_Widget* pWidget);
  virtual FWL_HGRIDCOLROW InsertColRow(FX_BOOL bColumn, int32_t nIndex = -1);
  virtual int32_t CountColRows(FX_BOOL bColumn);
  virtual FWL_HGRIDCOLROW GetColRow(FX_BOOL bColumn, int32_t nIndex);
  virtual int32_t GetIndex(FWL_HGRIDCOLROW hColRow);
  virtual FX_FLOAT GetSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit);
  virtual FWL_ERR SetSize(FWL_HGRIDCOLROW hColRow,
                          FX_FLOAT fSize,
                          FWL_GRIDUNIT eUnit);
  FX_FLOAT GetMinSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit);
  FWL_ERR SetMinSize(FWL_HGRIDCOLROW hColRow,
                     FX_FLOAT fSize,
                     FWL_GRIDUNIT eUnit);
  FX_FLOAT GetMaxSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit);
  FWL_ERR SetMaxSize(FWL_HGRIDCOLROW hColRow,
                     FX_FLOAT fSize,
                     FWL_GRIDUNIT eUnit);
  virtual FX_BOOL DeleteColRow(FWL_HGRIDCOLROW hColRow);
  virtual FX_BOOL IsColumn(FWL_HGRIDCOLROW hColRow);
  virtual int32_t GetWidgetPos(IFWL_Widget* pWidget, FX_BOOL bColumn);
  virtual FWL_ERR SetWidgetPos(IFWL_Widget* pWidget,
                               int32_t iPos,
                               FX_BOOL bColumn);
  virtual int32_t GetWidgetSpan(IFWL_Widget* pWidget, FX_BOOL bColumn);
  virtual FWL_ERR SetWidgetSpan(IFWL_Widget* pWidget,
                                int32_t iSpan,
                                FX_BOOL bColumn);
  virtual FX_FLOAT GetWidgetSize(IFWL_Widget* pWidget,
                                 FWL_GRIDSIZE eSize,
                                 FWL_GRIDUNIT& eUnit);
  virtual FWL_ERR SetWidgetSize(IFWL_Widget* pWidget,
                                FWL_GRIDSIZE eSize,
                                FX_FLOAT fSize,
                                FWL_GRIDUNIT eUit);
  virtual FX_BOOL GetWidgetMargin(IFWL_Widget* pWidget,
                                  FWL_GRIDMARGIN eMargin,
                                  FX_FLOAT& fMargin);
  virtual FWL_ERR SetWidgetMargin(IFWL_Widget* pWidget,
                                  FWL_GRIDMARGIN eMargin,
                                  FX_FLOAT fMargin);
  virtual FWL_ERR RemoveWidgetMargin(IFWL_Widget* pWidget,
                                     FWL_GRIDMARGIN eMargin);
  virtual FX_FLOAT GetGridSize(FWL_GRIDSIZE eSize, FWL_GRIDUNIT& eUnit);
  virtual FWL_ERR SetGridSize(FWL_GRIDSIZE eSize,
                              FX_FLOAT fSize,
                              FWL_GRIDUNIT eUit);

 protected:
  CFWL_GridWidgetInfo* GetWidgetInfo(IFWL_Widget* pWidget);
  void ProcFixedColRow(CFWL_GridColRow* pColRow,
                       int32_t nIndex,
                       FX_FLOAT fColRowSize,
                       FX_BOOL bColumn);
  void ProcAutoColRow(CFWL_GridColRow* pColRow,
                      int32_t nIndex,
                      FX_BOOL bColumn);
  void ProcScaledColRow(CFWL_GridColRow* pColRow,
                        int32_t nIndex,
                        FX_FLOAT fColRowSize,
                        FX_BOOL bColumn);
  void CalcWidgetWidth(IFWL_Widget* pWidget,
                       CFWL_GridWidgetInfo* pInfo,
                       FX_FLOAT fColunmWidth);
  void CalcWidgetHeigt(IFWL_Widget* pWidget,
                       CFWL_GridWidgetInfo* pInfo,
                       FX_FLOAT fRowHeigt);
  FX_FLOAT CalcAutoColumnWidgetWidth(IFWL_Widget* pWidget,
                                     CFWL_GridWidgetInfo* pInfo);
  FX_FLOAT CalcAutoColumnWidgetHeight(IFWL_Widget* pWidget,
                                      CFWL_GridWidgetInfo* pInfo);
  FX_FLOAT ProcessColumns(FX_FLOAT fWidth);
  FX_FLOAT ProcessRows(FX_FLOAT fHeight);
  FX_FLOAT ProcessUnCertainColumns();
  FX_FLOAT ProcessUnCertainRows();
  FX_BOOL SetColRowActualSize(CFWL_GridColRow* pColRow,
                              FX_FLOAT fSize,
                              FX_BOOL bSetBeyond = FALSE);
  FX_FLOAT SetWidgetActualWidth(CFWL_GridWidgetInfo* pInfo, FX_FLOAT fWidth);
  FX_FLOAT SetWidgetActualHeight(CFWL_GridWidgetInfo* pInfo, FX_FLOAT fHeight);
  void SetAllWidgetsRect();
  FX_BOOL IsGrid(IFWL_Widget* pWidget);
  void SetSpanAutoColRowSize(const CFX_PtrArray& spanAutos,
                             FX_FLOAT fTotalSize);
  void SetSpanScaledColRowSize(const CFX_PtrArray& spanScaleds,
                               FX_FLOAT fTotalSize,
                               FX_FLOAT fTotalScaledNum);
  void SetScaledColRowsSize(const CFX_PtrArray& spanScaleds,
                            FX_FLOAT fTotalSize,
                            FX_FLOAT fTotalScaledNum);
  CFX_PtrArray m_Rows;
  CFX_PtrArray m_Columns;
  CFX_PtrArray m_Widgets;
  CFX_MapPtrToPtr m_mapWidgetInfo;
  CFWL_GridLength m_Size[6];
  friend class CFWL_GridImpDelegate;
};
class CFWL_GridImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_GridImpDelegate(CFWL_GridImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  CFWL_GridImp* m_pOwner;
};
#endif
