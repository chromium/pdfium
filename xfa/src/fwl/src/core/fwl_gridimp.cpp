// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fwl/src/core/include/fwl_targetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_threadimp.h"
#include "xfa/src/fwl/src/core/include/fwl_noteimp.h"
#include "xfa/src/fwl/src/core/include/fwl_widgetimp.h"
#include "xfa/src/fwl/src/core/include/fwl_contentimp.h"
#include "xfa/src/fwl/src/core/include/fwl_gridimp.h"

// static
IFWL_Grid* IFWL_Grid::Create(const CFWL_WidgetImpProperties& properties) {
  IFWL_Grid* pGrid = new IFWL_Grid;
  CFWL_GridImp* pGridImpl = new CFWL_GridImp(properties, nullptr);
  pGrid->SetImpl(pGridImpl);
  pGridImpl->SetInterface(pGrid);
  return pGrid;
}
IFWL_Grid::IFWL_Grid() {}
FWL_HGRIDCOLROW IFWL_Grid::InsertColRow(FX_BOOL bColumn, int32_t nIndex) {
  return static_cast<CFWL_GridImp*>(GetImpl())->InsertColRow(bColumn, nIndex);
}
int32_t IFWL_Grid::CountColRows(FX_BOOL bColumn) {
  return static_cast<CFWL_GridImp*>(GetImpl())->CountColRows(bColumn);
}
FWL_HGRIDCOLROW IFWL_Grid::GetColRow(FX_BOOL bColumn, int32_t nIndex) {
  return static_cast<CFWL_GridImp*>(GetImpl())->GetColRow(bColumn, nIndex);
}
int32_t IFWL_Grid::GetIndex(FWL_HGRIDCOLROW hColRow) {
  return static_cast<CFWL_GridImp*>(GetImpl())->GetIndex(hColRow);
}
FX_FLOAT IFWL_Grid::GetSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit) {
  return static_cast<CFWL_GridImp*>(GetImpl())->GetSize(hColRow, eUnit);
}
FWL_ERR IFWL_Grid::SetSize(FWL_HGRIDCOLROW hColRow,
                           FX_FLOAT fSize,
                           FWL_GRIDUNIT eUnit) {
  return static_cast<CFWL_GridImp*>(GetImpl())->SetSize(hColRow, fSize, eUnit);
}
FX_FLOAT IFWL_Grid::GetMinSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit) {
  return static_cast<CFWL_GridImp*>(GetImpl())->GetMinSize(hColRow, eUnit);
}
FWL_ERR IFWL_Grid::SetMinSize(FWL_HGRIDCOLROW hColRow,
                              FX_FLOAT fSize,
                              FWL_GRIDUNIT eUnit) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->SetMinSize(hColRow, fSize, eUnit);
}
FX_FLOAT IFWL_Grid::GetMaxSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit) {
  return static_cast<CFWL_GridImp*>(GetImpl())->GetMaxSize(hColRow, eUnit);
}
FWL_ERR IFWL_Grid::SetMaxSize(FWL_HGRIDCOLROW hColRow,
                              FX_FLOAT fSize,
                              FWL_GRIDUNIT eUnit) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->SetMaxSize(hColRow, fSize, eUnit);
}
FX_BOOL IFWL_Grid::DeleteColRow(FWL_HGRIDCOLROW hColRow) {
  return static_cast<CFWL_GridImp*>(GetImpl())->DeleteColRow(hColRow);
}
FX_BOOL IFWL_Grid::IsColumn(FWL_HGRIDCOLROW hColRow) {
  return static_cast<CFWL_GridImp*>(GetImpl())->IsColumn(hColRow);
}
int32_t IFWL_Grid::GetWidgetPos(IFWL_Widget* pWidget, FX_BOOL bColumn) {
  return static_cast<CFWL_GridImp*>(GetImpl())->GetWidgetPos(pWidget, bColumn);
}
FWL_ERR IFWL_Grid::SetWidgetPos(IFWL_Widget* pWidget,
                                int32_t iPos,
                                FX_BOOL bColumn) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->SetWidgetPos(pWidget, iPos, bColumn);
}
int32_t IFWL_Grid::GetWidgetSpan(IFWL_Widget* pWidget, FX_BOOL bColumn) {
  return static_cast<CFWL_GridImp*>(GetImpl())->GetWidgetSpan(pWidget, bColumn);
}
FWL_ERR IFWL_Grid::SetWidgetSpan(IFWL_Widget* pWidget,
                                 int32_t iSpan,
                                 FX_BOOL bColumn) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->SetWidgetSpan(pWidget, iSpan, bColumn);
}
FX_FLOAT IFWL_Grid::GetWidgetSize(IFWL_Widget* pWidget,
                                  FWL_GRIDSIZE eSize,
                                  FWL_GRIDUNIT& eUnit) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->GetWidgetSize(pWidget, eSize, eUnit);
}
FWL_ERR IFWL_Grid::SetWidgetSize(IFWL_Widget* pWidget,
                                 FWL_GRIDSIZE eSize,
                                 FX_FLOAT fSize,
                                 FWL_GRIDUNIT eUit) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->SetWidgetSize(pWidget, eSize, fSize, eUit);
}
FX_BOOL IFWL_Grid::GetWidgetMargin(IFWL_Widget* pWidget,
                                   FWL_GRIDMARGIN eMargin,
                                   FX_FLOAT& fMargin) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->GetWidgetMargin(pWidget, eMargin, fMargin);
}
FWL_ERR IFWL_Grid::SetWidgetMargin(IFWL_Widget* pWidget,
                                   FWL_GRIDMARGIN eMargin,
                                   FX_FLOAT fMargin) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->SetWidgetMargin(pWidget, eMargin, fMargin);
}
FWL_ERR IFWL_Grid::RemoveWidgetMargin(IFWL_Widget* pWidget,
                                      FWL_GRIDMARGIN eMargin) {
  return static_cast<CFWL_GridImp*>(GetImpl())
      ->RemoveWidgetMargin(pWidget, eMargin);
}
FX_FLOAT IFWL_Grid::GetGridSize(FWL_GRIDSIZE eSize, FWL_GRIDUNIT& eUnit) {
  return static_cast<CFWL_GridImp*>(GetImpl())->GetGridSize(eSize, eUnit);
}
FWL_ERR IFWL_Grid::SetGridSize(FWL_GRIDSIZE eSize,
                               FX_FLOAT fSize,
                               FWL_GRIDUNIT eUit) {
  return static_cast<CFWL_GridImp*>(GetImpl())->SetGridSize(eSize, fSize, eUit);
}

