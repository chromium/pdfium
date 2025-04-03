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

    void SetDocument(CXFA_Document* pDocument) { document_ = pDocument; }
    CXFA_Document* GetDocument() const { return document_; }

   private:
    cppgc::Member<CXFA_Document> document_;
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Document();

  void Trace(cppgc::Visitor* visitor) const;

  bool HasScriptContext() const { return !!script_context_; }
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

  LayoutProcessorIface* GetLayoutProcessor() const { return layout_processor_; }
  CXFA_Node* GetRoot() const { return root_node_; }
  void SetRoot(CXFA_Node* pNewRoot) { root_node_ = pNewRoot; }

  bool is_strict_scoping() const { return strict_scoping_; }
  void set_is_strict_scoping() { strict_scoping_ = true; }

  bool is_scripting() const { return scripting_; }
  void set_is_scripting() { scripting_ = true; }

  bool IsInteractive();
  XFA_VERSION GetCurVersionMode() const { return cur_version_mode_; }
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
  std::unique_ptr<CFXJSE_Engine> script_context_;
  cppgc::Member<CXFA_FFNotify> const notify_;
  cppgc::Member<CXFA_NodeOwner> const node_owner_;
  cppgc::Member<CXFA_Node> root_node_;
  cppgc::Member<LayoutProcessorIface> layout_processor_;
  cppgc::Member<CXFA_LocaleMgr> locale_mgr_;
  cppgc::Member<CScript_DataWindow> script_data_window_;
  cppgc::Member<CScript_EventPseudoModel> script_event_;
  cppgc::Member<CScript_HostPseudoModel> script_host_;
  cppgc::Member<CScript_LogPseudoModel> script_log_;
  cppgc::Member<CScript_LayoutPseudoModel> script_layout_;
  cppgc::Member<CScript_SignaturePseudoModel> script_signature_;
  std::map<uint32_t, cppgc::Member<CXFA_Node>> rg_global_binding_;
  std::vector<cppgc::Member<CXFA_Node>> pending_page_set_;
  XFA_VERSION cur_version_mode_ = XFA_VERSION_DEFAULT;
  std::optional<bool> interactive_;
  bool strict_scoping_ = false;
  bool scripting_ = false;
};

#endif  // XFA_FXFA_PARSER_CXFA_DOCUMENT_H_
