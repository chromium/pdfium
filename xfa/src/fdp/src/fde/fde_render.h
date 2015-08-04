// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_RENDER_IMP
#define _FDE_RENDER_IMP
class CFDE_RenderContext;
class CFDE_RenderContext : public IFDE_RenderContext,
                           public CFX_ThreadLock,
                           public CFX_Target {
 public:
  CFDE_RenderContext();
  virtual ~CFDE_RenderContext();
  virtual void Release() { delete this; }
  virtual FX_BOOL StartRender(IFDE_RenderDevice* pRenderDevice,
                              IFDE_CanvasSet* pCanvasSet,
                              const CFX_Matrix& tmDoc2Device);
  virtual FDE_RENDERSTATUS GetStatus() const { return m_eStatus; }
  virtual FDE_RENDERSTATUS DoRender(IFX_Pause* pPause = NULL);
  virtual void StopRender();
  void RenderPath(IFDE_PathSet* pPathSet, FDE_HVISUALOBJ hPath);
  void RenderText(IFDE_TextSet* pTextSet, FDE_HVISUALOBJ hText);
  FX_BOOL ApplyClip(IFDE_VisualSet* pVisualSet,
                    FDE_HVISUALOBJ hObj,
                    FDE_HDEVICESTATE& hState);
  void RestoreClip(FDE_HDEVICESTATE hState);

 protected:
  FDE_RENDERSTATUS m_eStatus;
  IFDE_RenderDevice* m_pRenderDevice;
  IFDE_SolidBrush* m_pSolidBrush;
  CFX_Matrix m_Transform;
  FXTEXT_CHARPOS* m_pCharPos;
  int32_t m_iCharPosCount;
  IFDE_VisualSetIterator* m_pIterator;
};
#endif
