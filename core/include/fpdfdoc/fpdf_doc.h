// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDF_DOC_H_
#define _FPDF_DOC_H_
#ifndef _FPDF_PARSER_
#include "../fpdfapi/fpdf_parser.h"
#endif
#ifndef _FPDF_RENDER_
#include "../fpdfapi/fpdf_render.h"
#endif
class CPDF_Action;
class CPDF_Annot;
class CPDF_AnnotList;
class CPDF_Bookmark;
class CPDF_BookmarkTree;
class CPDF_Dest;
class CPDF_Link;
class CPDF_LinkList;
class CPDF_Metadata;
class CPDF_NameTree;
class CPDF_NumberTree;
class CPDF_TextObject;
class CPDF_ViewerPreferences;
class CPDF_Page;
class CPDF_RenderOptions;
class CXML_Element;
class CPDF_OCContext;
class CPDF_DocJSActions;
class CPDF_ActionFields;
class CPDF_AAction;
class CPDF_FileSpec;
class CPDF_IconFit;
class CPDF_DefaultAppearance;
class CPDF_InterForm;
class CPDF_FormField;
class CPDF_FormNotify;
class CPDF_FormControl;
class CPDF_LWinParam;
class CFieldTree;
class CPDF_ApSettings;
class CPDF_NameTree : public CFX_Object
{
public:

    CPDF_NameTree(CPDF_Dictionary* pRoot)
    {
        m_pRoot = pRoot;
    }

    CPDF_NameTree(CPDF_Document* pDoc, FX_BSTR category);

    CPDF_Object*		LookupValue(int nIndex, CFX_ByteString& csName) const;

    CPDF_Object*		LookupValue(const CFX_ByteString& csName) const;

    CPDF_Array*			LookupNamedDest(CPDF_Document* pDoc, FX_BSTR sName);

    int					GetIndex(const CFX_ByteString& csName) const;

    int					GetCount() const;


    CPDF_Dictionary*	GetRoot() const
    {
        return m_pRoot;
    }

protected:

    CPDF_Dictionary*		m_pRoot;
};
class CPDF_BookmarkTree : public CFX_Object
{
public:
    CPDF_BookmarkTree(CPDF_Document* pDoc) : m_pDocument(pDoc) {}

    CPDF_Bookmark		GetFirstChild(const CPDF_Bookmark& parent) const;

    CPDF_Bookmark		GetNextSibling(const CPDF_Bookmark& bookmark) const;

    CPDF_Document*		GetDocument() const { return m_pDocument; }

protected:
    CPDF_Document*		m_pDocument;
};
#define PDFBOOKMARK_ITALIC			1
#define PDFBOOKMARK_BOLD			2
class CPDF_Bookmark : public CFX_Object
{
public:

    CPDF_Bookmark() : m_pDict(NULL) {}

    explicit CPDF_Bookmark(CPDF_Dictionary* pDict) : m_pDict(pDict) {}

    CPDF_Dictionary* GetDict() const { return m_pDict; }

    operator bool() const { return m_pDict != NULL; }

    FX_DWORD			GetColorRef() const;

    FX_DWORD			GetFontStyle() const;

    CFX_WideString		GetTitle() const;

    CPDF_Dest			GetDest(CPDF_Document* pDocument) const;

    CPDF_Action			GetAction() const;

    CPDF_Dictionary*	m_pDict;
};
#define PDFZOOM_XYZ					1
#define PDFZOOM_FITPAGE				2
#define PDFZOOM_FITHORZ				3
#define PDFZOOM_FITVERT				4
#define PDFZOOM_FITRECT				5
#define PDFZOOM_FITBBOX				6
#define PDFZOOM_FITBHORZ			7
#define PDFZOOM_FITBVERT			8
class CPDF_Dest : public CFX_Object
{
public:

    CPDF_Dest(CPDF_Object* pObj = NULL)
    {
        m_pObj = pObj;
    }

    operator CPDF_Object* () const
    {
        return m_pObj;
    }

    CFX_ByteString		GetRemoteName();

    int					GetPageIndex(CPDF_Document* pDoc);

    FX_DWORD			GetPageObjNum();

    int					GetZoomMode();

    FX_FLOAT			GetParam(int index);


    CPDF_Object*		m_pObj;
};
class CPDF_OCContext : public CFX_Object, public IPDF_OCContext
{
public:

    enum UsageType {
        View = 0,
        Design,
        Print,
        Export
    };

    CPDF_OCContext(CPDF_Document *pDoc, UsageType eUsageType = View);

    virtual ~CPDF_OCContext();

    CPDF_Document*	GetDocument() const
    {
        return m_pDocument;
    }

    UsageType		GetUsageType() const
    {
        return m_eUsageType;
    }

    FX_BOOL			CheckOCGVisible(const CPDF_Dictionary *pOCGDict);

    void			ResetOCContext();
protected:

    FX_BOOL			LoadOCGStateFromConfig(FX_BSTR csConfig, const CPDF_Dictionary *pOCGDict, FX_BOOL &bValidConfig) const;

    FX_BOOL			LoadOCGState(const CPDF_Dictionary *pOCGDict) const;

    FX_BOOL			GetOCGVisible(const CPDF_Dictionary *pOCGDict);

    FX_BOOL			GetOCGVE(CPDF_Array *pExpression, FX_BOOL bFromConfig, int nLevel = 0);

    FX_BOOL			LoadOCMDState(const CPDF_Dictionary *pOCMDDict, FX_BOOL bFromConfig);

    CPDF_Document		*m_pDocument;

