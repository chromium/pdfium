// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_field.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>

#include "constants/access_permissions.h"
#include "constants/annotation_flags.h"
#include "constants/form_flags.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfdoc/cpdf_formcontrol.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/span.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interactiveform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fxjs/cjs_color.h"
#include "fxjs/cjs_delaydata.h"
#include "fxjs/cjs_document.h"
#include "fxjs/cjs_icon.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "v8/include/v8-container.h"

namespace {

constexpr wchar_t kCheckSelector = L'4';
constexpr wchar_t kCircleSelector = L'l';
constexpr wchar_t kCrossSelector = L'8';
constexpr wchar_t kDiamondSelector = L'u';
constexpr wchar_t kSquareSelector = L'n';
constexpr wchar_t kStarSelector = L'H';

bool IsCheckBoxOrRadioButton(const CPDF_FormField* pFormField) {
  return pFormField->GetFieldType() == FormFieldType::kCheckBox ||
         pFormField->GetFieldType() == FormFieldType::kRadioButton;
}

bool IsComboBoxOrListBox(const CPDF_FormField* pFormField) {
  return pFormField->GetFieldType() == FormFieldType::kComboBox ||
         pFormField->GetFieldType() == FormFieldType::kListBox;
}

bool IsComboBoxOrTextField(const CPDF_FormField* pFormField) {
  return pFormField->GetFieldType() == FormFieldType::kComboBox ||
         pFormField->GetFieldType() == FormFieldType::kTextField;
}

void UpdateFormField(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                     CPDF_FormField* pFormField,
                     bool bResetAP) {
  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  if (bResetAP) {
    std::vector<ObservedPtr<CPDFSDK_Widget>> widgets;
    pForm->GetWidgets(pFormField, &widgets);

    if (IsComboBoxOrTextField(pFormField)) {
      for (auto& pWidget : widgets) {
        if (pWidget) {
          std::optional<WideString> sValue = pWidget->OnFormat();
          if (pWidget) {  // Not redundant, may be clobbered by OnFormat.
            pWidget->ResetAppearance(sValue, CPDFSDK_Widget::kValueUnchanged);
          }
        }
      }
    } else {
      for (auto& pWidget : widgets) {
        if (pWidget) {
          pWidget->ResetAppearance(std::nullopt,
                                   CPDFSDK_Widget::kValueUnchanged);
        }
      }
    }
  }

  // Refresh the widget list. The calls in |bResetAP| may have caused widgets
  // to be removed from the list. We need to call |GetWidgets| again to be
  // sure none of the widgets have been deleted.
  std::vector<ObservedPtr<CPDFSDK_Widget>> widgets;
  pForm->GetWidgets(pFormField, &widgets);
  for (auto& pWidget : widgets) {
    if (pWidget) {
      pFormFillEnv->UpdateAllViews(pWidget.Get());
    }
  }
  pFormFillEnv->SetChangeMark();
}

void UpdateFormControl(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                       CPDF_FormControl* pFormControl,
                       bool bResetAP) {
  DCHECK(pFormControl);
  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  ObservedPtr<CPDFSDK_Widget> observed_widget(pForm->GetWidget(pFormControl));
  if (observed_widget) {
    if (bResetAP) {
      FormFieldType fieldType = observed_widget->GetFieldType();
      if (fieldType == FormFieldType::kComboBox ||
          fieldType == FormFieldType::kTextField) {
        std::optional<WideString> sValue = observed_widget->OnFormat();
        if (!observed_widget) {
          return;
        }
        observed_widget->ResetAppearance(sValue,
                                         CPDFSDK_Widget::kValueUnchanged);
      } else {
        observed_widget->ResetAppearance(std::nullopt,
                                         CPDFSDK_Widget::kValueUnchanged);
      }
      if (!observed_widget) {
        return;
      }
    }
    pFormFillEnv->UpdateAllViews(observed_widget.Get());
  }
  pFormFillEnv->SetChangeMark();
}

struct FieldNameData {
  FieldNameData(WideString field_name_in, int control_index_in)
      : field_name(field_name_in), control_index(control_index_in) {}

  WideString field_name;
  int control_index;
};

std::optional<FieldNameData> ParseFieldName(const WideString& field_name) {
  auto reverse_it = field_name.rbegin();
  while (reverse_it != field_name.rend()) {
    if (*reverse_it == L'.') {
      break;
    }
    ++reverse_it;
  }
  if (reverse_it == field_name.rend()) {
    return std::nullopt;
  }
  WideString suffixal = field_name.Last(reverse_it - field_name.rbegin());
  int control_index = FXSYS_wtoi(suffixal.c_str());
  if (control_index == 0) {
    suffixal.TrimBack(L' ');
    if (suffixal != L"0") {
      return std::nullopt;
    }
  }
  return FieldNameData(field_name.First(field_name.rend() - reverse_it - 1),
                       control_index);
}

std::vector<CPDF_FormField*> GetFormFieldsForName(
    CPDFSDK_FormFillEnvironment* pFormFillEnv,
    const WideString& csFieldName) {
  std::vector<CPDF_FormField*> fields;
  CPDFSDK_InteractiveForm* pReaderForm = pFormFillEnv->GetInteractiveForm();
  CPDF_InteractiveForm* pForm = pReaderForm->GetInteractiveForm();
  const size_t sz = pForm->CountFields(csFieldName);
  for (size_t i = 0; i < sz; ++i) {
    CPDF_FormField* pFormField = pForm->GetField(i, csFieldName);
    if (pFormField) {
      fields.push_back(pFormField);
    }
  }
  return fields;
}

CFX_Color GetFormControlColor(CPDF_FormControl* pFormControl,
                              ByteStringView entry) {
  switch (pFormControl->GetColorARGB(entry).color_type) {
    case CFX_Color::Type::kTransparent:
      return CFX_Color(CFX_Color::Type::kTransparent);
    case CFX_Color::Type::kGray:
      return CFX_Color(CFX_Color::Type::kGray,
                       pFormControl->GetOriginalColorComponent(0, entry));
    case CFX_Color::Type::kRGB:
      return CFX_Color(CFX_Color::Type::kRGB,
                       pFormControl->GetOriginalColorComponent(0, entry),
                       pFormControl->GetOriginalColorComponent(1, entry),
                       pFormControl->GetOriginalColorComponent(2, entry));
    case CFX_Color::Type::kCMYK:
      return CFX_Color(CFX_Color::Type::kCMYK,
                       pFormControl->GetOriginalColorComponent(0, entry),
                       pFormControl->GetOriginalColorComponent(1, entry),
                       pFormControl->GetOriginalColorComponent(2, entry),
                       pFormControl->GetOriginalColorComponent(3, entry));
  }
}

bool SetWidgetDisplayStatus(CPDFSDK_Widget* pWidget, int value) {
  if (!pWidget) {
    return false;
  }

  uint32_t dwFlag = pWidget->GetFlags();
  switch (value) {
    case 0:
      dwFlag &= ~pdfium::annotation_flags::kInvisible;
      dwFlag &= ~pdfium::annotation_flags::kHidden;
      dwFlag &= ~pdfium::annotation_flags::kNoView;
      dwFlag |= pdfium::annotation_flags::kPrint;
      break;
    case 1:
      dwFlag &= ~pdfium::annotation_flags::kInvisible;
      dwFlag &= ~pdfium::annotation_flags::kNoView;
      dwFlag |= (pdfium::annotation_flags::kHidden |
                 pdfium::annotation_flags::kPrint);
      break;
    case 2:
      dwFlag &= ~pdfium::annotation_flags::kInvisible;
      dwFlag &= ~pdfium::annotation_flags::kPrint;
      dwFlag &= ~pdfium::annotation_flags::kHidden;
      dwFlag &= ~pdfium::annotation_flags::kNoView;
      break;
    case 3:
      dwFlag |= pdfium::annotation_flags::kNoView;
      dwFlag |= pdfium::annotation_flags::kPrint;
      dwFlag &= ~pdfium::annotation_flags::kHidden;
      break;
  }

  if (dwFlag != pWidget->GetFlags()) {
    pWidget->SetFlags(dwFlag);
    return true;
  }

  return false;
}

void SetBorderStyle(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                    const WideString& swFieldName,
                    int nControlIndex,
                    const ByteString& bsString) {
  DCHECK(pFormFillEnv);

  BorderStyle nBorderStyle;
  if (bsString == "solid") {
    nBorderStyle = BorderStyle::kSolid;
  } else if (bsString == "beveled") {
    nBorderStyle = BorderStyle::kBeveled;
  } else if (bsString == "dashed") {
    nBorderStyle = BorderStyle::kDash;
  } else if (bsString == "inset") {
    nBorderStyle = BorderStyle::kInset;
  } else if (bsString == "underline") {
    nBorderStyle = BorderStyle::kUnderline;
  } else {
    return;
  }

  std::vector<CPDF_FormField*> FieldArray =
      GetFormFieldsForName(pFormFillEnv, swFieldName);
  auto* pForm = pFormFillEnv->GetInteractiveForm();
  for (CPDF_FormField* pFormField : FieldArray) {
    if (nControlIndex < 0) {
      bool bSet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormField->GetControl(i));
        if (pWidget) {
          if (pWidget->GetBorderStyle() != nBorderStyle) {
            pWidget->SetBorderStyle(nBorderStyle);
            bSet = true;
          }
        }
      }
      if (bSet) {
        UpdateFormField(pFormFillEnv, pFormField, true);
      }
    } else {
      if (nControlIndex >= pFormField->CountControls()) {
        return;
      }
      if (CPDF_FormControl* pFormControl =
              pFormField->GetControl(nControlIndex)) {
        CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl);
        if (pWidget) {
          if (pWidget->GetBorderStyle() != nBorderStyle) {
            pWidget->SetBorderStyle(nBorderStyle);
            UpdateFormControl(pFormFillEnv, pFormControl, true);
          }
        }
      }
    }
  }
}

