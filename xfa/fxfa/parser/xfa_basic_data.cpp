// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/xfa_basic_data.h"

#include <utility>

#include "fxjs/xfa/cjx_boolean.h"
#include "fxjs/xfa/cjx_container.h"
#include "fxjs/xfa/cjx_datawindow.h"
#include "fxjs/xfa/cjx_delta.h"
#include "fxjs/xfa/cjx_desc.h"
#include "fxjs/xfa/cjx_draw.h"
#include "fxjs/xfa/cjx_encrypt.h"
#include "fxjs/xfa/cjx_eventpseudomodel.h"
#include "fxjs/xfa/cjx_exclgroup.h"
#include "fxjs/xfa/cjx_extras.h"
#include "fxjs/xfa/cjx_field.h"
#include "fxjs/xfa/cjx_form.h"
#include "fxjs/xfa/cjx_handler.h"
#include "fxjs/xfa/cjx_hostpseudomodel.h"
#include "fxjs/xfa/cjx_instancemanager.h"
#include "fxjs/xfa/cjx_layoutpseudomodel.h"
#include "fxjs/xfa/cjx_logpseudomodel.h"
#include "fxjs/xfa/cjx_manifest.h"
#include "fxjs/xfa/cjx_model.h"
#include "fxjs/xfa/cjx_node.h"
#include "fxjs/xfa/cjx_occur.h"
#include "fxjs/xfa/cjx_packet.h"
#include "fxjs/xfa/cjx_script.h"
#include "fxjs/xfa/cjx_signaturepseudomodel.h"
#include "fxjs/xfa/cjx_source.h"
#include "fxjs/xfa/cjx_subform.h"
#include "fxjs/xfa/cjx_textnode.h"
#include "fxjs/xfa/cjx_tree.h"
#include "fxjs/xfa/cjx_treelist.h"
#include "fxjs/xfa/cjx_wsdlconnection.h"
#include "fxjs/xfa/cjx_xfa.h"
#include "xfa/fxfa/fxfa_basic.h"