    UsageType			m_eUsageType;

    CFX_MapPtrTemplate<const CPDF_Dictionary*, void*>	m_OCGStates;
};
class CPDF_LWinParam : public CFX_Object
{
public:

    CPDF_LWinParam(CPDF_Dictionary* pDict)
    {
        m_pDict = pDict;
    }

    operator CPDF_Dictionary* () const
    {
        return m_pDict;
    }

    inline CFX_ByteString	GetFileName()
    {
        return m_pDict->GetString("F");
    }


    inline CFX_ByteString	GetDefaultDirectory()
    {
        return m_pDict->GetString("D");
    }


    inline CFX_ByteString	GetOperation()
    {
        return m_pDict->GetString("O");
    }


    inline CFX_ByteString	GetParameter()
    {
        return m_pDict->GetString("P");
    }

    CPDF_Dictionary*		m_pDict;
};
class CPDF_ActionFields : public CFX_Object
{
public:

    CPDF_ActionFields(const CPDF_Action* pAction)
    {
        m_pAction = (CPDF_Action*)pAction;
    }

    operator CPDF_Action*() const
    {
        return m_pAction;
    }

    FX_DWORD				GetFieldsCount() const;

    void					GetAllFields(CFX_PtrArray& fieldObjects) const;

    CPDF_Object*			GetField(FX_DWORD iIndex) const;

    CPDF_Action*			m_pAction;
};

#define PDFNAMED_NEXTPAGE		1
#define PDFNAMED_PREVPAGE		2
#define PDFNAMED_FIRSTPAGE		3
#define PDFNAMED_LASTPAGE		4
#define PDFJS_MAXLENGTH			64
class CPDF_Action : public CFX_Object
{
public:

    CPDF_Action(CPDF_Dictionary* pDict = NULL)
    {
        m_pDict = pDict;
    }

    operator CPDF_Dictionary* () const
    {
        return m_pDict;
    }

    enum ActionType {
        Unknown = 0,
        GoTo,
        GoToR,
        GoToE,
        Launch,
        Thread,
        URI,
        Sound,
        Movie,
        Hide,
        Named,
        SubmitForm,
        ResetForm,
        ImportData,
        JavaScript,
        SetOCGState,
        Rendition,
        Trans,
        GoTo3DView
    };

    CFX_ByteString		GetTypeName() const
    {
        return m_pDict->GetString("S");
    }

    ActionType			GetType() const;



    CPDF_Dest			GetDest(CPDF_Document* pDoc) const;





    CFX_WideString		GetFilePath() const;




    FX_BOOL				GetNewWindow() const
    {
        return m_pDict->GetBoolean("NewWindow");
    }




    CPDF_LWinParam		GetWinParam() const;




    CFX_ByteString		GetURI(CPDF_Document* pDoc) const;




    FX_BOOL				GetMouseMap() const
    {
        return m_pDict->GetBoolean("IsMap");
    }




    CPDF_ActionFields	GetWidgets() const
    {
        return this;
    }




    FX_BOOL				GetHideStatus() const
    {
        return m_pDict->GetBoolean("H", TRUE);
    }




    CFX_ByteString		GetNamedAction() const
    {
        return m_pDict->GetString("N");
    }




    FX_DWORD			GetFlags() const
    {
        return m_pDict->GetInteger("Flags");
    }




    CFX_WideString		GetJavaScript() const;




    CPDF_Dictionary*	GetAnnot() const;




    FX_INT32			GetOperationType() const;




    CPDF_Stream*		GetSoundStream() const
    {
        return m_pDict->GetStream("Sound");
    }

    FX_FLOAT			GetVolume() const
    {
        return m_pDict->GetNumber("Volume");
    }

    FX_BOOL				IsSynchronous() const
    {
        return m_pDict->GetBoolean("Synchronous");
    }

    FX_BOOL				IsRepeat() const
    {
        return m_pDict->GetBoolean("Repeat");
    }

    FX_BOOL				IsMixPlay() const
    {
        return m_pDict->GetBoolean("Mix");
    }




    FX_DWORD			GetSubActionsCount() const;

    CPDF_Action			GetSubAction(FX_DWORD iIndex) const;


    CPDF_Dictionary*	m_pDict;
};
class CPDF_AAction : public CFX_Object
{
public:

    CPDF_AAction(CPDF_Dictionary* pDict = NULL)
    {
        m_pDict = pDict;
    }

    operator CPDF_Dictionary*()	const
    {
        return m_pDict;
    }

    enum AActionType {
        CursorEnter = 0,
        CursorExit,
        ButtonDown,
        ButtonUp,
        GetFocus,
        LoseFocus,
        PageOpen,
        PageClose,
        PageVisible,
        PageInvisible,
        OpenPage,
        ClosePage,
        KeyStroke,
        Format,
        Validate,
        Calculate,
        CloseDocument,
        SaveDocument,
        DocumentSaved,
        PrintDocument,
        DocumentPrinted
    };

    FX_BOOL				ActionExist(AActionType eType) const;

    CPDF_Action			GetAction(AActionType eType) const;

    FX_POSITION			GetStartPos() const;

    CPDF_Action			GetNextAction(FX_POSITION& pos, AActionType& eType) const;

    CPDF_Dictionary*	m_pDict;
};
class CPDF_DocJSActions : public CFX_Object
{
public:
    CPDF_DocJSActions(CPDF_Document* pDoc);


    int					CountJSActions() const;

    CPDF_Action			GetJSAction(int index, CFX_ByteString& csName) const;

    CPDF_Action			GetJSAction(const CFX_ByteString& csName) const;