void SetCurrentValueIndices(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                            const WideString& swFieldName,
                            int nControlIndex,
                            const std::vector<uint32_t>& array) {
  DCHECK(pFormFillEnv);
  std::vector<CPDF_FormField*> FieldArray =
      GetFormFieldsForName(pFormFillEnv, swFieldName);

  for (CPDF_FormField* pFormField : FieldArray) {
    if (!IsComboBoxOrListBox(pFormField)) {
      continue;
    }

    uint32_t dwFieldFlags = pFormField->GetFieldFlags();
    pFormField->ClearSelection(NotificationOption::kNotify);
    for (size_t i = 0; i < array.size(); ++i) {
      if (i != 0 && !(dwFieldFlags & pdfium::form_flags::kChoiceMultiSelect)) {
        break;
      }
      if (array[i] < static_cast<uint32_t>(pFormField->CountOptions()) &&
          !pFormField->IsItemSelected(array[i])) {
        pFormField->SetItemSelection(array[i],
                                     NotificationOption::kDoNotNotify);
      }
    }
    UpdateFormField(pFormFillEnv, pFormField, true);
  }
}

void SetDisplay(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                const WideString& swFieldName,
                int nControlIndex,
                int number) {
  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  std::vector<CPDF_FormField*> FieldArray =
      GetFormFieldsForName(pFormFillEnv, swFieldName);
  for (CPDF_FormField* pFormField : FieldArray) {
    if (nControlIndex < 0) {
      bool bAnySet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        CPDF_FormControl* pFormControl = pFormField->GetControl(i);
        DCHECK(pFormControl);

        CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl);
        if (SetWidgetDisplayStatus(pWidget, number)) {
          bAnySet = true;
        }
      }

      if (bAnySet) {
        UpdateFormField(pFormFillEnv, pFormField, false);
      }
    } else {
      if (nControlIndex >= pFormField->CountControls()) {
        return;
      }

      CPDF_FormControl* pFormControl = pFormField->GetControl(nControlIndex);
      if (!pFormControl) {
        return;
      }

      CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl);
      if (SetWidgetDisplayStatus(pWidget, number)) {
        UpdateFormControl(pFormFillEnv, pFormControl, false);
      }
    }
  }
}

void SetHidden(CPDFSDK_FormFillEnvironment* pFormFillEnv,
               const WideString& swFieldName,
               int nControlIndex,
               bool b) {
  int display = b ? 1 /*Hidden*/ : 0 /*Visible*/;
  SetDisplay(pFormFillEnv, swFieldName, nControlIndex, display);
}

void SetLineWidth(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                  const WideString& swFieldName,
                  int nControlIndex,
                  int number) {
  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  std::vector<CPDF_FormField*> FieldArray =
      GetFormFieldsForName(pFormFillEnv, swFieldName);
  for (CPDF_FormField* pFormField : FieldArray) {
    if (nControlIndex < 0) {
      bool bSet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        CPDF_FormControl* pFormControl = pFormField->GetControl(i);
        DCHECK(pFormControl);

        if (CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl)) {
          if (number != pWidget->GetBorderWidth()) {
            pWidget->SetBorderWidth(number);
            bSet = true;
          }
        }
      }
      if (bSet) {
        UpdateFormField(pFormFillEnv, pFormField, true);
      }
    } else {
      if (nControlIndex >= pFormField->CountControls()) {
        return;
      }
      if (CPDF_FormControl* pFormControl =
              pFormField->GetControl(nControlIndex)) {
        if (CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl)) {
          if (number != pWidget->GetBorderWidth()) {
            pWidget->SetBorderWidth(number);
            UpdateFormControl(pFormFillEnv, pFormControl, true);
          }
        }
      }
    }
  }
}

void SetRect(CPDFSDK_FormFillEnvironment* pFormFillEnv,
             const WideString& swFieldName,
             int nControlIndex,
             const CFX_FloatRect& rect) {
  CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
  std::vector<CPDF_FormField*> FieldArray =
      GetFormFieldsForName(pFormFillEnv, swFieldName);
  for (CPDF_FormField* pFormField : FieldArray) {
    if (nControlIndex < 0) {
      bool bSet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        CPDF_FormControl* pFormControl = pFormField->GetControl(i);
        DCHECK(pFormControl);

        if (CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl)) {
          CFX_FloatRect crRect = rect;

          CPDF_Page* pPDFPage = pWidget->GetPDFPage();
          crRect.Intersect(pPDFPage->GetBBox());

          if (!crRect.IsEmpty()) {
            CFX_FloatRect rcOld = pWidget->GetRect();
            if (crRect.left != rcOld.left || crRect.right != rcOld.right ||
                crRect.top != rcOld.top || crRect.bottom != rcOld.bottom) {
              pWidget->SetRect(crRect);
              bSet = true;
            }
          }
        }
      }

      if (bSet) {
        UpdateFormField(pFormFillEnv, pFormField, true);
      }

      continue;
    }

    if (nControlIndex >= pFormField->CountControls()) {
      return;
    }
    if (CPDF_FormControl* pFormControl =
            pFormField->GetControl(nControlIndex)) {
      if (CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl)) {
        CFX_FloatRect crRect = rect;
        CPDF_Page* pPDFPage = pWidget->GetPDFPage();
        crRect.Intersect(pPDFPage->GetBBox());
        if (!crRect.IsEmpty()) {
          CFX_FloatRect rcOld = pWidget->GetRect();
          if (crRect.left != rcOld.left || crRect.right != rcOld.right ||
              crRect.top != rcOld.top || crRect.bottom != rcOld.bottom) {
            pWidget->SetRect(crRect);
            UpdateFormControl(pFormFillEnv, pFormControl, true);
          }
        }
      }
    }
  }
}

void SetFieldValue(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                   const WideString& swFieldName,
                   int nControlIndex,
                   const std::vector<WideString>& strArray) {
  DCHECK(pFormFillEnv);
  if (strArray.empty()) {
    return;
  }

  std::vector<CPDF_FormField*> FieldArray =
      GetFormFieldsForName(pFormFillEnv, swFieldName);

  for (CPDF_FormField* pFormField : FieldArray) {
    if (pFormField->GetFullName() != swFieldName) {
      continue;
    }

    switch (pFormField->GetFieldType()) {
      case FormFieldType::kTextField:
      case FormFieldType::kComboBox:
        if (pFormField->GetValue() != strArray[0]) {
          pFormField->SetValue(strArray[0], NotificationOption::kNotify);
          UpdateFormField(pFormFillEnv, pFormField, false);
        }
        break;
      case FormFieldType::kCheckBox:
      case FormFieldType::kRadioButton:
        if (pFormField->GetValue() != strArray[0]) {
          pFormField->SetValue(strArray[0], NotificationOption::kNotify);
          UpdateFormField(pFormFillEnv, pFormField, false);
        }
        break;
      case FormFieldType::kListBox: {
        bool bModified = false;
        for (const auto& str : strArray) {
          if (!pFormField->IsItemSelected(pFormField->FindOption(str))) {
            bModified = true;
            break;
          }
        }
        if (bModified) {
          pFormField->ClearSelection(NotificationOption::kNotify);
          for (const auto& str : strArray) {
            int index = pFormField->FindOption(str);
            if (!pFormField->IsItemSelected(index)) {
              pFormField->SetItemSelection(index, NotificationOption::kNotify);
            }
          }
          UpdateFormField(pFormFillEnv, pFormField, false);
        }
        break;
      }
      default:
        break;
    }
  }
}

