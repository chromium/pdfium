// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TXTEDTBLOCK_H
#define _FDE_TXTEDTBLOCK_H
#ifdef FDE_USEFORMATBLOCK
class CFDE_TxtEdtEngine;
class CFDE_TxtEdtBlock;
class CFDE_TxtEdtFieldFormatParser;
class CFDE_TxtEdtField;
class CFDE_TxtEdtField_Integer;
class CFDE_TxtEdtField_Float;
class CFDE_TxtEdtField_Password;
class CFDE_TxtEdtField_String;
class CFDE_TxtEdtField_Fixed;
#define FDE_FORMAT_EDIT_FIELD_HADERSIZE 3
#define FDE_FORMAT_EDIT_FIELD_TAILSIZE 1
enum FDE_FORMAT_FIELD_INSERT_RET {
  FDE_FORMAT_FIELD_INSERT_RET_S_NORMAL = 0,
  FDE_FORMAT_FIELD_INSERT_RET_S_FULL,
  FDE_FORMAT_FIELD_INSERT_RET_F_FULL,
  FDE_FORMAT_FIELD_INSERT_RET_F_INVALIDATE,
};
enum FDE_FORMAT_FIELD_DELETE_RET {
  FDE_FORMAT_FIELD_DELETE_RET_S = 0,
  FDE_FORMAT_FIELD_DELETE_RET_F_INVALIDATE,
  FDE_FORMAT_FIELD_DELETE_RET_F_BOUNDARY,
};
enum FDE_FORMAT_FIELD_VALIDATE_RET {
  FDE_FORMAT_FIELD_VALIDATE_S = 0,
  FDE_FORMAT_FIELD_VALIDATE_F_FULL,
  FDE_FORMAT_FIELD_VALIDATE_F_INVALIDATE,
};
enum FDE_FORMAT_CARET_DIRECTION {
  FDE_FORMAT_CARET_FORWARD,
  FDE_FORMAT_CARET_MIDDLE,
  FDE_FORMAT_CARET_BACKWARD
};
class CFDE_TxtEdtBlock {
 public:
  CFDE_TxtEdtBlock(CFDE_TxtEdtEngine* pEngine,
                   const CFX_WideString& wsBlock,
                   int32_t nPosition);
  ~CFDE_TxtEdtBlock();
  void GetDisplayText(CFX_WideString& wsDisplay);
  int32_t GetLength() const;
  void GetBlockText(CFX_WideString& wsBlock);
  int32_t CountField() const;
  void GetFieldText(int32_t nIndex, CFX_WideString& wsField);
  int32_t GetFieldTextLength() const;

  int32_t GetPos() const;
  void GetRealText(CFX_WideString& wsText) const;
  void Backup();
  void Restore();
  void SetIndex(int32_t nIndex) { m_nIndex = nIndex; }
  int32_t GetIndex() const { return m_nIndex; }

 private:
  CFDE_TxtEdtEngine* m_pEngine;
  int32_t m_nDisplayLength;
  int32_t m_nIndex;

  int32_t m_nPosition;
  CFX_ArrayTemplate<CFDE_TxtEdtField*> m_FieldArr;
  CFX_ArrayTemplate<CFDE_TxtEdtField*> m_EditFieldArr;
};
class CFDE_TxtEdtFieldFormatParser {
 public:
  CFDE_TxtEdtFieldFormatParser();
  ~CFDE_TxtEdtFieldFormatParser();
  FX_BOOL Parse(const CFX_WideString& wsFormat);
  int32_t CountItems() const;
  void GetItem(int32_t nIndex,
               CFX_WideString& wsKey,
               CFX_WideString& wsValue) const;

 private:
  typedef struct {
    int32_t nKeyStart;
    int32_t nKeyCount;
    int32_t nValStart;
    int32_t nValCount;
  } FDE_TXTEDTFORMATITEM, *FDE_LPTXTEDTFORMATITEM;

  CFX_WideString m_wsFormat;
  CFX_ArrayTemplate<FDE_LPTXTEDTFORMATITEM> m_ItemArr;
};
class CFDE_TxtEdtField {
 public:
  static CFDE_TxtEdtField* Create(const CFX_WideString& wsField,
                                  int32_t nIndex,
                                  CFDE_TxtEdtBlock* pBlock);
  virtual void Release();
  virtual int32_t Insert(int32_t nIndex,
                         const CFX_WideString& wsIns,
                         int32_t& nCaret,
                         FX_BOOL& bBefore);
  virtual int32_t Delete(int32_t nIndex,
                         int32_t nCount,
                         CFX_WideString& wsDel,
                         int32_t& nCaret,
                         FX_BOOL& bBefore);
  virtual int32_t Replace(int32_t nIndex,
                          int32_t nCount,
                          const CFX_WideString& wsIns,
                          CFX_WideString& wsDel,
                          int32_t& nCaret,
                          FX_BOOL& bBefore);
  virtual void GetDisplayText(CFX_WideString& wsDisplay);
  virtual int32_t GetDisplayLength();
  virtual void GetFieldText(CFX_WideString& wsField);
  virtual int32_t GetFieldTextLength() const;
  virtual int32_t GetRealIndex(int32_t nIndex) const;

