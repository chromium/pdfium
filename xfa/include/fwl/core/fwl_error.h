// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ERROR_H
#define _FWL_ERROR_H
typedef FX_INT32	FWL_ERR;
#define FWL_ERR_Succeeded						0
#define FWL_ERR_Indefinite						-1
#define FWL_ERR_Parameter_Invalid				-100
#define FWL_ERR_Property_Invalid				-200
#define FWL_ERR_Intermediate_Value__Invalid		-300
#define FWL_ERR_Method_Not_Supported			-400
#define FWL_ERR_Out_Of_Memory					-500
#if defined (__WIN32__) || defined (_WIN32)
#define	_FWL_ALARM_IF_FAIL(arg, alarm) { if (!(arg)) ::OutputDebugString(alarm); }
#elif defined (__linux) || defined (linux) || defined (__APPLE__) || defined (__MACOSX__)
#define _FWL_ALARM_IF_FAIL(arg, alarm) { if (!(arg)) printf(alarm); }
#else
#endif
#define _FWL_RETURN_IF_FAIL(arg) { if (!(arg)) return; }
#define _FWL_RETURN_VALUE_IF_FAIL(arg, val) { if (!(arg)) return val; }
#define _FWL_GOTO_POSITION_IF_FAIL(arg, pos) { if (!(arg)) goto pos; }
#if defined (__WIN32__) || defined (_WIN32)
#define	\
    _FWL_ERR_CHECK_ALARM_IF_FAIL(arg) \
    { \
        if ((arg) != FWL_ERR_Succeeded) \
        { \
            char buf[16]; \
            sprintf(buf, "Error code is %d\n", arg); \
            ::OutputDebugString(buf); \
        } \
    }
#elif defined (__linux) || defined (linux) || defined (__APPLE__) || defined (__MACOSX__)
#define \
    _FWL_ERR_CHECK_ALARM_IF_FAIL(arg) \
    { \
        if ((arg) != FWL_ERR_Succeeded) \
            printf("%d\n", arg); \
    }
#else
#endif
#define _FWL_ERR_CHECK_RETURN_IF_FAIL(arg) { if ((arg) != FWL_ERR_Succeeded) return; }
#define _FWL_ERR_CHECK_RETURN_VALUE_IF_FAIL(arg, val) { if ((arg) != FWL_ERR_Succeeded) return val; }
#define _FWL_ERR_CHECK_GOTO_POSITION_IF_FAIL(arg, pos) { if ((arg) != FWL_ERR_Succeeded) goto pos; }
#endif
