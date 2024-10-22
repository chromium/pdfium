// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_DOCUMENT_H_
#define XFA_FXFA_PARSER_CXFA_DOCUMENT_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/persistent.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_nodeowner.h"

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

namespace cppgc {
class Heap;
}  // namespace cppgc

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

class CXFA_Document final : public cppgc::GarbageCollected<CXFA_Document> {
 public:
  class LayoutProcessorIface
      : public cppgc::GarbageCollected<LayoutProcessorIface> {
   public:
    LayoutProcessorIface();
    virtual ~LayoutProcessorIface();

    virtual void Trace(cppgc::Visitor* visitor) const;
    virtual void SetForceRelayout() = 0;
    virtual void SetHasChangedContainer() = 0;

    void SetDocument(CXFA_Document* pDocument) { m_pDocument = pDocument; }
    CXFA_Document* GetDocument() const { return m_pDocument; }

   private:
    cppgc::Member<CXFA_Document> m_pDocument;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Document();

  void Trace(cppgc::Visitor* visitor) const;

  bool HasScriptContext() const { return !!m_pScriptContext; }
  CFXJSE_Engine* InitScriptContext(CJS_Runtime* fxjs_runtime);

  // Only safe to call in situations where the context is known to exist,
  // and always returns non-NULL in those situations. In other words, we have
  // to call InitScriptContext() first to avoid a situation where the context
  // won't have an isolate set into it.
  CFXJSE_Engine* GetScriptContext() const;

  CXFA_FFNotify* GetNotify() const { return notify_; }
  CXFA_NodeOwner* GetNodeOwner() { return node_owner_; }
  cppgc::Heap* GetHeap() const;
  CXFA_LocaleMgr* GetLocaleMgr();
  CXFA_Object* GetXFAObject(XFA_HashCode wsNodeNameHash);
  CXFA_Node* GetNodeByID(CXFA_Node* pRoot, WideStringView wsID) const;
  CXFA_Node* GetNotBindNode(
      pdfium::span<cppgc::Member<CXFA_Object>> arrayNodes) const;

  LayoutProcessorIface* GetLayoutProcessor() const {
    return m_pLayoutProcessor;
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
  void DoDataRemerge();
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

  size_t GetPendingNodesCount() const;
  CXFA_Node* GetPendingNodeAtIndex(size_t index) const;
  void AppendPendingNode(CXFA_Node* node);
  void ClearPendingNodes();
  void SetPendingNodesUnusedAndUnbound();

 private:
  friend class CXFADocumentTest_ParseXFAVersion_Test;
  friend class CXFADocumentTest_ParseUseHref_Test;
  friend class CXFADocumentTest_ParseUse_Test;

  static XFA_VERSION ParseXFAVersion(const WideString& wsTemplateNS);
  static void ParseUseHref(const WideString& wsUseVal,
                           WideStringView& wsURI,
                           WideStringView& wsID,
                           WideStringView& wsSOM);
  static void ParseUse(const WideString& wsUseVal,
                       WideStringView& wsID,
                       WideStringView& wsSOM);

  CXFA_Document(CXFA_FFNotify* notify,
                cppgc::Heap* heap,
                LayoutProcessorIface* pLayout);

  UnownedPtr<cppgc::Heap> heap_;
  std::unique_ptr<CFXJSE_Engine> m_pScriptContext;
  cppgc::Member<CXFA_FFNotify> const notify_;
  cppgc::Member<CXFA_NodeOwner> const node_owner_;
  cppgc::Member<CXFA_Node> m_pRootNode;
  cppgc::Member<LayoutProcessorIface> m_pLayoutProcessor;
  cppgc::Member<CXFA_LocaleMgr> m_pLocaleMgr;
  cppgc::Member<CScript_DataWindow> m_pScriptDataWindow;
  cppgc::Member<CScript_EventPseudoModel> m_pScriptEvent;
  cppgc::Member<CScript_HostPseudoModel> m_pScriptHost;
  cppgc::Member<CScript_LogPseudoModel> m_pScriptLog;
  cppgc::Member<CScript_LayoutPseudoModel> m_pScriptLayout;
  cppgc::Member<CScript_SignaturePseudoModel> m_pScriptSignature;
  std::map<uint32_t, cppgc::Member<CXFA_Node>> m_rgGlobalBinding;
  std::vector<cppgc::Member<CXFA_Node>> m_pPendingPageSet;
  XFA_VERSION m_eCurVersionMode = XFA_VERSION_DEFAULT;
  std::optional<bool> m_Interactive;
  bool m_bStrictScoping = false;
  bool m_bScripting = false;
};

#endif  // XFA_FXFA_PARSER_CXFA_DOCUMENT_H_
