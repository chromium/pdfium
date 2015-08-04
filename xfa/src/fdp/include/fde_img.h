// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_IMAGE
#define _FDE_IMAGE
class IFDE_Image;
#define FDE_IMAGEFORMAT_Unknown -1
#define FDE_IMAGEFORMAT_BMP 0
#define FDE_IMAGEFORMAT_GIF 1
#define FDE_IMAGEFORMAT_JPEG 2
#define FDE_IMAGEFORMAT_PNG 3
#define FDE_IMAGEFORMAT_TIFF 4
#define FDE_IMAGEFORMAT_JPEG2000 5
#define FDE_IMAGEFORMAT_JBig2 6
class IFDE_Image {
 public:
  static IFDE_Image* Create(IFX_Stream* pStream,
                            int32_t iFormat = FDE_IMAGEFORMAT_Unknown);
  virtual ~IFDE_Image() {}
  virtual void Release() = 0;
  virtual FX_BOOL LoadImage() = 0;
  virtual void FreeImage() = 0;
  virtual int32_t CountFrames() const = 0;
  virtual FX_BOOL LoadFrame(int32_t index) = 0;
  virtual CFX_DIBitmap* GetFrameImage() = 0;
  virtual int32_t GetImageFormat() const = 0;
  virtual int32_t GetImageWidth() const = 0;
  virtual int32_t GetImageHeight() const = 0;
  virtual int32_t GetDelayTime(int32_t iFrameIndex) const = 0;
  virtual int32_t GetLoopCount() const = 0;
  virtual FX_BOOL StartLoadImage(CFX_DIBitmap* pDIBitmap,
                                 int32_t dibX,
                                 int32_t dibY,
                                 int32_t dibCX,
                                 int32_t dibCY,
                                 int32_t imgX,
                                 int32_t imgY,
                                 int32_t imgCX,
                                 int32_t imgCY,
                                 int32_t iFrameIndex = 0) = 0;
  virtual int32_t DoLoadImage(IFX_Pause* pPause = NULL) = 0;
  virtual void StopLoadImage() = 0;
};
#define FDE_IMAGEFILTER_Unknown -1
#define FDE_IMAGEFILTER_Opacity 0
#define FDE_IMAGEFILTER_GrayScale 1
#define FDE_IMAGEFILTER_BlackWhite 2
#define FDE_IMAGEFILTER_InvertColor 3
#define FDE_IMAGEFILTER_TransparentColor 4
#define FDE_IMAGEFILTER_MaskColor 5
#define FDE_IMAGEFILTER_Brightness 6
#define FDE_IMAGEFILTER_Contrast 7
typedef struct _FDE_IMAGEFILTERPARAMS : public CFX_Target {
  int32_t iFilterType;
} FDE_IMAGEFILTERPARAMS, *FDE_LPIMAGEFILTERPARAMS;
typedef FDE_IMAGEFILTERPARAMS const* FDE_LPCIMAGEFILTERPARAMS;
typedef struct _FDE_OPACITYPARAMS : public FDE_IMAGEFILTERPARAMS {
  int32_t iOpacity;
} FDE_OPACITYPARAMS, *FDE_LPOPACITYPARAMS;
typedef FDE_OPACITYPARAMS const* FDE_LPCOPACITYPARAMS;
typedef struct _FDE_BLACKWHITEPARAMS : public FDE_IMAGEFILTERPARAMS {
  FX_ARGB dwBlackColor;
  FX_ARGB dwWhiteColor;
} FDE_BLACKWHITEPARAMS, *FDE_LPBLACKWHITEPARAMS;
typedef FDE_BLACKWHITEPARAMS const* FDE_LPCBLACKWHITEPARAMS;
typedef struct _FDE_TRANSPARENTPARAMS : public FDE_IMAGEFILTERPARAMS {
  FX_ARGB color;
} FDE_TRANSPARENTPARAMS, *FDE_LPTRANSPARENTPARAMS;
typedef FDE_TRANSPARENTPARAMS const* FDE_LPCTRANSPARENTPARAMS;
typedef struct _FDE_MASKCOLORPARAMS : public FDE_IMAGEFILTERPARAMS {
  FX_ARGB color;
} FDE_MASKCOLORPARAMS, *FDE_LPMASKCOLORPARAMS;
typedef FDE_MASKCOLORPARAMS const* FDE_LPCMASKCOLORPARAMS;
typedef struct _FDE_BRIGHTNESSPARAMS : public FDE_IMAGEFILTERPARAMS {
  int32_t iBrightness;
} FDE_BRIGHTNESSPARAMS, *FDE_LPBRIGHTNESSPARAMS;
typedef FDE_BRIGHTNESSPARAMS const* FDE_LPCBRIGHTNESSPARAMS;
typedef struct _FDE_CONTRASTPARAMS : public FDE_IMAGEFILTERPARAMS {
  int32_t iContrast;
} FDE_CONTRASTPARAMS, *FDE_LPCONTRASTPARAMS;
typedef FDE_CONTRASTPARAMS const* FDE_LPCCONTRASTPARAMS;
#endif