    int					FindJSAction(const CFX_ByteString& csName) const;


    CPDF_Document*		GetDocument() const
    {
        return m_pDocument;
    }

protected:

    CPDF_Document*		m_pDocument;
};
class CPDF_FileSpec : public CFX_Object
{
public:

    CPDF_FileSpec();

    CPDF_FileSpec(CPDF_Object *pObj)
    {
        m_pObj = pObj;
    }

    operator CPDF_Object*() const
    {
        return m_pObj;
    }

    FX_BOOL			IsURL() const;

    FX_BOOL			GetFileName(CFX_WideString &wsFileName) const;

    CPDF_Stream*	GetFileStream() const;

    void			SetFileName(FX_WSTR wsFileName, FX_BOOL bURL = FALSE);
protected:

    CPDF_Object		*m_pObj;
};
class CPDF_LinkList : public CFX_Object
{
public:

    CPDF_LinkList(CPDF_Document* pDoc)
    {
        m_pDocument = pDoc;
    }

    ~CPDF_LinkList();

    CPDF_Link			GetLinkAtPoint(CPDF_Page* pPage, FX_FLOAT pdf_x, FX_FLOAT pdf_y);

    int					CountLinks(CPDF_Page* pPage);

    CPDF_Link			GetLink(CPDF_Page* pPage, int index);

    CPDF_Document*		GetDocument() const
    {
        return m_pDocument;
    }
protected:

    CPDF_Document*		m_pDocument;

    CFX_MapPtrToPtr		m_PageMap;

    CFX_PtrArray*		GetPageLinks(CPDF_Page* pPage);

    void				LoadPageLinks(CPDF_Page* pPage, CFX_PtrArray* pList);
};
class CPDF_Link : public CFX_Object
{
public:

    CPDF_Link(CPDF_Dictionary* pDict = NULL)
    {
        m_pDict = pDict;
    }

    operator CPDF_Dictionary*() const
    {
        return m_pDict;
    }

    CFX_FloatRect		GetRect();



    CPDF_Dest			GetDest(CPDF_Document* pDoc);

    CPDF_Action			GetAction();


    CPDF_Dictionary*	m_pDict;
};
#define ANNOTFLAG_INVISIBLE			1
#define ANNOTFLAG_HIDDEN			2
#define ANNOTFLAG_PRINT				4
#define ANNOTFLAG_NOZOOM			8
#define ANNOTFLAG_NOROTATE			0x10
#define ANNOTFLAG_NOVIEW			0x20
#define ANNOTFLAG_READONLY			0x40
#define ANNOTFLAG_LOCKED			0x80
#define ANNOTFLAG_TOGGLENOVIEW		0x100
class CPDF_Annot : public CFX_PrivateData, public CFX_Object
{
public:

    CPDF_Annot(CPDF_Dictionary* pDict);

    ~CPDF_Annot();

    CPDF_Dictionary*	m_pAnnotDict;

    CFX_ByteString		GetSubType() const;

    FX_DWORD			GetFlags() const
    {
        return m_pAnnotDict->GetInteger("F");
    }

    void				GetRect(CFX_FloatRect& rect) const;

    enum AppearanceMode	{
        Normal,
        Rollover,
        Down
    };

    FX_BOOL				DrawAppearance(const CPDF_Page* pPage, CFX_RenderDevice* pDevice, const CFX_AffineMatrix* pUser2Device,
                                       AppearanceMode mode, const CPDF_RenderOptions* pOptions);

    FX_BOOL				DrawInContext(const CPDF_Page* pPage, const CPDF_RenderContext* pContext,
                                      const CFX_AffineMatrix* pUser2Device, AppearanceMode mode);

    void				ClearCachedAP();


    void				DrawBorder(CFX_RenderDevice* pDevice, const CFX_AffineMatrix* pUser2Device,
                                   const CPDF_RenderOptions* pOptions);

    CPDF_PageObject*	GetBorder(FX_BOOL bPrint, const CPDF_RenderOptions* pOptions);



    int					CountIRTNotes();

    CPDF_Annot*			GetIRTNote(int index);


    CPDF_Form*			GetAPForm(const CPDF_Page* pPage, AppearanceMode mode);
private:

    CFX_MapPtrToPtr		m_APMap;
protected:
    friend class		CPDF_AnnotList;

    CPDF_AnnotList*		m_pList;

    CPDF_Reference*		NewAnnotRef();
};
class CPDF_AnnotList : public CFX_Object
{
public:

    CPDF_AnnotList(CPDF_Page* pPage);

    ~CPDF_AnnotList();

    void	GetAnnotMatrix(const CPDF_Dictionary* pAnnotDict, const CFX_Matrix* pUser2Device, CFX_Matrix &matrix) const;

    void	GetAnnotRect(const CPDF_Dictionary* pAnnotDict, const CFX_Matrix* pUser2Device, CPDF_Rect &rtAnnot) const;

    void				DisplayAnnots(const CPDF_Page* pPage, CFX_RenderDevice* pDevice,
                                      CFX_AffineMatrix* pMatrix, FX_BOOL bShowWidget,
                                      CPDF_RenderOptions* pOptions);

    void				DisplayAnnots(const CPDF_Page* pPage, CPDF_RenderContext* pContext,
                                      FX_BOOL bPrinting, CFX_AffineMatrix* pMatrix, FX_BOOL bShowWidget,
                                      CPDF_RenderOptions* pOptions)
    {
        DisplayAnnots(pPage, NULL, pContext, bPrinting, pMatrix, bShowWidget ? 3 : 1, pOptions, NULL);
    }

