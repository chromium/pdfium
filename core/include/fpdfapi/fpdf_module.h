// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_MODULE_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_MODULE_H_

#include "../fxcrt/fx_ext.h"

class CCodec_ModuleMgr;
class CFX_AffineMatrix;
class CFX_BitmapDevice;
class CFX_DIBSource;
class CPDF_ColorSpace;
class CPDF_Dictionary;
class CPDF_DocPageData;
class CPDF_DocRenderData;
class CPDF_Document;
class CPDF_FontGlobals;
class CPDF_Image;
class CPDF_ModuleMgr;
class CPDF_Page;
class CPDF_PageModuleDef;
class CPDF_PageObjects;
class CPDF_PageRenderCache;
class CPDF_RenderConfig;
class CPDF_RenderModuleDef;
class CPDF_RenderOptions;
class CPDF_SecurityHandler;
class CPDF_Stream;
class ICodec_FaxModule;
class ICodec_FlateModule;
class ICodec_IccModule;
class ICodec_Jbig2Module;
class ICodec_JpegModule;
class ICodec_JpxModule;
class IPDF_FontMapper;

#define ADDIN_NAME_CJK			"Eastern Asian Language Support"
#define ADDIN_NAME_DECODER		"JPEG2000 and JBIG2 Image Decoders"

class CPDF_ModuleMgr 
{
public:

    static void	Create();

    static CPDF_ModuleMgr*	Get();

    static void	Destroy();



    void		SetCodecModule(CCodec_ModuleMgr* pModule)
    {
        m_pCodecModule = pModule;
    }
    CCodec_ModuleMgr*		GetCodecModule()
    {
        return m_pCodecModule;
    }

    void		InitPageModule();

    void		InitRenderModule();

    void		SetDownloadCallback(FX_BOOL (*callback)(FX_LPCSTR module_name));

    FX_BOOL		DownloadModule(FX_LPCSTR module_name);

    void		NotifyModuleAvailable(FX_LPCSTR module_name);



    CPDF_RenderModuleDef*	GetRenderModule() const
    {
        return m_pRenderModule;
    }

    CPDF_PageModuleDef*		GetPageModule() const
    {
        return m_pPageModule;
    }




    void					LoadEmbeddedGB1CMaps();

    void					LoadEmbeddedCNS1CMaps();

    void					LoadEmbeddedJapan1CMaps();

    void					LoadEmbeddedKorea1CMaps();

    ICodec_FaxModule*		GetFaxModule();
    ICodec_JpegModule*		GetJpegModule();
    ICodec_JpxModule*		GetJpxModule();
    ICodec_Jbig2Module*		GetJbig2Module();
    ICodec_IccModule*		GetIccModule();
    ICodec_FlateModule*		GetFlateModule();

    void					RegisterSecurityHandler(FX_LPCSTR name, CPDF_SecurityHandler * (*CreateHandler)(void* param), void* param);

    CPDF_SecurityHandler*	CreateSecurityHandler(FX_LPCSTR name);

    void					SetPrivateData(FX_LPVOID module_id, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback);

    FX_LPVOID				GetPrivateData(FX_LPVOID module_id);

    int						m_FileBufSize;
protected:

    CPDF_ModuleMgr();

    ~CPDF_ModuleMgr();
    void					Initialize();

    void					InitModules();



    CCodec_ModuleMgr*		m_pCodecModule;

    CPDF_RenderModuleDef*	m_pRenderModule;

    CPDF_PageModuleDef*		m_pPageModule;


    FX_BOOL (*m_pDownloadCallback)(FX_LPCSTR module_name);

    CFX_MapByteStringToPtr	m_SecurityHandlerMap;

    CFX_PrivateData			m_privateData;
};
class CPDF_PageModuleDef 
{
public:
    virtual ~CPDF_PageModuleDef() {}

    virtual CPDF_DocPageData*	CreateDocData(CPDF_Document* pDoc)
    {
        return NULL;
    }

    virtual void				ReleaseDoc(CPDF_Document*) {}
    virtual void				ClearDoc(CPDF_Document*) {}

    virtual CPDF_FontGlobals*	GetFontGlobals()
    {
        return NULL;
    }

    virtual void				ClearStockFont(CPDF_Document* pDoc) {}

    virtual void				NotifyCJKAvailable() {}

    virtual CPDF_ColorSpace*	GetStockCS(int family)
    {
        return NULL;
    }
};
class CPDF_RenderModuleDef 
{
public:
    virtual ~CPDF_RenderModuleDef() {}

    virtual CPDF_DocRenderData*	CreateDocData(CPDF_Document* pDoc)
    {
        return NULL;
    }

    virtual void	DestroyDocData(CPDF_DocRenderData*) {}
    virtual void	ClearDocData(CPDF_DocRenderData*) {}

    virtual CPDF_DocRenderData* GetRenderData()
    {
        return NULL;
    }

    virtual CPDF_PageRenderCache*	CreatePageCache(CPDF_Page* pPage)
    {
        return NULL;
    }

    virtual void	DestroyPageCache(CPDF_PageRenderCache*) {}

    virtual void	NotifyDecoderAvailable() {}

    virtual CPDF_RenderConfig* GetConfig()
    {
        return NULL;
    }
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_MODULE_H_
