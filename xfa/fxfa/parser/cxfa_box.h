// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_BOX_H_
#define XFA_FXFA_PARSER_CXFA_BOX_H_

#include <tuple>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fxfa/parser/cxfa_node.h"

class CFGAS_GEGraphics;
class CXFA_Edge;
class CXFA_Fill;
class CXFA_Stroke;

class CXFA_Box : public CXFA_Node {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Box() override;

  XFA_AttributeValue GetPresence();
  std::tuple<XFA_AttributeValue, bool, float> Get3DStyle();

  size_t CountEdges();
  CXFA_Edge* GetEdgeIfExists(size_t nIndex);
  CXFA_Fill* GetOrCreateFillIfPossible();

  std::vector<CXFA_Stroke*> GetStrokes();

  void Draw(CFGAS_GEGraphics* pGS,
            const CFX_RectF& rtWidget,
            const CFX_Matrix& matrix,
            bool forceRound);

 protected:
  CXFA_Box(CXFA_Document* pDoc,
           XFA_PacketType ePacket,
           Mask<XFA_XDPPACKET> validPackets,
           XFA_ObjectType oType,
           XFA_Element eType,
           pdfium::span<const PropertyData> properties,
           pdfium::span<const AttributeData> attributes,
           CJX_Object* js_node);

  XFA_AttributeValue GetHand();

 private:
  bool IsCircular();
  absl::optional<int32_t> GetStartAngle();
  absl::optional<int32_t> GetSweepAngle();

  std::vector<CXFA_Stroke*> GetStrokesInternal(bool bNull);
  void DrawFill(const std::vector<CXFA_Stroke*>& strokes,
                CFGAS_GEGraphics* pGS,
                CFX_RectF rtWidget,
                const CFX_Matrix& matrix,
                bool forceRound);
  void StrokeArcOrRounded(CFGAS_GEGraphics* pGS,
                          CFX_RectF rtWidget,
                          const CFX_Matrix& matrix,
                          bool forceRound);
  void GetPathArcOrRounded(CFX_RectF rtDraw,
                           bool forceRound,
                           CFGAS_GEPath* fillPath);
};

#endif  // XFA_FXFA_PARSER_CXFA_BOX_H_