    void				DisplayAnnots(const CPDF_Page* pPage, CPDF_RenderContext* pContext,
                                      FX_BOOL bPrinting, CFX_AffineMatrix* pMatrix, FX_BOOL bShowWidget,
                                      CPDF_RenderOptions* pOptions, FX_RECT *pClipRect)
    {
        DisplayAnnots(pPage, NULL, pContext, bPrinting, pMatrix, bShowWidget ? 3 : 1, pOptions, pClipRect);
    }

    void				DisplayAnnots(const CPDF_Page* pPage, CFX_RenderDevice* pDevice, CPDF_RenderContext* pContext,
                                      FX_BOOL bPrinting, CFX_AffineMatrix* pMatrix, FX_DWORD dwAnnotFlags,
                                      CPDF_RenderOptions* pOptions, FX_RECT* pClipRect);



    CPDF_Annot*			GetAt(int index)
    {
        return (CPDF_Annot*)m_AnnotList.GetAt(index);
    }

    int					Count()
    {
        return m_AnnotList.GetSize();
    }

    int					GetIndex(CPDF_Annot* pAnnot);


    CPDF_Document*		GetDocument() const
    {
        return m_pDocument;
    }
protected:

    CFX_PtrArray		m_AnnotList;

    CPDF_Dictionary*	m_pPageDict;

    CPDF_Document*		m_pDocument;

    CFX_PtrArray		m_Borders;

    void				DisplayPass(const CPDF_Page* pPage, CFX_RenderDevice* pDevice,
                                    CPDF_RenderContext* pContext, FX_BOOL bPrinting, CFX_AffineMatrix* pMatrix,
                                    FX_BOOL bWidget, CPDF_RenderOptions* pOptions, FX_RECT* clip_rect);
    friend class		CPDF_Annot;
};
#define COLORTYPE_TRANSPARENT	0
#define COLORTYPE_GRAY			1
#define COLORTYPE_RGB			2
#define COLORTYPE_CMYK			3
class CPDF_DefaultAppearance : public CFX_Object
{
public:

    CPDF_DefaultAppearance(const CFX_ByteString& csDA = "")
    {
        m_csDA = csDA;
    }

    CPDF_DefaultAppearance(const CPDF_DefaultAppearance& cDA)
    {
        m_csDA = (CFX_ByteString)(CPDF_DefaultAppearance&)cDA;
    }


    operator CFX_ByteString() const
    {
        return m_csDA;
    }

    const CPDF_DefaultAppearance& operator =(const CFX_ByteString& csDA)
    {
        m_csDA = csDA;
        return *this;
    }

    const CPDF_DefaultAppearance& operator =(const CPDF_DefaultAppearance& cDA)
    {
        m_csDA = (CFX_ByteString)(CPDF_DefaultAppearance&)cDA;
        return *this;
    }



    FX_BOOL				HasFont();

    CFX_ByteString		GetFontString();

    void				GetFont(CFX_ByteString& csFontNameTag, FX_FLOAT& fFontSize);




    FX_BOOL				HasColor(FX_BOOL bStrokingOperation = FALSE);

    CFX_ByteString		GetColorString(FX_BOOL bStrokingOperation = FALSE);

    void				GetColor(int& iColorType, FX_FLOAT fc[4], FX_BOOL bStrokingOperation = FALSE);

    void				GetColor(FX_ARGB& color, int& iColorType, FX_BOOL bStrokingOperation = FALSE);




    FX_BOOL				HasTextMatrix();

    CFX_ByteString		GetTextMatrixString();

    CFX_AffineMatrix	GetTextMatrix();

protected:

    CFX_ByteString		m_csDA;
};
#define FIELDTYPE_UNKNOWN			0
#define FIELDTYPE_PUSHBUTTON		1
#define FIELDTYPE_CHECKBOX			2
#define FIELDTYPE_RADIOBUTTON		3
#define FIELDTYPE_COMBOBOX			4
#define FIELDTYPE_LISTBOX			5
#define FIELDTYPE_TEXTFIELD			6
#define FIELDTYPE_SIGNATURE			7
class CPDF_InterForm : public CFX_PrivateData, public CFX_Object
{
public:

    CPDF_InterForm(CPDF_Document* pDocument, FX_BOOL bUpdateAP);

    ~CPDF_InterForm();



    static void				EnableUpdateAP(FX_BOOL bUpdateAP);

    static FX_BOOL			UpdatingAPEnabled();


    static CFX_ByteString	GenerateNewResourceName(const CPDF_Dictionary* pResDict, FX_LPCSTR csType, int iMinLen = 2, FX_LPCSTR csPrefix = "");



    static CPDF_Font*		AddSystemDefaultFont(const CPDF_Document* pDocument);

    static CPDF_Font*		AddSystemFont(const CPDF_Document* pDocument, CFX_ByteString csFontName, FX_BYTE iCharSet = 1);

    static CPDF_Font*		AddSystemFont(const CPDF_Document* pDocument, CFX_WideString csFontName, FX_BYTE iCharSet = 1);

    static CPDF_Font*		AddStandardFont(const CPDF_Document* pDocument, CFX_ByteString csFontName);

    static CFX_ByteString	GetNativeFont(FX_BYTE iCharSet, FX_LPVOID pLogFont = NULL);

    static CFX_ByteString	GetNativeFont(FX_LPVOID pLogFont = NULL);

    static FX_BYTE			GetNativeCharSet();

    static CPDF_Font*		AddNativeFont(FX_BYTE iCharSet, const CPDF_Document* pDocument);

