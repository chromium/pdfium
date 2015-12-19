// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfdoc/fpdf_doc.h"
#include "doc_utils.h"

FX_BOOL PDF_FormField_IsUnison(CPDF_FormField* pField) {
  FX_BOOL bUnison = FALSE;
  if (pField->GetType() == CPDF_FormField::CheckBox) {
    bUnison = TRUE;
  } else {
    FX_DWORD dwFlags = pField->GetFieldFlags();
    bUnison = ((dwFlags & 0x2000000) != 0);
  }
  return bUnison;
}
CPDF_FormField::CPDF_FormField(CPDF_InterForm* pForm, CPDF_Dictionary* pDict) {
  m_pDict = pDict;
  m_Type = Unknown;
  m_pForm = pForm;
  m_pFont = NULL;
  m_FontSize = 0;
  SyncFieldFlags();
}
CPDF_FormField::~CPDF_FormField() {}
void CPDF_FormField::SyncFieldFlags() {
  CFX_ByteString type_name = FPDF_GetFieldAttr(m_pDict, "FT")
                                 ? FPDF_GetFieldAttr(m_pDict, "FT")->GetString()
                                 : CFX_ByteString();
  FX_DWORD flags = FPDF_GetFieldAttr(m_pDict, "Ff")
                       ? FPDF_GetFieldAttr(m_pDict, "Ff")->GetInteger()
                       : 0;
  m_Flags = 0;
  if (flags & 1) {
    m_Flags |= FORMFIELD_READONLY;
  }
  if (flags & 2) {
    m_Flags |= FORMFIELD_REQUIRED;
  }
  if (flags & 4) {
    m_Flags |= FORMFIELD_NOEXPORT;
  }
  if (type_name == "Btn") {
    if (flags & 0x8000) {
      m_Type = RadioButton;
      if (flags & 0x4000) {
        m_Flags |= FORMRADIO_NOTOGGLEOFF;
      }
      if (flags & 0x2000000) {
        m_Flags |= FORMRADIO_UNISON;
      }
    } else if (flags & 0x10000) {
      m_Type = PushButton;
    } else {
      m_Type = CheckBox;
    }
  } else if (type_name == "Tx") {
    if (flags & 0x100000) {
      m_Type = File;
    } else if (flags & 0x2000000) {
      m_Type = RichText;
    } else {
      m_Type = Text;
      if (flags & 0x1000) {
        m_Flags |= FORMTEXT_MULTILINE;
      }
      if (flags & 0x2000) {
        m_Flags |= FORMTEXT_PASSWORD;
      }
      if (flags & 0x800000) {
        m_Flags |= FORMTEXT_NOSCROLL;
      }
      if (flags & 0x100000) {
        m_Flags |= FORMTEXT_COMB;
      }
    }
    LoadDA();
  } else if (type_name == "Ch") {
    if (flags & 0x20000) {
      m_Type = ComboBox;
      if (flags & 0x40000) {
        m_Flags |= FORMCOMBO_EDIT;
      }
    } else {
      m_Type = ListBox;
      if (flags & 0x200000) {
        m_Flags |= FORMLIST_MULTISELECT;
      }
    }
    LoadDA();
  } else if (type_name == "Sig") {
    m_Type = Sign;
  }
}
CFX_WideString CPDF_FormField::GetFullName() {
  return ::GetFullName(m_pDict);
}
FX_BOOL CPDF_FormField::ResetField(FX_BOOL bNotify) {
  switch (m_Type) {
    case CPDF_FormField::CheckBox:
    case CPDF_FormField::RadioButton: {
      CFX_ByteArray statusArray;
      if (bNotify && m_pForm->m_pFormNotify) {
        SaveCheckedFieldStatus(this, statusArray);
      }
      int iCount = CountControls();
      if (iCount) {
        if (PDF_FormField_IsUnison(this)) {
          for (int i = 0; i < iCount; i++) {
            CheckControl(i, GetControl(i)->IsDefaultChecked(), FALSE);
          }
        } else {
          for (int i = 0; i < iCount; i++) {
            CPDF_FormControl* pControl = GetControl(i);
            FX_BOOL bChecked = pControl->IsDefaultChecked();
            CheckControl(i, bChecked, FALSE);
          }
        }
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        m_pForm->m_pFormNotify->AfterCheckedStatusChange(this, statusArray);
      }
    } break;
    case CPDF_FormField::ComboBox: {
      CFX_WideString csValue;
      ClearSelection();
      int iIndex = GetDefaultSelectedItem();
      if (iIndex >= 0) {
        csValue = GetOptionLabel(iIndex);
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        int iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csValue);
        if (iRet < 0) {
          return FALSE;
        }
      }
      SetItemSelection(iIndex, TRUE);
      if (bNotify && m_pForm->m_pFormNotify) {
        m_pForm->m_pFormNotify->AfterValueChange(this);
      }
    } break;
    case CPDF_FormField::ListBox: {
      CFX_WideString csValue;
      ClearSelection();
      int iIndex = GetDefaultSelectedItem();
      if (iIndex >= 0) {
        csValue = GetOptionLabel(iIndex);
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        int iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, csValue);
        if (iRet < 0) {
          return FALSE;
        }
      }
      SetItemSelection(iIndex, TRUE);
      if (bNotify && m_pForm->m_pFormNotify) {
        m_pForm->m_pFormNotify->AfterSelectionChange(this);
      }
    } break;
    case CPDF_FormField::Text:
    case CPDF_FormField::RichText:
    case CPDF_FormField::File:
    default: {
      CPDF_Object* pDV = FPDF_GetFieldAttr(m_pDict, "DV");
      CFX_WideString csDValue;
      if (pDV) {
        csDValue = pDV->GetUnicodeText();
      }
      CPDF_Object* pV = FPDF_GetFieldAttr(m_pDict, "V");
      CFX_WideString csValue;
      if (pV) {
        csValue = pV->GetUnicodeText();
      }
      CPDF_Object* pRV = FPDF_GetFieldAttr(m_pDict, "RV");
      if (!pRV && (csDValue == csValue)) {
        return FALSE;
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        int iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csDValue);
        if (iRet < 0) {
          return FALSE;
        }
      }
      if (pDV) {
        CPDF_Object* pClone = pDV->Clone();
        if (!pClone) {
          return FALSE;
        }
        m_pDict->SetAt("V", pClone);
        if (pRV) {
          CPDF_Object* pCloneR = pDV->Clone();
          m_pDict->SetAt("RV", pCloneR);
        }
      } else {
        m_pDict->RemoveAt("V");
        m_pDict->RemoveAt("RV");
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        m_pForm->m_pFormNotify->AfterValueChange(this);
      }
      m_pForm->m_bUpdated = TRUE;
    } break;
  }
  return TRUE;
}
int CPDF_FormField::GetControlIndex(const CPDF_FormControl* pControl) {
  if (!pControl) {
    return -1;
  }
  for (int i = 0; i < m_ControlList.GetSize(); i++) {
    if (m_ControlList.GetAt(i) == pControl)
      return i;
  }
  return -1;
}
int CPDF_FormField::GetFieldType() {
  switch (m_Type) {
    case PushButton:
      return FIELDTYPE_PUSHBUTTON;
    case CheckBox:
      return FIELDTYPE_CHECKBOX;
    case RadioButton:
      return FIELDTYPE_RADIOBUTTON;
    case ComboBox:
      return FIELDTYPE_COMBOBOX;
    case ListBox:
      return FIELDTYPE_LISTBOX;
    case Text:
    case RichText:
    case File:
      return FIELDTYPE_TEXTFIELD;
    case Sign:
      return FIELDTYPE_SIGNATURE;
    default:
      break;
  }
  return FIELDTYPE_UNKNOWN;
}
CPDF_AAction CPDF_FormField::GetAdditionalAction() {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "AA");
  if (!pObj) {
    return NULL;
  }
  return pObj->GetDict();
}
CFX_WideString CPDF_FormField::GetAlternateName() {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "TU");
  if (!pObj) {
    return L"";
  }
  return pObj->GetUnicodeText();
}
CFX_WideString CPDF_FormField::GetMappingName() {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "TM");
  if (!pObj) {
    return L"";
  }
  return pObj->GetUnicodeText();
}
FX_DWORD CPDF_FormField::GetFieldFlags() {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "Ff");
  if (!pObj) {
    return 0;
  }
  return pObj->GetInteger();
}
CFX_ByteString CPDF_FormField::GetDefaultStyle() {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "DS");
  if (!pObj) {
    return "";
  }
  return pObj->GetString();
}
CFX_WideString CPDF_FormField::GetRichTextString() {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "RV");
  if (!pObj) {
    return L"";
  }
  return pObj->GetUnicodeText();
}
CFX_WideString CPDF_FormField::GetValue(FX_BOOL bDefault) {
  if (GetType() == CheckBox || GetType() == RadioButton) {
    return GetCheckValue(bDefault);
  }
  CPDF_Object* pValue = FPDF_GetFieldAttr(m_pDict, bDefault ? "DV" : "V");
  if (!pValue) {
    if (!bDefault) {
      if (m_Type == RichText) {
        pValue = FPDF_GetFieldAttr(m_pDict, "V");
      }
      if (!pValue && m_Type != Text) {
        pValue = FPDF_GetFieldAttr(m_pDict, "DV");
      }
    }
    if (!pValue) {
      return CFX_WideString();
    }
  }
  switch (pValue->GetType()) {
    case PDFOBJ_STRING:
    case PDFOBJ_STREAM:
      return pValue->GetUnicodeText();
    case PDFOBJ_ARRAY:
      pValue = pValue->AsArray()->GetElementValue(0);
      if (pValue)
        return pValue->GetUnicodeText();
      break;
  }
  return CFX_WideString();
}
CFX_WideString CPDF_FormField::GetValue() {
  return GetValue(FALSE);
}
CFX_WideString CPDF_FormField::GetDefaultValue() {
  return GetValue(TRUE);
}
FX_BOOL CPDF_FormField::SetValue(const CFX_WideString& value,
                                 FX_BOOL bDefault,
                                 FX_BOOL bNotify) {
  switch (m_Type) {
    case CheckBox:
    case RadioButton: {
      SetCheckValue(value, bDefault, bNotify);
      return TRUE;
    }
    case File:
    case RichText:
    case Text:
    case ComboBox: {
      CFX_WideString csValue = value;
      if (bNotify && m_pForm->m_pFormNotify) {
        int iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csValue);
        if (iRet < 0) {
          return FALSE;
        }
      }
      int iIndex = FindOptionValue(csValue);
      if (iIndex < 0) {
        CFX_ByteString bsEncodeText = PDF_EncodeText(csValue);
        m_pDict->SetAtString(bDefault ? "DV" : "V", bsEncodeText);
        if (m_Type == RichText && !bDefault) {
          m_pDict->SetAtString("RV", bsEncodeText);
        }
        m_pDict->RemoveAt("I");
      } else {
        m_pDict->SetAtString(bDefault ? "DV" : "V", PDF_EncodeText(csValue));
        if (bDefault) {
        } else {
          ClearSelection();
          SetItemSelection(iIndex, TRUE);
        }
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        m_pForm->m_pFormNotify->AfterValueChange(this);
      }
      m_pForm->m_bUpdated = TRUE;
    } break;
    case ListBox: {
      int iIndex = FindOptionValue(value);
      if (iIndex < 0) {
        return FALSE;
      }
      if (bDefault && iIndex == GetDefaultSelectedItem()) {
        return FALSE;
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        CFX_WideString csValue = value;
        int iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, csValue);
        if (iRet < 0) {
          return FALSE;
        }
      }
      if (bDefault) {
      } else {
        ClearSelection();
        SetItemSelection(iIndex, TRUE);
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        m_pForm->m_pFormNotify->AfterSelectionChange(this);
      }
      m_pForm->m_bUpdated = TRUE;
      break;
    }
    default:
      break;
  }
  if (CPDF_InterForm::m_bUpdateAP) {
    UpdateAP(NULL);
  }
  return TRUE;
}
FX_BOOL CPDF_FormField::SetValue(const CFX_WideString& value, FX_BOOL bNotify) {
  return SetValue(value, FALSE, bNotify);
}
int CPDF_FormField::GetMaxLen() {
  if (CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "MaxLen"))
    return pObj->GetInteger();

  for (int i = 0; i < m_ControlList.GetSize(); i++) {
    CPDF_FormControl* pControl = m_ControlList.GetAt(i);
    if (!pControl)
      continue;

    CPDF_Dictionary* pWidgetDict = pControl->m_pWidgetDict;
    if (pWidgetDict->KeyExist("MaxLen"))
      return pWidgetDict->GetInteger("MaxLen");
  }
  return 0;
}
int CPDF_FormField::CountSelectedItems() {
  CPDF_Object* pValue = FPDF_GetFieldAttr(m_pDict, "V");
  if (!pValue) {
    pValue = FPDF_GetFieldAttr(m_pDict, "I");
    if (!pValue)
      return 0;
  }

  if (pValue->IsString() || pValue->IsNumber())
    return pValue->GetString().IsEmpty() ? 0 : 1;
  if (CPDF_Array* pArray = pValue->AsArray())
    return pArray->GetCount();
  return 0;
}
int CPDF_FormField::GetSelectedIndex(int index) {
  CPDF_Object* pValue = FPDF_GetFieldAttr(m_pDict, "V");
  if (!pValue) {
    pValue = FPDF_GetFieldAttr(m_pDict, "I");
    if (!pValue)
      return -1;
  }
  if (pValue->IsNumber())
    return pValue->GetInteger();

  CFX_WideString sel_value;
  if (pValue->IsString()) {
    if (index != 0)
      return -1;
    sel_value = pValue->GetUnicodeText();
  } else {
    CPDF_Array* pArray = pValue->AsArray();
    if (!pArray || index < 0)
      return -1;

    CPDF_Object* elementValue = pArray->GetElementValue(index);
    sel_value =
        elementValue ? elementValue->GetUnicodeText() : CFX_WideString();
  }
  if (index < CountSelectedOptions()) {
    int iOptIndex = GetSelectedOptionIndex(index);
    CFX_WideString csOpt = GetOptionValue(iOptIndex);
    if (csOpt == sel_value) {
      return iOptIndex;
    }
  }
  int nOpts = CountOptions();
  for (int i = 0; i < nOpts; i++) {
    if (sel_value == GetOptionValue(i)) {
      return i;
    }
  }
  return -1;
}
FX_BOOL CPDF_FormField::ClearSelection(FX_BOOL bNotify) {
  if (bNotify && m_pForm->m_pFormNotify) {
    int iRet = 0;
    CFX_WideString csValue;
    int iIndex = GetSelectedIndex(0);
    if (iIndex >= 0) {
      csValue = GetOptionLabel(iIndex);
    }
    if (GetType() == ListBox) {
      iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, csValue);
    }
    if (GetType() == ComboBox) {
      iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csValue);
    }
    if (iRet < 0) {
      return FALSE;
    }
  }
  m_pDict->RemoveAt("V");
  m_pDict->RemoveAt("I");
  if (bNotify && m_pForm->m_pFormNotify) {
    if (GetType() == ListBox) {
      m_pForm->m_pFormNotify->AfterSelectionChange(this);
    }
    if (GetType() == ComboBox) {
      m_pForm->m_pFormNotify->AfterValueChange(this);
    }
  }
  if (CPDF_InterForm::m_bUpdateAP) {
    UpdateAP(NULL);
  }
  m_pForm->m_bUpdated = TRUE;
  return TRUE;
}
FX_BOOL CPDF_FormField::IsItemSelected(int index) {
  ASSERT(GetType() == ComboBox || GetType() == ListBox);
  if (index < 0 || index >= CountOptions()) {
    return FALSE;
  }
  if (IsOptionSelected(index)) {
    return TRUE;
  }
  CFX_WideString opt_value = GetOptionValue(index);
  CPDF_Object* pValue = FPDF_GetFieldAttr(m_pDict, "V");
  if (!pValue) {
    pValue = FPDF_GetFieldAttr(m_pDict, "I");
    if (!pValue) {
      return FALSE;
    }
  }

  if (pValue->IsString())
    return pValue->GetUnicodeText() == opt_value;

  if (pValue->IsNumber()) {
    if (pValue->GetString().IsEmpty())
      return FALSE;
    return (pValue->GetInteger() == index);
  }

  CPDF_Array* pArray = pValue->AsArray();
  if (!pArray)
    return FALSE;

  int iPos = -1;
  for (int j = 0; j < CountSelectedOptions(); j++) {
    if (GetSelectedOptionIndex(j) == index) {
      iPos = j;
      break;
    }
  }
  for (FX_DWORD i = 0; i < pArray->GetCount(); i++)
    if (pArray->GetElementValue(i)->GetUnicodeText() == opt_value &&
        (int)i == iPos) {
      return TRUE;
    }
  return FALSE;
}
FX_BOOL CPDF_FormField::SetItemSelection(int index,
                                         FX_BOOL bSelected,
                                         FX_BOOL bNotify) {
  ASSERT(GetType() == ComboBox || GetType() == ListBox);
  if (index < 0 || index >= CountOptions()) {
    return FALSE;
  }
  CFX_WideString opt_value = GetOptionValue(index);
  if (bNotify && m_pForm->m_pFormNotify) {
    int iRet = 0;
    if (GetType() == ListBox) {
      iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, opt_value);
    }
    if (GetType() == ComboBox) {
      iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, opt_value);
    }
    if (iRet < 0) {
      return FALSE;
    }
  }
  if (!bSelected) {
    CPDF_Object* pValue = FPDF_GetFieldAttr(m_pDict, "V");
    if (pValue) {
      if (m_Type == ListBox) {
        SelectOption(index, FALSE);
        if (pValue->IsString()) {
          if (pValue->GetUnicodeText() == opt_value) {
            m_pDict->RemoveAt("V");
          }
        } else if (pValue->IsArray()) {
          CPDF_Array* pArray = new CPDF_Array;
          int iCount = CountOptions();
          for (int i = 0; i < iCount; i++) {
            if (i != index) {
              if (IsItemSelected(i)) {
                opt_value = GetOptionValue(i);
                pArray->AddString(PDF_EncodeText(opt_value));
              }
            }
          }
          if (pArray->GetCount() < 1) {
            pArray->Release();
          } else {
            m_pDict->SetAt("V", pArray);
          }
        }
      } else if (m_Type == ComboBox) {
        m_pDict->RemoveAt("V");
        m_pDict->RemoveAt("I");
      }
    }
  } else {
    if (m_Type == ListBox) {
      SelectOption(index, TRUE);
      if (!(m_Flags & FORMLIST_MULTISELECT)) {
        m_pDict->SetAtString("V", PDF_EncodeText(opt_value));
      } else {
        CPDF_Array* pArray = new CPDF_Array;
        int iCount = CountOptions();
        for (int i = 0; i < iCount; i++) {
          FX_BOOL bSelected;
          if (i != index) {
            bSelected = IsItemSelected(i);
          } else {
            bSelected = TRUE;
          }
          if (bSelected) {
            opt_value = GetOptionValue(i);
            pArray->AddString(PDF_EncodeText(opt_value));
          }
        }
        m_pDict->SetAt("V", pArray);
      }
    } else if (m_Type == ComboBox) {
      m_pDict->SetAtString("V", PDF_EncodeText(opt_value));
      CPDF_Array* pI = new CPDF_Array;
      pI->AddInteger(index);
      m_pDict->SetAt("I", pI);
    }
  }
  if (bNotify && m_pForm->m_pFormNotify) {
    if (GetType() == ListBox) {
      m_pForm->m_pFormNotify->AfterSelectionChange(this);
    }
    if (GetType() == ComboBox) {
      m_pForm->m_pFormNotify->AfterValueChange(this);
    }
  }
  if (CPDF_InterForm::m_bUpdateAP) {
    UpdateAP(NULL);
  }
  m_pForm->m_bUpdated = TRUE;
  return TRUE;
}
FX_BOOL CPDF_FormField::IsItemDefaultSelected(int index) {
  ASSERT(GetType() == ComboBox || GetType() == ListBox);
  if (index < 0 || index >= CountOptions()) {
    return FALSE;
  }
  int iDVIndex = GetDefaultSelectedItem();
  if (iDVIndex < 0) {
    return FALSE;
  }
  return (iDVIndex == index);
}
int CPDF_FormField::GetDefaultSelectedItem() {
  ASSERT(GetType() == ComboBox || GetType() == ListBox);
  CPDF_Object* pValue = FPDF_GetFieldAttr(m_pDict, "DV");
  if (!pValue) {
    return -1;
  }
  CFX_WideString csDV = pValue->GetUnicodeText();
  if (csDV.IsEmpty()) {
    return -1;
  }
  int iCount = CountOptions();
  for (int i = 0; i < iCount; i++) {
    if (csDV == GetOptionValue(i)) {
      return i;
    }
  }
  return -1;
}
void CPDF_FormField::UpdateAP(CPDF_FormControl* pControl) {
  if (m_Type == PushButton) {
    return;
  }
  if (m_Type == RadioButton || m_Type == CheckBox) {
    return;
  }
  if (!m_pForm->m_bGenerateAP) {
    return;
  }
  for (int i = 0; i < CountControls(); i++) {
    CPDF_FormControl* pControl = GetControl(i);
    FPDF_GenerateAP(m_pForm->m_pDocument, pControl->m_pWidgetDict);
  }
}
int CPDF_FormField::CountOptions() {
  CPDF_Array* pArray = ToArray(FPDF_GetFieldAttr(m_pDict, "Opt"));
  return pArray ? pArray->GetCount() : 0;
}
CFX_WideString CPDF_FormField::GetOptionText(int index, int sub_index) {
  CPDF_Array* pArray = ToArray(FPDF_GetFieldAttr(m_pDict, "Opt"));
  if (!pArray)
    return CFX_WideString();

  CPDF_Object* pOption = pArray->GetElementValue(index);
  if (!pOption)
    return CFX_WideString();
  if (CPDF_Array* pOptionArray = pOption->AsArray())
    pOption = pOptionArray->GetElementValue(sub_index);

  CPDF_String* pString = ToString(pOption);
  return pString ? pString->GetUnicodeText() : CFX_WideString();
}
CFX_WideString CPDF_FormField::GetOptionLabel(int index) {
  return GetOptionText(index, 1);
}
CFX_WideString CPDF_FormField::GetOptionValue(int index) {
  return GetOptionText(index, 0);
}
int CPDF_FormField::FindOption(CFX_WideString csOptLabel) {
  int iCount = CountOptions();
  for (int i = 0; i < iCount; i++) {
    CFX_WideString csValue = GetOptionValue(i);
    if (csValue == csOptLabel) {
      return i;
    }
  }
  return -1;
}
int CPDF_FormField::FindOptionValue(const CFX_WideString& csOptValue,
                                    int iStartIndex) {
  if (iStartIndex < 0) {
    iStartIndex = 0;
  }
  int iCount = CountOptions();
  for (; iStartIndex < iCount; iStartIndex++) {
    CFX_WideString csValue = GetOptionValue(iStartIndex);
    if (csValue == csOptValue) {
      return iStartIndex;
    }
  }
  return -1;
}
#ifdef PDF_ENABLE_XFA
int CPDF_FormField::InsertOption(CFX_WideString csOptLabel,
                                 int index,
                                 FX_BOOL bNotify) {
  if (csOptLabel.IsEmpty())
    return -1;

  if (bNotify && m_pForm->m_pFormNotify) {
    int iRet = 0;
    if (GetType() == ListBox)
      iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, csOptLabel);
    if (GetType() == ComboBox)
      iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csOptLabel);
    if (iRet < 0)
      return -1;
  }

  CFX_ByteString csStr = PDF_EncodeText(csOptLabel, csOptLabel.GetLength());
  CPDF_Object* pValue = FPDF_GetFieldAttr(m_pDict, "Opt");
  CPDF_Array* pOpt = ToArray(pValue);
  if (!pOpt) {
    pOpt = new CPDF_Array;
    m_pDict->SetAt("Opt", pOpt);
  }

  int iCount = (int)pOpt->GetCount();
  if (index < 0 || index >= iCount) {
    pOpt->AddString(csStr);
    index = iCount;
  } else {
    CPDF_String* pString = new CPDF_String(csStr, FALSE);
    pOpt->InsertAt(index, pString);
  }

  if (bNotify && m_pForm->m_pFormNotify) {
    if (GetType() == ListBox)
      m_pForm->m_pFormNotify->AfterSelectionChange(this);
    if (GetType() == ComboBox)
      m_pForm->m_pFormNotify->AfterValueChange(this);
  }
  m_pForm->m_bUpdated = TRUE;
  return index;
}
FX_BOOL CPDF_FormField::ClearOptions(FX_BOOL bNotify) {
  if (bNotify && m_pForm->m_pFormNotify) {
    int iRet = 0;
    CFX_WideString csValue;
    int iIndex = GetSelectedIndex(0);
    if (iIndex >= 0)
      csValue = GetOptionLabel(iIndex);
    if (GetType() == ListBox)
      iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, csValue);
    if (GetType() == ComboBox)
      iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csValue);
    if (iRet < 0)
      return FALSE;
  }

  m_pDict->RemoveAt("Opt");
  m_pDict->RemoveAt("V");
  m_pDict->RemoveAt("DV");
  m_pDict->RemoveAt("I");
  m_pDict->RemoveAt("TI");

  if (bNotify && m_pForm->m_pFormNotify) {
    if (GetType() == ListBox)
      m_pForm->m_pFormNotify->AfterSelectionChange(this);
    if (GetType() == ComboBox)
      m_pForm->m_pFormNotify->AfterValueChange(this);
  }

  m_pForm->m_bUpdated = TRUE;
  return TRUE;
}
#endif  // PDF_ENABLE_XFA
FX_BOOL CPDF_FormField::CheckControl(int iControlIndex,
                                     FX_BOOL bChecked,
                                     FX_BOOL bNotify) {
  ASSERT(GetType() == CheckBox || GetType() == RadioButton);
  CPDF_FormControl* pControl = GetControl(iControlIndex);
  if (!pControl) {
    return FALSE;
  }
  if (!bChecked && pControl->IsChecked() == bChecked) {
    return FALSE;
  }
  CFX_ByteArray statusArray;
  if (bNotify && m_pForm->m_pFormNotify) {
    SaveCheckedFieldStatus(this, statusArray);
  }
  CFX_WideString csWExport = pControl->GetExportValue();
  CFX_ByteString csBExport = PDF_EncodeText(csWExport);
  int iCount = CountControls();
  FX_BOOL bUnison = PDF_FormField_IsUnison(this);
  for (int i = 0; i < iCount; i++) {
    CPDF_FormControl* pCtrl = GetControl(i);
    if (bUnison) {
      CFX_WideString csEValue = pCtrl->GetExportValue();
      if (csEValue == csWExport) {
        if (pCtrl->GetOnStateName() == pControl->GetOnStateName()) {
          pCtrl->CheckControl(bChecked);
        } else if (bChecked) {
          pCtrl->CheckControl(FALSE);
        }
      } else if (bChecked) {
        pCtrl->CheckControl(FALSE);
      }
    } else {
      if (i == iControlIndex) {
        pCtrl->CheckControl(bChecked);
      } else if (bChecked) {
        pCtrl->CheckControl(FALSE);
      }
    }
  }
  CPDF_Object* pOpt = FPDF_GetFieldAttr(m_pDict, "Opt");
  if (!ToArray(pOpt)) {
    if (bChecked) {
      m_pDict->SetAtName("V", csBExport);
    } else {
      CFX_ByteString csV;
      CPDF_Object* pV = FPDF_GetFieldAttr(m_pDict, "V");
      if (pV) {
        csV = pV->GetString();
      }
      if (csV == csBExport) {
        m_pDict->SetAtName("V", "Off");
      }
    }
  } else if (bChecked) {
    CFX_ByteString csIndex;
    csIndex.Format("%d", iControlIndex);
    m_pDict->SetAtName("V", csIndex);
  }
  if (bNotify && m_pForm->m_pFormNotify) {
    m_pForm->m_pFormNotify->AfterCheckedStatusChange(this, statusArray);
  }
  m_pForm->m_bUpdated = TRUE;
  return TRUE;
}
CFX_WideString CPDF_FormField::GetCheckValue(FX_BOOL bDefault) {
  ASSERT(GetType() == CheckBox || GetType() == RadioButton);
  CFX_WideString csExport = L"Off";
  FX_BOOL bChecked;
  int iCount = CountControls();
  for (int i = 0; i < iCount; i++) {
    CPDF_FormControl* pControl = GetControl(i);
    if (bDefault) {
      bChecked = pControl->IsDefaultChecked();
    } else {
      bChecked = pControl->IsChecked();
    }
    if (bChecked) {
      csExport = pControl->GetExportValue();
      break;
    }
  }
  return csExport;
}
FX_BOOL CPDF_FormField::SetCheckValue(const CFX_WideString& value,
                                      FX_BOOL bDefault,
                                      FX_BOOL bNotify) {
  ASSERT(GetType() == CheckBox || GetType() == RadioButton);
  CFX_ByteArray statusArray;
  if (bNotify && m_pForm->m_pFormNotify) {
    SaveCheckedFieldStatus(this, statusArray);
  }
  int iCount = CountControls();
  for (int i = 0; i < iCount; i++) {
    CPDF_FormControl* pControl = GetControl(i);
    CFX_WideString csExport = pControl->GetExportValue();
    if (csExport == value) {
      if (bDefault) {
      } else {
        CheckControl(GetControlIndex(pControl), TRUE);
      }
      break;
    } else {
      if (bDefault) {
      } else {
        CheckControl(GetControlIndex(pControl), FALSE);
      }
    }
  }
  if (bNotify && m_pForm->m_pFormNotify) {
    m_pForm->m_pFormNotify->AfterCheckedStatusChange(this, statusArray);
  }
  m_pForm->m_bUpdated = TRUE;
  return TRUE;
}
int CPDF_FormField::GetTopVisibleIndex() {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "TI");
  if (!pObj) {
    return 0;
  }
  return pObj->GetInteger();
}
int CPDF_FormField::CountSelectedOptions() {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "I");
  if (!pObj) {
    return 0;
  }
  CPDF_Array* pArray = pObj->GetArray();
  if (!pArray) {
    return 0;
  }
  return (int)pArray->GetCount();
}
int CPDF_FormField::GetSelectedOptionIndex(int index) {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "I");
  if (!pObj) {
    return -1;
  }
  CPDF_Array* pArray = pObj->GetArray();
  if (!pArray) {
    return -1;
  }
  int iCount = (int)pArray->GetCount();
  if (iCount > 0 && index < iCount) {
    return pArray->GetInteger(index);
  }
  return -1;
}
FX_BOOL CPDF_FormField::IsOptionSelected(int iOptIndex) {
  CPDF_Object* pObj = FPDF_GetFieldAttr(m_pDict, "I");
  if (!pObj) {
    return FALSE;
  }
  CPDF_Array* pArray = pObj->GetArray();
  if (!pArray) {
    return FALSE;
  }
  int iCount = (int)pArray->GetCount();
  for (int i = 0; i < iCount; i++) {
    if (pArray->GetInteger(i) == iOptIndex) {
      return TRUE;
    }
  }
  return FALSE;
}
FX_BOOL CPDF_FormField::SelectOption(int iOptIndex,
                                     FX_BOOL bSelected,
                                     FX_BOOL bNotify) {
  CPDF_Array* pArray = m_pDict->GetArray("I");
  if (!pArray) {
    if (!bSelected) {
      return TRUE;
    }
    pArray = new CPDF_Array;
    m_pDict->SetAt("I", pArray);
  }
  FX_BOOL bReturn = FALSE;
  for (int i = 0; i < (int)pArray->GetCount(); i++) {
    int iFind = pArray->GetInteger(i);
    if (iFind == iOptIndex) {
      if (bSelected) {
        return TRUE;
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        int iRet = 0;
        CFX_WideString csValue = GetOptionLabel(iOptIndex);
        if (GetType() == ListBox) {
          iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, csValue);
        }
        if (GetType() == ComboBox) {
          iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csValue);
        }
        if (iRet < 0) {
          return FALSE;
        }
      }
      pArray->RemoveAt(i);
      bReturn = TRUE;
      break;
    } else if (iFind > iOptIndex) {
      if (!bSelected) {
        continue;
      }
      if (bNotify && m_pForm->m_pFormNotify) {
        int iRet = 0;
        CFX_WideString csValue = GetOptionLabel(iOptIndex);
        if (GetType() == ListBox) {
          iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, csValue);
        }
        if (GetType() == ComboBox) {
          iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csValue);
        }
        if (iRet < 0) {
          return FALSE;
        }
      }
      CPDF_Number* pNum = new CPDF_Number(iOptIndex);
      pArray->InsertAt(i, pNum);
      bReturn = TRUE;
      break;
    }
  }
  if (!bReturn) {
    if (bSelected) {
      pArray->AddInteger(iOptIndex);
    }
    if (pArray->GetCount() == 0) {
      m_pDict->RemoveAt("I");
    }
  }
  if (bNotify && m_pForm->m_pFormNotify) {
    if (GetType() == ListBox) {
      m_pForm->m_pFormNotify->AfterSelectionChange(this);
    }
    if (GetType() == ComboBox) {
      m_pForm->m_pFormNotify->AfterValueChange(this);
    }
  }
  m_pForm->m_bUpdated = TRUE;
  return TRUE;
}
FX_BOOL CPDF_FormField::ClearSelectedOptions(FX_BOOL bNotify) {
  if (bNotify && m_pForm->m_pFormNotify) {
    int iRet = 0;
    CFX_WideString csValue;
    int iIndex = GetSelectedIndex(0);
    if (iIndex >= 0) {
      csValue = GetOptionLabel(iIndex);
    }
    if (GetType() == ListBox) {
      iRet = m_pForm->m_pFormNotify->BeforeSelectionChange(this, csValue);
    }
    if (GetType() == ComboBox) {
      iRet = m_pForm->m_pFormNotify->BeforeValueChange(this, csValue);
    }
    if (iRet < 0) {
      return FALSE;
    }
  }
  m_pDict->RemoveAt("I");
  if (bNotify && m_pForm->m_pFormNotify) {
    if (GetType() == ListBox) {
      m_pForm->m_pFormNotify->AfterSelectionChange(this);
    }
    if (GetType() == ComboBox) {
      m_pForm->m_pFormNotify->AfterValueChange(this);
    }
  }
  m_pForm->m_bUpdated = TRUE;
  return TRUE;
}
void CPDF_FormField::LoadDA() {
  CFX_ByteString DA;
  if (CPDF_Object* pObj_t = FPDF_GetFieldAttr(m_pDict, "DA")) {
    DA = pObj_t->GetString();
  }
  if (DA.IsEmpty() && m_pForm->m_pFormDict) {
    DA = m_pForm->m_pFormDict->GetString("DA");
  }
  if (DA.IsEmpty()) {
    return;
  }
  CPDF_SimpleParser syntax(DA);
  syntax.FindTagParam("Tf", 2);
  CFX_ByteString font_name = syntax.GetWord();
  CPDF_Dictionary* pFontDict = NULL;
  if (m_pForm->m_pFormDict && m_pForm->m_pFormDict->GetDict("DR") &&
      m_pForm->m_pFormDict->GetDict("DR")->GetDict("Font"))
    pFontDict = m_pForm->m_pFormDict->GetDict("DR")->GetDict("Font")->GetDict(
        font_name);

  if (!pFontDict) {
    return;
  }
  m_pFont = m_pForm->m_pDocument->LoadFont(pFontDict);
  m_FontSize = FX_atof(syntax.GetWord());
}
