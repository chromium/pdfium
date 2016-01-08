// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_IJAVASCRIPT_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_IJAVASCRIPT_H_

#include "core/include/fxcrt/fx_string.h"
#include "core/include/fxcrt/fx_system.h"

#ifdef PDF_ENABLE_XFA
#include "xfa/include/fxjse/fxjse.h"
#endif  // PDF_ENABLE_XFA

class CPDFDoc_Environment;
class CPDFSDK_Annot;
class CPDFSDK_Document;
class CPDF_Bookmark;
class CPDF_FormField;

// Records the details of an event and triggers JS execution for it.
class IJS_Context {
 public:
  virtual FX_BOOL RunScript(const CFX_WideString& script,
                            CFX_WideString* info) = 0;

  virtual void OnApp_Init() = 0;

  virtual void OnDoc_Open(CPDFSDK_Document* pDoc,
                          const CFX_WideString& strTargetName) = 0;
  virtual void OnDoc_WillPrint(CPDFSDK_Document* pDoc) = 0;
  virtual void OnDoc_DidPrint(CPDFSDK_Document* pDoc) = 0;
  virtual void OnDoc_WillSave(CPDFSDK_Document* pDoc) = 0;
  virtual void OnDoc_DidSave(CPDFSDK_Document* pDoc) = 0;
  virtual void OnDoc_WillClose(CPDFSDK_Document* pDoc) = 0;

  virtual void OnPage_Open(CPDFSDK_Document* pTarget) = 0;
  virtual void OnPage_Close(CPDFSDK_Document* pTarget) = 0;
  virtual void OnPage_InView(CPDFSDK_Document* pTarget) = 0;
  virtual void OnPage_OutView(CPDFSDK_Document* pTarget) = 0;

  virtual void OnField_MouseDown(FX_BOOL bModifier,
                                 FX_BOOL bShift,
                                 CPDF_FormField* pTarget) = 0;
  virtual void OnField_MouseEnter(FX_BOOL bModifier,
                                  FX_BOOL bShift,
                                  CPDF_FormField* pTarget) = 0;
  virtual void OnField_MouseExit(FX_BOOL bModifier,
                                 FX_BOOL bShift,
                                 CPDF_FormField* pTarget) = 0;
  virtual void OnField_MouseUp(FX_BOOL bModifier,
                               FX_BOOL bShift,
                               CPDF_FormField* pTarget) = 0;
  virtual void OnField_Focus(FX_BOOL bModifier,
                             FX_BOOL bShift,
                             CPDF_FormField* pTarget,
                             const CFX_WideString& Value) = 0;
  virtual void OnField_Blur(FX_BOOL bModifier,
                            FX_BOOL bShift,
                            CPDF_FormField* pTarget,
                            const CFX_WideString& Value) = 0;

  virtual void OnField_Calculate(CPDF_FormField* pSource,
                                 CPDF_FormField* pTarget,
                                 CFX_WideString& Value,
                                 FX_BOOL& bRc) = 0;
  virtual void OnField_Format(CPDF_FormField* pTarget,
                              CFX_WideString& Value,
                              FX_BOOL bWillCommit) = 0;
  virtual void OnField_Keystroke(CFX_WideString& strChange,
                                 const CFX_WideString& strChangeEx,
                                 FX_BOOL KeyDown,
                                 FX_BOOL bModifier,
                                 int& nSelEnd,
                                 int& nSelStart,
                                 FX_BOOL bShift,
                                 CPDF_FormField* pTarget,
                                 CFX_WideString& Value,
                                 FX_BOOL bWillCommit,
                                 FX_BOOL bFieldFull,
                                 FX_BOOL& bRc) = 0;
  virtual void OnField_Validate(CFX_WideString& strChange,
                                const CFX_WideString& strChangeEx,
                                FX_BOOL bKeyDown,
                                FX_BOOL bModifier,
                                FX_BOOL bShift,
                                CPDF_FormField* pTarget,
                                CFX_WideString& Value,
                                FX_BOOL& bRc) = 0;

  virtual void OnScreen_Focus(FX_BOOL bModifier,
                              FX_BOOL bShift,
                              CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_Blur(FX_BOOL bModifier,
                             FX_BOOL bShift,
                             CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_Open(FX_BOOL bModifier,
                             FX_BOOL bShift,
                             CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_Close(FX_BOOL bModifier,
                              FX_BOOL bShift,
                              CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_MouseDown(FX_BOOL bModifier,
                                  FX_BOOL bShift,
                                  CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_MouseUp(FX_BOOL bModifier,
                                FX_BOOL bShift,
                                CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_MouseEnter(FX_BOOL bModifier,
                                   FX_BOOL bShift,
                                   CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_MouseExit(FX_BOOL bModifier,
                                  FX_BOOL bShift,
                                  CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_InView(FX_BOOL bModifier,
                               FX_BOOL bShift,
                               CPDFSDK_Annot* pScreen) = 0;
  virtual void OnScreen_OutView(FX_BOOL bModifier,
                                FX_BOOL bShift,
                                CPDFSDK_Annot* pScreen) = 0;

  virtual void OnBookmark_MouseUp(CPDF_Bookmark* pBookMark) = 0;
  virtual void OnLink_MouseUp(CPDFSDK_Document* pTarget) = 0;

  virtual void OnMenu_Exec(CPDFSDK_Document* pTarget,
                           const CFX_WideString&) = 0;
  virtual void OnBatchExec(CPDFSDK_Document* pTarget) = 0;
  virtual void OnConsole_Exec() = 0;
  virtual void OnExternal_Exec() = 0;

  virtual void EnableMessageBox(FX_BOOL bEnable) = 0;

 protected:
  virtual ~IJS_Context() {}
};

// Owns the FJXS objects needed to actually execute JS.
class IJS_Runtime {
 public:
  static void Initialize(unsigned int slot, void* isolate);
  static IJS_Runtime* Create(CPDFDoc_Environment* pEnv);
  virtual ~IJS_Runtime() {}

  virtual IJS_Context* NewContext() = 0;
  virtual void ReleaseContext(IJS_Context* pContext) = 0;
  virtual IJS_Context* GetCurrentContext() = 0;
  virtual void SetReaderDocument(CPDFSDK_Document* pReaderDoc) = 0;
  virtual CPDFSDK_Document* GetReaderDocument() = 0;
  virtual int Execute(IJS_Context* cc,
                      const wchar_t* script,
                      CFX_WideString* info) = 0;

#ifdef PDF_ENABLE_XFA
  virtual FX_BOOL GetHValueByName(const CFX_ByteStringC& utf8Name,
                                  FXJSE_HVALUE hValue) = 0;
  virtual FX_BOOL SetHValueByName(const CFX_ByteStringC& utf8Name,
                                  FXJSE_HVALUE hValue) = 0;
#endif  // PDF_ENABLE_XFA

 protected:
  IJS_Runtime() {}
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_IJAVASCRIPT_H_
