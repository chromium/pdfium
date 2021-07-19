// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFAPP_H_
#define XFA_FXFA_CXFA_FFAPP_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "xfa/fwl/cfwl_app.h"

class CFWL_WidgetMgr;
class CXFA_FFDoc;
class CXFA_FWLAdapterWidgetMgr;
class CXFA_FWLTheme;
class CXFA_FontMgr;
class IFX_SeekableReadStream;

class CXFA_FFApp : public cppgc::GarbageCollected<CXFA_FFApp>,
                   public CFWL_App::AdapterIface {
 public:
  class CallbackIface {
   public:
    virtual ~CallbackIface() = default;

    /**
     * Returns the language of the running host application. Such as zh_CN
     */
    virtual WideString GetLanguage() = 0;

    /**
     * Returns the platform of the machine running the script. Such as WIN
     */
    virtual WideString GetPlatform() = 0;

    /**
     * Get application name, such as Phantom.
     */
    virtual WideString GetAppName() = 0;

    /**
     * Get application message box title.
     */
    virtual WideString GetAppTitle() const = 0;

    /**
     * Causes the system to play a sound.
     * @param[in] dwType The system code for the appropriate sound.0 (Error)1
     * (Warning)2 (Question)3 (Status)4 (Default)
     */
    virtual void Beep(uint32_t dwType) = 0;

    /**
     * Displays a message box.
     * @param[in] wsMessage    - Message string to display in box.
     * @param[in] wsTitle      - Title string for box.
     * @param[in] dwIconType   - Icon type, refer to XFA_MBICON.
     * @param[in] dwButtonType - Button type, refer to XFA_MESSAGEBUTTON.
     * @return A valid integer representing the value of the button pressed by
     * the user, refer to XFA_ID.
     */
    virtual int32_t MsgBox(const WideString& wsMessage,
                           const WideString& wsTitle,
                           uint32_t dwIconType,
                           uint32_t dwButtonType) = 0;

    /**
     * Get a response from the user.
     * @param[in] wsQuestion      - Message string to display in box.
     * @param[in] wsTitle         - Title string for box.
     * @param[in] wsDefaultAnswer - Initial contents for answer.
     * @param[in] bMask           - Mask the user input with asterisks when
     * true,
     * @return A string containing the user's response.
     */
    virtual WideString Response(const WideString& wsQuestion,
                                const WideString& wsTitle,
                                const WideString& wsDefaultAnswer,
                                bool bMask) = 0;

    /**
     * Download something from somewhere.
     * @param[in] wsURL - http, ftp, such as
     * "http://www.w3.org/TR/REC-xml-names/".
     */
    virtual RetainPtr<IFX_SeekableReadStream> DownloadURL(
        const WideString& wsURL) = 0;

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
     * @param[in] wsHeader      any additional HTTP headers to be included in
     * the post.
     * @param[out] wsResponse   decoded response from server.
     * @return true Server permitted the post request, false otherwise.
     */
    virtual bool PostRequestURL(const WideString& wsURL,
                                const WideString& wsData,
                                const WideString& wsContentType,
                                const WideString& wsEncode,
                                const WideString& wsHeader,
                                WideString& wsResponse) = 0;

    /**
     * PUT data to the given url.
     * @param[in] wsURL         the URL being uploaded.
     * @param[in] wsData            the data being uploaded.
     * @param[in] wsEncode      the encode of data including UTF-8, UTF-16,
     * ISO8859-1, any recognized [IANA]character encoding
     * @return true Server permitted the post request, false otherwise.
     */
    virtual bool PutRequestURL(const WideString& wsURL,
                               const WideString& wsData,
                               const WideString& wsEncode) = 0;

    virtual CFX_Timer::HandlerIface* GetTimerHandler() const = 0;
    virtual cppgc::Heap* GetGCHeap() const = 0;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFApp() override;

  // CFWL_App::AdapterIface:
  void Trace(cppgc::Visitor* visitor) const override;
  CFWL_WidgetMgr::AdapterIface* GetWidgetMgrAdapter() override;
  CFX_Timer::HandlerIface* GetTimerHandler() override;
  IFWL_ThemeProvider* GetThemeProvider() override;
  cppgc::Heap* GetHeap() override;

  bool LoadFWLTheme(CXFA_FFDoc* doc);
  CFWL_WidgetMgr* GetFWLWidgetMgr() const { return m_pFWLApp->GetWidgetMgr(); }
  CallbackIface* GetAppProvider() const { return m_pProvider.Get(); }
  CFWL_App* GetFWLApp() const { return m_pFWLApp; }
  CXFA_FontMgr* GetXFAFontMgr() const { return m_pXFAFontMgr; }

 private:
  explicit CXFA_FFApp(CallbackIface* pProvider);

  UnownedPtr<CallbackIface> const m_pProvider;
  cppgc::Member<CXFA_FontMgr> m_pXFAFontMgr;
  cppgc::Member<CXFA_FWLAdapterWidgetMgr> m_pAdapterWidgetMgr;
  cppgc::Member<CXFA_FWLTheme> m_pFWLTheme;
  cppgc::Member<CFWL_App> m_pFWLApp;
};

#endif  // XFA_FXFA_CXFA_FFAPP_H_
