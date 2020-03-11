// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_DOCUMENT_H_
#define XFA_FXFA_PARSER_CXFA_DOCUMENT_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/optional.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_nodeowner.h"

enum XFA_VERSION {
  XFA_VERSION_UNKNOWN = 0,
  XFA_VERSION_200 = 200,
  XFA_VERSION_202 = 202,
  XFA_VERSION_204 = 204,
  XFA_VERSION_205 = 205,
  XFA_VERSION_206 = 206,
  XFA_VERSION_207 = 207,
  XFA_VERSION_208 = 208,
  XFA_VERSION_300 = 300,
  XFA_VERSION_301 = 301,
  XFA_VERSION_303 = 303,
  XFA_VERSION_306 = 306,
  XFA_VERSION_DEFAULT = XFA_VERSION_303,
  XFA_VERSION_MIN = 200,
  XFA_VERSION_MAX = 400,
};

class CFXJSE_Engine;
class CJS_Runtime;
class CScript_DataWindow;
class CScript_EventPseudoModel;
class CScript_HostPseudoModel;
class CScript_LayoutPseudoModel;
class CScript_LogPseudoModel;
class CScript_SignaturePseudoModel;
class CXFA_FFNotify;
class CXFA_Node;
class CXFA_Object;

class CXFA_Document final : public CXFA_NodeOwner {
 public:
  class LayoutProcessorIface {
   public:
    LayoutProcessorIface();
    virtual ~LayoutProcessorIface();
    virtual void SetForceRelayout(bool enable) = 0;
    virtual void AddChangedContainer(CXFA_Node* pContainer) = 0;

    void SetDocument(CXFA_Document* pDocument) { m_pDocument = pDocument; }
    CXFA_Document* GetDocument() const { return m_pDocument.Get(); }

   private:
    UnownedPtr<CXFA_Document> m_pDocument;
  };

  CXFA_Document(CXFA_FFNotify* notify,
                std::unique_ptr<LayoutProcessorIface> pLayout);
  ~CXFA_Document() override;

  bool HasScriptContext() const { return !!m_pScriptContext; }
  CFXJSE_Engine* InitScriptContext(CJS_Runtime* fxjs_runtime);

  // Only safe to call in situations where the context is known to exist,
  // and always returns non-NULL in those situations. In other words, we have
  // to call InitScriptContext() first to avoid a situation where the context
  // won't have an isolate set into it.
  CFXJSE_Engine* GetScriptContext() const;

  CXFA_FFNotify* GetNotify() const { return notify_.Get(); }
  CXFA_LocaleMgr* GetLocaleMgr();
  CXFA_Object* GetXFAObject(XFA_HashCode wsNodeNameHash);
  CXFA_Node* GetNodeByID(CXFA_Node* pRoot, WideStringView wsID) const;
  CXFA_Node* GetNotBindNode(
      const std::vector<UnownedPtr<CXFA_Object>>& arrayNodes) const;

  LayoutProcessorIface* GetLayoutProcessor() const {
    return m_pLayoutProcessor.get();
  }

  CXFA_Node* GetRoot() const { return m_pRootNode; }
  void SetRoot(CXFA_Node* pNewRoot) { m_pRootNode = pNewRoot; }

  bool is_strict_scoping() const { return m_bStrictScoping; }
  void set_is_strict_scoping() { m_bStrictScoping = true; }

  bool is_scripting() const { return m_bScripting; }
  void set_is_scripting() { m_bScripting = true; }

  bool IsInteractive();
  XFA_VERSION GetCurVersionMode() const { return m_eCurVersionMode; }
  XFA_VERSION RecognizeXFAVersionNumber(const WideString& wsTemplateNS);
  FormType GetFormType() const;

  CXFA_Node* CreateNode(XFA_PacketType packet, XFA_Element eElement);

  void DoProtoMerge();
  void DoDataMerge();
  void DoDataRemerge(bool bDoDataMerge);
  CXFA_Node* DataMerge_CopyContainer(CXFA_Node* pTemplateNode,
                                     CXFA_Node* pFormNode,
                                     CXFA_Node* pDataScope,
                                     bool bOneInstance,
                                     bool bDataMerge,
                                     bool bUpLevel);
  void DataMerge_UpdateBindingRelations(CXFA_Node* pFormUpdateRoot);

  void ClearLayoutData();

  CXFA_Node* GetGlobalBinding(uint32_t dwNameHash);
  void RegisterGlobalBinding(uint32_t dwNameHash, CXFA_Node* pDataNode);
  void SetPendingNodesUnusedAndUnbound();

  std::vector<CXFA_Node*> m_pPendingPageSet;

 private:
  UnownedPtr<CXFA_FFNotify> const notify_;
  CXFA_Node* m_pRootNode = nullptr;
  std::map<uint32_t, CXFA_Node*> m_rgGlobalBinding;
  std::unique_ptr<CFXJSE_Engine> m_pScriptContext;
  std::unique_ptr<LayoutProcessorIface> m_pLayoutProcessor;
  std::unique_ptr<CXFA_LocaleMgr> m_pLocaleMgr;
  std::unique_ptr<CScript_DataWindow> m_pScriptDataWindow;
  std::unique_ptr<CScript_EventPseudoModel> m_pScriptEvent;
  std::unique_ptr<CScript_HostPseudoModel> m_pScriptHost;
  std::unique_ptr<CScript_LogPseudoModel> m_pScriptLog;
  std::unique_ptr<CScript_LayoutPseudoModel> m_pScriptLayout;
  std::unique_ptr<CScript_SignaturePseudoModel> m_pScriptSignature;
  XFA_VERSION m_eCurVersionMode = XFA_VERSION_DEFAULT;
  Optional<bool> m_Interactive;
  bool m_bStrictScoping = false;
  bool m_bScripting = false;
};

#endif  // XFA_FXFA_PARSER_CXFA_DOCUMENT_H_
