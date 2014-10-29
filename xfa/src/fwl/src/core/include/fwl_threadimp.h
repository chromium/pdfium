// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_THREAD_IMP_H
#define _FWL_THREAD_IMP_H
class CFWL_Target;
class CFWL_NoteDriver;
class IFWL_NoteThread;
class IFWL_NoteDriver;
class CFWL_Thread;
class CFWL_NoteThread;
class CFWL_ControlThread;
class CFWL_Thread : public CFX_Object
{
public:
    CFWL_Thread();
    virtual ~CFWL_Thread();
    virtual void	Release()
    {
        delete this;
    }
    virtual FWL_ERR Run(FWL_HTHREAD hThread);
};
class CFWL_ControlThread : public CFWL_Thread
{
public:
    CFWL_ControlThread(IFWL_Thread * defHandler)
    {
        _defHandler = defHandler;
    }
    virtual ~CFWL_ControlThread();
    virtual FWL_ERR Run(FWL_HTHREAD hThread);
protected:
    IFWL_Thread *	_defHandler;
};
class CFWL_NoteThread : public CFWL_Thread
{
public:
    CFWL_NoteThread();
    virtual ~CFWL_NoteThread();
    virtual FWL_ERR Run(FWL_HTHREAD hThread);
    virtual IFWL_NoteDriver* GetNoteDriver();
protected:
    CFWL_NoteDriver		*m_pNoteDriver;
    FWL_HTHREAD			 m_hThread;
    static CFWL_ControlThread	 *	_assistantThreadHandler;
    static FWL_HTHREAD				_assistantThread;
    static FX_INT32					_refCount;
};
#endif