    static CPDF_Font*		AddNativeFont(const CPDF_Document* pDocument);




    FX_BOOL					ValidateFieldName(CFX_WideString& csNewFieldName, int iType);

    FX_BOOL					ValidateFieldName(const CPDF_FormField* pField, CFX_WideString& csNewFieldName);

    FX_BOOL					ValidateFieldName(const CPDF_FormControl* pControl, CFX_WideString& csNewFieldName);




    FX_DWORD				CountFields(const CFX_WideString &csFieldName = L"");

    CPDF_FormField*			GetField(FX_DWORD index, const CFX_WideString &csFieldName = L"");

    void					GetAllFieldNames(CFX_WideStringArray& allFieldNames);

    FX_BOOL					IsValidFormField(const void* pField);

    CPDF_FormField*			GetFieldByDict(CPDF_Dictionary* pFieldDict) const;




    FX_DWORD				CountControls(CFX_WideString csFieldName = L"");

    CPDF_FormControl*		GetControl(FX_DWORD index, CFX_WideString csFieldName = L"");

    FX_BOOL					IsValidFormControl(const void* pControl);

    int						CountPageControls(CPDF_Page* pPage) const;

    CPDF_FormControl*		GetPageControl(CPDF_Page* pPage, int index) const;


    CPDF_FormControl*		GetControlAtPoint(CPDF_Page* pPage, FX_FLOAT pdf_x, FX_FLOAT pdf_y) const;

    CPDF_FormControl*		GetControlByDict(CPDF_Dictionary* pWidgetDict) const;




    FX_DWORD				CountInternalFields(const CFX_WideString& csFieldName = L"") const;

    CPDF_Dictionary*		GetInternalField(FX_DWORD index, const CFX_WideString& csFieldName = L"") const;





    CPDF_Document*			GetDocument() const
    {
        return m_pDocument;
    }

    CPDF_Dictionary*		GetFormDict() const
    {
        return m_pFormDict;
    }




    FX_BOOL					NeedConstructAP();

    void					NeedConstructAP(FX_BOOL bNeedAP);




    int						CountFieldsInCalculationOrder();

    CPDF_FormField*			GetFieldInCalculationOrder(int index);

    int						FindFieldInCalculationOrder(const CPDF_FormField* pField);




    FX_DWORD				CountFormFonts();

    CPDF_Font*				GetFormFont(FX_DWORD index, CFX_ByteString& csNameTag);

    CPDF_Font*				GetFormFont(CFX_ByteString csNameTag);

    CPDF_Font*				GetFormFont(CFX_ByteString csFontName, CFX_ByteString& csNameTag);

    CPDF_Font*				GetNativeFormFont(FX_BYTE iCharSet, CFX_ByteString& csNameTag);

    CPDF_Font*				GetNativeFormFont(CFX_ByteString& csNameTag);

    FX_BOOL					FindFormFont(const CPDF_Font* pFont, CFX_ByteString& csNameTag);

    FX_BOOL					FindFormFont(CFX_ByteString csFontName, CPDF_Font*& pFont, CFX_ByteString& csNameTag);

    inline FX_BOOL			FindFormFont(CFX_WideString csFontName, CPDF_Font*& pFont, CFX_ByteString& csNameTag)
    {
        return FindFormFont(PDF_EncodeText(csFontName), pFont, csNameTag);
    }





    void					AddFormFont(const CPDF_Font* pFont, CFX_ByteString& csNameTag);

    CPDF_Font*				AddNativeFormFont(FX_BYTE iCharSet, CFX_ByteString& csNameTag);

    CPDF_Font*				AddNativeFormFont(CFX_ByteString& csNameTag);

    void					RemoveFormFont(const CPDF_Font* pFont);

    void					RemoveFormFont(CFX_ByteString csNameTag);




    CPDF_DefaultAppearance	GetDefaultAppearance();

    CPDF_Font*				GetDefaultFormFont();



    int						GetFormAlignment();




    CPDF_FormField*			CheckRequiredFields(const CFX_PtrArray *fields = NULL, FX_BOOL bIncludeOrExclude = TRUE) const;

    CFDF_Document* 			ExportToFDF(FX_WSTR pdf_path, FX_BOOL bSimpleFileSpec = FALSE) const;

    CFDF_Document*			ExportToFDF(FX_WSTR pdf_path, CFX_PtrArray& fields, FX_BOOL bIncludeOrExclude = TRUE, FX_BOOL bSimpleFileSpec = FALSE) const;

    FX_BOOL					ImportFromFDF(const CFDF_Document* pFDFDoc, FX_BOOL bNotify = FALSE);




    FX_BOOL					ResetForm(const CFX_PtrArray& fields, FX_BOOL bIncludeOrExclude = TRUE, FX_BOOL bNotify = FALSE);

    FX_BOOL					ResetForm(FX_BOOL bNotify = FALSE);

    void					ReloadForm();

    CPDF_FormNotify*		GetFormNotify() const
    {
        return m_pFormNotify;
    }

    void					SetFormNotify(const CPDF_FormNotify* pNotify);


    int						GetPageWithWidget(int iCurPage, FX_BOOL bNext);



    FX_BOOL					IsUpdated()
    {
        return m_bUpdated;
    }

    void					ClearUpdatedFlag()
    {
        m_bUpdated = FALSE;
    }


    FX_BOOL					HasXFAForm() const;

    void					FixPageFields(const CPDF_Page* pPage);
protected:

    static FX_BOOL			m_bUpdateAP;

    void					LoadField(CPDF_Dictionary* pFieldDict, int nLevel = 0);

