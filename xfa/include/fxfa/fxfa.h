// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXFA_H_
#define FXFA_H_

class CFX_Graphics;
class CPDF_Document;
class CXFA_Node;
class CXFA_NodeList;
class CXFA_WidgetAcc;
class IFDE_XMLElement;
class IFWL_AdapterTimerMgr;
class IFX_Font;
class IXFA_App;
class IXFA_AppProvider;
class IXFA_ChecksumContext;
class IXFA_DocHandler;
class IXFA_DocProvider;
class IXFA_DocView;
class IXFA_FontMgr;
class IXFA_MenuHandler;
class IXFA_PageView;
class IXFA_PageViewRender;
class IXFA_WidgetAccIterator;
class IXFA_WidgetHandler;
class IXFA_WidgetIterator;

class IXFA_Doc {
 public:
  virtual ~IXFA_Doc() {}

 protected:
  IXFA_Doc() {}
};

class IXFA_Widget {
 public:
  virtual ~IXFA_Widget() {}

 protected:
  IXFA_Widget() {}
};

#include "fxfa_basic.h"
#include "fxfa_widget.h"
#define XFA_MBICON_Error 0
#define XFA_MBICON_Warning 1
#define XFA_MBICON_Question 2
#define XFA_MBICON_Status 3
#define XFA_MB_OK 0
#define XFA_MB_OKCancel 1
#define XFA_MB_YesNo 2
#define XFA_MB_YesNoCancel 3
#define XFA_IDOK 1
#define XFA_IDCancel 2
#define XFA_IDNo 3
#define XFA_IDYes 4
#define XFA_IDS_ValidateFailed 1
#define XFA_IDS_CalcOverride 2
#define XFA_IDS_ModifyField 3
#define XFA_IDS_NotModifyField 4
#define XFA_IDS_AppName 5
#define XFA_IDS_ImageFilter 6
#define XFA_IDS_UNKNOW_CATCHED 7
#define XFA_IDS_Unable_TO_SET 8
#define XFA_IDS_VALUE_EXCALMATORY 9
#define XFA_IDS_INVALID_ENUM_VALUE 10
#define XFA_IDS_UNSUPPORT_METHOD 11
#define XFA_IDS_UNSUPPORT_PROP 12
#define XFA_IDS_INVAlID_PROP_SET 13
#define XFA_IDS_NOT_DEFAUL_VALUE 14
#define XFA_IDS_UNABLE_SET_LANGUAGE 15
#define XFA_IDS_UNABLE_SET_NUMPAGES 16
#define XFA_IDS_UNABLE_SET_PLATFORM 17
#define XFA_IDS_UNABLE_SET_VALIDATIONENABLE 18
#define XFA_IDS_UNABLE_SET_VARIATION 19
#define XFA_IDS_UNABLE_SET_VERSION 20
#define XFA_IDS_UNABLE_SET_READY 21
#define XFA_IDS_NUMBER_OF_OCCUR 22
#define XFA_IDS_UNABLE_SET_CLASS_NAME 23
#define XFA_IDS_UNABLE_SET_LENGTH_VALUE 24
#define XFA_IDS_UNSUPPORT_CHAR 25
#define XFA_IDS_BAD_SUFFIX 26
#define XFA_IDS_EXPECTED_IDENT 27
#define XFA_IDS_EXPECTED_STRING 28
#define XFA_IDS_INVALIDATE_CHAR 29
#define XFA_IDS_REDEFINITION 30
#define XFA_IDS_INVALIDATE_TOKEN 31
#define XFA_IDS_INVALIDATE_EXPRESSION 32
#define XFA_IDS_UNDEFINE_IDENTIFIER 33
#define XFA_IDS_INVALIDATE_LEFTVALUE 34
#define XFA_IDS_COMPILER_ERROR 35
#define XFA_IDS_CANNOT_MODIFY_VALUE 36
#define XFA_IDS_ERROR_PARAMETERS 37
#define XFA_IDS_EXPECT_ENDIF 38
#define XFA_IDS_UNEXPECTED_EXPRESSION 39
#define XFA_IDS_CONDITION_IS_NULL 40
#define XFA_IDS_ILLEGALBREAK 41
#define XFA_IDS_ILLEGALCONTINUE 42
#define XFA_IDS_EXPECTED_OPERATOR 43
#define XFA_IDS_DIVIDE_ZERO 44
#define XFA_IDS_CANNOT_COVERT_OBJECT 45
#define XFA_IDS_NOT_FOUND_CONTAINER 46
#define XFA_IDS_NOT_FOUND_PROPERTY 47
#define XFA_IDS_NOT_FOUND_METHOD 48
#define XFA_IDS_NOT_FOUND_CONST 49
#define XFA_IDS_NOT_ASSIGN_OBJECT 50
#define XFA_IDS_IVALIDATE_INSTRUCTION 51
#define XFA_IDS_EXPECT_NUMBER 52
#define XFA_IDS_VALIDATE_OUT_ARRAY 53
#define XFA_IDS_CANNOT_ASSIGN_IDENT 54
#define XFA_IDS_NOT_FOUNT_FUNCTION 55
#define XFA_IDS_NOT_ARRAY 56
#define XFA_IDS_OUT_ARRAY 57
#define XFA_IDS_NOT_SUPPORT_CALC 58
#define XFA_IDS_ARGUMENT_NOT_ARRAY 59
#define XFA_IDS_ARGUMENT_EXPECT_CONTAINER 60
#define XFA_IDS_ACCESS_PROPERTY_IN_NOT_OBJECT 61
#define XFA_IDS_FUNCTION_IS_BUILDIN 62
#define XFA_IDS_ERROR_MSG 63
#define XFA_IDS_INDEX_OUT_OF_BOUNDS 64
#define XFA_IDS_INCORRECT_NUMBER_OF_METHOD 65
#define XFA_IDS_ARGUMENT_MISMATCH 66
#define XFA_IDS_INVALID_ENUMERATE 67
#define XFA_IDS_INVALID_APPEND 68
#define XFA_IDS_SOM_EXPECTED_LIST 69
#define XFA_IDS_NOT_HAVE_PROPERTY 70
#define XFA_IDS_INVALID_NODE_TYPE 71
#define XFA_IDS_VIOLATE_BOUNDARY 72
#define XFA_IDS_SERVER_DENY 73
#define XFA_IDS_StringWeekDay_Sun 74
#define XFA_IDS_StringWeekDay_Mon 75
#define XFA_IDS_StringWeekDay_Tue 76
#define XFA_IDS_StringWeekDay_Wed 77
#define XFA_IDS_StringWeekDay_Thu 78
#define XFA_IDS_StringWeekDay_Fri 79
#define XFA_IDS_StringWeekDay_Sat 80
#define XFA_IDS_StringMonth_Jan 81
#define XFA_IDS_StringMonth_Feb 82
#define XFA_IDS_StringMonth_March 83
#define XFA_IDS_StringMonth_April 84
#define XFA_IDS_StringMonth_May 85
#define XFA_IDS_StringMonth_June 86
#define XFA_IDS_StringMonth_July 87
#define XFA_IDS_StringMonth_Aug 88
#define XFA_IDS_StringMonth_Sept 89
#define XFA_IDS_StringMonth_Oct 90
#define XFA_IDS_StringMonth_Nov 91
#define XFA_IDS_StringMonth_Dec 92
#define XFA_IDS_String_Today 93
#define XFA_IDS_ValidateLimit 94
#define XFA_IDS_ValidateNullWarning 95
#define XFA_IDS_ValidateNullError 96
#define XFA_IDS_ValidateWarning 97
#define XFA_IDS_ValidateError 98
#define XFA_IDS_ValidateNumberError 99

