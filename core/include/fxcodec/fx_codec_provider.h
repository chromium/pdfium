// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCODEC_FX_CODEC_PROVIDER_H_
#define CORE_INCLUDE_FXCODEC_FX_CODEC_PROVIDER_H_

class IFX_JpegProvider
{
public:
    virtual void		Release() = 0;

    virtual void*		CreateDecoder(const uint8_t* src_buf, FX_DWORD src_size, int width, int height, int nComps, FX_BOOL ColorTransform) = 0;


    virtual void		DestroyDecoder(void* pDecoder) = 0;

    virtual void		DownScale(void* pDecoder, int dest_width, int dest_height) = 0;

    virtual FX_BOOL		Rewind(void* pDecoder) = 0;

    virtual uint8_t*	GetNextLine(void* pDecoder) = 0;

    virtual FX_DWORD	GetSrcOffset(void* pDecoder) = 0;


    virtual FX_BOOL		LoadInfo(const uint8_t* src_buf, FX_DWORD src_size, int& width, int& height,
                                 int& num_components, int& bits_per_components, FX_BOOL& color_transform,
                                 uint8_t** icc_buf_ptr = NULL, FX_DWORD* icc_length = NULL) = 0;

    virtual FX_BOOL		Encode(const class CFX_DIBSource* pSource, uint8_t*& dest_buf, FX_STRSIZE& dest_size, int quality = 75,
                               const uint8_t* icc_buf = NULL, FX_DWORD icc_length = 0) = 0;

    virtual void*		Start() = 0;

    virtual void		Finish(void* pContext) = 0;

    virtual void		Input(void* pContext, const uint8_t* src_buf, FX_DWORD src_size) = 0;

    virtual int			ReadHeader(void* pContext, int* width, int* height, int* nComps) = 0;


    virtual int			StartScanline(void* pContext, int down_scale) = 0;


    virtual FX_BOOL		ReadScanline(void* pContext, uint8_t* dest_buf) = 0;


    virtual FX_DWORD	GetAvailInput(void* pContext, uint8_t** avail_buf_ptr = NULL) = 0;

protected:
    ~IFX_JpegProvider() { }
};

#endif  // CORE_INCLUDE_FXCODEC_FX_CODEC_PROVIDER_H_
