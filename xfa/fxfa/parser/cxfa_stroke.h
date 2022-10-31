// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_STROKE_H_
#define XFA_FXFA_PARSER_CXFA_STROKE_H_

#include "core/fxcrt/mask.h"
#include "core/fxge/dib/fx_dib.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/cxfa_node.h"

class CFGAS_GEGraphics;
class CFGAS_GEPath;
class CXFA_Node;

void XFA_StrokeTypeSetLineDash(CFGAS_GEGraphics* pGraphics,
                               XFA_AttributeValue iStrokeType,
                               XFA_AttributeValue iCapType);

class CXFA_Stroke : public CXFA_Node {
 public:
  enum class SameStyleOption {
    kNoPresence = 1 << 0,
    kCorner = 1 << 1,
  };
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Stroke() override;

  bool IsCorner() const { return GetElementType() == XFA_Element::Corner; }
  bool IsVisible();
  bool IsInverted();

  XFA_AttributeValue GetCapType();
  XFA_AttributeValue GetStrokeType();
  XFA_AttributeValue GetJoinType();
  float GetRadius() const;
  float GetThickness() const;

  CXFA_Measurement GetMSThickness() const;
  void SetMSThickness(CXFA_Measurement msThinkness);

  FX_ARGB GetColor() const;
  void SetColor(FX_ARGB argb);

  bool SameStyles(CXFA_Stroke* stroke, Mask<SameStyleOption> dwFlags);

  void Stroke(CFGAS_GEGraphics* pGS,
              const CFGAS_GEPath& pPath,
              const CFX_Matrix& matrix);

 protected:
  CXFA_Stroke(CXFA_Document* pDoc,
              XFA_PacketType ePacket,
              Mask<XFA_XDPPACKET> validPackets,
              XFA_ObjectType oType,
              XFA_Element eType,
              pdfium::span<const PropertyData> properties,
              pdfium::span<const AttributeData> attributes,
              CJX_Object* js_node);
};

#endif  // XFA_FXFA_PARSER_CXFA_STROKE_H_