// Probably should be called IXFA_AppDelegate.
class IXFA_AppProvider {
 public:
  virtual ~IXFA_AppProvider() {}

  /**
   * Specifies the name of the client application in which a form currently
   * exists. Such as Exchange-Pro.
   */
  virtual void SetAppType(const CFX_WideStringC& wsAppType) = 0;
  virtual void GetAppType(CFX_WideString& wsAppType) = 0;
  virtual void SetFoxitAppType(const CFX_WideStringC& wsFoxitAppType) {}
  virtual void GetFoxitAppType(CFX_WideString& wsFoxitAppType) {
    wsFoxitAppType.Empty();
  }

  /**
   * Returns the language of the running host application. Such as zh_CN
   */
  virtual void GetLanguage(CFX_WideString& wsLanguage) = 0;

  /**
   * Returns the platform of the machine running the script. Such as WIN
   */
  virtual void GetPlatform(CFX_WideString& wsPlatform) = 0;

  /**
   * Indicates the packaging of the application that is running the script. Such
   * as Full
   */
  virtual void GetVariation(CFX_WideString& wsVariation) = 0;

  /**
   * Indicates the version number of the current application. Such as 9
   */
  virtual void GetVersion(CFX_WideString& wsVersion) = 0;
  virtual void GetFoxitVersion(CFX_WideString& wsFoxitVersion) {
    wsFoxitVersion.Empty();
  }

  /**
   * Get application name, such as Phantom.
   */
  virtual void GetAppName(CFX_WideString& wsName) = 0;
  virtual void GetFoxitAppName(CFX_WideString& wsFoxitName) {
    wsFoxitName.Empty();
  }

  /**
   * Causes the system to play a sound.
   * @param[in] dwType The system code for the appropriate sound.0 (Error)1
   * (Warning)2 (Question)3 (Status)4 (Default)
   */
  virtual void Beep(FX_DWORD dwType) = 0;

  /**
   * Displays a message box.
   * @param[in] dwIconType    Icon type, refer to XFA_MBICON.
   * @param[in] dwButtonType  Button type, refer to XFA_MESSAGEBUTTON.
   * @return A valid integer representing the value of the button pressed by the
   * user, refer to XFA_ID.
   */
  virtual int32_t MsgBox(const CFX_WideStringC& wsMessage,
                         const CFX_WideStringC& wsTitle = FX_WSTRC(L""),
                         FX_DWORD dwIconType = 0,
                         FX_DWORD dwButtonType = 0) = 0;

  /**
   * Get a response from the user.
   * @param[in] bMark - Mask the user input with * (asterisks) when true,
   */
  virtual void Response(CFX_WideString& wsAnswer,
                        const CFX_WideStringC& wsQuestion,
                        const CFX_WideStringC& wsTitle = FX_WSTRC(L""),
                        const CFX_WideStringC& wsDefaultAnswer = FX_WSTRC(L""),
                        FX_BOOL bMark = TRUE) = 0;

