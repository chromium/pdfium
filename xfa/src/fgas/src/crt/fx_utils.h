// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_UTILS_IMP
#define _FX_UTILS_IMP
class CFX_BaseMassArrayImp : public CFX_Target
{
public:
    CFX_BaseMassArrayImp(FX_INT32 iChunkSize, FX_INT32 iBlockSize);
    ~CFX_BaseMassArrayImp();
    FX_LPBYTE	AddSpace()
    {
        return AddSpaceTo(m_iBlockCount);
    }
    FX_LPBYTE	AddSpaceTo(FX_INT32 index);
    FX_LPBYTE	GetAt(FX_INT32 index) const;
    FX_INT32	Append(const CFX_BaseMassArrayImp &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1);
    FX_INT32	Copy(const CFX_BaseMassArrayImp &src, FX_INT32 iStart = 0, FX_INT32 iCount = -1);
    FX_INT32	RemoveLast(FX_INT32 iCount = -1);
    void		RemoveAll(FX_BOOL bLeaveMemory = FALSE);
    FX_INT32		m_iChunkSize;
    FX_INT32		m_iBlockSize;
    FX_INT32		m_iChunkCount;
    FX_INT32		m_iBlockCount;
    CFX_PtrArray	*m_pData;
protected:
    void	Append(FX_INT32 iDstStart, const CFX_BaseMassArrayImp &src, FX_INT32 iSrcStart = 0, FX_INT32 iSrcCount = -1);
};
#endif