    CPDF_Object*			GetFieldAttr(CPDF_Dictionary* pFieldDict, const FX_CHAR* name);

    CPDF_FormField*			AddTerminalField(const CPDF_Dictionary* pFieldDict);

    CPDF_FormControl*		AddControl(const CPDF_FormField* pField, const CPDF_Dictionary* pWidgetDict);

    void					FDF_ImportField(CPDF_Dictionary* pField, const CFX_WideString& parent_name, FX_BOOL bNotify = FALSE, int nLevel = 0);

    FX_BOOL					ValidateFieldName(CFX_WideString& csNewFieldName, int iType, const CPDF_FormField* pExcludedField, const CPDF_FormControl* pExcludedControl);

    int						CompareFieldName(const CFX_WideString& name1, const CFX_WideString& name2);

    int						CompareFieldName(const CFX_ByteString& name1, const CFX_ByteString& name2);

    CPDF_Document*			m_pDocument;

    FX_BOOL					m_bGenerateAP;

    CPDF_Dictionary*		m_pFormDict;

    CFX_MapPtrToPtr			m_ControlMap;

    CFieldTree *m_pFieldTree;

    CFX_ByteString			m_bsEncoding;

    CPDF_FormNotify*		m_pFormNotify;

    FX_BOOL					m_bUpdated;
    friend class CPDF_FormControl;
    friend class CPDF_FormField;
};
#define FORMFIELD_READONLY		0x01
#define FORMFIELD_REQUIRED		0x02
#define FORMFIELD_NOEXPORT		0x04
#define FORMRADIO_NOTOGGLEOFF	0x100
#define FORMRADIO_UNISON		0x200
#define FORMTEXT_MULTILINE		0x100
#define FORMTEXT_PASSWORD		0x200
#define FORMTEXT_NOSCROLL		0x400
#define FORMTEXT_COMB			0x800
#define FORMCOMBO_EDIT			0x100
#define FORMLIST_MULTISELECT	0x100
class CPDF_FormField : public CFX_Object
{
public:

    enum Type {
        Unknown,
        PushButton,
        RadioButton,
        CheckBox,
        Text,
        RichText,
        File,
        ListBox,
        ComboBox,
        Sign
    };

    CFX_WideString			GetFullName();

    Type					GetType()
    {
        return m_Type;
    }

    FX_DWORD				GetFlags()
    {
        return m_Flags;
    }

    CPDF_InterForm*			GetInterForm() const
    {
        return m_pForm;
    }

    CPDF_Dictionary*		GetFieldDict() const
    {
        return m_pDict;
    }

    void					SetFieldDict(CPDF_Dictionary* pDict)
    {
        m_pDict = pDict;
    }

    FX_BOOL					ResetField(FX_BOOL bNotify = FALSE);



    int						CountControls()
    {
        return m_ControlList.GetSize();
    }

    CPDF_FormControl*		GetControl(int index)
    {
        return (CPDF_FormControl*)m_ControlList.GetAt(index);
    }

    int						GetControlIndex(const CPDF_FormControl* pControl);




    int						GetFieldType();




    CPDF_AAction			GetAdditionalAction();




    CFX_WideString			GetAlternateName();




    CFX_WideString			GetMappingName();




    FX_DWORD				GetFieldFlags();




    CFX_ByteString			GetDefaultStyle();




    CFX_WideString			GetRichTextString();



    CFX_WideString			GetValue();

    CFX_WideString			GetDefaultValue();

    FX_BOOL					SetValue(const CFX_WideString& value, FX_BOOL bNotify = FALSE);





    int						GetMaxLen();




    int						CountSelectedItems();

    int						GetSelectedIndex(int index);

    FX_BOOL					ClearSelection(FX_BOOL bNotify = FALSE);

    FX_BOOL					IsItemSelected(int index);

    FX_BOOL					SetItemSelection(int index, FX_BOOL bSelected, FX_BOOL bNotify = FALSE);

    FX_BOOL					IsItemDefaultSelected(int index);

    int						GetDefaultSelectedItem();




    int						CountOptions();

    CFX_WideString			GetOptionLabel(int index);

    CFX_WideString			GetOptionValue(int index);

    int						FindOption(CFX_WideString csOptLabel);

    int						FindOptionValue(FX_LPCWSTR csOptValue, int iStartIndex = 0);




    FX_BOOL					CheckControl(int iControlIndex, FX_BOOL bChecked, FX_BOOL bNotify = FALSE);




    int						GetTopVisibleIndex();




    int						CountSelectedOptions();

    int						GetSelectedOptionIndex(int index);

    FX_BOOL					IsOptionSelected(int iOptIndex);

    FX_BOOL					SelectOption(int iOptIndex, FX_BOOL bSelected, FX_BOOL bNotify = FALSE);

    FX_BOOL					ClearSelectedOptions(FX_BOOL bNotify = FALSE);




    FX_FLOAT				GetFontSize()
    {
        return m_FontSize;
    }

    CPDF_Font*				GetFont()
    {
        return m_pFont;
    }

protected:

    CPDF_FormField(CPDF_InterForm* pForm, CPDF_Dictionary* pDict);

    ~CPDF_FormField();

    CPDF_FormField::Type	m_Type;

    FX_DWORD				m_Flags;

    CPDF_InterForm*			m_pForm;

    CPDF_Dictionary*		m_pDict;

    CFX_PtrArray			m_ControlList;
    friend class			CPDF_InterForm;
    friend class			CPDF_FormControl;



    CFX_WideString			GetValue(FX_BOOL bDefault);