wchar_t GetSelectorFromCaptionForFieldType(const WideString& caption,
                                           CPDF_FormField::Type type) {
  if (!caption.IsEmpty()) {
    return caption[0];
  }

  switch (type) {
    case CPDF_FormField::kRadioButton:
      return kCircleSelector;
    default:
      return kCheckSelector;
  }
}

}  // namespace

const JSPropertySpec CJS_Field::PropertySpecs[] = {
    {"alignment", get_alignment_static, set_alignment_static},
    {"borderStyle", get_border_style_static, set_border_style_static},
    {"buttonAlignX", get_button_align_x_static, set_button_align_x_static},
    {"buttonAlignY", get_button_align_y_static, set_button_align_y_static},
    {"buttonFitBounds", get_button_fit_bounds_static,
     set_button_fit_bounds_static},
    {"buttonPosition", get_button_position_static, set_button_position_static},
    {"buttonScaleHow", get_button_scale_how_static,
     set_button_scale_how_static},
    {"buttonScaleWhen", get_button_scale_when_static,
     set_button_scale_when_static},
    {"calcOrderIndex", get_calc_order_index_static,
     set_calc_order_index_static},
    {"charLimit", get_char_limit_static, set_char_limit_static},
    {"comb", get_comb_static, set_comb_static},
    {"commitOnSelChange", get_commit_on_sel_change_static,
     set_commit_on_sel_change_static},
    {"currentValueIndices", get_current_value_indices_static,
     set_current_value_indices_static},
    {"defaultStyle", get_default_style_static, set_default_style_static},
    {"defaultValue", get_default_value_static, set_default_value_static},
    {"doNotScroll", get_do_not_scroll_static, set_do_not_scroll_static},
    {"doNotSpellCheck", get_do_not_spell_check_static,
     set_do_not_spell_check_static},
    {"delay", get_delay_static, set_delay_static},
    {"display", get_display_static, set_display_static},
    {"doc", get_doc_static, set_doc_static},
    {"editable", get_editable_static, set_editable_static},
    {"exportValues", get_export_values_static, set_export_values_static},
    {"hidden", get_hidden_static, set_hidden_static},
    {"fileSelect", get_file_select_static, set_file_select_static},
    {"fillColor", get_fill_color_static, set_fill_color_static},
    {"lineWidth", get_line_width_static, set_line_width_static},
    {"highlight", get_highlight_static, set_highlight_static},
    {"multiline", get_multiline_static, set_multiline_static},
    {"multipleSelection", get_multiple_selection_static,
     set_multiple_selection_static},
    {"name", get_name_static, set_name_static},
    {"numItems", get_num_items_static, set_num_items_static},
    {"page", get_page_static, set_page_static},
    {"password", get_password_static, set_password_static},
    {"print", get_print_static, set_print_static},
    {"radiosInUnison", get_radios_in_unison_static,
     set_radios_in_unison_static},
    {"readonly", get_readonly_static, set_readonly_static},
    {"rect", get_rect_static, set_rect_static},
    {"required", get_required_static, set_required_static},
    {"richText", get_rich_text_static, set_rich_text_static},
    {"richValue", get_rich_value_static, set_rich_value_static},
    {"rotation", get_rotation_static, set_rotation_static},
    {"source", get_source_static, set_source_static},
    {"strokeColor", get_stroke_color_static, set_stroke_color_static},
    {"style", get_style_static, set_style_static},
    {"submitName", get_submit_name_static, set_submit_name_static},
    {"textColor", get_text_color_static, set_text_color_static},
    {"textFont", get_text_font_static, set_text_font_static},
    {"textSize", get_text_size_static, set_text_size_static},
    {"type", get_type_static, set_type_static},
    {"userName", get_user_name_static, set_user_name_static},
    {"value", get_value_static, set_value_static},
    {"valueAsString", get_value_as_string_static, set_value_as_string_static}};

const JSMethodSpec CJS_Field::MethodSpecs[] = {
    {"browseForFileToSubmit", browseForFileToSubmit_static},
    {"buttonGetCaption", buttonGetCaption_static},
    {"buttonGetIcon", buttonGetIcon_static},
    {"buttonImportIcon", buttonImportIcon_static},
    {"buttonSetCaption", buttonSetCaption_static},
    {"buttonSetIcon", buttonSetIcon_static},
    {"checkThisBox", checkThisBox_static},
    {"clearItems", clearItems_static},
    {"defaultIsChecked", defaultIsChecked_static},
    {"deleteItemAt", deleteItemAt_static},
    {"getArray", getArray_static},
    {"getItemAt", getItemAt_static},
    {"getLock", getLock_static},
    {"insertItemAt", insertItemAt_static},
    {"isBoxChecked", isBoxChecked_static},
    {"isDefaultChecked", isDefaultChecked_static},
    {"setAction", setAction_static},
    {"setFocus", setFocus_static},
    {"setItems", setItems_static},
    {"setLock", setLock_static},
    {"signatureGetModifications", signatureGetModifications_static},
    {"signatureGetSeedValue", signatureGetSeedValue_static},
    {"signatureInfo", signatureInfo_static},
    {"signatureSetSeedValue", signatureSetSeedValue_static},
    {"signatureSign", signatureSign_static},
    {"signatureValidate", signatureValidate_static}};

uint32_t CJS_Field::ObjDefnID = 0;
const char CJS_Field::kName[] = "Field";

// static
uint32_t CJS_Field::GetObjDefnID() {
  return ObjDefnID;
}

// static
void CJS_Field::DefineJSObjects(CFXJS_Engine* pEngine) {
  ObjDefnID = pEngine->DefineObj(CJS_Field::kName, FXJSOBJTYPE_DYNAMIC,
                                 JSConstructor<CJS_Field>, JSDestructor);
  DefineProps(pEngine, ObjDefnID, PropertySpecs);
  DefineMethods(pEngine, ObjDefnID, MethodSpecs);
}

CJS_Field::CJS_Field(v8::Local<v8::Object> pObject, CJS_Runtime* pRuntime)
    : CJS_Object(pObject, pRuntime) {}

CJS_Field::~CJS_Field() = default;

bool CJS_Field::AttachField(CJS_Document* document,
                            const WideString& csFieldName) {
  js_doc_.Reset(document);
  form_fill_env_.Reset(document->GetFormFillEnv());
  can_set_ = form_fill_env_->HasPermissions(
      pdfium::access_permissions::kFillForm |
      pdfium::access_permissions::kModifyAnnotation |
      pdfium::access_permissions::kModifyContent);

  CPDFSDK_InteractiveForm* pRDForm = form_fill_env_->GetInteractiveForm();
  CPDF_InteractiveForm* pForm = pRDForm->GetInteractiveForm();
  WideString swFieldNameTemp = csFieldName;
  swFieldNameTemp.Replace(L"..", L".");

  if (pForm->CountFields(swFieldNameTemp) <= 0) {
    std::optional<FieldNameData> parsed_data = ParseFieldName(swFieldNameTemp);
    if (!parsed_data.has_value()) {
      return false;
    }

    field_name_ = parsed_data.value().field_name;
    form_control_index_ = parsed_data.value().control_index;
    return true;
  }

  field_name_ = swFieldNameTemp;
  form_control_index_ = -1;

  return true;
}

std::vector<CPDF_FormField*> CJS_Field::GetFormFields() const {
  return GetFormFieldsForName(form_fill_env_.Get(), field_name_);
}

CPDF_FormField* CJS_Field::GetFirstFormField() const {
  std::vector<CPDF_FormField*> fields = GetFormFields();
  return fields.empty() ? nullptr : fields[0];
}

CPDF_FormControl* CJS_Field::GetSmartFieldControl(CPDF_FormField* pFormField) {
  if (!pFormField->CountControls() ||
      form_control_index_ >= pFormField->CountControls()) {
    return nullptr;
  }
  if (form_control_index_ < 0) {
    return pFormField->GetControl(0);
  }
  return pFormField->GetControl(form_control_index_);
}

CJS_Result CJS_Field::get_alignment(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  switch (pFormControl->GetControlAlignment()) {
    case 0:
      return CJS_Result::Success(pRuntime->NewString("left"));
    case 1:
      return CJS_Result::Success(pRuntime->NewString("center"));
    case 2:
      return CJS_Result::Success(pRuntime->NewString("right"));
  }
  return CJS_Result::Success(pRuntime->NewString(""));
}