  virtual int32_t GetDocumentCountInBatch() = 0;
  virtual int32_t GetCurDocumentInBatch() = 0;

  /**
   * Download something from somewhere.
   * @param[in] wsURL - http, ftp, such as
   * "http://www.w3.org/TR/REC-xml-names/".
   */
  virtual IFX_FileRead* DownloadURL(const CFX_WideStringC& wsURL) = 0;

  /**
   * POST data to the given url.
   * @param[in] wsURL         the URL being uploaded.
   * @param[in] wsData        the data being uploaded.
   * @param[in] wsContentType the content type of data including text/html,
   * text/xml, text/plain, multipart/form-data,
   *                          application/x-www-form-urlencoded,
   * application/octet-stream, any valid MIME type.
   * @param[in] wsEncode      the encode of data including UTF-8, UTF-16,
   * ISO8859-1, any recognized [IANA]character encoding
   * @param[in] wsHeader      any additional HTTP headers to be included in the
   * post.
   * @param[out] wsResponse   decoded response from server.
   * @return TRUE Server permitted the post request, FALSE otherwise.
   */
  virtual FX_BOOL PostRequestURL(const CFX_WideStringC& wsURL,
                                 const CFX_WideStringC& wsData,
                                 const CFX_WideStringC& wsContentType,
                                 const CFX_WideStringC& wsEncode,
                                 const CFX_WideStringC& wsHeader,
                                 CFX_WideString& wsResponse) = 0;

  /**
   * PUT data to the given url.
   * @param[in] wsURL         the URL being uploaded.
   * @param[in] wsData            the data being uploaded.
   * @param[in] wsEncode      the encode of data including UTF-8, UTF-16,
   * ISO8859-1, any recognized [IANA]character encoding
   * @return TRUE Server permitted the post request, FALSE otherwise.
   */
  virtual FX_BOOL PutRequestURL(const CFX_WideStringC& wsURL,
                                const CFX_WideStringC& wsData,
                                const CFX_WideStringC& wsEncode) = 0;

  virtual void LoadString(int32_t iStringID, CFX_WideString& wsString) = 0;
  virtual FX_BOOL ShowFileDialog(const CFX_WideStringC& wsTitle,
                                 const CFX_WideStringC& wsFilter,
                                 CFX_WideStringArray& wsPathArr,
                                 FX_BOOL bOpen = TRUE) = 0;
  virtual IFWL_AdapterTimerMgr* GetTimerMgr() = 0;
};
class IXFA_FontMgr {
 public:
  static IXFA_FontMgr* CreateDefault();
  virtual ~IXFA_FontMgr();

  virtual IFX_Font* GetFont(IXFA_Doc* hDoc,
                            const CFX_WideStringC& wsFontFamily,
                            FX_DWORD dwFontStyles,
                            FX_WORD wCodePage = 0xFFFF) = 0;
  virtual IFX_Font* GetDefaultFont(IXFA_Doc* hDoc,
                                   const CFX_WideStringC& wsFontFamily,
                                   FX_DWORD dwFontStyles,
                                   FX_WORD wCodePage = 0xFFFF) = 0;
};
class IXFA_App {
 public:
  static IXFA_App* Create(IXFA_AppProvider* pProvider);
  virtual ~IXFA_App();

  virtual IXFA_DocHandler* GetDocHandler() = 0;
  virtual IXFA_Doc* CreateDoc(IXFA_DocProvider* pProvider,
                              IFX_FileRead* pStream,
                              FX_BOOL bTakeOverFile = TRUE) = 0;
  virtual IXFA_Doc* CreateDoc(IXFA_DocProvider* pProvider,
                              CPDF_Document* pPDFDoc) = 0;
  virtual IXFA_AppProvider* GetAppProvider() = 0;
  virtual void SetDefaultFontMgr(IXFA_FontMgr* pFontMgr) = 0;
  virtual IXFA_MenuHandler* GetMenuHandler() = 0;
};
class IXFA_MenuHandler {
 public:
  virtual ~IXFA_MenuHandler() {}