    FX_BOOL					SetValue(const CFX_WideString& value, FX_BOOL bDefault, FX_BOOL bNotify);


    void					SyncFieldFlags();

    int						FindListSel(CPDF_String* str);

    CFX_WideString			GetOptionText(int index, int sub_index);

    void					LoadDA();

    void					UpdateAP(CPDF_FormControl* pControl);



    CFX_WideString			GetCheckValue(FX_BOOL bDefault);

    FX_BOOL					SetCheckValue(const CFX_WideString& value, FX_BOOL bDefault, FX_BOOL bNotify);


    FX_FLOAT				m_FontSize;

    CPDF_Font*				m_pFont;
};
CPDF_Object*	FPDF_GetFieldAttr(CPDF_Dictionary* pFieldDict, const FX_CHAR* name, int nLevel = 0);
class CPDF_IconFit : public CFX_Object
{
public:

    CPDF_IconFit(CPDF_Dictionary* pDict = NULL)
    {
        m_pDict = pDict;
    }

    operator CPDF_Dictionary*() const
    {
        return m_pDict;
    }




    enum ScaleMethod {
        Always = 0,
        Bigger,
        Smaller,
        Never
    };

    ScaleMethod				GetScaleMethod();




    FX_BOOL					IsProportionalScale();




    void					GetIconPosition(FX_FLOAT& fLeft, FX_FLOAT& fBottom);




    FX_BOOL					GetFittingBounds();


    CPDF_Dictionary*		m_pDict;
};

#define TEXTPOS_CAPTION		0
#define TEXTPOS_ICON		1
#define TEXTPOS_BELOW		2
#define TEXTPOS_ABOVE		3
#define TEXTPOS_RIGHT		4
#define TEXTPOS_LEFT		5
#define TEXTPOS_OVERLAID	6
class CPDF_FormControl : public CFX_Object
{
public:

    CPDF_FormField::Type	GetType()
    {
        return m_pField->GetType();
    }

    CPDF_InterForm*			GetInterForm() const
    {
        return m_pForm;
    }

    CPDF_FormField*			GetField() const
    {
        return m_pField;
    }

    CPDF_Dictionary*		GetWidget() const
    {
        return m_pWidgetDict;
    }

    CFX_FloatRect			GetRect();

    void					DrawControl(CFX_RenderDevice* pDevice, CFX_AffineMatrix* pMatrix,
                                        CPDF_Page* pPage, CPDF_Annot::AppearanceMode mode, const CPDF_RenderOptions* pOptions = NULL);



    CFX_ByteString			GetCheckedAPState();

    CFX_WideString			GetExportValue();

    FX_BOOL					IsChecked();

    FX_BOOL					IsDefaultChecked();




    enum HighlightingMode	{
        None = 0,
        Invert,
        Outline,
        Push,
        Toggle
    };

    HighlightingMode		GetHighlightingMode();




    FX_BOOL					HasMKEntry(CFX_ByteString csEntry);




    int						GetRotation();




    inline FX_ARGB			GetBorderColor(int& iColorType)
    {
        return GetColor(iColorType, "BC");
    }

    inline FX_FLOAT			GetOriginalBorderColor(int index)
    {
        return GetOriginalColor(index, "BC");
    }

    inline void				GetOriginalBorderColor(int& iColorType, FX_FLOAT fc[4])
    {
        GetOriginalColor(iColorType, fc, "BC");
    }




    inline FX_ARGB			GetBackgroundColor(int& iColorType)
    {
        return GetColor(iColorType, "BG");
    }

    inline FX_FLOAT			GetOriginalBackgroundColor(int index)
    {
        return GetOriginalColor(index, "BG");
    }

    inline void				GetOriginalBackgroundColor(int& iColorType, FX_FLOAT fc[4])
    {
        GetOriginalColor(iColorType, fc, "BG");
    }




    inline CFX_WideString	GetNormalCaption()
    {
        return GetCaption("CA");
    }




    inline CFX_WideString	GetRolloverCaption()
    {
        return GetCaption("RC");
    }




    inline CFX_WideString	GetDownCaption()
    {
        return GetCaption("AC");
    }




    inline CPDF_Stream*		GetNormalIcon()
    {
        return GetIcon("I");
    }




    inline CPDF_Stream*		GetRolloverIcon()
    {
        return GetIcon("RI");
    }




    inline CPDF_Stream*		GetDownIcon()
    {
        return GetIcon("IX");
    }




    CPDF_IconFit			GetIconFit();




    int						GetTextPosition();




    CPDF_Action				GetAction();




    CPDF_AAction			GetAdditionalAction();




    CPDF_DefaultAppearance	GetDefaultAppearance();

    CPDF_Font*				GetDefaultControlFont();




    int						GetControlAlignment();

protected:

    CPDF_FormControl(CPDF_FormField* pField, CPDF_Dictionary* pWidgetDict);

    CFX_ByteString			GetOnStateName();

    void					SetOnStateName(const CFX_ByteString& csOn);

    void					CheckControl(FX_BOOL bChecked);

    FX_ARGB					GetColor(int& iColorType, CFX_ByteString csEntry);

    FX_FLOAT				GetOriginalColor(int index, CFX_ByteString csEntry);

    void					GetOriginalColor(int& iColorType, FX_FLOAT fc[4], CFX_ByteString csEntry);

    CFX_WideString			GetCaption(CFX_ByteString csEntry);

    CPDF_Stream*			GetIcon(CFX_ByteString csEntry);

    CPDF_ApSettings			GetMK(FX_BOOL bCreate);