namespace {

struct PacketRecord {
  XFA_PacketType packet_type;
  uint32_t hash;
  uint32_t flags;
  const wchar_t* name;
  const wchar_t* uri;
};

const PacketRecord g_PacketTable[] = {
#undef PCKT____
#define PCKT____(a, b, c, d, e, f)                                          \
  {XFA_PacketType::c, a, XFA_XDPPACKET_FLAGS_##e | XFA_XDPPACKET_FLAGS_##f, \
   L##b, d},
#include "xfa/fxfa/parser/packets.inc"
#undef PCKT____
};

struct ElementRecord {
  uint32_t hash;  // Hashed as wide string.
  XFA_Element element;
  XFA_Element parent;
  const char* name;
};

const ElementRecord g_ElementTable[] = {
#undef ELEM____
#define ELEM____(a, b, c, d) {a, XFA_Element::c, XFA_Element::d, b},
#include "xfa/fxfa/parser/elements.inc"
#undef ELEM____
};

struct AttributeRecord {
  uint32_t hash;  // Hashed as wide string.
  XFA_Attribute attribute;
  XFA_ScriptType script_type;
  const char* name;
};

const AttributeRecord g_AttributeTable[] = {
#undef ATTR____
#define ATTR____(a, b, c, d) {a, XFA_Attribute::c, XFA_ScriptType::d, b},
#include "xfa/fxfa/parser/attributes.inc"
#undef ATTR____
};

struct AttributeValueRecord {
  uint32_t uHash;  // |pName| hashed as WideString.
  XFA_AttributeValue eName;
  const char* pName;
};

const AttributeValueRecord g_AttributeValueTable[] = {
#undef VALUE____
#define VALUE____(a, b, c) {a, XFA_AttributeValue::c, b},
#include "xfa/fxfa/parser/attribute_values.inc"
#undef VALUE____
};

struct ElementAttributeRecord {
  XFA_Element element;
  XFA_Attribute attribute;
  XFA_ATTRIBUTE_CALLBACK callback;
};

const ElementAttributeRecord g_ElementAttributeTable[] = {
#undef ELEM_ATTR____
#define ELEM_ATTR____(a, b, c) {XFA_Element::a, XFA_Attribute::b, c##_static},
#include "xfa/fxfa/parser/element_attributes.inc"
#undef ELEM_ATTR____
};

}  // namespace

XFA_PACKETINFO XFA_GetPacketByIndex(XFA_PacketType ePacket) {
  const PacketRecord* pRecord = &g_PacketTable[static_cast<uint8_t>(ePacket)];
  return {pRecord->name, pRecord->packet_type, pRecord->uri, pRecord->flags};
}

Optional<XFA_PACKETINFO> XFA_GetPacketByName(WideStringView wsName) {
  uint32_t hash = FX_HashCode_GetW(wsName, false);
  auto* elem = std::lower_bound(
      std::begin(g_PacketTable), std::end(g_PacketTable), hash,
      [](const PacketRecord& a, uint32_t hash) { return a.hash < hash; });
  if (elem != std::end(g_PacketTable) && elem->name == wsName)
    return XFA_GetPacketByIndex(elem->packet_type);
  return {};
}

ByteStringView XFA_ElementToName(XFA_Element elem) {
  return g_ElementTable[static_cast<size_t>(elem)].name;
}

XFA_Element XFA_GetElementByName(WideStringView name) {
  uint32_t hash = FX_HashCode_GetW(name, false);
  auto* elem = std::lower_bound(
      std::begin(g_ElementTable), std::end(g_ElementTable), hash,
      [](const ElementRecord& a, uint32_t hash) { return a.hash < hash; });
  if (elem != std::end(g_ElementTable) && name.EqualsASCII(elem->name))
    return elem->element;
  return XFA_Element::Unknown;
}

ByteStringView XFA_AttributeToName(XFA_Attribute attr) {
  return g_AttributeTable[static_cast<size_t>(attr)].name;
}

Optional<XFA_ATTRIBUTEINFO> XFA_GetAttributeByName(WideStringView name) {
  uint32_t hash = FX_HashCode_GetW(name, false);
  auto* elem = std::lower_bound(
      std::begin(g_AttributeTable), std::end(g_AttributeTable), hash,
      [](const AttributeRecord& a, uint32_t hash) { return a.hash < hash; });
  if (elem != std::end(g_AttributeTable) && name.EqualsASCII(elem->name)) {
    XFA_ATTRIBUTEINFO result;
    result.attribute = elem->attribute;
    result.eValueType = elem->script_type;
    return result;
  }
  return {};
}

ByteStringView XFA_AttributeValueToName(XFA_AttributeValue item) {
  return g_AttributeValueTable[static_cast<int32_t>(item)].pName;
}

Optional<XFA_AttributeValue> XFA_GetAttributeValueByName(WideStringView name) {
  auto* it = std::lower_bound(std::begin(g_AttributeValueTable),
                              std::end(g_AttributeValueTable),
                              FX_HashCode_GetW(name, false),
                              [](const AttributeValueRecord& arg,
                                 uint32_t hash) { return arg.uHash < hash; });
  if (it != std::end(g_AttributeValueTable) && name.EqualsASCII(it->pName))
    return it->eName;

  return {};
}

Optional<XFA_SCRIPTATTRIBUTEINFO> XFA_GetScriptAttributeByName(
    XFA_Element element,
    WideStringView attribute_name) {
  Optional<XFA_ATTRIBUTEINFO> attr = XFA_GetAttributeByName(attribute_name);
  if (!attr.has_value())
    return {};

  while (element != XFA_Element::Unknown) {
    auto compound_key = std::make_pair(element, attr.value().attribute);
    auto* it = std::lower_bound(
        std::begin(g_ElementAttributeTable), std::end(g_ElementAttributeTable),
        compound_key,
        [](const ElementAttributeRecord& arg,
           const std::pair<XFA_Element, XFA_Attribute>& key) {
          return std::make_pair(arg.element, arg.attribute) < key;
        });
    if (it != std::end(g_ElementAttributeTable) &&
        compound_key == std::make_pair(it->element, it->attribute)) {
      XFA_SCRIPTATTRIBUTEINFO result;
      result.attribute = attr.value().attribute;
      result.eValueType = attr.value().eValueType;
      result.callback = it->callback;
      return result;
    }
    element = g_ElementTable[static_cast<size_t>(element)].parent;
  }
  return {};
}