CJS_Result CJS_Field::set_alignment(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_border_style(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDFSDK_Widget* pWidget = form_fill_env_->GetInteractiveForm()->GetWidget(
      GetSmartFieldControl(pFormField));
  if (!pWidget) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  switch (pWidget->GetBorderStyle()) {
    case BorderStyle::kSolid:
      return CJS_Result::Success(pRuntime->NewString("solid"));
    case BorderStyle::kDash:
      return CJS_Result::Success(pRuntime->NewString("dashed"));
    case BorderStyle::kBeveled:
      return CJS_Result::Success(pRuntime->NewString("beveled"));
    case BorderStyle::kInset:
      return CJS_Result::Success(pRuntime->NewString("inset"));
    case BorderStyle::kUnderline:
      return CJS_Result::Success(pRuntime->NewString("underline"));
  }
  return CJS_Result::Success(pRuntime->NewString(""));
}

CJS_Result CJS_Field::set_border_style(CJS_Runtime* pRuntime,
                                       v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  ByteString byte_str = pRuntime->ToByteString(vp);
  if (delay_) {
    AddDelay_String(FP_BORDERSTYLE, byte_str);
  } else {
    SetBorderStyle(form_fill_env_.Get(), field_name_, form_control_index_,
                   byte_str);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_button_align_x(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_IconFit IconFit = pFormControl->GetIconFit();
  CFX_PointF pos = IconFit.GetIconBottomLeftPosition();
  return CJS_Result::Success(pRuntime->NewNumber(static_cast<int32_t>(pos.x)));
}

CJS_Result CJS_Field::set_button_align_x(CJS_Runtime* pRuntime,
                                         v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_button_align_y(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_IconFit IconFit = pFormControl->GetIconFit();
  CFX_PointF pos = IconFit.GetIconBottomLeftPosition();
  return CJS_Result::Success(pRuntime->NewNumber(static_cast<int32_t>(pos.y)));
}

CJS_Result CJS_Field::set_button_align_y(CJS_Runtime* pRuntime,
                                         v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_button_fit_bounds(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(
      pRuntime->NewBoolean(pFormControl->GetIconFit().GetFittingBounds()));
}

CJS_Result CJS_Field::set_button_fit_bounds(CJS_Runtime* pRuntime,
                                            v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_button_position(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(
      pRuntime->NewNumber(pFormControl->GetTextPosition()));
}

CJS_Result CJS_Field::set_button_position(CJS_Runtime* pRuntime,
                                          v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_button_scale_how(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(
      pRuntime->NewBoolean(!pFormControl->GetIconFit().IsProportionalScale()));
}

CJS_Result CJS_Field::set_button_scale_how(CJS_Runtime* pRuntime,
                                           v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_button_scale_when(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_IconFit IconFit = pFormControl->GetIconFit();
  CPDF_IconFit::ScaleMethod scale_method = IconFit.GetScaleMethod();
  switch (scale_method) {
    case CPDF_IconFit::ScaleMethod::kAlways:
    case CPDF_IconFit::ScaleMethod::kBigger:
    case CPDF_IconFit::ScaleMethod::kNever:
    case CPDF_IconFit::ScaleMethod::kSmaller:
      return CJS_Result::Success(
          pRuntime->NewNumber(static_cast<int>(scale_method)));
  }
}

CJS_Result CJS_Field::set_button_scale_when(CJS_Runtime* pRuntime,
                                            v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_calc_order_index(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsComboBoxOrTextField(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDFSDK_InteractiveForm* pRDForm = form_fill_env_->GetInteractiveForm();
  CPDF_InteractiveForm* pForm = pRDForm->GetInteractiveForm();
  return CJS_Result::Success(
      pRuntime->NewNumber(pForm->FindFieldInCalculationOrder(pFormField)));
}

CJS_Result CJS_Field::set_calc_order_index(CJS_Runtime* pRuntime,
                                           v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_char_limit(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }
  return CJS_Result::Success(
      pRuntime->NewNumber(static_cast<int32_t>(pFormField->GetMaxLen())));
}

CJS_Result CJS_Field::set_char_limit(CJS_Runtime* pRuntime,
                                     v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_comb(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kTextComb)));
}

CJS_Result CJS_Field::set_comb(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_commit_on_sel_change(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsComboBoxOrListBox(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  uint32_t dwFieldFlags = pFormField->GetFieldFlags();
  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(dwFieldFlags & pdfium::form_flags::kChoiceCommitOnSelChange)));
}

CJS_Result CJS_Field::set_commit_on_sel_change(CJS_Runtime* pRuntime,
                                               v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_current_value_indices(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsComboBoxOrListBox(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  int count = pFormField->CountSelectedItems();
  if (count <= 0) {
    return CJS_Result::Success(pRuntime->NewNumber(-1));
  }
  if (count == 1) {
    return CJS_Result::Success(
        pRuntime->NewNumber(pFormField->GetSelectedIndex(0)));
  }

  v8::Local<v8::Array> SelArray = pRuntime->NewArray();
  for (int i = 0; i < count; i++) {
    pRuntime->PutArrayElement(
        SelArray, i, pRuntime->NewNumber(pFormField->GetSelectedIndex(i)));
  }
  if (SelArray.IsEmpty()) {
    return CJS_Result::Success(pRuntime->NewArray());
  }
  return CJS_Result::Success(SelArray);
}

CJS_Result CJS_Field::set_current_value_indices(CJS_Runtime* pRuntime,
                                                v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  std::vector<uint32_t> array;
  if (vp->IsNumber()) {
    array.push_back(pRuntime->ToInt32(vp));
  } else if (fxv8::IsArray(vp)) {
    v8::Local<v8::Array> SelArray = pRuntime->ToArray(vp);
    for (size_t i = 0; i < pRuntime->GetArrayLength(SelArray); i++) {
      array.push_back(
          pRuntime->ToInt32(pRuntime->GetArrayElement(SelArray, i)));
    }
  }

  if (delay_) {
    AddDelay_WordArray(FP_CURRENTVALUEINDICES, array);
  } else {
    SetCurrentValueIndices(form_fill_env_.Get(), field_name_,
                           form_control_index_, array);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_default_style(CJS_Runtime* pRuntime) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::set_default_style(CJS_Runtime* pRuntime,
                                        v8::Local<v8::Value> vp) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::get_default_value(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() == FormFieldType::kPushButton ||
      pFormField->GetFieldType() == FormFieldType::kSignature) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(
      pRuntime->NewString(pFormField->GetDefaultValue().AsStringView()));
}

CJS_Result CJS_Field::set_default_value(CJS_Runtime* pRuntime,
                                        v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_do_not_scroll(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kTextDoNotScroll)));
}

CJS_Result CJS_Field::set_do_not_scroll(CJS_Runtime* pRuntime,
                                        v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_do_not_spell_check(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsComboBoxOrTextField(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  uint32_t dwFieldFlags = pFormField->GetFieldFlags();
  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(dwFieldFlags & pdfium::form_flags::kTextDoNotSpellCheck)));
}

CJS_Result CJS_Field::set_do_not_spell_check(CJS_Runtime* pRuntime,
                                             v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_delay(CJS_Runtime* pRuntime) {
  return CJS_Result::Success(pRuntime->NewBoolean(delay_));
}

CJS_Result CJS_Field::set_delay(CJS_Runtime* pRuntime,
                                v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  SetDelay(pRuntime->ToBoolean(vp));
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_display(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDFSDK_InteractiveForm* pForm = form_fill_env_->GetInteractiveForm();
  CPDFSDK_Widget* pWidget = pForm->GetWidget(GetSmartFieldControl(pFormField));
  if (!pWidget) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  uint32_t dwFlag = pWidget->GetFlags();
  if (pdfium::annotation_flags::kInvisible & dwFlag ||
      pdfium::annotation_flags::kHidden & dwFlag) {
    return CJS_Result::Success(pRuntime->NewNumber(1));
  }

  if (pdfium::annotation_flags::kPrint & dwFlag) {
    if (pdfium::annotation_flags::kNoView & dwFlag) {
      return CJS_Result::Success(pRuntime->NewNumber(3));
    }
    return CJS_Result::Success(pRuntime->NewNumber(0));
  }
  return CJS_Result::Success(pRuntime->NewNumber(2));
}

CJS_Result CJS_Field::set_display(CJS_Runtime* pRuntime,
                                  v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  if (delay_) {
    AddDelay_Int(FP_DISPLAY, pRuntime->ToInt32(vp));
  } else {
    SetDisplay(form_fill_env_.Get(), field_name_, form_control_index_,
               pRuntime->ToInt32(vp));
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_doc(CJS_Runtime* pRuntime) {
  return CJS_Result::Success(js_doc_->ToV8Object());
}

CJS_Result CJS_Field::set_doc(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::get_editable(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kComboBox) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kChoiceEdit)));
}

CJS_Result CJS_Field::set_editable(CJS_Runtime* pRuntime,
                                   v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_export_values(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsCheckBoxOrRadioButton(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  v8::Local<v8::Array> ExportValuesArray = pRuntime->NewArray();
  if (form_control_index_ < 0) {
    for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
      CPDF_FormControl* pFormControl = pFormField->GetControl(i);
      pRuntime->PutArrayElement(
          ExportValuesArray, i,
          pRuntime->NewString(pFormControl->GetExportValue().AsStringView()));
    }
  } else {
    if (form_control_index_ >= pFormField->CountControls()) {
      return CJS_Result::Failure(JSMessage::kValueError);
    }

    CPDF_FormControl* pFormControl =
        pFormField->GetControl(form_control_index_);
    if (!pFormControl) {
      return CJS_Result::Failure(JSMessage::kBadObjectError);
    }

    pRuntime->PutArrayElement(
        ExportValuesArray, 0,
        pRuntime->NewString(pFormControl->GetExportValue().AsStringView()));
  }
  return CJS_Result::Success(ExportValuesArray);
}

CJS_Result CJS_Field::set_export_values(CJS_Runtime* pRuntime,
                                        v8::Local<v8::Value> vp) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsCheckBoxOrRadioButton(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  if (!fxv8::IsArray(vp)) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_file_select(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kTextFileSelect)));
}

CJS_Result CJS_Field::set_file_select(CJS_Runtime* pRuntime,
                                      v8::Local<v8::Value> vp) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_fill_color(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CFX_Color color = GetFormControlColor(pFormControl, pdfium::appearance::kBG);
  v8::Local<v8::Value> array =
      CJS_Color::ConvertPWLColorToArray(pRuntime, color);
  if (array.IsEmpty()) {
    return CJS_Result::Success(pRuntime->NewArray());
  }
  return CJS_Result::Success(array);
}

CJS_Result CJS_Field::set_fill_color(CJS_Runtime* pRuntime,
                                     v8::Local<v8::Value> vp) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields();
  if (FieldArray.empty()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  if (!fxv8::IsArray(vp)) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_hidden(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDFSDK_InteractiveForm* pForm = form_fill_env_->GetInteractiveForm();
  CPDFSDK_Widget* pWidget = pForm->GetWidget(GetSmartFieldControl(pFormField));
  if (!pWidget) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  uint32_t dwFlags = pWidget->GetFlags();
  return CJS_Result::Success(
      pRuntime->NewBoolean(pdfium::annotation_flags::kInvisible & dwFlags ||
                           pdfium::annotation_flags::kHidden & dwFlags));
}

CJS_Result CJS_Field::set_hidden(CJS_Runtime* pRuntime,
                                 v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  if (delay_) {
    AddDelay_Bool(FP_HIDDEN, pRuntime->ToBoolean(vp));
  } else {
    SetHidden(form_fill_env_.Get(), field_name_, form_control_index_,
              pRuntime->ToBoolean(vp));
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_highlight(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  int eHM = pFormControl->GetHighlightingMode();
  switch (eHM) {
    case CPDF_FormControl::kNone:
      return CJS_Result::Success(pRuntime->NewString("none"));
    case CPDF_FormControl::kPush:
      return CJS_Result::Success(pRuntime->NewString("push"));
    case CPDF_FormControl::kInvert:
      return CJS_Result::Success(pRuntime->NewString("invert"));
    case CPDF_FormControl::kOutline:
      return CJS_Result::Success(pRuntime->NewString("outline"));
    case CPDF_FormControl::kToggle:
      return CJS_Result::Success(pRuntime->NewString("toggle"));
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::set_highlight(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_line_width(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDFSDK_InteractiveForm* pForm = form_fill_env_->GetInteractiveForm();
  if (!pFormField->CountControls()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormField->GetControl(0));
  if (!pWidget) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(pRuntime->NewNumber(pWidget->GetBorderWidth()));
}

CJS_Result CJS_Field::set_line_width(CJS_Runtime* pRuntime,
                                     v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  if (delay_) {
    AddDelay_Int(FP_LINEWIDTH, pRuntime->ToInt32(vp));
  } else {
    SetLineWidth(form_fill_env_.Get(), field_name_, form_control_index_,
                 pRuntime->ToInt32(vp));
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_multiline(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kTextMultiline)));
}

CJS_Result CJS_Field::set_multiline(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_multiple_selection(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kListBox) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  uint32_t dwFieldFlags = pFormField->GetFieldFlags();
  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(dwFieldFlags & pdfium::form_flags::kChoiceMultiSelect)));
}

CJS_Result CJS_Field::set_multiple_selection(CJS_Runtime* pRuntime,
                                             v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_name(CJS_Runtime* pRuntime) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields();
  if (FieldArray.empty()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(pRuntime->NewString(field_name_.AsStringView()));
}

CJS_Result CJS_Field::set_name(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::get_num_items(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsComboBoxOrListBox(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewNumber(pFormField->CountOptions()));
}

CJS_Result CJS_Field::set_num_items(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::get_page(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  std::vector<ObservedPtr<CPDFSDK_Widget>> widgets;
  form_fill_env_->GetInteractiveForm()->GetWidgets(pFormField, &widgets);
  if (widgets.empty()) {
    return CJS_Result::Success(pRuntime->NewNumber(-1));
  }

  v8::Local<v8::Array> PageArray = pRuntime->NewArray();
  int i = 0;
  for (const auto& pWidget : widgets) {
    if (!pWidget) {
      return CJS_Result::Failure(JSMessage::kBadObjectError);
    }

    pRuntime->PutArrayElement(
        PageArray, i,
        pRuntime->NewNumber(pWidget->GetPageView()->GetPageIndex()));
    ++i;
  }
  return CJS_Result::Success(PageArray);
}

CJS_Result CJS_Field::set_page(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return CJS_Result::Failure(JSMessage::kReadOnlyError);
}

CJS_Result CJS_Field::get_password(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kTextPassword)));
}

CJS_Result CJS_Field::set_password(CJS_Runtime* pRuntime,
                                   v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_print(CJS_Runtime* pRuntime) {
  CPDFSDK_InteractiveForm* pForm = form_fill_env_->GetInteractiveForm();
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDFSDK_Widget* pWidget = pForm->GetWidget(GetSmartFieldControl(pFormField));
  if (!pWidget) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pWidget->GetFlags() & pdfium::annotation_flags::kPrint)));
}

CJS_Result CJS_Field::set_print(CJS_Runtime* pRuntime,
                                v8::Local<v8::Value> vp) {
  CPDFSDK_InteractiveForm* pForm = form_fill_env_->GetInteractiveForm();
  std::vector<CPDF_FormField*> FieldArray = GetFormFields();
  if (FieldArray.empty()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  for (CPDF_FormField* pFormField : FieldArray) {
    if (form_control_index_ < 0) {
      bool bSet = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; ++i) {
        if (CPDFSDK_Widget* pWidget =
                pForm->GetWidget(pFormField->GetControl(i))) {
          uint32_t dwFlags = pWidget->GetFlags();
          if (pRuntime->ToBoolean(vp)) {
            dwFlags |= pdfium::annotation_flags::kPrint;
          } else {
            dwFlags &= ~pdfium::annotation_flags::kPrint;
          }

          if (dwFlags != pWidget->GetFlags()) {
            pWidget->SetFlags(dwFlags);
            bSet = true;
          }
        }
      }

      if (bSet) {
        UpdateFormField(form_fill_env_.Get(), pFormField, false);
      }

      continue;
    }

    if (form_control_index_ >= pFormField->CountControls()) {
      return CJS_Result::Failure(JSMessage::kValueError);
    }

    if (CPDF_FormControl* pFormControl =
            pFormField->GetControl(form_control_index_)) {
      if (CPDFSDK_Widget* pWidget = pForm->GetWidget(pFormControl)) {
        uint32_t dwFlags = pWidget->GetFlags();
        if (pRuntime->ToBoolean(vp)) {
          dwFlags |= pdfium::annotation_flags::kPrint;
        } else {
          dwFlags &= ~pdfium::annotation_flags::kPrint;
        }

        if (dwFlags != pWidget->GetFlags()) {
          pWidget->SetFlags(dwFlags);
          UpdateFormControl(form_fill_env_.Get(),
                            pFormField->GetControl(form_control_index_), false);
        }
      }
    }
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_radios_in_unison(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kRadioButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  uint32_t dwFieldFlags = pFormField->GetFieldFlags();
  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(dwFieldFlags & pdfium::form_flags::kButtonRadiosInUnison)));
}

CJS_Result CJS_Field::set_radios_in_unison(CJS_Runtime* pRuntime,
                                           v8::Local<v8::Value> vp) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields();
  if (FieldArray.empty()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_readonly(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kReadOnly)));
}

CJS_Result CJS_Field::set_readonly(CJS_Runtime* pRuntime,
                                   v8::Local<v8::Value> vp) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  const bool bReadOnly = pRuntime->ToBoolean(vp);
  const uint32_t dwFlags = pFormField->GetFieldFlags();
  const uint32_t dwNewFlags = bReadOnly
                                  ? (dwFlags | pdfium::form_flags::kReadOnly)
                                  : (dwFlags & ~pdfium::form_flags::kReadOnly);
  if (dwNewFlags != dwFlags) {
    pFormField->SetFieldFlags(dwNewFlags);
  }

  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_rect(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDFSDK_InteractiveForm* pForm = form_fill_env_->GetInteractiveForm();
  CPDFSDK_Widget* pWidget = pForm->GetWidget(GetSmartFieldControl(pFormField));
  if (!pWidget) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CFX_FloatRect crRect = pWidget->GetRect();
  v8::Local<v8::Array> rcArray = pRuntime->NewArray();
  pRuntime->PutArrayElement(
      rcArray, 0, pRuntime->NewNumber(static_cast<int32_t>(crRect.left)));
  pRuntime->PutArrayElement(
      rcArray, 1, pRuntime->NewNumber(static_cast<int32_t>(crRect.top)));
  pRuntime->PutArrayElement(
      rcArray, 2, pRuntime->NewNumber(static_cast<int32_t>(crRect.right)));
  pRuntime->PutArrayElement(
      rcArray, 3, pRuntime->NewNumber(static_cast<int32_t>(crRect.bottom)));

  return CJS_Result::Success(rcArray);
}

CJS_Result CJS_Field::set_rect(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  if (!fxv8::IsArray(vp)) {
    return CJS_Result::Failure(JSMessage::kValueError);
  }

  v8::Local<v8::Array> rcArray = pRuntime->ToArray(vp);
  if (pRuntime->GetArrayLength(rcArray) < 4) {
    return CJS_Result::Failure(JSMessage::kValueError);
  }

  float f0 = static_cast<float>(
      pRuntime->ToInt32(pRuntime->GetArrayElement(rcArray, 0)));
  float f1 = static_cast<float>(
      pRuntime->ToInt32(pRuntime->GetArrayElement(rcArray, 1)));
  float f2 = static_cast<float>(
      pRuntime->ToInt32(pRuntime->GetArrayElement(rcArray, 2)));
  float f3 = static_cast<float>(
      pRuntime->ToInt32(pRuntime->GetArrayElement(rcArray, 3)));

  CFX_FloatRect crRect(f0, f1, f2, f3);
  if (delay_) {
    AddDelay_Rect(FP_RECT, crRect);
  } else {
    SetRect(form_fill_env_.Get(), field_name_, form_control_index_, crRect);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_required(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() == FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kRequired)));
}

CJS_Result CJS_Field::set_required(CJS_Runtime* pRuntime,
                                   v8::Local<v8::Value> vp) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields();
  if (FieldArray.empty()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_rich_text(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      !!(pFormField->GetFieldFlags() & pdfium::form_flags::kTextRichText)));
}

CJS_Result CJS_Field::set_rich_text(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_rich_value(CJS_Runtime* pRuntime) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::set_rich_value(CJS_Runtime* pRuntime,
                                     v8::Local<v8::Value> vp) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_rotation(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(pRuntime->NewNumber(pFormControl->GetRotation()));
}

CJS_Result CJS_Field::set_rotation(CJS_Runtime* pRuntime,
                                   v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_source(CJS_Runtime* pRuntime) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::set_source(CJS_Runtime* pRuntime,
                                 v8::Local<v8::Value> vp) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_stroke_color(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CFX_Color color = GetFormControlColor(pFormControl, pdfium::appearance::kBC);
  v8::Local<v8::Value> array =
      CJS_Color::ConvertPWLColorToArray(pRuntime, color);
  if (array.IsEmpty()) {
    return CJS_Result::Success(pRuntime->NewArray());
  }
  return CJS_Result::Success(array);
}

CJS_Result CJS_Field::set_stroke_color(CJS_Runtime* pRuntime,
                                       v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  if (!fxv8::IsArray(vp)) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_style(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsCheckBoxOrRadioButton(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  wchar_t selector = GetSelectorFromCaptionForFieldType(
      pFormControl->GetNormalCaption(), pFormControl->GetType());

  ByteString csBCaption;
  switch (selector) {
    case kCircleSelector:
      csBCaption = "circle";
      break;
    case kCrossSelector:
      csBCaption = "cross";
      break;
    case kDiamondSelector:
      csBCaption = "diamond";
      break;
    case kSquareSelector:
      csBCaption = "square";
      break;
    case kStarSelector:
      csBCaption = "star";
      break;
    case kCheckSelector:
    default:
      csBCaption = "check";
      break;
  }
  return CJS_Result::Success(pRuntime->NewString(csBCaption.AsStringView()));
}

CJS_Result CJS_Field::set_style(CJS_Runtime* pRuntime,
                                v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_submit_name(CJS_Runtime* pRuntime) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::set_submit_name(CJS_Runtime* pRuntime,
                                      v8::Local<v8::Value> vp) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_text_color(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_DefaultAppearance FieldAppearance = pFormControl->GetDefaultAppearance();
  std::optional<CFX_Color::TypeAndARGB> maybe_type_argb_pair =
      FieldAppearance.GetColorARGB();

  CFX_Color crRet;
  if (maybe_type_argb_pair.has_value() &&
      maybe_type_argb_pair.value().color_type !=
          CFX_Color::Type::kTransparent) {
    FX_BGR_STRUCT<uint8_t> bgr =
        ArgbToBGRStruct(maybe_type_argb_pair.value().argb);
    crRet = CFX_Color(CFX_Color::Type::kRGB, bgr.red / 255.0f,
                      bgr.green / 255.0f, bgr.blue / 255.0f);
  }

  v8::Local<v8::Value> array =
      CJS_Color::ConvertPWLColorToArray(pRuntime, crRet);
  if (array.IsEmpty()) {
    return CJS_Result::Success(pRuntime->NewArray());
  }
  return CJS_Result::Success(array);
}

CJS_Result CJS_Field::set_text_color(CJS_Runtime* pRuntime,
                                     v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  if (!fxv8::IsArray(vp)) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_text_font(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  FormFieldType fieldType = pFormField->GetFieldType();
  if (fieldType != FormFieldType::kPushButton &&
      fieldType != FormFieldType::kComboBox &&
      fieldType != FormFieldType::kListBox &&
      fieldType != FormFieldType::kTextField) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  std::optional<WideString> wsFontName =
      pFormControl->GetDefaultControlFontName();
  if (!wsFontName.has_value()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(
      pRuntime->NewString(wsFontName.value().AsStringView()));
}

CJS_Result CJS_Field::set_text_font(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);

  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  if (pRuntime->ToByteString(vp).IsEmpty()) {
    return CJS_Result::Failure(JSMessage::kValueError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_text_size(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDF_DefaultAppearance field_appearance =
      pFormControl->GetDefaultAppearance();
  return CJS_Result::Success(pRuntime->NewNumber(
      static_cast<int>(field_appearance.GetFontSizeOrZero())));
}

CJS_Result CJS_Field::set_text_size(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_type(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  switch (pFormField->GetFieldType()) {
    case FormFieldType::kUnknown:
      return CJS_Result::Success(pRuntime->NewString("unknown"));
    case FormFieldType::kPushButton:
      return CJS_Result::Success(pRuntime->NewString("button"));
    case FormFieldType::kCheckBox:
      return CJS_Result::Success(pRuntime->NewString("checkbox"));
    case FormFieldType::kRadioButton:
      return CJS_Result::Success(pRuntime->NewString("radiobutton"));
    case FormFieldType::kComboBox:
      return CJS_Result::Success(pRuntime->NewString("combobox"));
    case FormFieldType::kListBox:
      return CJS_Result::Success(pRuntime->NewString("listbox"));
    case FormFieldType::kTextField:
      return CJS_Result::Success(pRuntime->NewString("text"));
    case FormFieldType::kSignature:
      return CJS_Result::Success(pRuntime->NewString("signature"));
#ifdef PDF_ENABLE_XFA
    default:
      return CJS_Result::Success(pRuntime->NewString("unknown"));
#endif
  }
}

CJS_Result CJS_Field::set_type(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::get_user_name(CJS_Runtime* pRuntime) {
  DCHECK(form_fill_env_);

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  return CJS_Result::Success(
      pRuntime->NewString(pFormField->GetAlternateName().AsStringView()));
}

CJS_Result CJS_Field::set_user_name(CJS_Runtime* pRuntime,
                                    v8::Local<v8::Value> vp) {
  DCHECK(form_fill_env_);
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_value(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  v8::Local<v8::Value> ret;
  switch (pFormField->GetFieldType()) {
    case FormFieldType::kPushButton:
      return CJS_Result::Failure(JSMessage::kObjectTypeError);
    case FormFieldType::kComboBox:
    case FormFieldType::kTextField:
      ret = pRuntime->NewString(pFormField->GetValue().AsStringView());
      break;
    case FormFieldType::kListBox: {
      if (pFormField->CountSelectedItems() > 1) {
        v8::Local<v8::Array> ValueArray = pRuntime->NewArray();
        v8::Local<v8::Value> ElementValue;
        int iIndex;
        for (int i = 0, sz = pFormField->CountSelectedItems(); i < sz; i++) {
          iIndex = pFormField->GetSelectedIndex(i);
          ElementValue = pRuntime->NewString(
              pFormField->GetOptionValue(iIndex).AsStringView());
          if (pRuntime->ToWideString(ElementValue).IsEmpty()) {
            ElementValue = pRuntime->NewString(
                pFormField->GetOptionLabel(iIndex).AsStringView());
          }
          pRuntime->PutArrayElement(ValueArray, i, ElementValue);
        }
        ret = ValueArray;
      } else {
        ret = pRuntime->NewString(pFormField->GetValue().AsStringView());
      }
      break;
    }
    case FormFieldType::kCheckBox:
    case FormFieldType::kRadioButton: {
      bool bFind = false;
      for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
        if (pFormField->GetControl(i)->IsChecked()) {
          ret = pRuntime->NewString(
              pFormField->GetControl(i)->GetExportValue().AsStringView());
          bFind = true;
          break;
        }
      }
      if (!bFind) {
        ret = pRuntime->NewString("Off");
      }

      break;
    }
    default:
      ret = pRuntime->NewString(pFormField->GetValue().AsStringView());
      break;
  }
  return CJS_Result::Success(pRuntime->MaybeCoerceToNumber(ret));
}

CJS_Result CJS_Field::set_value(CJS_Runtime* pRuntime,
                                v8::Local<v8::Value> vp) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  std::vector<WideString> strArray;
  if (fxv8::IsArray(vp)) {
    v8::Local<v8::Array> ValueArray = pRuntime->ToArray(vp);
    for (size_t i = 0; i < pRuntime->GetArrayLength(ValueArray); i++) {
      strArray.push_back(
          pRuntime->ToWideString(pRuntime->GetArrayElement(ValueArray, i)));
    }
  } else {
    strArray.push_back(pRuntime->ToWideString(vp));
  }

  if (delay_) {
    AddDelay_WideStringArray(FP_VALUE, strArray);
  } else {
    SetFieldValue(form_fill_env_.Get(), field_name_, form_control_index_,
                  strArray);
  }
  return CJS_Result::Success();
}

CJS_Result CJS_Field::get_value_as_string(CJS_Runtime* pRuntime) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() == FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  if (pFormField->GetFieldType() == FormFieldType::kCheckBox) {
    if (!pFormField->CountControls()) {
      return CJS_Result::Failure(JSMessage::kBadObjectError);
    }
    return CJS_Result::Success(pRuntime->NewString(
        pFormField->GetControl(0)->IsChecked() ? "Yes" : "Off"));
  }

  if (pFormField->GetFieldType() == FormFieldType::kRadioButton &&
      !(pFormField->GetFieldFlags() &
        pdfium::form_flags::kButtonRadiosInUnison)) {
    for (int i = 0, sz = pFormField->CountControls(); i < sz; i++) {
      if (pFormField->GetControl(i)->IsChecked()) {
        return CJS_Result::Success(pRuntime->NewString(
            pFormField->GetControl(i)->GetExportValue().AsStringView()));
      }
    }
    return CJS_Result::Success(pRuntime->NewString("Off"));
  }

  if (pFormField->GetFieldType() == FormFieldType::kListBox &&
      (pFormField->CountSelectedItems() > 1)) {
    return CJS_Result::Success(pRuntime->NewString(""));
  }
  return CJS_Result::Success(
      pRuntime->NewString(pFormField->GetValue().AsStringView()));
}

CJS_Result CJS_Field::set_value_as_string(CJS_Runtime* pRuntime,
                                          v8::Local<v8::Value> vp) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::browseForFileToSubmit(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if ((pFormField->GetFieldFlags() & pdfium::form_flags::kTextFileSelect) &&
      (pFormField->GetFieldType() == FormFieldType::kTextField)) {
    WideString wsFileName = form_fill_env_->JS_fieldBrowse();
    if (!wsFileName.IsEmpty()) {
      pFormField->SetValue(wsFileName, NotificationOption::kDoNotNotify);
      UpdateFormField(form_fill_env_.Get(), pFormField, true);
    }
    return CJS_Result::Success();
  }
  return CJS_Result::Failure(JSMessage::kObjectTypeError);
}

CJS_Result CJS_Field::buttonGetCaption(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  int nface = 0;
  if (params.size() >= 1) {
    nface = pRuntime->ToInt32(params[0]);
  }

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (nface == 0) {
    return CJS_Result::Success(
        pRuntime->NewString(pFormControl->GetNormalCaption().AsStringView()));
  }
  if (nface == 1) {
    return CJS_Result::Success(
        pRuntime->NewString(pFormControl->GetDownCaption().AsStringView()));
  }
  if (nface == 2) {
    return CJS_Result::Success(
        pRuntime->NewString(pFormControl->GetRolloverCaption().AsStringView()));
  }
  return CJS_Result::Failure(JSMessage::kValueError);
}

CJS_Result CJS_Field::buttonGetIcon(CJS_Runtime* pRuntime,
                                    pdfium::span<v8::Local<v8::Value>> params) {
  if (params.size() >= 1) {
    int nFace = pRuntime->ToInt32(params[0]);
    if (nFace < 0 || nFace > 2) {
      return CJS_Result::Failure(JSMessage::kValueError);
    }
  }

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (pFormField->GetFieldType() != FormFieldType::kPushButton) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
  if (!pFormControl) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  v8::Local<v8::Object> pObj = pRuntime->NewFXJSBoundObject(
      CJS_Icon::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
  if (pObj.IsEmpty()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  auto* pJS_Icon = static_cast<CJS_Icon*>(
      CFXJS_Engine::GetBinding(pRuntime->GetIsolate(), pObj));
  return pJS_Icon ? CJS_Result::Success(pJS_Icon->ToV8Object())
                  : CJS_Result::Failure(JSMessage::kBadObjectError);
}

CJS_Result CJS_Field::buttonImportIcon(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::buttonSetCaption(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::buttonSetIcon(CJS_Runtime* pRuntime,
                                    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::checkThisBox(CJS_Runtime* pRuntime,
                                   pdfium::span<v8::Local<v8::Value>> params) {
  const size_t nSize = params.size();
  if (nSize == 0) {
    return CJS_Result::Failure(JSMessage::kParamError);
  }

  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  int nWidget = pRuntime->ToInt32(params[0]);
  bool bCheckit = true;
  if (nSize >= 2) {
    bCheckit = pRuntime->ToBoolean(params[1]);
  }

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsCheckBoxOrRadioButton(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  if (nWidget < 0 || nWidget >= pFormField->CountControls()) {
    return CJS_Result::Failure(JSMessage::kValueError);
  }

  // TODO(weili): Check whether anything special needed for radio button.
  // (When pFormField->GetFieldType() == FormFieldType::kRadioButton.)
  pFormField->CheckControl(nWidget, bCheckit, NotificationOption::kNotify);
  UpdateFormField(form_fill_env_.Get(), pFormField, true);
  return CJS_Result::Success();
}

CJS_Result CJS_Field::clearItems(CJS_Runtime* pRuntime,
                                 pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::defaultIsChecked(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  if (!can_set_) {
    return CJS_Result::Failure(JSMessage::kReadOnlyError);
  }

  if (params.empty()) {
    return CJS_Result::Failure(JSMessage::kParamError);
  }

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  int nWidget = pRuntime->ToInt32(params[0]);
  if (nWidget < 0 || nWidget >= pFormField->CountControls()) {
    return CJS_Result::Failure(JSMessage::kValueError);
  }

  return CJS_Result::Success(
      pRuntime->NewBoolean(IsCheckBoxOrRadioButton(pFormField)));
}

CJS_Result CJS_Field::deleteItemAt(CJS_Runtime* pRuntime,
                                   pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::getArray(CJS_Runtime* pRuntime,
                               pdfium::span<v8::Local<v8::Value>> params) {
  std::vector<CPDF_FormField*> FieldArray = GetFormFields();
  if (FieldArray.empty()) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  std::vector<std::unique_ptr<WideString>> swSort;
  for (CPDF_FormField* pFormField : FieldArray) {
    swSort.push_back(std::make_unique<WideString>(pFormField->GetFullName()));
  }

  std::sort(swSort.begin(), swSort.end(),
            [](const std::unique_ptr<WideString>& p1,
               const std::unique_ptr<WideString>& p2) { return *p1 < *p2; });

  v8::Local<v8::Array> FormFieldArray = pRuntime->NewArray();
  int j = 0;
  for (const auto& pStr : swSort) {
    v8::Local<v8::Object> pObj = pRuntime->NewFXJSBoundObject(
        CJS_Field::GetObjDefnID(), FXJSOBJTYPE_DYNAMIC);
    if (pObj.IsEmpty()) {
      return CJS_Result::Failure(JSMessage::kBadObjectError);
    }

    auto* pJSField = static_cast<CJS_Field*>(
        CFXJS_Engine::GetBinding(pRuntime->GetIsolate(), pObj));
    pJSField->AttachField(js_doc_.Get(), *pStr);
    pRuntime->PutArrayElement(FormFieldArray, j++,
                              pJSField
                                  ? v8::Local<v8::Value>(pJSField->ToV8Object())
                                  : v8::Local<v8::Value>());
  }
  return CJS_Result::Success(FormFieldArray);
}

CJS_Result CJS_Field::getItemAt(CJS_Runtime* pRuntime,
                                pdfium::span<v8::Local<v8::Value>> params) {
  const size_t nSize = params.size();
  int nIdx = -1;
  if (nSize >= 1) {
    nIdx = pRuntime->ToInt32(params[0]);
  }

  bool bExport = true;
  if (nSize >= 2) {
    bExport = pRuntime->ToBoolean(params[1]);
  }

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (!IsComboBoxOrListBox(pFormField)) {
    return CJS_Result::Failure(JSMessage::kObjectTypeError);
  }

  if (nIdx == -1 || nIdx > pFormField->CountOptions()) {
    nIdx = pFormField->CountOptions() - 1;
  }
  if (!bExport) {
    return CJS_Result::Success(
        pRuntime->NewString(pFormField->GetOptionLabel(nIdx).AsStringView()));
  }

  WideString strval = pFormField->GetOptionValue(nIdx);
  if (strval.IsEmpty()) {
    return CJS_Result::Success(
        pRuntime->NewString(pFormField->GetOptionLabel(nIdx).AsStringView()));
  }
  return CJS_Result::Success(pRuntime->NewString(strval.AsStringView()));
}

CJS_Result CJS_Field::getLock(CJS_Runtime* pRuntime,
                              pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::insertItemAt(CJS_Runtime* pRuntime,
                                   pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::isBoxChecked(CJS_Runtime* pRuntime,
                                   pdfium::span<v8::Local<v8::Value>> params) {
  int nIndex = -1;
  if (params.size() >= 1) {
    nIndex = pRuntime->ToInt32(params[0]);
  }

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (nIndex < 0 || nIndex >= pFormField->CountControls()) {
    return CJS_Result::Failure(JSMessage::kValueError);
  }

  return CJS_Result::Success(
      pRuntime->NewBoolean((IsCheckBoxOrRadioButton(pFormField) &&
                            pFormField->GetControl(nIndex)->IsChecked() != 0)));
}

CJS_Result CJS_Field::isDefaultChecked(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  int nIndex = -1;
  if (params.size() >= 1) {
    nIndex = pRuntime->ToInt32(params[0]);
  }

  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  if (nIndex < 0 || nIndex >= pFormField->CountControls()) {
    return CJS_Result::Failure(JSMessage::kValueError);
  }

  return CJS_Result::Success(pRuntime->NewBoolean(
      (IsCheckBoxOrRadioButton(pFormField) &&
       pFormField->GetControl(nIndex)->IsDefaultChecked() != 0)));
}

CJS_Result CJS_Field::setAction(CJS_Runtime* pRuntime,
                                pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::setFocus(CJS_Runtime* pRuntime,
                               pdfium::span<v8::Local<v8::Value>> params) {
  CPDF_FormField* pFormField = GetFirstFormField();
  if (!pFormField) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  int32_t nCount = pFormField->CountControls();
  if (nCount < 1) {
    return CJS_Result::Failure(JSMessage::kBadObjectError);
  }

  CPDFSDK_InteractiveForm* pForm = form_fill_env_->GetInteractiveForm();
  CPDFSDK_Widget* pWidget = nullptr;
  if (nCount == 1) {
    pWidget = pForm->GetWidget(pFormField->GetControl(0));
  } else {
    IPDF_Page* pPage = form_fill_env_->GetCurrentPage();
    if (!pPage) {
      return CJS_Result::Failure(JSMessage::kBadObjectError);
    }
    CPDFSDK_PageView* pCurPageView = form_fill_env_->GetOrCreatePageView(pPage);
    for (int32_t i = 0; i < nCount; i++) {
      if (CPDFSDK_Widget* pTempWidget =
              pForm->GetWidget(pFormField->GetControl(i))) {
        if (pTempWidget->GetPDFPage() == pCurPageView->GetPDFPage()) {
          pWidget = pTempWidget;
          break;
        }
      }
    }
  }

  if (pWidget) {
    ObservedPtr<CPDFSDK_Annot> pObserved(pWidget);
    form_fill_env_->SetFocusAnnot(pObserved);
  }

  return CJS_Result::Success();
}

CJS_Result CJS_Field::setItems(CJS_Runtime* pRuntime,
                               pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Success();
}

CJS_Result CJS_Field::setLock(CJS_Runtime* pRuntime,
                              pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::signatureGetModifications(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::signatureGetSeedValue(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::signatureInfo(CJS_Runtime* pRuntime,
                                    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::signatureSetSeedValue(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::signatureSign(CJS_Runtime* pRuntime,
                                    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

CJS_Result CJS_Field::signatureValidate(
    CJS_Runtime* pRuntime,
    pdfium::span<v8::Local<v8::Value>> params) {
  return CJS_Result::Failure(JSMessage::kNotSupportedError);
}

void CJS_Field::SetDelay(bool bDelay) {
  delay_ = bDelay;
  if (delay_) {
    return;
  }

  if (js_doc_) {
    js_doc_->DoFieldDelay(field_name_, form_control_index_);
  }
}

void CJS_Field::AddDelay_Int(FIELD_PROP prop, int32_t n) {
  auto pNewData =
      std::make_unique<CJS_DelayData>(prop, form_control_index_, field_name_);
  pNewData->num = n;
  js_doc_->AddDelayData(std::move(pNewData));
}

void CJS_Field::AddDelay_Bool(FIELD_PROP prop, bool b) {
  auto pNewData =
      std::make_unique<CJS_DelayData>(prop, form_control_index_, field_name_);
  pNewData->b = b;
  js_doc_->AddDelayData(std::move(pNewData));
}

void CJS_Field::AddDelay_String(FIELD_PROP prop, const ByteString& str) {
  auto pNewData =
      std::make_unique<CJS_DelayData>(prop, form_control_index_, field_name_);
  pNewData->bytestring = str;
  js_doc_->AddDelayData(std::move(pNewData));
}

void CJS_Field::AddDelay_Rect(FIELD_PROP prop, const CFX_FloatRect& rect) {
  auto pNewData =
      std::make_unique<CJS_DelayData>(prop, form_control_index_, field_name_);
  pNewData->rect = rect;
  js_doc_->AddDelayData(std::move(pNewData));
}

void CJS_Field::AddDelay_WordArray(FIELD_PROP prop,
                                   const std::vector<uint32_t>& array) {
  auto pNewData =
      std::make_unique<CJS_DelayData>(prop, form_control_index_, field_name_);
  pNewData->wordarray = array;
  js_doc_->AddDelayData(std::move(pNewData));
}

void CJS_Field::AddDelay_WideStringArray(FIELD_PROP prop,
                                         const std::vector<WideString>& array) {
  auto pNewData =
      std::make_unique<CJS_DelayData>(prop, form_control_index_, field_name_);
  pNewData->widestringarray = array;
  js_doc_->AddDelayData(std::move(pNewData));
}

void CJS_Field::DoDelay(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                        CJS_DelayData* pData) {
  DCHECK(pFormFillEnv);
  switch (pData->eProp) {
    case FP_BORDERSTYLE:
      SetBorderStyle(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                     pData->bytestring);
      break;
    case FP_CURRENTVALUEINDICES:
      SetCurrentValueIndices(pFormFillEnv, pData->sFieldName,
                             pData->nControlIndex, pData->wordarray);
      break;
    case FP_DISPLAY:
      SetDisplay(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                 pData->num);
      break;
    case FP_HIDDEN:
      SetHidden(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                pData->b);
      break;
    case FP_LINEWIDTH:
      SetLineWidth(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                   pData->num);
      break;
    case FP_RECT:
      SetRect(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
              pData->rect);
      break;
    case FP_VALUE:
      SetFieldValue(pFormFillEnv, pData->sFieldName, pData->nControlIndex,
                    pData->widestringarray);
      break;
  }
}