CFWL_GridImp::CFWL_GridImp(const CFWL_WidgetImpProperties& properties,
                           IFWL_Widget* pOuter)
    : CFWL_ContentImp(properties, pOuter) {
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
}
CFWL_GridImp::~CFWL_GridImp() {
  int32_t iCount = m_Columns.GetSize();
  for (int32_t i = 0; i < iCount; i++) {
    delete static_cast<CFWL_GridColRow*>(m_Columns[i]);
  }
  m_Columns.RemoveAll();
  iCount = m_Rows.GetSize();
  for (int32_t j = 0; j < iCount; j++) {
    delete static_cast<CFWL_GridColRow*>(m_Rows[j]);
  }
  m_Rows.RemoveAll();
  FX_POSITION ps = m_mapWidgetInfo.GetStartPosition();
  while (ps) {
    IFWL_Widget* pWidget;
    CFWL_GridWidgetInfo* pInfo;
    m_mapWidgetInfo.GetNextAssoc(ps, (void*&)pWidget, (void*&)pInfo);
    delete pInfo;
  }
  m_mapWidgetInfo.RemoveAll();
  delete m_pDelegate;
  m_pDelegate = nullptr;
}
FWL_ERR CFWL_GridImp::GetClassName(CFX_WideString& wsClass) const {
  wsClass = FWL_CLASS_Grid;
  return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_GridImp::GetClassID() const {
  return FWL_CLASSHASH_Grid;
}
FWL_ERR CFWL_GridImp::Initialize() {
  if (CFWL_ContentImp::Initialize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  m_pDelegate = new CFWL_GridImpDelegate(this);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_GridImp::Finalize() {
  if (CFWL_ContentImp::Finalize() != FWL_ERR_Succeeded)
    return FWL_ERR_Indefinite;
  delete m_pDelegate;
  m_pDelegate = nullptr;
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_GridImp::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    rect.left = 0;
    rect.top = 0;
    rect.width = ProcessUnCertainColumns();
    rect.height = ProcessUnCertainRows();
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_GridImp::SetWidgetRect(const CFX_RectF& rect) {
  CFWL_WidgetImp::SetWidgetRect(rect);
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_GridImp::Update() {
  if (IsLocked()) {
    return FWL_ERR_Indefinite;
  }
  ProcessColumns(m_pProperties->m_rtWidget.width);
  ProcessRows(m_pProperties->m_rtWidget.height);
  SetAllWidgetsRect();
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_GridImp::DrawWidget(CFX_Graphics* pGraphics,
                                 const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return FWL_ERR_Indefinite;
  if ((m_pProperties->m_dwStyleExes & FWL_GRIDSTYLEEXT_ShowGridLines) == 0) {
    return FWL_ERR_Succeeded;
  }
  pGraphics->SaveGraphState();
  if (pMatrix) {
    pGraphics->ConcatMatrix(pMatrix);
  }
  {
    FX_BOOL bDrawLine = FALSE;
    CFX_Path path;
    path.Create();
    int32_t iColumns = m_Columns.GetSize();
    for (int32_t i = 1; i < iColumns; i++) {
      CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(m_Columns[i]);
      if (!pColRow) {
        continue;
      }
      bDrawLine = TRUE;
      path.AddLine(pColRow->m_fActualPos, 0, pColRow->m_fActualPos,
                   m_pProperties->m_rtWidget.height);
    }
    int32_t iRows = m_Rows.GetSize();
    for (int32_t j = 1; j < iRows; j++) {
      CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(m_Rows[j]);
      if (!pColRow) {
        continue;
      }
      bDrawLine = TRUE;
      path.AddLine(0, pColRow->m_fActualPos, m_pProperties->m_rtWidget.width,
                   pColRow->m_fActualPos);
    }
    if (bDrawLine) {
      CFX_Color cr(0xFFFF0000);
      pGraphics->SetStrokeColor(&cr);
      pGraphics->StrokePath(&path);
    }
  }
  pGraphics->RestoreGraphState();
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_GridImp::InsertWidget(IFWL_Widget* pChild, int32_t nIndex) {
  if (!pChild)
    return FWL_ERR_Indefinite;
  CFWL_ContentImp::InsertWidget(pChild, nIndex);
  if (!m_mapWidgetInfo.GetValueAt(pChild)) {
    CFWL_GridWidgetInfo* pInfo = new CFWL_GridWidgetInfo;
    m_mapWidgetInfo.SetAt(pChild, pInfo);
    m_Widgets.Add(pChild);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_GridImp::RemoveWidget(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FWL_ERR_Indefinite;
  CFWL_ContentImp::RemoveWidget(pWidget);
  if (CFWL_GridWidgetInfo* pInfo = static_cast<CFWL_GridWidgetInfo*>(
          m_mapWidgetInfo.GetValueAt(pWidget))) {
    m_mapWidgetInfo.RemoveKey(pWidget);
    delete pInfo;
    int32_t nIndex = m_Widgets.Find(pWidget);
    m_Widgets.RemoveAt(nIndex, 1);
  }
  return FWL_ERR_Succeeded;
}
FWL_HGRIDCOLROW CFWL_GridImp::InsertColRow(FX_BOOL bColumn, int32_t nIndex) {
  if (bColumn) {
    if (nIndex < 0 || nIndex > m_Columns.GetSize()) {
      nIndex = m_Columns.GetSize();
    }
    CFWL_GridColRow* pColumn = new CFWL_GridColRow;
    m_Columns.InsertAt(nIndex, pColumn, 1);
    return (FWL_HGRIDCOLROW)pColumn;
  }
  if (nIndex < 0 || nIndex > m_Rows.GetSize()) {
    nIndex = m_Rows.GetSize();
  }
  CFWL_GridColRow* pRow = new CFWL_GridColRow;
  m_Rows.InsertAt(nIndex, pRow, 1);
  return (FWL_HGRIDCOLROW)pRow;
}
int32_t CFWL_GridImp::CountColRows(FX_BOOL bColumn) {
  if (bColumn) {
    return m_Columns.GetSize();
  }
  return m_Rows.GetSize();
}
FWL_HGRIDCOLROW CFWL_GridImp::GetColRow(FX_BOOL bColumn, int32_t nIndex) {
  if (bColumn) {
    if (nIndex < 0 || nIndex >= m_Columns.GetSize()) {
      return NULL;
    }
    return (FWL_HGRIDCOLROW)m_Columns[nIndex];
  }
  if (nIndex < 0 || nIndex >= m_Rows.GetSize()) {
    return NULL;
  }
  return (FWL_HGRIDCOLROW)m_Rows[nIndex];
}
int32_t CFWL_GridImp::GetIndex(FWL_HGRIDCOLROW hColRow) {
  if (IsColumn(hColRow)) {
    return m_Columns.Find(hColRow);
  }
  return m_Rows.Find(hColRow);
}
FX_FLOAT CFWL_GridImp::GetSize(FWL_HGRIDCOLROW hColRow, FWL_GRIDUNIT& eUnit) {
  if (!hColRow)
    return -1;
  CFWL_GridColRow* pColRow = reinterpret_cast<CFWL_GridColRow*>(hColRow);
  eUnit = pColRow->m_Size.eUnit;
  return pColRow->m_Size.fLength;
}
FWL_ERR CFWL_GridImp::SetSize(FWL_HGRIDCOLROW hColRow,
                              FX_FLOAT fSize,
                              FWL_GRIDUNIT eUnit) {
  if (!hColRow)
    return FWL_ERR_Indefinite;
  CFWL_GridColRow* pColRow = reinterpret_cast<CFWL_GridColRow*>(hColRow);
  pColRow->m_Size.eUnit = eUnit;
  pColRow->m_Size.fLength = fSize;
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_GridImp::GetMinSize(FWL_HGRIDCOLROW hColRow,
                                  FWL_GRIDUNIT& eUnit) {
  if (!hColRow)
    return -1;
  CFWL_GridColRow* pColRow = reinterpret_cast<CFWL_GridColRow*>(hColRow);
  eUnit = pColRow->m_MinSize.eUnit;
  return pColRow->m_MinSize.fLength;
}
FWL_ERR CFWL_GridImp::SetMinSize(FWL_HGRIDCOLROW hColRow,
                                 FX_FLOAT fSize,
                                 FWL_GRIDUNIT eUnit) {
  if (!hColRow)
    return FWL_ERR_Indefinite;
  CFWL_GridColRow* pColRow = reinterpret_cast<CFWL_GridColRow*>(hColRow);
  pColRow->m_MinSize.eUnit = eUnit;
  pColRow->m_MinSize.fLength = fSize;
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_GridImp::GetMaxSize(FWL_HGRIDCOLROW hColRow,
                                  FWL_GRIDUNIT& eUnit) {
  if (!hColRow)
    return -1;
  CFWL_GridColRow* pColRow = reinterpret_cast<CFWL_GridColRow*>(hColRow);
  eUnit = pColRow->m_MaxSize.eUnit;
  return pColRow->m_MaxSize.fLength;
}
FWL_ERR CFWL_GridImp::SetMaxSize(FWL_HGRIDCOLROW hColRow,
                                 FX_FLOAT fSize,
                                 FWL_GRIDUNIT eUnit) {
  if (!hColRow)
    return FWL_ERR_Indefinite;
  CFWL_GridColRow* pColRow = reinterpret_cast<CFWL_GridColRow*>(hColRow);
  pColRow->m_MaxSize.eUnit = eUnit;
  pColRow->m_MaxSize.fLength = fSize;
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_GridImp::DeleteColRow(FWL_HGRIDCOLROW hColRow) {
  int32_t nIndex = m_Columns.Find(hColRow);
  if (nIndex >= 0) {
    m_Columns.RemoveAt(nIndex);
    delete reinterpret_cast<CFWL_GridColRow*>(hColRow);
    return TRUE;
  }
  nIndex = m_Rows.Find(hColRow);
  if (nIndex >= 0) {
    delete reinterpret_cast<CFWL_GridColRow*>(hColRow);
    m_Rows.RemoveAt(nIndex);
    return TRUE;
  }
  return FALSE;
}
FX_BOOL CFWL_GridImp::IsColumn(FWL_HGRIDCOLROW hColRow) {
  return m_Columns.Find(hColRow) != -1;
}
int32_t CFWL_GridImp::GetWidgetPos(IFWL_Widget* pWidget, FX_BOOL bColumn) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    return bColumn ? pInfo->m_iColumn : pInfo->m_iRow;
  }
  return -1;
}
FWL_ERR CFWL_GridImp::SetWidgetPos(IFWL_Widget* pWidget,
                                   int32_t iPos,
                                   FX_BOOL bColumn) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    bColumn ? pInfo->m_iColumn = iPos : pInfo->m_iRow = iPos;
  }
  return FWL_ERR_Succeeded;
}
int32_t CFWL_GridImp::GetWidgetSpan(IFWL_Widget* pWidget, FX_BOOL bColumn) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    return bColumn ? pInfo->m_iColumnSpan : pInfo->m_iRowSpan;
  }
  return 0;
}
FWL_ERR CFWL_GridImp::SetWidgetSpan(IFWL_Widget* pWidget,
                                    int32_t iSpan,
                                    FX_BOOL bColumn) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    bColumn ? pInfo->m_iColumnSpan = iSpan : pInfo->m_iRowSpan = iSpan;
  }
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_GridImp::GetWidgetSize(IFWL_Widget* pWidget,
                                     FWL_GRIDSIZE eSize,
                                     FWL_GRIDUNIT& eUnit) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    eUnit = pInfo->m_Size[eSize].eUnit;
    return pInfo->m_Size[eSize].fLength;
  }
  return 0;
}
FWL_ERR CFWL_GridImp::SetWidgetSize(IFWL_Widget* pWidget,
                                    FWL_GRIDSIZE eSize,
                                    FX_FLOAT fSize,
                                    FWL_GRIDUNIT eUit) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    pInfo->m_Size[eSize].fLength = fSize;
    pInfo->m_Size[eSize].eUnit = eUit;
  }
  return FWL_ERR_Succeeded;
}
FX_BOOL CFWL_GridImp::GetWidgetMargin(IFWL_Widget* pWidget,
                                      FWL_GRIDMARGIN eMargin,
                                      FX_FLOAT& fMargin) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    fMargin = pInfo->m_Margin[eMargin];
    return (pInfo->m_dwMarginFlag & (1 << eMargin)) != 0;
  }
  return FALSE;
}
FWL_ERR CFWL_GridImp::SetWidgetMargin(IFWL_Widget* pWidget,
                                      FWL_GRIDMARGIN eMargin,
                                      FX_FLOAT fMargin) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    pInfo->m_Margin[eMargin] = fMargin;
    pInfo->m_dwMarginFlag |= (1 << eMargin);
  }
  return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_GridImp::RemoveWidgetMargin(IFWL_Widget* pWidget,
                                         FWL_GRIDMARGIN eMargin) {
  CFWL_GridWidgetInfo* pInfo =
      static_cast<CFWL_GridWidgetInfo*>(GetWidgetInfo(pWidget));
  if (pInfo) {
    pInfo->m_dwMarginFlag &= ~(1 << eMargin);
  }
  return FWL_ERR_Succeeded;
}
FX_FLOAT CFWL_GridImp::GetGridSize(FWL_GRIDSIZE eSize, FWL_GRIDUNIT& eUnit) {
  eUnit = m_Size[eSize].eUnit;
  return m_Size[eSize].fLength;
}
FWL_ERR CFWL_GridImp::SetGridSize(FWL_GRIDSIZE eSize,
                                  FX_FLOAT fSize,
                                  FWL_GRIDUNIT eUit) {
  m_Size[eSize].fLength = fSize;
  m_Size[eSize].eUnit = eUit;
  return FWL_ERR_Succeeded;
}
CFWL_GridWidgetInfo* CFWL_GridImp::GetWidgetInfo(IFWL_Widget* pWidget) {
  return static_cast<CFWL_GridWidgetInfo*>(m_mapWidgetInfo.GetValueAt(pWidget));
}
void CFWL_GridImp::ProcFixedColRow(CFWL_GridColRow* pColRow,
                                   int32_t nIndex,
                                   FX_FLOAT fColRowSize,
                                   FX_BOOL bColumn) {
  pColRow->m_fActualSize = fColRowSize;
  FX_POSITION ps = m_mapWidgetInfo.GetStartPosition();
  while (ps) {
    void* key = nullptr;
    void* value = nullptr;
    m_mapWidgetInfo.GetNextAssoc(ps, key, value);
    IFWL_Widget* pWidget = static_cast<IFWL_Widget*>(key);
    CFWL_GridWidgetInfo* pInfo = static_cast<CFWL_GridWidgetInfo*>(value);
    if (bColumn) {
      if (pInfo->m_iColumn == nIndex && pInfo->m_iColumnSpan == 1) {
        CalcWidgetWidth(pWidget, pInfo, pColRow->m_fActualSize);
      }
    } else {
      if (pInfo->m_iRow == nIndex && pInfo->m_iRowSpan == 1) {
        CalcWidgetHeigt(pWidget, pInfo, pColRow->m_fActualSize);
      }
    }
  }
}
void CFWL_GridImp::ProcAutoColRow(CFWL_GridColRow* pColRow,
                                  int32_t nIndex,
                                  FX_BOOL bColumn) {
  if (!pColRow)
    return;
  FX_FLOAT fMaxSize = 0, fWidgetSize = 0;
  FX_POSITION ps = m_mapWidgetInfo.GetStartPosition();
  while (ps) {
    IFWL_Widget* pWidget = NULL;
    CFWL_GridWidgetInfo* pInfo = NULL;
    m_mapWidgetInfo.GetNextAssoc(ps, (void*&)pWidget, (void*&)pInfo);
    if (!pWidget || !pInfo) {
      continue;
    }
    if (bColumn) {
      if (pInfo->m_iColumn != nIndex || pInfo->m_iColumnSpan != 1) {
        continue;
      }
      fWidgetSize = CalcAutoColumnWidgetWidth(pWidget, pInfo);
      if (fMaxSize < fWidgetSize) {
        fMaxSize = fWidgetSize;
      }
    } else {
      if (pInfo->m_iRow != nIndex || pInfo->m_iRowSpan != 1) {
        continue;
      }
      fWidgetSize = CalcAutoColumnWidgetHeight(pWidget, pInfo);
      if (fMaxSize < fWidgetSize) {
        fMaxSize = fWidgetSize;
      }
    }
  }
  SetColRowActualSize(pColRow, fMaxSize);
}
void CFWL_GridImp::ProcScaledColRow(CFWL_GridColRow* pColRow,
                                    int32_t nIndex,
                                    FX_FLOAT fColRowSize,
                                    FX_BOOL bColumn) {
  if (fColRowSize > 0) {
    ProcFixedColRow(pColRow, nIndex, fColRowSize, bColumn);
  }
}
void CFWL_GridImp::CalcWidgetWidth(IFWL_Widget* pWidget,
                                   CFWL_GridWidgetInfo* pInfo,
                                   FX_FLOAT fColunmWidth) {
  if (pInfo->m_Size[FWL_GRIDSIZE_Width].eUnit == FWL_GRIDUNIT_Fixed) {
    SetWidgetActualWidth(pInfo, pInfo->m_Size[FWL_GRIDSIZE_Width].fLength);
  } else {
    FX_FLOAT fWidth = 0;
    FX_FLOAT fLeftMargin = 0, fRightMargin = 0;
    FX_BOOL bLeftMargin =
        GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Left, fLeftMargin);
    FX_BOOL bRightMargin =
        GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Right, fRightMargin);
    if (bLeftMargin && bRightMargin) {
      fWidth = fColunmWidth - fLeftMargin - fRightMargin;
    } else {
      CFX_RectF rtAuto;
      pWidget->GetWidgetRect(rtAuto, TRUE);
      fWidth = rtAuto.Width();
    }
    SetWidgetActualWidth(pInfo, fWidth);
  }
}
void CFWL_GridImp::CalcWidgetHeigt(IFWL_Widget* pWidget,
                                   CFWL_GridWidgetInfo* pInfo,
                                   FX_FLOAT fRowHeigt) {
  if (pInfo->m_Size[FWL_GRIDSIZE_Height].eUnit == FWL_GRIDUNIT_Fixed) {
    SetWidgetActualHeight(pInfo, pInfo->m_Size[FWL_GRIDSIZE_Height].fLength);
  } else {
    FX_FLOAT fHeight = 0;
    FX_FLOAT fTopMargin = 0, fBottomMargin = 0;
    FX_BOOL bTopMargin =
        GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Top, fTopMargin);
    FX_BOOL bBottomMargin =
        GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Bottom, fBottomMargin);
    if (bTopMargin && bBottomMargin) {
      fHeight = fRowHeigt - fTopMargin - fBottomMargin;
    } else {
      CFX_RectF rtAuto;
      pWidget->GetWidgetRect(rtAuto, TRUE);
      fHeight = rtAuto.Height();
    }
    SetWidgetActualHeight(pInfo, fHeight);
  }
}
FX_FLOAT CFWL_GridImp::CalcAutoColumnWidgetWidth(IFWL_Widget* pWidget,
                                                 CFWL_GridWidgetInfo* pInfo) {
  FX_FLOAT fLeftMargin = 0, fRightMargin = 0;
  FX_BOOL bLeftMargin =
      GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Left, fLeftMargin);
  FX_BOOL bRightMargin =
      GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Right, fRightMargin);
  if (pInfo->m_Size[FWL_GRIDSIZE_Width].eUnit == FWL_GRIDUNIT_Fixed) {
    SetWidgetActualWidth(pInfo, pInfo->m_Size[FWL_GRIDSIZE_Width].fLength);
  } else {
    CFX_RectF rtAuto;
    pWidget->GetWidgetRect(rtAuto, TRUE);
    FX_FLOAT fWidth = rtAuto.width;
    SetWidgetActualWidth(pInfo, fWidth);
  }
  FX_FLOAT fTotal = pInfo->m_fActualWidth;
  if (bLeftMargin) {
    fTotal += fLeftMargin;
  }
  if (bRightMargin) {
    fTotal += fRightMargin;
  }
  return fTotal;
}
FX_FLOAT CFWL_GridImp::CalcAutoColumnWidgetHeight(IFWL_Widget* pWidget,
                                                  CFWL_GridWidgetInfo* pInfo) {
  FX_FLOAT fTopMargin = 0, fBottomMargin = 0;
  FX_BOOL bTopMargin = GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Top, fTopMargin);
  FX_BOOL bBottomMargin =
      GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Bottom, fBottomMargin);
  if (pInfo->m_Size[FWL_GRIDSIZE_Height].eUnit == FWL_GRIDUNIT_Fixed) {
    SetWidgetActualHeight(pInfo, pInfo->m_Size[FWL_GRIDSIZE_Height].fLength);
  } else {
    CFX_RectF rtAuto;
    pWidget->GetWidgetRect(rtAuto, TRUE);
    FX_FLOAT fHeight = rtAuto.height;
    SetWidgetActualHeight(pInfo, fHeight);
  }
  FX_FLOAT fTotal = pInfo->m_fActualHeight;
  if (bTopMargin) {
    fTotal += fTopMargin;
  }
  if (bBottomMargin) {
    fTotal += fBottomMargin;
  }
  return fTotal;
}
FX_FLOAT CFWL_GridImp::ProcessColumns(FX_FLOAT fWidth) {
  if (fWidth <= 0) {
    return ProcessUnCertainColumns();
  }
  int32_t iColumns = m_Columns.GetSize();
  if (iColumns < 1) {
    return fWidth;
  }
  FX_FLOAT fFixedWidth = 0;
  FX_FLOAT fAutoWidth = 0;
  CFX_PtrArray autoColumns;
  CFX_PtrArray scaledColumns;
  FX_FLOAT fScaledColumnNum = 0;
  for (int32_t i = 0; i < iColumns; i++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(m_Columns[i]);
    if (!pColRow) {
      continue;
    }
    switch (pColRow->m_Size.eUnit) {
      case FWL_GRIDUNIT_Fixed: {
        SetColRowActualSize(pColRow, pColRow->m_Size.fLength);
        fFixedWidth += pColRow->m_fActualSize;
        break;
      }
      case FWL_GRIDUNIT_Auto: {
        ProcAutoColRow(pColRow, i, TRUE);
        autoColumns.Add(pColRow);
        break;
      }
      case FWL_GRIDUNIT_Scaled:
      default: {
        fScaledColumnNum += pColRow->m_Size.fLength;
        scaledColumns.Add(pColRow);
        SetColRowActualSize(pColRow, 0);
      }
    }
  }
  FX_POSITION ps = m_mapWidgetInfo.GetStartPosition();
  while (ps) {
    IFWL_Widget* pWidget = NULL;
    CFWL_GridWidgetInfo* pInfo = NULL;
    m_mapWidgetInfo.GetNextAssoc(ps, (void*&)pWidget, (void*&)pInfo);
    if (!pInfo || pInfo->m_iColumnSpan < 2) {
      continue;
    }
    CFX_PtrArray spanAutoColumns;
    FX_FLOAT fSpanSize = 0;
    int32_t iAutoColRows = 0;
    int32_t iScaledColRows = 0;
    for (int32_t i = 0; i < pInfo->m_iColumnSpan; i++) {
      CFWL_GridColRow* pColumn = reinterpret_cast<CFWL_GridColRow*>(
          GetColRow(TRUE, pInfo->m_iColumn + i));
      if (!pColumn) {
        break;
      }
      fSpanSize += pColumn->m_fActualSize;
      if (pColumn->m_Size.eUnit == FWL_GRIDUNIT_Auto) {
        iAutoColRows++;
        spanAutoColumns.Add(pColumn);
      } else if (pColumn->m_Size.eUnit == FWL_GRIDUNIT_Scaled) {
        iScaledColRows++;
      }
    }
    if (iAutoColRows < 1) {
      continue;
    }
    FX_FLOAT fWidgetWidth = CalcAutoColumnWidgetWidth(pWidget, pInfo);
    if (fWidgetWidth > fSpanSize) {
      if (iScaledColRows > 0) {
      } else {
        SetSpanAutoColRowSize(spanAutoColumns, fWidgetWidth - fSpanSize);
      }
    }
  }
  int32_t iAutoCols = autoColumns.GetSize();
  for (int32_t k = 0; k < iAutoCols; k++) {
    fAutoWidth += static_cast<CFWL_GridColRow*>(autoColumns[k])->m_fActualSize;
  }
  FX_FLOAT fScaledWidth = fWidth - fFixedWidth - fAutoWidth;
  if (fScaledWidth > 0 && fScaledColumnNum > 0) {
    SetScaledColRowsSize(scaledColumns, fScaledWidth, fScaledColumnNum);
  }
  return fWidth;
}
FX_FLOAT CFWL_GridImp::ProcessRows(FX_FLOAT fHeight) {
  if (fHeight <= 0) {
    return ProcessUnCertainRows();
  }
  int32_t iRows = m_Rows.GetSize();
  if (iRows < 1) {
    return fHeight;
  }
  FX_FLOAT fFixedHeight = 0;
  FX_FLOAT fAutoHeigt = 0;
  CFX_PtrArray autoRows;
  CFX_PtrArray scaledRows;
  FX_FLOAT fScaledRowNum = 0;
  for (int32_t i = 0; i < iRows; i++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(m_Rows[i]);
    if (!pColRow) {
      continue;
    }
    switch (pColRow->m_Size.eUnit) {
      case FWL_GRIDUNIT_Fixed: {
        SetColRowActualSize(pColRow, pColRow->m_Size.fLength);
        fFixedHeight += pColRow->m_fActualSize;
        break;
      }
      case FWL_GRIDUNIT_Auto: {
        ProcAutoColRow(pColRow, i, FALSE);
        autoRows.Add(pColRow);
        break;
      }
      case FWL_GRIDUNIT_Scaled:
      default: {
        fScaledRowNum += pColRow->m_Size.fLength;
        scaledRows.Add(pColRow);
        SetColRowActualSize(pColRow, 0);
        break;
      }
    }
  }
  FX_POSITION ps = m_mapWidgetInfo.GetStartPosition();
  while (ps) {
    IFWL_Widget* pWidget = NULL;
    CFWL_GridWidgetInfo* pInfo = NULL;
    m_mapWidgetInfo.GetNextAssoc(ps, (void*&)pWidget, (void*&)pInfo);
    if (!pInfo || pInfo->m_iRowSpan < 2) {
      continue;
    }
    CFX_PtrArray spanAutoRows;
    FX_FLOAT fSpanSize = 0;
    int32_t iAutoColRows = 0;
    int32_t iScaledColRows = 0;
    for (int32_t i = 0; i < pInfo->m_iRowSpan; i++) {
      CFWL_GridColRow* pRow = reinterpret_cast<CFWL_GridColRow*>(
          GetColRow(FALSE, pInfo->m_iRow + i));
      if (!pRow) {
        break;
      }
      fSpanSize += pRow->m_fActualSize;
      if (pRow->m_Size.eUnit == FWL_GRIDUNIT_Auto) {
        iAutoColRows++;
        spanAutoRows.Add(pRow);
      } else if (pRow->m_Size.eUnit == FWL_GRIDUNIT_Scaled) {
        iScaledColRows++;
      }
    }
    if (iAutoColRows < 1) {
      continue;
    }
    FX_FLOAT fWidgetHeight = CalcAutoColumnWidgetHeight(pWidget, pInfo);
    if (fWidgetHeight > fSpanSize) {
      if (iScaledColRows > 0) {
      } else {
        SetSpanAutoColRowSize(spanAutoRows, fWidgetHeight - fSpanSize);
      }
    }
  }
  int32_t iAutoRows = autoRows.GetSize();
  for (int32_t k = 0; k < iAutoRows; k++) {
    fAutoHeigt +=
        reinterpret_cast<CFWL_GridColRow*>(autoRows[k])->m_fActualSize;
  }
  FX_FLOAT fScaledHeight = fHeight - fFixedHeight - fAutoHeigt;
  if (fScaledHeight > 0 && fScaledRowNum > 0) {
    SetScaledColRowsSize(scaledRows, fScaledHeight, fScaledRowNum);
  }
  return fHeight;
}
FX_FLOAT CFWL_GridImp::ProcessUnCertainColumns() {
  int32_t iColumns = m_Columns.GetSize();
  if (iColumns < 1) {
    CFWL_GridColRow* pColRow = new CFWL_GridColRow;
    pColRow->m_Size.eUnit = FWL_GRIDUNIT_Auto;
    ProcAutoColRow(pColRow, 0, TRUE);
    FX_FLOAT fWidth = pColRow->m_fActualSize;
    delete pColRow;
    return fWidth;
  }
  FX_FLOAT fFixedWidth = 0;
  CFX_PtrArray autoColumns;
  CFX_PtrArray scaledColumns;
  FX_FLOAT fScaledColumnNum = 0;
  FX_FLOAT fScaledMaxPerWidth = 0;
  for (int32_t i = 0; i < iColumns; i++) {
    CFWL_GridColRow* pColRow = reinterpret_cast<CFWL_GridColRow*>(m_Columns[i]);
    if (!pColRow) {
      continue;
    }
    switch (pColRow->m_Size.eUnit) {
      case FWL_GRIDUNIT_Fixed: {
        SetColRowActualSize(pColRow, pColRow->m_Size.fLength);
        fFixedWidth += pColRow->m_fActualSize;
        break;
      }
      case FWL_GRIDUNIT_Auto: {
        ProcAutoColRow(pColRow, i, TRUE);
        autoColumns.Add(pColRow);
        break;
      }
      case FWL_GRIDUNIT_Scaled:
      default: {
        ProcAutoColRow(pColRow, i, TRUE);
        fScaledColumnNum += pColRow->m_Size.fLength;
        scaledColumns.Add(pColRow);
        if (pColRow->m_Size.fLength <= 0) {
          break;
        }
        FX_FLOAT fPerWidth = pColRow->m_fActualSize / pColRow->m_Size.fLength;
        if (fPerWidth > fScaledMaxPerWidth) {
          fScaledMaxPerWidth = fPerWidth;
        }
      }
    }
  }
  iColumns = scaledColumns.GetSize();
  for (int32_t j = 0; j < iColumns; j++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(scaledColumns[j]);
    if (!pColRow) {
      continue;
    }
    SetColRowActualSize(pColRow, fScaledMaxPerWidth * pColRow->m_Size.fLength);
  }
  FX_POSITION ps = m_mapWidgetInfo.GetStartPosition();
  while (ps) {
    IFWL_Widget* pWidget = NULL;
    CFWL_GridWidgetInfo* pInfo = NULL;
    m_mapWidgetInfo.GetNextAssoc(ps, (void*&)pWidget, (void*&)pInfo);
    if (!pInfo || pInfo->m_iColumnSpan < 2) {
      continue;
    }
    CFX_PtrArray spanAutoColumns;
    CFX_PtrArray spanScaledColumns;
    FX_FLOAT fSpanSize = 0;
    FX_FLOAT fScaledSum = 0;
    int32_t iAutoColRows = 0;
    int32_t iScaledColRows = 0;
    for (int32_t i = 0; i < pInfo->m_iColumnSpan; i++) {
      CFWL_GridColRow* pColumn = reinterpret_cast<CFWL_GridColRow*>(
          GetColRow(TRUE, pInfo->m_iColumn + i));
      if (!pColumn) {
        break;
      }
      fSpanSize += pColumn->m_fActualSize;
      if (pColumn->m_Size.eUnit == FWL_GRIDUNIT_Auto) {
        iAutoColRows++;
        spanAutoColumns.Add(pColumn);
      } else if (pColumn->m_Size.eUnit == FWL_GRIDUNIT_Scaled) {
        iScaledColRows++;
        fScaledSum += pColumn->m_Size.fLength;
        spanScaledColumns.Add(pColumn);
      }
    }
    if (iAutoColRows < 1 && iScaledColRows < 1) {
      continue;
    }
    FX_FLOAT fWidgetWidth = CalcAutoColumnWidgetWidth(pWidget, pInfo);
    if (fWidgetWidth > fSpanSize) {
      if (iScaledColRows > 0) {
        if (fScaledSum <= 0) {
          continue;
        }
        SetSpanScaledColRowSize(spanScaledColumns, fWidgetWidth - fSpanSize,
                                fScaledSum);
      } else {
        SetSpanAutoColRowSize(spanAutoColumns, fWidgetWidth - fSpanSize);
      }
    }
  }
  FX_FLOAT fAutoWidth = 0;
  int32_t iAutoCols = autoColumns.GetSize();
  for (int32_t m = 0; m < iAutoCols; m++) {
    fAutoWidth += static_cast<CFWL_GridColRow*>(autoColumns[m])->m_fActualSize;
  }
  FX_FLOAT fScaledWidth = 0;
  iColumns = scaledColumns.GetSize();
  for (int32_t n = 0; n < iColumns; n++) {
    fScaledWidth +=
        static_cast<CFWL_GridColRow*>(scaledColumns[n])->m_fActualSize;
  }
  return fFixedWidth + fAutoWidth + fScaledWidth;
}
FX_FLOAT CFWL_GridImp::ProcessUnCertainRows() {
  int32_t iRows = m_Rows.GetSize();
  if (iRows < 1) {
    CFWL_GridColRow* pColRow = new CFWL_GridColRow;
    pColRow->m_Size.eUnit = FWL_GRIDUNIT_Auto;
    ProcAutoColRow(pColRow, 0, FALSE);
    FX_FLOAT fWidth = pColRow->m_fActualSize;
    delete pColRow;
    return fWidth;
  }
  FX_FLOAT fFixedHeight = 0;
  CFX_PtrArray autoRows;
  CFX_PtrArray scaledRows;
  FX_FLOAT fScaledRowNum = 0;
  FX_FLOAT fScaledMaxPerHeight = 0;
  for (int32_t i = 0; i < iRows; i++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(m_Rows[i]);
    if (!pColRow) {
      continue;
    }
    switch (pColRow->m_Size.eUnit) {
      case FWL_GRIDUNIT_Fixed: {
        SetColRowActualSize(pColRow, pColRow->m_Size.fLength);
        fFixedHeight += pColRow->m_fActualSize;
        break;
      }
      case FWL_GRIDUNIT_Auto: {
        ProcAutoColRow(pColRow, i, FALSE);
        autoRows.Add(pColRow);
        break;
      }
      case FWL_GRIDUNIT_Scaled:
      default: {
        ProcAutoColRow(pColRow, i, FALSE);
        fScaledRowNum += pColRow->m_Size.fLength;
        scaledRows.Add(pColRow);
        if (pColRow->m_Size.fLength > 0) {
          FX_FLOAT fPerHeight =
              pColRow->m_fActualSize / pColRow->m_Size.fLength;
          if (fPerHeight > fScaledMaxPerHeight) {
            fScaledMaxPerHeight = fPerHeight;
          }
        }
        break;
      }
    }
  }
  iRows = scaledRows.GetSize();
  for (int32_t j = 0; j < iRows; j++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(scaledRows[j]);
    if (!pColRow) {
      continue;
    }
    SetColRowActualSize(pColRow, fScaledMaxPerHeight * pColRow->m_Size.fLength);
  }
  FX_POSITION ps = m_mapWidgetInfo.GetStartPosition();
  while (ps) {
    void* key = nullptr;
    void* value = nullptr;
    m_mapWidgetInfo.GetNextAssoc(ps, key, value);
    IFWL_Widget* pWidget = static_cast<IFWL_Widget*>(key);
    CFWL_GridWidgetInfo* pInfo = static_cast<CFWL_GridWidgetInfo*>(value);
    if (pInfo->m_iRowSpan < 2) {
      continue;
    }
    CFX_PtrArray spanAutoRows;
    CFX_PtrArray spanScaledRows;
    FX_FLOAT fSpanSize = 0;
    FX_FLOAT fScaledSum = 0;
    int32_t iAutoColRows = 0;
    int32_t iScaledColRows = 0;
    for (int32_t i = 0; i < pInfo->m_iRowSpan; i++) {
      CFWL_GridColRow* pRow = reinterpret_cast<CFWL_GridColRow*>(
          GetColRow(FALSE, pInfo->m_iRow + i));
      if (!pRow) {
        break;
      }
      fSpanSize += pRow->m_fActualSize;
      if (pRow->m_Size.eUnit == FWL_GRIDUNIT_Auto) {
        iAutoColRows++;
        spanAutoRows.Add(pRow);
      } else if (pRow->m_Size.eUnit == FWL_GRIDUNIT_Scaled) {
        iScaledColRows++;
        fScaledSum += pRow->m_Size.fLength;
        spanScaledRows.Add(pRow);
      }
    }
    if (iAutoColRows < 1 && iScaledColRows < 1) {
      continue;
    }
    FX_FLOAT fWidgetHeight = CalcAutoColumnWidgetHeight(pWidget, pInfo);
    if (fWidgetHeight > fSpanSize) {
      if (iScaledColRows > 0) {
        if (fScaledSum <= 0) {
          continue;
        }
        SetSpanScaledColRowSize(spanScaledRows, fWidgetHeight - fSpanSize,
                                fScaledSum);
      } else {
        SetSpanAutoColRowSize(spanAutoRows, fWidgetHeight - fSpanSize);
      }
    }
  }
  FX_FLOAT fAutoHeigt = 0;
  int32_t iAutoRows = autoRows.GetSize();
  for (int32_t m = 0; m < iAutoRows; m++) {
    fAutoHeigt += static_cast<CFWL_GridColRow*>(autoRows[m])->m_fActualSize;
  }
  FX_FLOAT fScaledHeight = 0;
  iRows = scaledRows.GetSize();
  for (int32_t n = 0; n < iRows; n++) {
    fScaledHeight +=
        static_cast<CFWL_GridColRow*>(scaledRows[n])->m_fActualSize;
  }
  return fFixedHeight + fAutoHeigt + fScaledHeight;
}
FX_BOOL CFWL_GridImp::SetColRowActualSize(CFWL_GridColRow* pColRow,
                                          FX_FLOAT fSize,
                                          FX_BOOL bSetBeyond) {
  if (pColRow->m_MinSize.eUnit == FWL_GRIDUNIT_Fixed &&
      fSize < pColRow->m_MinSize.fLength) {
    pColRow->m_fActualSize = pColRow->m_MinSize.fLength;
    return FALSE;
  }
  if (pColRow->m_MaxSize.eUnit == FWL_GRIDUNIT_Fixed &&
      fSize > pColRow->m_MaxSize.fLength) {
    pColRow->m_fActualSize = pColRow->m_MaxSize.fLength;
    return FALSE;
  }
  if (bSetBeyond) {
    return TRUE;
  }
  pColRow->m_fActualSize = fSize;
  return TRUE;
}
FX_FLOAT CFWL_GridImp::SetWidgetActualWidth(CFWL_GridWidgetInfo* pInfo,
                                            FX_FLOAT fWidth) {
  if (pInfo->m_Size[FWL_GRIDSIZE_MinWidth].eUnit == FWL_GRIDUNIT_Fixed &&
      fWidth < pInfo->m_Size[FWL_GRIDSIZE_MinWidth].fLength) {
    fWidth = pInfo->m_Size[FWL_GRIDSIZE_MinWidth].fLength;
  }
  if (pInfo->m_Size[FWL_GRIDSIZE_MaxWidth].eUnit == FWL_GRIDUNIT_Fixed &&
      fWidth > pInfo->m_Size[FWL_GRIDSIZE_MaxWidth].fLength) {
    fWidth = pInfo->m_Size[FWL_GRIDSIZE_MaxWidth].fLength;
  }
  pInfo->m_fActualWidth = fWidth;
  return fWidth;
}
FX_FLOAT CFWL_GridImp::SetWidgetActualHeight(CFWL_GridWidgetInfo* pInfo,
                                             FX_FLOAT fHeight) {
  if (pInfo->m_Size[FWL_GRIDSIZE_MinHeight].eUnit == FWL_GRIDUNIT_Fixed &&
      fHeight < pInfo->m_Size[FWL_GRIDSIZE_MinHeight].fLength) {
    fHeight = pInfo->m_Size[FWL_GRIDSIZE_MinHeight].fLength;
  }
  if (pInfo->m_Size[FWL_GRIDSIZE_MaxHeight].eUnit == FWL_GRIDUNIT_Fixed &&
      fHeight > pInfo->m_Size[FWL_GRIDSIZE_MaxHeight].fLength) {
    fHeight = pInfo->m_Size[FWL_GRIDSIZE_MaxHeight].fLength;
  }
  pInfo->m_fActualHeight = fHeight;
  return fHeight;
}
void CFWL_GridImp::SetAllWidgetsRect() {
  FX_FLOAT fStartLeft = 0;
  int32_t iColumns = m_Columns.GetSize();
  for (int32_t i = 0; i < iColumns; i++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(m_Columns[i]);
    if (!pColRow) {
      continue;
    }
    pColRow->m_fActualPos = fStartLeft;
    fStartLeft += pColRow->m_fActualSize;
  }
  FX_FLOAT fStartTop = 0;
  int32_t iRows = m_Rows.GetSize();
  for (int32_t j = 0; j < iRows; j++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(m_Rows[j]);
    if (!pColRow) {
      continue;
    }
    pColRow->m_fActualPos = fStartTop;
    fStartTop += pColRow->m_fActualSize;
  }
  FX_POSITION ps = m_mapWidgetInfo.GetStartPosition();
  while (ps) {
    IFWL_Widget* pWidget = NULL;
    CFWL_GridWidgetInfo* pInfo = NULL;
    m_mapWidgetInfo.GetNextAssoc(ps, (void*&)pWidget, (void*&)pInfo);
    if (!pWidget || !pInfo) {
      continue;
    }
    FX_FLOAT fColumnStart = 0;
    CFWL_GridColRow* pColumn =
        reinterpret_cast<CFWL_GridColRow*>(GetColRow(TRUE, pInfo->m_iColumn));
    if (pColumn) {
      fColumnStart = pColumn->m_fActualPos;
    }
    FX_FLOAT fRowStart = 0;
    CFWL_GridColRow* pRow =
        reinterpret_cast<CFWL_GridColRow*>(GetColRow(FALSE, pInfo->m_iRow));
    if (pRow) {
      fRowStart = pRow->m_fActualPos;
    }
    FX_FLOAT fColumnWidth = 0;
    if (iColumns > 0) {
      for (int32_t j = 0; j < pInfo->m_iColumnSpan; j++) {
        CFWL_GridColRow* pCol = reinterpret_cast<CFWL_GridColRow*>(
            GetColRow(TRUE, pInfo->m_iColumn + j));
        if (!pCol) {
          break;
        }
        fColumnWidth += pCol->m_fActualSize;
      }
    } else {
      fColumnWidth = m_pProperties->m_rtWidget.width;
    }
    FX_FLOAT fRowHeight = 0;
    if (iRows > 0) {
      for (int32_t k = 0; k < pInfo->m_iRowSpan; k++) {
        CFWL_GridColRow* pR = reinterpret_cast<CFWL_GridColRow*>(
            GetColRow(FALSE, pInfo->m_iRow + k));
        if (!pR) {
          break;
        }
        fRowHeight += pR->m_fActualSize;
      }
    } else {
      fRowHeight = m_pProperties->m_rtWidget.height;
    }
    FX_FLOAT fLeftMargin = 0, fRightMargin = 0;
    FX_BOOL bLeftMargin =
        GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Left, fLeftMargin);
    FX_BOOL bRightMargin =
        GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Right, fRightMargin);
    FX_FLOAT fTopMargin = 0, fBottomMargin = 0;
    FX_BOOL bTopMargin =
        GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Top, fTopMargin);
    FX_BOOL bBottomMargin =
        GetWidgetMargin(pWidget, FWL_GRIDMARGIN_Bottom, fBottomMargin);
    FWL_LAYOUTDATA ltd;
    ltd.fWidth = 0;
    ltd.fHeight = 0;
    if (pInfo->m_Size[FWL_GRIDSIZE_Width].eUnit == FWL_GRIDUNIT_Fixed) {
      SetWidgetActualWidth(pInfo, pInfo->m_Size[FWL_GRIDSIZE_Width].fLength);
      ltd.fWidth = pInfo->m_fActualWidth;
    } else {
      if (bLeftMargin && bRightMargin) {
        SetWidgetActualWidth(pInfo, fColumnWidth - fLeftMargin - fRightMargin);
        ltd.fWidth = pInfo->m_fActualWidth;
      } else {
        CFX_RectF rtAuto;
        pWidget->GetWidgetRect(rtAuto, TRUE);
        SetWidgetActualWidth(pInfo, rtAuto.width);
      }
    }
    if (pInfo->m_Size[FWL_GRIDSIZE_Height].eUnit == FWL_GRIDUNIT_Fixed) {
      SetWidgetActualHeight(pInfo, pInfo->m_Size[FWL_GRIDSIZE_Height].fLength);
      ltd.fHeight = pInfo->m_fActualHeight;
    } else {
      if (bTopMargin && bBottomMargin) {
        SetWidgetActualHeight(pInfo, fRowHeight - fTopMargin - fBottomMargin);
        ltd.fHeight = pInfo->m_fActualHeight;
      } else {
        CFX_RectF rtAuto;
        pWidget->GetWidgetRect(rtAuto, TRUE);
        SetWidgetActualHeight(pInfo, rtAuto.height);
      }
    }
    if (bLeftMargin && bRightMargin &&
        pInfo->m_Size[FWL_GRIDSIZE_Width].eUnit == FWL_GRIDUNIT_Fixed) {
      fLeftMargin =
          fColumnStart + fLeftMargin +
          (fColumnWidth - fLeftMargin - fRightMargin - pInfo->m_fActualWidth) /
              2;
    } else if (bLeftMargin) {
      fLeftMargin = fColumnStart + fLeftMargin;
    } else if (bRightMargin) {
      fLeftMargin =
          fColumnStart + fColumnWidth - fRightMargin - pInfo->m_fActualWidth;
    } else {
      fLeftMargin = fColumnStart;
    }
    if (bTopMargin && bBottomMargin &&
        pInfo->m_Size[FWL_GRIDSIZE_Height].eUnit == FWL_GRIDUNIT_Fixed) {
      fTopMargin =
          fRowStart + fTopMargin +
          (fRowHeight - fTopMargin - fBottomMargin - pInfo->m_fActualHeight) /
              2;
    } else if (bTopMargin) {
      fTopMargin = fRowStart + fTopMargin;
    } else if (bBottomMargin) {
      fTopMargin =
          fRowStart + fRowHeight - fBottomMargin - pInfo->m_fActualHeight;
    } else {
      fTopMargin = fRowStart;
    }
    CFX_RectF rtWidget, rtOld;
    rtWidget.Set(fLeftMargin, fTopMargin, pInfo->m_fActualWidth,
                 pInfo->m_fActualHeight);
    pWidget->GetWidgetRect(rtOld);
    if (rtWidget == rtOld) {
      continue;
    }
    pWidget->SetWidgetRect(rtWidget);
    if (rtWidget.width == rtOld.width && rtWidget.height == rtOld.height) {
      continue;
    }
    pWidget->Update();
  }
}
FX_BOOL CFWL_GridImp::IsGrid(IFWL_Widget* pWidget) {
  if (!pWidget)
    return FALSE;
  return pWidget->GetClassID() == FWL_CLASSHASH_Grid;
}
void CFWL_GridImp::SetSpanAutoColRowSize(const CFX_PtrArray& spanAutos,
                                         FX_FLOAT fTotalSize) {
  int32_t iAutoColRows = spanAutos.GetSize();
  if (iAutoColRows < 1) {
    return;
  }
  CFX_PtrArray autoNoMinMaxs;
  FX_FLOAT fAutoPer = fTotalSize / iAutoColRows;
  for (int32_t j = 0; j < iAutoColRows; j++) {
    CFWL_GridColRow* pColumn = static_cast<CFWL_GridColRow*>(spanAutos[j]);
    FX_FLOAT fOrgSize = pColumn->m_fActualSize;
    if (SetColRowActualSize(pColumn, pColumn->m_fActualSize + fAutoPer, TRUE)) {
      autoNoMinMaxs.Add(pColumn);
    } else {
      fTotalSize -= pColumn->m_fActualSize - fOrgSize;
      int32_t iNoMinMax = iAutoColRows - (j + 1 - autoNoMinMaxs.GetSize());
      if (iNoMinMax > 0 && fTotalSize > 0) {
        fAutoPer = fTotalSize / iNoMinMax;
      } else {
        break;
      }
    }
  }
  int32_t iNormals = autoNoMinMaxs.GetSize();
  if (fTotalSize > 0) {
    if (iNormals == iAutoColRows) {
      fAutoPer = fTotalSize / iNormals;
      for (int32_t k = 0; k < iNormals; k++) {
        CFWL_GridColRow* pColumn =
            static_cast<CFWL_GridColRow*>(autoNoMinMaxs[k]);
        pColumn->m_fActualSize += fAutoPer;
      }
    } else {
      SetSpanAutoColRowSize(autoNoMinMaxs, fTotalSize);
    }
  } else {
  }
}
void CFWL_GridImp::SetSpanScaledColRowSize(const CFX_PtrArray& spanScaleds,
                                           FX_FLOAT fTotalSize,
                                           FX_FLOAT fTotalScaledNum) {
  int32_t iScaledColRows = spanScaleds.GetSize();
  if (iScaledColRows < 1) {
    return;
  }
  CFX_PtrArray autoNoMinMaxs;
  FX_FLOAT fPerSize = fTotalSize / fTotalScaledNum;
  for (int32_t i = 0; i < iScaledColRows; i++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(spanScaleds[i]);
    if (SetColRowActualSize(pColRow, pColRow->m_fActualSize +
                                         fPerSize * pColRow->m_Size.fLength,
                            TRUE)) {
      autoNoMinMaxs.Add(pColRow);
    } else {
      fTotalSize -= pColRow->m_fActualSize;
      fTotalScaledNum -= pColRow->m_Size.fLength;
      int32_t iNoMinMax = iScaledColRows - (i + 1 - autoNoMinMaxs.GetSize());
      if (iNoMinMax > 0 && fTotalSize > 0) {
        fPerSize = fTotalSize / fTotalScaledNum;
      } else {
        break;
      }
    }
  }
  int32_t iNormals = autoNoMinMaxs.GetSize();
  if (fTotalSize > 0) {
    if (iNormals == iScaledColRows) {
      fPerSize = fTotalSize / fTotalScaledNum;
      for (int32_t j = 0; j < iNormals; j++) {
        CFWL_GridColRow* pColumn =
            static_cast<CFWL_GridColRow*>(autoNoMinMaxs[j]);
        pColumn->m_fActualSize += fPerSize * pColumn->m_Size.fLength;
      }
    } else {
      SetSpanScaledColRowSize(autoNoMinMaxs, fTotalSize, fTotalScaledNum);
    }
  } else {
  }
}
void CFWL_GridImp::SetScaledColRowsSize(const CFX_PtrArray& spanScaleds,
                                        FX_FLOAT fTotalSize,
                                        FX_FLOAT fTotalScaledNum) {
  int32_t iScaledColRows = spanScaleds.GetSize();
  if (iScaledColRows < 1) {
    return;
  }
  CFX_PtrArray autoNoMinMaxs;
  FX_FLOAT fPerSize = fTotalSize / fTotalScaledNum;
  for (int32_t i = 0; i < iScaledColRows; i++) {
    CFWL_GridColRow* pColRow = static_cast<CFWL_GridColRow*>(spanScaleds[i]);
    if (!pColRow) {
      continue;
    }
    FX_FLOAT fSize = fPerSize * pColRow->m_Size.fLength;
    FX_FLOAT fOrgSize = pColRow->m_fActualSize;
    if (SetColRowActualSize(pColRow, fSize, TRUE)) {
      autoNoMinMaxs.Add(pColRow);
    } else {
      fTotalSize -= pColRow->m_fActualSize - fOrgSize;
      fTotalScaledNum -= pColRow->m_Size.fLength;
      int32_t iNoMinMax = iScaledColRows - (i + 1 - autoNoMinMaxs.GetSize());
      if (iNoMinMax > 0 && fTotalSize > 0) {
        fPerSize = fTotalSize / fTotalScaledNum;
      } else {
        break;
      }
    }
  }
  int32_t iNormals = autoNoMinMaxs.GetSize();
  if (fTotalSize > 0) {
    if (iNormals == iScaledColRows) {
      fPerSize = fTotalSize / fTotalScaledNum;
      for (int32_t i = 0; i < iNormals; i++) {
        CFWL_GridColRow* pColRow =
            static_cast<CFWL_GridColRow*>(autoNoMinMaxs[i]);
        if (!pColRow) {
          continue;
        }
        FX_FLOAT fSize = fPerSize * pColRow->m_Size.fLength;
        pColRow->m_fActualSize = fSize;
      }
    } else {
      SetScaledColRowsSize(autoNoMinMaxs, fTotalSize, fTotalScaledNum);
    }
  } else {
  }
}
CFWL_GridImpDelegate::CFWL_GridImpDelegate(CFWL_GridImp* pOwner)
    : m_pOwner(pOwner) {
}
int32_t CFWL_GridImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (pMessage->GetClassID() != FWL_MSGHASH_Mouse) {
    return 0;
  }
  CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
  if (pMsg->m_dwCmd != FWL_MSGMOUSECMD_LButtonDown) {
    return 0;
  }
  return 1;
}
FWL_ERR CFWL_GridImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                           const CFX_Matrix* pMatrix) {
  return m_pOwner->DrawWidget(pGraphics, pMatrix);
}
