// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDFXFA_PAGEVIEW_H_
#define _FPDFXFA_PAGEVIEW_H_

class CPDFXFA_Page {
 public:
  CPDFXFA_Page(CPDFXFA_Document* pDoc, int page_index);
  ~CPDFXFA_Page();

  void Release();
  void AddRef() { m_iRef++; }
  FX_BOOL LoadPage();
  FX_BOOL LoadPDFPage(CPDF_Dictionary* pageDict);
  CPDFXFA_Document* GetDocument() { return m_pDocument; }
  int GetPageIndex() { return m_iPageIndex; }
  CPDF_Page* GetPDFPage() { return m_pPDFPage; }
  IXFA_PageView* GetXFAPageView() { return m_pXFAPageView; }

  FX_FLOAT GetPageWidth();
  FX_FLOAT GetPageHeight();

  void DeviceToPage(int start_x,
                    int start_y,
                    int size_x,
                    int size_y,
                    int rotate,
                    int device_x,
                    int device_y,
                    double* page_x,
                    double* page_y);
  void PageToDevice(int start_x,
                    int start_y,
                    int size_x,
                    int size_y,
                    int rotate,
                    double page_x,
                    double page_y,
                    int* device_x,
                    int* device_y);

  void GetDisplayMatrix(CFX_Matrix& matrix,
                        int xPos,
                        int yPos,
                        int xSize,
                        int ySize,
                        int iRotate) const;

 protected:
  FX_BOOL LoadPDFPage();
  FX_BOOL LoadXFAPageView();

 private:
  CPDF_Page* m_pPDFPage;
  IXFA_PageView* m_pXFAPageView;
  int m_iPageIndex;
  CPDFXFA_Document* m_pDocument;
  int m_iRef;
};

#endif