    CPDF_InterForm*			m_pForm;

    CPDF_FormField*			m_pField;

    CPDF_Dictionary*		m_pWidgetDict;
    friend class			CPDF_InterForm;
    friend class			CPDF_FormField;
};
class CPDF_FormNotify : public CFX_Object
{
public:

    virtual ~CPDF_FormNotify() {}

    virtual int		BeforeValueChange(const CPDF_FormField* pField, CFX_WideString& csValue)
    {
        return 0;
    }

    virtual int		AfterValueChange(const CPDF_FormField* pField)
    {
        return 0;
    }

    virtual int		BeforeSelectionChange(const CPDF_FormField* pField, CFX_WideString& csValue)
    {
        return 0;
    }

    virtual int		AfterSelectionChange(const CPDF_FormField* pField)
    {
        return 0;
    }

    virtual int		AfterCheckedStatusChange(const CPDF_FormField* pField, const CFX_ByteArray& statusArray)
    {
        return 0;
    }

    virtual int		BeforeFormReset(const CPDF_InterForm* pForm)
    {
        return 0;
    }

    virtual int		AfterFormReset(const CPDF_InterForm* pForm)
    {
        return 0;
    }

    virtual int		BeforeFormImportData(const CPDF_InterForm* pForm)
    {
        return 0;
    }

    virtual int		AfterFormImportData(const CPDF_InterForm* pForm)
    {
        return 0;
    }
};
FX_BOOL		FPDF_GenerateAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict);
class CPDF_PageLabel : public CFX_Object
{
public:

    CPDF_PageLabel(CPDF_Document* pDocument)
    {
        m_pDocument = pDocument;
    }


    CFX_WideString					GetLabel(int nPage) const;

    FX_INT32						GetPageByLabel(FX_BSTR bsLabel) const;


    FX_INT32						GetPageByLabel(FX_WSTR wsLabel) const;

protected:
    CPDF_Document*					m_pDocument;
};
class CPDF_Metadata
{
public:

    CPDF_Metadata();


    ~CPDF_Metadata();

    void				LoadDoc(CPDF_Document *pDoc);


    FX_INT32			GetString(FX_BSTR bsItem, CFX_WideString &wsStr);

    CXML_Element*		GetRoot() const;

    CXML_Element*		GetRDF() const;

protected:
    FX_LPVOID	m_pData;
};
class CPDF_ViewerPreferences
{
public:

    CPDF_ViewerPreferences(CPDF_Document *pDoc);


    ~CPDF_ViewerPreferences();


    FX_BOOL IsDirectionR2L() const;

    FX_BOOL PrintScaling() const;

    FX_INT32 NumCopies() const;

    CPDF_Array* PrintPageRange() const;

    CFX_ByteString Duplex() const;

protected:
    CPDF_Document*	m_pDoc;
};
class CPDF_ApSettings : public CFX_Object
{
public:

    CPDF_ApSettings(CPDF_Dictionary* pDict = NULL)
    {
        m_pDict = pDict;
    }

    operator CPDF_Dictionary* () const
    {
        return m_pDict;
    }

    FX_BOOL					HasMKEntry(FX_BSTR csEntry);



    int						GetRotation();




    inline FX_ARGB			GetBorderColor(int& iColorType)
    {
        return GetColor(iColorType, FX_BSTRC("BC"));
    }

    inline FX_FLOAT			GetOriginalBorderColor(int index)
    {
        return GetOriginalColor(index, FX_BSTRC("BC"));
    }

    inline void				GetOriginalBorderColor(int& iColorType, FX_FLOAT fc[4])
    {
        GetOriginalColor(iColorType, fc, FX_BSTRC("BC"));
    }




    inline FX_ARGB			GetBackgroundColor(int& iColorType)
    {
        return GetColor(iColorType, FX_BSTRC("BG"));
    }

    inline FX_FLOAT			GetOriginalBackgroundColor(int index)
    {
        return GetOriginalColor(index, FX_BSTRC("BG"));
    }

    inline void				GetOriginalBackgroundColor(int& iColorType, FX_FLOAT fc[4])
    {
        GetOriginalColor(iColorType, fc, FX_BSTRC("BG"));
    }




    inline CFX_WideString	GetNormalCaption()
    {
        return GetCaption(FX_BSTRC("CA"));
    }




    inline CFX_WideString	GetRolloverCaption()
    {
        return GetCaption(FX_BSTRC("RC"));
    }




    inline CFX_WideString	GetDownCaption()
    {
        return GetCaption(FX_BSTRC("AC"));
    }




    inline CPDF_Stream*		GetNormalIcon()
    {
        return GetIcon(FX_BSTRC("I"));
    }




    inline CPDF_Stream*		GetRolloverIcon()
    {
        return GetIcon(FX_BSTRC("RI"));
    }




    inline CPDF_Stream*		GetDownIcon()
    {
        return GetIcon(FX_BSTRC("IX"));
    }




    CPDF_IconFit			GetIconFit();




    int						GetTextPosition();

    CPDF_Dictionary*		m_pDict;
protected:

    FX_ARGB					GetColor(int& iColorType, FX_BSTR csEntry);

    FX_FLOAT				GetOriginalColor(int index, FX_BSTR csEntry);

    void					GetOriginalColor(int& iColorType, FX_FLOAT fc[4], FX_BSTR csEntry);

    CFX_WideString			GetCaption(FX_BSTR csEntry);

    CPDF_Stream*			GetIcon(FX_BSTR csEntry);
    friend class			CPDF_FormControl;
};
#endif
