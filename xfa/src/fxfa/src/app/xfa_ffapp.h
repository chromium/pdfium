// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FFAPP_H_
#define XFA_FFAPP_H_

class CXFA_FileRead : public IFX_FileRead {
 public:
  explicit CXFA_FileRead(const CFX_ArrayTemplate<CPDF_Stream*>& streams);

  virtual FX_FILESIZE GetSize();
  virtual FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);

  virtual void Release() { delete this; }

 protected:
  CFX_ObjectArray<CPDF_StreamAcc> m_Data;
};
class CXFA_FWLAdapterWidgetMgr;
class CXFA_FWLTheme;
class CXFA_FFDocHandler;
class CXFA_FFMenuHandler;
class CXFA_FontMgr;
class CXFA_FFApp : public IXFA_App, public IFWL_AdapterNative {
 public:
  CXFA_FFApp(IXFA_AppProvider* pProvider);
  ~CXFA_FFApp() override;

  // IFXFA_App:
  IXFA_DocHandler* GetDocHandler() override;
  IXFA_Doc* CreateDoc(IXFA_DocProvider* pProvider,
                      IFX_FileRead* pStream,
                      FX_BOOL bTakeOverFile) override;
  IXFA_Doc* CreateDoc(IXFA_DocProvider* pProvider,
                      CPDF_Document* pPDFDoc) override;
  IXFA_AppProvider* GetAppProvider() override { return m_pProvider; }
  void SetDefaultFontMgr(IXFA_FontMgr* pFontMgr) override;
  IXFA_MenuHandler* GetMenuHandler() override;

  // IFWL_AdapterNative:
  IFWL_AdapterWidgetMgr* GetWidgetMgr(
      IFWL_WidgetMgrDelegate* pDelegate) override;
  IFWL_AdapterThreadMgr* GetThreadMgr() override;
  IFWL_AdapterTimerMgr* GetTimerMgr() override;
  IFWL_AdapterCursorMgr* GetCursorMgr() override;
  IFWL_AdapterMonitorMgr* GetMonitorMgr() override;
  IFWL_AdapterClipboardMgr* GetClipboardMgr() override;

  CXFA_FontMgr* GetXFAFontMgr();
  IFX_FontMgr* GetFDEFontMgr();
  CXFA_FWLTheme* GetFWLTheme();
  IFWL_WidgetMgrDelegate* GetWidgetMgrDelegate() {
    return m_pWidgetMgrDelegate;
  }

 protected:
  CXFA_FFDocHandler* m_pDocHandler;
  IFWL_App* m_pFWLApp;
  CXFA_FWLTheme* m_pFWLTheme;
  IXFA_AppProvider* m_pProvider;
  CXFA_FontMgr* m_pFontMgr;
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  IFX_FontSourceEnum* m_pFontSource;
#endif
  CXFA_FWLAdapterWidgetMgr* m_pAdapterWidgetMgr;
  IFWL_WidgetMgrDelegate* m_pWidgetMgrDelegate;
  IFX_FontMgr* m_pFDEFontMgr;
  CXFA_FFMenuHandler* m_pMenuHandler;
  CFWL_SDAdapterThreadMgr* m_pAdapterThreadMgr;
};

#endif  // XFA_FFAPP_H_
