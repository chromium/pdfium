// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_FORMFIELD_H_
#define CORE_FPDFDOC_CPDF_FORMFIELD_H_

#include <stddef.h>
#include <stdint.h>

#include <utility>
#include <vector>

#include "core/fpdfdoc/cpdf_aaction.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Dictionary;
class CPDF_FormControl;
class CPDF_InteractiveForm;
class CPDF_Object;
class CPDF_String;

enum class NotificationOption : bool { kDoNotNotify = false, kNotify = true };

enum class FormFieldType : uint8_t {
  kUnknown = 0,
  kPushButton = 1,
  kCheckBox = 2,
  kRadioButton = 3,
  kComboBox = 4,
  kListBox = 5,
  kTextField = 6,
  kSignature = 7,
#ifdef PDF_ENABLE_XFA
  kXFA = 8,  // Generic XFA field, should use value below if possible.
  kXFA_CheckBox = 9,
  kXFA_ComboBox = 10,
  kXFA_ImageField = 11,
  kXFA_ListBox = 12,
  kXFA_PushButton = 13,
  kXFA_Signature = 14,
  kXFA_TextField = 15
#endif  // PDF_ENABLE_XFA
};

// If values are added to FormFieldType, these will need to be updated.
#ifdef PDF_ENABLE_XFA
constexpr size_t kFormFieldTypeCount = 16;
#else   // PDF_ENABLE_XFA
constexpr size_t kFormFieldTypeCount = 8;
#endif  // PDF_ENABLE_XFA

class CPDF_FormField {
 public:
  enum Type {
    kUnknown,
    kPushButton,
    kRadioButton,
    kCheckBox,
    kText,
    kRichText,
    kFile,
    kListBox,
    kComboBox,
    kSign
  };

  CPDF_FormField(CPDF_InteractiveForm* pForm, RetainPtr<CPDF_Dictionary> dict);
  ~CPDF_FormField();

  static std::optional<FormFieldType> IntToFormFieldType(int value);
  static WideString GetFullNameForDict(const CPDF_Dictionary* pFieldDict);
  static RetainPtr<const CPDF_Object> GetFieldAttrForDict(
      const CPDF_Dictionary* pFieldDict,
      ByteStringView name);
  static RetainPtr<CPDF_Object> GetMutableFieldAttrForDict(
      CPDF_Dictionary* pFieldDict,
      ByteStringView name);

  WideString GetFullName() const;
  Type GetType() const { return type_; }

  RetainPtr<const CPDF_Object> GetFieldAttr(ByteStringView name) const;
  RetainPtr<const CPDF_Dictionary> GetFieldDict() const;
  void ResetField();

  int CountControls() const;
  CPDF_FormControl* GetControl(int index) const;
  int GetControlIndex(const CPDF_FormControl* pControl) const;

  FormFieldType GetFieldType() const;

  CPDF_AAction GetAdditionalAction() const;
  WideString GetAlternateName() const;
  WideString GetMappingName() const;

  uint32_t GetFieldFlags() const;
  void SetFieldFlags(uint32_t dwFlags);

  bool IsRequired() const { return required_; }
  bool IsNoExport() const { return no_export_; }

  WideString GetValue() const;
  WideString GetDefaultValue() const;
  bool SetValue(const WideString& value, NotificationOption notify);

  int GetMaxLen() const;
  int CountSelectedItems() const;
  int GetSelectedIndex(int index) const;

  bool ClearSelection(NotificationOption notify);
  bool IsItemSelected(int index) const;
  void SetItemSelection(int index, NotificationOption notify);

  int GetDefaultSelectedItem() const;

  bool HasOptField() const;

  // These can only be called if HasOptField() returns true.
  int CountOptions() const;
  WideString GetOptionLabel(int index) const;
  WideString GetOptionValue(int index) const;
  int FindOption(const WideString& csOptValue) const;

  void CheckControl(int iControlIndex,
                    bool bChecked,
                    NotificationOption notify);

  int GetTopVisibleIndex() const;
  int CountSelectedOptions() const;
  int GetSelectedOptionIndex(int index) const;
  bool IsSelectedOption(const WideString& wsOptValue) const;
  bool IsSelectedIndex(int iOptIndex) const;
  void SelectOption(int iOptIndex);

  // Verifies if there is a valid selected indicies (/I) object and whether its
  // entries are consistent with the value (/V) object.
  bool UseSelectedIndicesObject() const;

  WideString GetCheckValue(bool bDefault) const;

 private:
  WideString GetValue(bool bDefault) const;
  bool SetValue(const WideString& value,
                bool bDefault,
                NotificationOption notify);
  void InitFieldFlags();
  int FindListSel(CPDF_String* str);
  WideString GetOptionText(int index, int sub_index) const;
  bool SetCheckValue(const WideString& value,
                     bool bDefault,
                     NotificationOption notify);
  void SetItemSelectionSelected(int index, const WideString& opt_value);
  bool NotifyListOrComboBoxBeforeChange(const WideString& value);
  void NotifyListOrComboBoxAfterChange();

  RetainPtr<const CPDF_Object> GetFieldAttrInternal(ByteStringView name) const;
  const CPDF_Dictionary* GetFieldDictInternal() const;
  RetainPtr<const CPDF_Object> GetDefaultValueObject() const;
  RetainPtr<const CPDF_Object> GetValueObject() const;

  // For choice fields.
  RetainPtr<const CPDF_Object> GetSelectedIndicesObject() const;

  // For choice fields.
  // Value object takes precedence over selected indices object.
  RetainPtr<const CPDF_Object> GetValueOrSelectedIndicesObject() const;

  const std::vector<UnownedPtr<CPDF_FormControl>>& GetControls() const;

  CPDF_FormField::Type type_ = kUnknown;
  bool required_ = false;
  bool no_export_ = false;
  bool is_multi_select_list_box_ = false;
  bool is_unison_ = false;
  bool use_selected_indices_ = false;
  UnownedPtr<CPDF_InteractiveForm> const form_;
  RetainPtr<CPDF_Dictionary> const dict_;
};

#endif  // CORE_FPDFDOC_CPDF_FORMFIELD_H_