  virtual FX_BOOL CanCopy(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL CanCut(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL CanPaste(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL CanSelectAll(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL CanDelete(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL CanDeSelect(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL Copy(IXFA_Widget* hWidget, CFX_WideString& wsText) = 0;
  virtual FX_BOOL Cut(IXFA_Widget* hWidget, CFX_WideString& wsText) = 0;
  virtual FX_BOOL Paste(IXFA_Widget* hWidget, const CFX_WideString& wsText) = 0;
  virtual FX_BOOL SelectAll(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL Delete(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL DeSelect(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL CanUndo(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL CanRedo(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL Undo(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL Redo(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL GetSuggestWords(IXFA_Widget* hWidget,
                                  CFX_PointF pointf,
                                  CFX_ByteStringArray& sSuggest) = 0;
  virtual FX_BOOL ReplaceSpellCheckWord(IXFA_Widget* hWidget,
                                        CFX_PointF pointf,
                                        const CFX_ByteStringC& bsReplace) = 0;
};
#define XFA_INVALIDATE_AllPages 0x00000000
#define XFA_INVALIDATE_CurrentPage 0x00000001
#define XFA_PRINTOPT_ShowDialog 0x00000001
#define XFA_PRINTOPT_CanCancel 0x00000002
#define XFA_PRINTOPT_ShrinkPage 0x00000004
#define XFA_PRINTOPT_AsImage 0x00000008
#define XFA_PRINTOPT_ReverseOrder 0x00000010
#define XFA_PRINTOPT_PrintAnnot 0x00000020
#define XFA_PAGEVIEWEVENT_PostAdded 1
#define XFA_PAGEVIEWEVENT_PostRemoved 3
#define XFA_WIDGETEVENT_PostAdded 2
#define XFA_WIDGETEVENT_PreRemoved 3
#define XFA_WIDGETEVENT_PostContentChanged 6
#define XFA_WIDGETEVENT_ListItemRemoved 7
#define XFA_WIDGETEVENT_ListItemAdded 8
#define XFA_WIDGETEVENT_AccessChanged 9
class IXFA_DocProvider {
 public:
  virtual ~IXFA_DocProvider() {}

  virtual void SetChangeMark(IXFA_Doc* hDoc) = 0;
  virtual void InvalidateRect(IXFA_PageView* pPageView,
                              const CFX_RectF& rt,
                              FX_DWORD dwFlags = 0) = 0;
  virtual void DisplayCaret(IXFA_Widget* hWidget,
                            FX_BOOL bVisible,
                            const CFX_RectF* pRtAnchor) = 0;
  virtual FX_BOOL GetPopupPos(IXFA_Widget* hWidget,
                              FX_FLOAT fMinPopup,
                              FX_FLOAT fMaxPopup,
                              const CFX_RectF& rtAnchor,
                              CFX_RectF& rtPopup) = 0;
  virtual FX_BOOL PopupMenu(IXFA_Widget* hWidget,
                            CFX_PointF ptPopup,
                            const CFX_RectF* pRectExclude = NULL) = 0;
  virtual void PageViewEvent(IXFA_PageView* pPageView, FX_DWORD dwFlags) = 0;
  virtual void WidgetEvent(IXFA_Widget* hWidget,
                           CXFA_WidgetAcc* pWidgetData,
                           FX_DWORD dwEvent,
                           void* pParam = NULL,
                           void* pAdditional = NULL) = 0;
  virtual FX_BOOL RenderCustomWidget(IXFA_Widget* hWidget,
                                     CFX_Graphics* pGS,
                                     CFX_Matrix* pMatrix,
                                     const CFX_RectF& rtUI) {
    return FALSE;
  }
  virtual int32_t CountPages(IXFA_Doc* hDoc) = 0;
  virtual int32_t GetCurrentPage(IXFA_Doc* hDoc) = 0;
  virtual void SetCurrentPage(IXFA_Doc* hDoc, int32_t iCurPage) = 0;
  virtual FX_BOOL IsCalculationsEnabled(IXFA_Doc* hDoc) = 0;
  virtual void SetCalculationsEnabled(IXFA_Doc* hDoc, FX_BOOL bEnabled) = 0;
  virtual void GetTitle(IXFA_Doc* hDoc, CFX_WideString& wsTitle) = 0;
  virtual void SetTitle(IXFA_Doc* hDoc, const CFX_WideStringC& wsTitle) = 0;
  virtual void ExportData(IXFA_Doc* hDoc,
                          const CFX_WideStringC& wsFilePath,
                          FX_BOOL bXDP = TRUE) = 0;
  virtual void ImportData(IXFA_Doc* hDoc,
                          const CFX_WideStringC& wsFilePath) = 0;
  virtual void GotoURL(IXFA_Doc* hDoc,
                       const CFX_WideStringC& bsURL,
                       FX_BOOL bAppend = TRUE) = 0;
  virtual FX_BOOL IsValidationsEnabled(IXFA_Doc* hDoc) = 0;
  virtual void SetValidationsEnabled(IXFA_Doc* hDoc, FX_BOOL bEnabled) = 0;
  virtual void SetFocusWidget(IXFA_Doc* hDoc, IXFA_Widget* hWidget) = 0;
  virtual void Print(IXFA_Doc* hDoc,
                     int32_t nStartPage,
                     int32_t nEndPage,
                     FX_DWORD dwOptions) = 0;
  virtual int32_t AbsPageCountInBatch(IXFA_Doc* hDoc) = 0;
  virtual int32_t AbsPageInBatch(IXFA_Doc* hDoc, IXFA_Widget* hWidget) = 0;
  virtual int32_t SheetCountInBatch(IXFA_Doc* hDoc) = 0;
  virtual int32_t SheetInBatch(IXFA_Doc* hDoc, IXFA_Widget* hWidget) = 0;
  virtual int32_t Verify(IXFA_Doc* hDoc,
                         CXFA_Node* pSigNode,
                         FX_BOOL bUsed = TRUE) {
    return 0;
  }
  virtual FX_BOOL Sign(IXFA_Doc* hDoc,
                       CXFA_NodeList* pNodeList,
                       const CFX_WideStringC& wsExpression,
                       const CFX_WideStringC& wsXMLIdent,
                       const CFX_WideStringC& wsValue = FX_WSTRC(L"open"),
                       FX_BOOL bUsed = TRUE) {
    return 0;
  }
  virtual CXFA_NodeList* Enumerate(IXFA_Doc* hDoc) { return 0; }
  virtual FX_BOOL Clear(IXFA_Doc* hDoc,
                        CXFA_Node* pSigNode,
                        FX_BOOL bCleared = TRUE) {
    return 0;
  }
  virtual void GetURL(IXFA_Doc* hDoc, CFX_WideString& wsDocURL) = 0;
  virtual FX_ARGB GetHighlightColor(IXFA_Doc* hDoc) = 0;
  virtual void AddDoRecord(IXFA_Widget* hWidget) = 0;

  virtual FX_BOOL SubmitData(IXFA_Doc* hDoc, CXFA_Submit submit) = 0;
  virtual FX_BOOL CheckWord(IXFA_Doc* hDoc, const CFX_ByteStringC& sWord) = 0;
  virtual FX_BOOL GetSuggestWords(IXFA_Doc* hDoc,
                                  const CFX_ByteStringC& sWord,
                                  CFX_ByteStringArray& sSuggest) = 0;
  virtual FX_BOOL GetPDFScriptObject(IXFA_Doc* hDoc,
                                     const CFX_ByteStringC& utf8Name,
                                     FXJSE_HVALUE hValue) = 0;
  virtual FX_BOOL GetGlobalProperty(IXFA_Doc* hDoc,
                                    const CFX_ByteStringC& szPropName,
                                    FXJSE_HVALUE hValue) = 0;
  virtual FX_BOOL SetGlobalProperty(IXFA_Doc* hDoc,
                                    const CFX_ByteStringC& szPropName,
                                    FXJSE_HVALUE hValue) = 0;
  virtual CPDF_Document* OpenPDF(IXFA_Doc* hDoc,
                                 IFX_FileRead* pFile,
                                 FX_BOOL bTakeOverFile) = 0;
  virtual IFX_FileRead* OpenLinkedFile(IXFA_Doc* hDoc,
                                       const CFX_WideString& wsLink) = 0;
};
#define XFA_DOCVIEW_View 0x00000000
#define XFA_DOCVIEW_MasterPage 0x00000001
#define XFA_DOCVIEW_Design 0x00000002
#define XFA_DOCTYPE_Dynamic 0
#define XFA_DOCTYPE_Static 1
#define XFA_DOCTYPE_XDP 2
#define XFA_PARSESTATUS_StatusErr -3
#define XFA_PARSESTATUS_StreamErr -2
#define XFA_PARSESTATUS_SyntaxErr -1
#define XFA_PARSESTATUS_Ready 0
#define XFA_PARSESTATUS_Done 100
class IXFA_DocHandler {
 public:
  virtual ~IXFA_DocHandler() {}

  virtual void ReleaseDoc(IXFA_Doc* hDoc) = 0;
  virtual IXFA_DocProvider* GetDocProvider(IXFA_Doc* hDoc) = 0;

  virtual FX_DWORD GetDocType(IXFA_Doc* hDoc) = 0;
  virtual int32_t StartLoad(IXFA_Doc* hDoc) = 0;
  virtual int32_t DoLoad(IXFA_Doc* hDoc, IFX_Pause* pPause = NULL) = 0;
  virtual void StopLoad(IXFA_Doc* hDoc) = 0;

  virtual IXFA_DocView* CreateDocView(IXFA_Doc* hDoc, FX_DWORD dwView = 0) = 0;

  virtual int32_t CountPackages(IXFA_Doc* hDoc) = 0;
  virtual void GetPackageName(IXFA_Doc* hDoc,
                              int32_t iPackage,
                              CFX_WideStringC& wsPackage) = 0;

  virtual FX_BOOL SavePackage(IXFA_Doc* hDoc,
                              const CFX_WideStringC& wsPackage,
                              IFX_FileWrite* pFile,
                              IXFA_ChecksumContext* pCSContext = NULL) = 0;
  virtual FX_BOOL CloseDoc(IXFA_Doc* hDoc) = 0;

  virtual FX_BOOL ImportData(IXFA_Doc* hDoc,
                             IFX_FileRead* pStream,
                             FX_BOOL bXDP = TRUE) = 0;
  virtual void SetJSERuntime(IXFA_Doc* hDoc, FXJSE_HRUNTIME hRuntime) = 0;
  virtual FXJSE_HVALUE GetXFAScriptObject(IXFA_Doc* hDoc) = 0;
  virtual XFA_ATTRIBUTEENUM GetRestoreState(IXFA_Doc* hDoc) = 0;
  virtual FX_BOOL RunDocScript(IXFA_Doc* hDoc,
                               XFA_SCRIPTTYPE eScriptType,
                               const CFX_WideStringC& wsScript,
                               FXJSE_HVALUE hRetValue,
                               FXJSE_HVALUE hThisObject) = 0;
};
enum XFA_EVENTTYPE {
  XFA_EVENT_Click,
  XFA_EVENT_Change,
  XFA_EVENT_DocClose,
  XFA_EVENT_DocReady,
  XFA_EVENT_Enter,
  XFA_EVENT_Exit,
  XFA_EVENT_Full,
  XFA_EVENT_IndexChange,
  XFA_EVENT_Initialize,
  XFA_EVENT_MouseDown,
  XFA_EVENT_MouseEnter,
  XFA_EVENT_MouseExit,
  XFA_EVENT_MouseUp,
  XFA_EVENT_PostExecute,
  XFA_EVENT_PostOpen,
  XFA_EVENT_PostPrint,
  XFA_EVENT_PostSave,
  XFA_EVENT_PostSign,
  XFA_EVENT_PostSubmit,
  XFA_EVENT_PreExecute,
  XFA_EVENT_PreOpen,
  XFA_EVENT_PrePrint,
  XFA_EVENT_PreSave,
  XFA_EVENT_PreSign,
  XFA_EVENT_PreSubmit,
  XFA_EVENT_Ready,
  XFA_EVENT_InitCalculate,
  XFA_EVENT_InitVariables,
  XFA_EVENT_Calculate,
  XFA_EVENT_Validate,
  XFA_EVENT_Unknown,
};
#define XFA_VALIDATE_preSubmit 1
#define XFA_VALIDATE_prePrint 2
#define XFA_VALIDATE_preExecute 3
#define XFA_VALIDATE_preSave 4
class CXFA_EventParam {
 public:
  CXFA_EventParam() {
    m_pTarget = NULL;
    m_eType = XFA_EVENT_Unknown;
    m_wsResult.Empty();
    Reset();
  }
  void Reset() {
    m_wsChange.Empty();
    m_iCommitKey = 0;
    m_wsFullText.Empty();
    m_bKeyDown = FALSE;
    m_bModifier = FALSE;
    m_wsNewContentType.Empty();
    m_wsNewText.Empty();
    m_wsPrevContentType.Empty();
    m_wsPrevText.Empty();
    m_bReenter = FALSE;
    m_iSelEnd = 0;
    m_iSelStart = 0;
    m_bShift = FALSE;
    m_wsSoapFaultCode.Empty();
    m_wsSoapFaultString.Empty();
    m_bIsFormReady = FALSE;
    m_iValidateActivities = XFA_VALIDATE_preSubmit;
  }
  CXFA_WidgetAcc* m_pTarget;
  XFA_EVENTTYPE m_eType;
  CFX_WideString m_wsResult;
  FX_BOOL m_bCancelAction;
  int32_t m_iCommitKey;
  FX_BOOL m_bKeyDown;
  FX_BOOL m_bModifier;
  FX_BOOL m_bReenter;
  int32_t m_iSelEnd;
  int32_t m_iSelStart;
  FX_BOOL m_bShift;
  CFX_WideString m_wsChange;
  CFX_WideString m_wsFullText;
  CFX_WideString m_wsNewContentType;
  CFX_WideString m_wsNewText;
  CFX_WideString m_wsPrevContentType;
  CFX_WideString m_wsPrevText;
  CFX_WideString m_wsSoapFaultCode;
  CFX_WideString m_wsSoapFaultString;
  FX_BOOL m_bIsFormReady;
  int32_t m_iValidateActivities;
};
#define XFA_EVENTERROR_Sucess 1
#define XFA_EVENTERROR_Error -1
#define XFA_EVENTERROR_NotExist 0
#define XFA_EVENTERROR_Disabled 2
enum XFA_WIDGETORDER {
  XFA_WIDGETORDER_PreOrder,
};
class IXFA_DocView {
 public:
  virtual ~IXFA_DocView() {}

  virtual IXFA_Doc* GetDoc() = 0;
  virtual int32_t StartLayout(int32_t iStartPage = 0) = 0;
  virtual int32_t DoLayout(IFX_Pause* pPause = NULL) = 0;
  virtual void StopLayout() = 0;

  virtual int32_t GetLayoutStatus() = 0;
  virtual void UpdateDocView() = 0;
  virtual int32_t CountPageViews() = 0;
  virtual IXFA_PageView* GetPageView(int32_t nIndex) = 0;
  virtual IXFA_Widget* GetWidgetByName(const CFX_WideStringC& wsName) = 0;
  virtual CXFA_WidgetAcc* GetWidgetAccByName(const CFX_WideStringC& wsName) = 0;
  virtual void ResetWidgetData(CXFA_WidgetAcc* pWidgetAcc = NULL) = 0;
  virtual int32_t ProcessWidgetEvent(CXFA_EventParam* pParam,
                                     CXFA_WidgetAcc* pWidgetAcc = NULL) = 0;
  virtual IXFA_WidgetHandler* GetWidgetHandler() = 0;
  virtual IXFA_WidgetIterator* CreateWidgetIterator() = 0;
  virtual IXFA_WidgetAccIterator* CreateWidgetAccIterator(
      XFA_WIDGETORDER eOrder = XFA_WIDGETORDER_PreOrder) = 0;
  virtual IXFA_Widget* GetFocusWidget() = 0;
  virtual void KillFocus() = 0;
  virtual FX_BOOL SetFocus(IXFA_Widget* hWidget) = 0;
};
#define XFA_TRAVERSEWAY_Tranvalse 0x0001
#define XFA_TRAVERSEWAY_Form 0x0002
#define XFA_WIDGETFILTER_Visible 0x0001
#define XFA_WIDGETFILTER_Viewable 0x0010
#define XFA_WIDGETFILTER_Printable 0x0020
#define XFA_WIDGETFILTER_Field 0x0100
#define XFA_WIDGETFILTER_AllType 0x0F00
class IXFA_PageView {
 public:
  virtual ~IXFA_PageView() {}

  virtual IXFA_DocView* GetDocView() = 0;
  virtual int32_t GetPageViewIndex() = 0;
  virtual void GetPageViewRect(CFX_RectF& rtPage) = 0;

  virtual void GetDisplayMatrix(CFX_Matrix& mt,
                                const CFX_Rect& rtDisp,
                                int32_t iRotate) = 0;

  virtual int32_t LoadPageView(IFX_Pause* pPause = NULL) = 0;
  virtual void UnloadPageView() = 0;
  virtual IXFA_Widget* GetWidgetByPos(FX_FLOAT fx, FX_FLOAT fy) = 0;
  virtual IXFA_WidgetIterator* CreateWidgetIterator(
      FX_DWORD dwTraverseWay = XFA_TRAVERSEWAY_Form,
      FX_DWORD dwWidgetFilter = XFA_WIDGETFILTER_Visible |
                                XFA_WIDGETFILTER_Viewable |
                                XFA_WIDGETFILTER_AllType) = 0;
};
class CXFA_RenderOptions {
 public:
  CXFA_RenderOptions() : m_bPrint(FALSE), m_bHighlight(TRUE) {}
  FX_BOOL m_bPrint;
  FX_BOOL m_bHighlight;
};
#define XFA_RENDERSTATUS_Ready 1
#define XFA_RENDERSTATUS_ToBeContinued 2
#define XFA_RENDERSTATUS_Done 3
#define XFA_RENDERSTATUS_Failed -1
class IXFA_RenderContext {
 public:
  virtual void Release() = 0;
  virtual int32_t StartRender(IXFA_PageView* pPageView,
                              CFX_Graphics* pGS,
                              const CFX_Matrix& pMatrix,
                              const CXFA_RenderOptions& options) = 0;
  virtual int32_t DoRender(IFX_Pause* pPause = NULL) = 0;
  virtual void StopRender() = 0;

 protected:
  ~IXFA_RenderContext() {}
};
IXFA_RenderContext* XFA_RenderContext_Create();
enum XFA_WIDGETTYPE {
  XFA_WIDGETTYPE_Barcode,
  XFA_WIDGETTYPE_PushButton,
  XFA_WIDGETTYPE_CheckButton,
  XFA_WIDGETTYPE_RadioButton,
  XFA_WIDGETTYPE_DatetimeEdit,
  XFA_WIDGETTYPE_DecimalField,
  XFA_WIDGETTYPE_NumericField,
  XFA_WIDGETTYPE_Signature,
  XFA_WIDGETTYPE_TextEdit,
  XFA_WIDGETTYPE_DropdownList,
  XFA_WIDGETTYPE_ListBox,
  XFA_WIDGETTYPE_ImageField,
  XFA_WIDGETTYPE_PasswordEdit,
  XFA_WIDGETTYPE_Arc,
  XFA_WIDGETTYPE_Rectangle,
  XFA_WIDGETTYPE_Image,
  XFA_WIDGETTYPE_Line,
  XFA_WIDGETTYPE_Text,
  XFA_WIDGETTYPE_ExcludeGroup,
  XFA_WIDGETTYPE_Subform,
  XFA_WIDGETTYPE_Unknown,
};
#define XFA_WIDGETSTATUS_Visible 0x00000001
#define XFA_WIDGETSTATUS_Invisible 0x00000002
#define XFA_WIDGETSTATUS_Hidden 0x00000004
#define XFA_WIDGETSTATUS_Viewable 0x00000010
#define XFA_WIDGETSTATUS_Printable 0x00000020
#define XFA_WIDGETSTATUS_Focused 0x00000100
class IXFA_WidgetHandler {
 public:
  virtual ~IXFA_WidgetHandler() {}

  virtual IXFA_Widget* CreateWidget(IXFA_Widget* hParent,
                                    XFA_WIDGETTYPE eType,
                                    IXFA_Widget* hBefore = NULL) = 0;
  virtual IXFA_PageView* GetPageView(IXFA_Widget* hWidget) = 0;
  virtual void GetRect(IXFA_Widget* hWidget, CFX_RectF& rt) = 0;
  virtual FX_DWORD GetStatus(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL GetBBox(IXFA_Widget* hWidget,
                          CFX_RectF& rtBox,
                          FX_DWORD dwStatus,
                          FX_BOOL bDrawFocus = FALSE) = 0;
  virtual CXFA_WidgetAcc* GetDataAcc(IXFA_Widget* hWidget) = 0;

  virtual void GetName(IXFA_Widget* hWidget,
                       CFX_WideString& wsName,
                       int32_t iNameType = 0) = 0;
  virtual FX_BOOL GetToolTip(IXFA_Widget* hWidget,
                             CFX_WideString& wsToolTip) = 0;
  virtual void SetPrivateData(IXFA_Widget* hWidget,
                              void* module_id,
                              void* pData,
                              PD_CALLBACK_FREEDATA callback) = 0;
  virtual void* GetPrivateData(IXFA_Widget* hWidget, void* module_id) = 0;
  virtual FX_BOOL OnMouseEnter(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL OnMouseExit(IXFA_Widget* hWidget) = 0;
  virtual FX_BOOL OnLButtonDown(IXFA_Widget* hWidget,
                                FX_DWORD dwFlags,
                                FX_FLOAT fx,
                                FX_FLOAT fy) = 0;
  virtual FX_BOOL OnLButtonUp(IXFA_Widget* hWidget,
                              FX_DWORD dwFlags,
                              FX_FLOAT fx,
                              FX_FLOAT fy) = 0;
  virtual FX_BOOL OnLButtonDblClk(IXFA_Widget* hWidget,
                                  FX_DWORD dwFlags,
                                  FX_FLOAT fx,
                                  FX_FLOAT fy) = 0;
  virtual FX_BOOL OnMouseMove(IXFA_Widget* hWidget,
                              FX_DWORD dwFlags,
                              FX_FLOAT fx,
                              FX_FLOAT fy) = 0;
  virtual FX_BOOL OnMouseWheel(IXFA_Widget* hWidget,
                               FX_DWORD dwFlags,
                               int16_t zDelta,
                               FX_FLOAT fx,
                               FX_FLOAT fy) = 0;
  virtual FX_BOOL OnRButtonDown(IXFA_Widget* hWidget,
                                FX_DWORD dwFlags,
                                FX_FLOAT fx,
                                FX_FLOAT fy) = 0;
  virtual FX_BOOL OnRButtonUp(IXFA_Widget* hWidget,
                              FX_DWORD dwFlags,
                              FX_FLOAT fx,
                              FX_FLOAT fy) = 0;
  virtual FX_BOOL OnRButtonDblClk(IXFA_Widget* hWidget,
                                  FX_DWORD dwFlags,
                                  FX_FLOAT fx,
                                  FX_FLOAT fy) = 0;

  virtual FX_BOOL OnKeyDown(IXFA_Widget* hWidget,
                            FX_DWORD dwKeyCode,
                            FX_DWORD dwFlags) = 0;
  virtual FX_BOOL OnKeyUp(IXFA_Widget* hWidget,
                          FX_DWORD dwKeyCode,
                          FX_DWORD dwFlags) = 0;
  virtual FX_BOOL OnChar(IXFA_Widget* hWidget,
                         FX_DWORD dwChar,
                         FX_DWORD dwFlags) = 0;
  virtual FX_DWORD OnHitTest(IXFA_Widget* hWidget,
                             FX_FLOAT fx,
                             FX_FLOAT fy) = 0;
  virtual FX_BOOL OnSetCursor(IXFA_Widget* hWidget,
                              FX_FLOAT fx,
                              FX_FLOAT fy) = 0;

  virtual void RenderWidget(IXFA_Widget* hWidget,
                            CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_BOOL bHighlight = FALSE) = 0;
  virtual FX_BOOL HasEvent(CXFA_WidgetAcc* pWidgetAcc,
                           XFA_EVENTTYPE eEventType) = 0;
  virtual int32_t ProcessEvent(CXFA_WidgetAcc* pWidgetAcc,
                               CXFA_EventParam* pParam) = 0;
};
class IXFA_WidgetIterator {
 public:
  virtual void Release() = 0;
  virtual void Reset() = 0;
  virtual IXFA_Widget* MoveToFirst() = 0;
  virtual IXFA_Widget* MoveToLast() = 0;
  virtual IXFA_Widget* MoveToNext() = 0;
  virtual IXFA_Widget* MoveToPrevious() = 0;
  virtual IXFA_Widget* GetCurrentWidget() = 0;
  virtual FX_BOOL SetCurrentWidget(IXFA_Widget* hWidget) = 0;

 protected:
  ~IXFA_WidgetIterator() {}
};
class IXFA_WidgetAccIterator {
 public:
  virtual void Release() = 0;
  virtual void Reset() = 0;
  virtual CXFA_WidgetAcc* MoveToFirst() = 0;
  virtual CXFA_WidgetAcc* MoveToLast() = 0;
  virtual CXFA_WidgetAcc* MoveToNext() = 0;
  virtual CXFA_WidgetAcc* MoveToPrevious() = 0;
  virtual CXFA_WidgetAcc* GetCurrentWidgetAcc() = 0;
  virtual FX_BOOL SetCurrentWidgetAcc(CXFA_WidgetAcc* hWidget) = 0;
  virtual void SkipTree() = 0;

 protected:
  ~IXFA_WidgetAccIterator() {}
};
IXFA_WidgetAccIterator* XFA_WidgetAccIterator_Create(
    CXFA_WidgetAcc* pTravelRoot,
    XFA_WIDGETORDER eOrder = XFA_WIDGETORDER_PreOrder);
class IXFA_ChecksumContext {
 public:
  virtual void Release() = 0;

  virtual FX_BOOL StartChecksum() = 0;
  virtual FX_BOOL UpdateChecksum(IFX_FileRead* pSrcFile,
                                 FX_FILESIZE offset = 0,
                                 size_t size = 0) = 0;
  virtual void FinishChecksum() = 0;
  virtual void GetChecksum(CFX_ByteString& bsChecksum) = 0;

 protected:
  ~IXFA_ChecksumContext() {}
};
IXFA_ChecksumContext* XFA_Checksum_Create();

#endif  // FXFA_H_
