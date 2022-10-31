// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_pushbuttontp.h"

#include "xfa/fgas/graphics/cfgas_gecolor.h"
#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fgas/graphics/cfgas_gepath.h"
#include "xfa/fwl/cfwl_pushbutton.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/ifwl_themeprovider.h"

namespace {

constexpr float kPushbuttonSizeCorner = 2.0f;

}  // namespace

CFWL_PushButtonTP::CFWL_PushButtonTP() : m_pThemeData(new PBThemeData) {
  SetThemeData();
}

CFWL_PushButtonTP::~CFWL_PushButtonTP() = default;

void CFWL_PushButtonTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  switch (pParams.GetPart()) {
    case CFWL_ThemePart::Part::kBorder: {
      DrawBorder(pParams.GetGraphics(), pParams.m_PartRect, pParams.m_matrix);
      break;
    }
    case CFWL_ThemePart::Part::kBackground: {
      const CFX_RectF& rect = pParams.m_PartRect;
      float fRight = rect.right();
      float fBottom = rect.bottom();

      CFGAS_GEPath strokePath;
      strokePath.MoveTo(
          CFX_PointF(rect.left + kPushbuttonSizeCorner, rect.top));
      strokePath.LineTo(CFX_PointF(fRight - kPushbuttonSizeCorner, rect.top));
      strokePath.LineTo(CFX_PointF(fRight, rect.top + kPushbuttonSizeCorner));
      strokePath.LineTo(CFX_PointF(fRight, fBottom - kPushbuttonSizeCorner));
      strokePath.LineTo(CFX_PointF(fRight - kPushbuttonSizeCorner, fBottom));
      strokePath.LineTo(CFX_PointF(rect.left + kPushbuttonSizeCorner, fBottom));
      strokePath.LineTo(CFX_PointF(rect.left, fBottom - kPushbuttonSizeCorner));
      strokePath.LineTo(
          CFX_PointF(rect.left, rect.top + kPushbuttonSizeCorner));
      strokePath.LineTo(
          CFX_PointF(rect.left + kPushbuttonSizeCorner, rect.top));

      CFGAS_GEPath fillPath;
      fillPath.AddSubpath(strokePath);

      CFGAS_GEGraphics* pGraphics = pParams.GetGraphics();
      CFGAS_GEGraphics::StateRestorer restorer(pGraphics);
      CFX_RectF rtInner(rect);
      rtInner.Deflate(kPushbuttonSizeCorner + 1, kPushbuttonSizeCorner + 1,
                      kPushbuttonSizeCorner, kPushbuttonSizeCorner);
      fillPath.AddRectangle(rtInner.left, rtInner.top, rtInner.width,
                            rtInner.height);

      int32_t iColor = GetColorID(pParams.m_dwStates);
      FillSolidRect(pGraphics, m_pThemeData->clrEnd[iColor], rect,
                    pParams.m_matrix);

      pGraphics->SetStrokeColor(CFGAS_GEColor(m_pThemeData->clrBorder[iColor]));
      pGraphics->StrokePath(strokePath, pParams.m_matrix);

      fillPath.Clear();
      fillPath.AddRectangle(rtInner.left, rtInner.top, rtInner.width,
                            rtInner.height);

      pGraphics->SetFillColor(CFGAS_GEColor(m_pThemeData->clrFill[iColor]));
      pGraphics->FillPath(fillPath, CFX_FillRenderOptions::FillType::kWinding,
                          pParams.m_matrix);
      if (pParams.m_dwStates & CFWL_PartState::kFocused) {
        rtInner.Inflate(1, 1, 0, 0);
        DrawFocus(pGraphics, rtInner, pParams.m_matrix);
      }
      break;
    }
    default:
      break;
  }
}

void CFWL_PushButtonTP::SetThemeData() {
  m_pThemeData->clrBorder[0] = ArgbEncode(255, 0, 60, 116);
  m_pThemeData->clrBorder[1] = ArgbEncode(255, 0, 60, 116);
  m_pThemeData->clrBorder[2] = ArgbEncode(255, 0, 60, 116);
  m_pThemeData->clrBorder[3] = ArgbEncode(255, 0, 60, 116);
  m_pThemeData->clrBorder[4] = ArgbEncode(255, 201, 199, 186);
  m_pThemeData->clrStart[0] = ArgbEncode(255, 255, 255, 255);
  m_pThemeData->clrStart[1] = ArgbEncode(255, 209, 204, 193);
  m_pThemeData->clrStart[2] = ArgbEncode(255, 255, 240, 207);
  m_pThemeData->clrStart[3] = ArgbEncode(255, 206, 231, 255);
  m_pThemeData->clrStart[4] = ArgbEncode(255, 245, 244, 234);
  m_pThemeData->clrEnd[0] = ArgbEncode(255, 214, 208, 197);
  m_pThemeData->clrEnd[1] = ArgbEncode(255, 242, 241, 238);
  m_pThemeData->clrEnd[2] = ArgbEncode(255, 229, 151, 0);
  m_pThemeData->clrEnd[3] = ArgbEncode(255, 105, 130, 238);
  m_pThemeData->clrEnd[4] = ArgbEncode(255, 245, 244, 234);
  m_pThemeData->clrFill[0] = ArgbEncode(255, 255, 255, 255);
  m_pThemeData->clrFill[1] = ArgbEncode(255, 226, 225, 218);
  m_pThemeData->clrFill[2] = ArgbEncode(255, 255, 255, 255);
  m_pThemeData->clrFill[3] = ArgbEncode(255, 255, 255, 255);
  m_pThemeData->clrFill[4] = ArgbEncode(255, 245, 244, 234);
}

int32_t CFWL_PushButtonTP::GetColorID(Mask<CFWL_PartState> dwStates) const {
  int32_t color = 0;
  if (dwStates & CFWL_PartState::kDisabled)
    color += 4;
  if (dwStates & CFWL_PartState::kDefault) {
    color += 3;
  } else {
    if (dwStates & CFWL_PartState::kHovered)
      color += 2;
    if (dwStates & CFWL_PartState::kPressed)
      color += 1;
  }
  return color;
}