  virtual int32_t NormalizeCaretPos(
      int32_t nIndex,
      FDE_FORMAT_CARET_DIRECTION eDirection = FDE_FORMAT_CARET_MIDDLE) const;

  virtual FX_BOOL GetEditableRange(int32_t& nBgn, int32_t& nEnd) const;
  virtual void Backup();
  virtual void Restore();
  virtual FX_BOOL IsFix() const { return FALSE; }
  void SetIndex(int32_t nIndex) { m_nIndex = nIndex; }
  int32_t GetIndex() const { return m_nIndex; }
  int32_t GetBlockIndex() const { return m_pBlock->GetIndex(); }

 protected:
  CFDE_TxtEdtField(int32_t nIndex, CFDE_TxtEdtBlock* pBlock);
  virtual ~CFDE_TxtEdtField() {}
  virtual int32_t Validate(const CFX_WideString& wsText) const;
  virtual void GetNormalizedFieldText(CFX_WideString& wsField) const;
  int32_t m_nLength;
  CFX_WideString m_wsField;
  CFX_WideString m_wsBackup;
  FX_WCHAR m_wcFill;
  FX_BOOL m_bReserveSpace;
  FX_BOOL m_bLeftAlignment;
  int32_t m_nIndex;
  CFDE_TxtEdtBlock* m_pBlock;
};
class CFDE_TxtEdtField_Integer : public CFDE_TxtEdtField {
 public:
  CFDE_TxtEdtField_Integer(const CFX_WideString& wsField,
                           int32_t nIndex,
                           CFDE_TxtEdtBlock* pBlock);

 protected:
  virtual ~CFDE_TxtEdtField_Integer() {}
  virtual int32_t Validate(const CFX_WideString& wsText) const;

 private:
  FX_BOOL m_bSign;
};
class CFDE_TxtEdtField_Float : public CFDE_TxtEdtField {
 public:
  CFDE_TxtEdtField_Float(const CFX_WideString& wsField,
                         int32_t nIndex,
                         CFDE_TxtEdtBlock* pBlock);

 protected:
  virtual ~CFDE_TxtEdtField_Float() {}
  virtual int32_t Validate(const CFX_WideString& wsText) const;

 private:
  FX_BOOL m_bSigned;
  int32_t m_nIntPartlength;
  int32_t m_nDecPartLength;
};
class CFDE_TxtEdtField_Password : public CFDE_TxtEdtField {
 public:
  CFDE_TxtEdtField_Password(const CFX_WideString& wsField,
                            int32_t nIndex,
                            CFDE_TxtEdtBlock* pBlock);

 protected:
  virtual ~CFDE_TxtEdtField_Password() {}
  virtual void GetNormalizedFieldText(CFX_WideString& wsField) const;

 private:
  FX_WCHAR m_wcAlias;
};
class CFDE_TxtEdtField_String : public CFDE_TxtEdtField {
 public:
  CFDE_TxtEdtField_String(const CFX_WideString& wsField,
                          int32_t nIndex,
                          CFDE_TxtEdtBlock* pBlock);

 protected:
  virtual ~CFDE_TxtEdtField_String() {}
};
class CFDE_TxtEdtField_Fixed : public CFDE_TxtEdtField {
 public:
  CFDE_TxtEdtField_Fixed(const CFX_WideString& wsField,
                         int32_t nIndex,
                         CFDE_TxtEdtBlock* pBlock);
  virtual int32_t Insert(int32_t nIndex,
                         const CFX_WideString& wsIns,
                         int32_t& nCaret,
                         FX_BOOL& bBefore) {
    return FALSE;
  }
  virtual int32_t Delete(int32_t nIndex,
                         int32_t nCount,
                         CFX_WideString& wsDel,
                         int32_t& nCaret,
                         FX_BOOL& bBefore) {
    return FALSE;
  }
  virtual int32_t Replace(int32_t nIndex,
                          int32_t nCount,
                          const CFX_WideString& wsIns,
                          CFX_WideString& wsDel,
                          int32_t& nCaret,
                          FX_BOOL& bBefore) {
    return FALSE;
  }
  virtual void GetDisplayText(CFX_WideString& wsDisplay);
  virtual int32_t NormalizeCaretPos(
      int32_t nIndex,
      FDE_FORMAT_CARET_DIRECTION eDirection) const;
  virtual FX_BOOL GetEditableRange(int32_t& nBgn, int32_t& nEnd) const {
    return FALSE;
  }
  virtual void Backup() {}
  virtual void Restore() {}

  virtual FX_BOOL IsFix() const { return TRUE; }

 protected:
  virtual ~CFDE_TxtEdtField_Fixed() {}
};
#endif
#endif
