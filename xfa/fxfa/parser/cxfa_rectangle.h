// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_RECTANGLE_H_
#define XFA_FXFA_PARSER_CXFA_RECTANGLE_H_

#include <vector>

#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fxfa/parser/cxfa_box.h"

class CXFA_Rectangle : public CXFA_Box {
 public:
  static CXFA_Rectangle* FromNode(CXFA_Node* pNode);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Rectangle() override;

  void GetFillPath(const std::vector<CXFA_Stroke*>& strokes,
                   const CFX_RectF& rtWidget,
                   CFGAS_GEPath* fillPath);
  void Draw(const std::vector<CXFA_Stroke*>& strokes,
            CFGAS_GEGraphics* pGS,
            CFX_RectF rtWidget,
            const CFX_Matrix& matrix);

 protected:
  CXFA_Rectangle(CXFA_Document* doc, XFA_PacketType packet);
  CXFA_Rectangle(CXFA_Document* pDoc,
                 XFA_PacketType ePacket,
                 Mask<XFA_XDPPACKET> validPackets,
                 XFA_ObjectType oType,
                 XFA_Element eType,
                 pdfium::span<const PropertyData> properties,
                 pdfium::span<const AttributeData> attributes,
                 CJX_Object* js_node);

  void Stroke(const std::vector<CXFA_Stroke*>& strokes,
              CFGAS_GEGraphics* pGS,
              CFX_RectF rtWidget,
              const CFX_Matrix& matrix);
  void StrokeEmbossed(CFGAS_GEGraphics* pGS,
                      CFX_RectF rt,
                      float fThickness,
                      const CFX_Matrix& matrix);
  void StrokeLowered(CFGAS_GEGraphics* pGS,
                     CFX_RectF rt,
                     float fThickness,
                     const CFX_Matrix& matrix);
  void StrokeRaised(CFGAS_GEGraphics* pGS,
                    CFX_RectF rt,
                    float fThickness,
                    const CFX_Matrix& matrix);
  void StrokeEtched(CFGAS_GEGraphics* pGS,
                    CFX_RectF rt,
                    float fThickness,
                    const CFX_Matrix& matrix);
  void StrokeRect(CFGAS_GEGraphics* pGraphic,
                  const CFX_RectF& rt,
                  float fLineWidth,
                  const CFX_Matrix& matrix,
                  FX_ARGB argbTopLeft,
                  FX_ARGB argbBottomRight);
  void GetPath(const std::vector<CXFA_Stroke*>& strokes,
               CFX_RectF rtWidget,
               CFGAS_GEPath& path,
               int32_t nIndex,
               bool bStart,
               bool bCorner);
};

#endif  // XFA_FXFA_PARSER_CXFA_RECTANGLE_H_
