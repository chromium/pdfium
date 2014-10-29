// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_APP_IMP_H
#define _FXFA_FORMFILLER_APP_IMP_H
class CXFA_FileRead : public IFX_FileRead, public CFX_Object
{
public:
    CXFA_FileRead(const CFX_ArrayTemplate<CPDF_Stream*> &streams);
    ~CXFA_FileRead();
    virtual FX_FILESIZE	GetSize();
    virtual FX_BOOL		ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);
    virtual void		Release()
    {
        delete this;
    }
protected:
    CFX_ArrayTemplate<CPDF_Stream*>			m_Streams;
    CFX_DWordArray							m_StreamSize;
    FX_DWORD								m_dwSize;
};
class CXFA_FileRead2 : public IFX_FileRead, public CFX_Object
{
public:
    CXFA_FileRead2(const CFX_ArrayTemplate<CPDF_Stream*> &streams);

    virtual FX_FILESIZE GetSize();
    virtual FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);

    virtual void Release()
    {
        delete this;
    }

protected:
    CFX_ObjectArray<CPDF_StreamAcc>			m_Data;
};
class CXFA_FWLAdapterWidgetMgr;
class CXFA_FWLTheme;
class CXFA_FFDocHandler;
class CXFA_FFMenuHandler;
class CXFA_FontMgr;
class CXFA_FFApp : public IXFA_App, public IFWL_AdapterNative, public CFX_Object
{
public:
    CXFA_FFApp(IXFA_AppProvider* pProvider);
    ~CXFA_FFApp();
    virtual void		Release()
    {
        delete this;
    }
    virtual IXFA_DocHandler*	GetDocHandler();
    virtual XFA_HDOC			CreateDoc(IXFA_DocProvider* pProvider, IFX_FileRead* pStream, FX_BOOL bTakeOverFile);
    virtual XFA_HDOC			CreateDoc(IXFA_DocProvider* pProvider, CPDF_Document* pPDFDoc);
    virtual	IXFA_AppProvider*	GetAppProvider()
    {
        return m_pProvider;
    }
    virtual void				SetDefaultFontMgr(IXFA_FontMgr* pFontMgr);
    virtual IXFA_MenuHandler*	GetMenuHandler();
    virtual IFWL_AdapterWidgetMgr*	GetWidgetMgr(IFWL_WidgetMgrDelegate* pDelegate);
    virtual IFWL_AdapterThreadMgr*	GetThreadMgr();
    virtual IFWL_AdapterTimerMgr*	GetTimerMgr();
    virtual IFWL_AdapterCursorMgr*	GetCursorMgr();
    virtual IFWL_AdapterMonitorMgr* GetMonitorMgr();
    virtual	IFWL_AdapterClipboardMgr* GetClipboardMgr();
    CXFA_FontMgr*		GetXFAFontMgr();
    IFX_FontMgr*		GetFDEFontMgr();
    CXFA_FWLTheme*		GetFWLTheme();
    IFWL_WidgetMgrDelegate* GetWidgetMgrDelegate()
    {
        return m_pWidgetMgrDelegate;
    }
protected:
    CXFA_FFDocHandler*		m_pDocHandler;
    IFWL_App*				m_pFWLApp;
    CXFA_FWLTheme*			m_pFWLTheme;
    IXFA_AppProvider*		m_pProvider;
    CXFA_FontMgr*			m_pFontMgr;
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    IFX_FontSourceEnum*		m_pFontSource;
#endif
    CXFA_FWLAdapterWidgetMgr* m_pAdapterWidgetMgr;
    IFWL_WidgetMgrDelegate*	m_pWidgetMgrDelegate;
    IFX_FontMgr*			m_pFDEFontMgr;
    CXFA_FFMenuHandler*		m_pMenuHandler;
    CFWL_SDAdapterThreadMgr* m_pAdapterThreadMgr;
};
#endif
