// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <memory>

#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_accessiblecontent.h"
#include "xfa/fxfa/parser/cxfa_acrobat.h"
#include "xfa/fxfa/parser/cxfa_acrobat7.h"
#include "xfa/fxfa/parser/cxfa_adbe_jsconsole.h"
#include "xfa/fxfa/parser/cxfa_adbe_jsdebugger.h"
#include "xfa/fxfa/parser/cxfa_addsilentprint.h"
#include "xfa/fxfa/parser/cxfa_addviewerpreferences.h"
#include "xfa/fxfa/parser/cxfa_adjustdata.h"
#include "xfa/fxfa/parser/cxfa_adobeextensionlevel.h"
#include "xfa/fxfa/parser/cxfa_agent.h"
#include "xfa/fxfa/parser/cxfa_alwaysembed.h"
#include "xfa/fxfa/parser/cxfa_amd.h"
#include "xfa/fxfa/parser/cxfa_appearancefilter.h"
#include "xfa/fxfa/parser/cxfa_arc.h"
#include "xfa/fxfa/parser/cxfa_area.h"
#include "xfa/fxfa/parser/cxfa_assist.h"
#include "xfa/fxfa/parser/cxfa_attributes.h"
#include "xfa/fxfa/parser/cxfa_autosave.h"
#include "xfa/fxfa/parser/cxfa_barcode.h"
#include "xfa/fxfa/parser/cxfa_base.h"
#include "xfa/fxfa/parser/cxfa_batchoutput.h"
#include "xfa/fxfa/parser/cxfa_behavioroverride.h"
#include "xfa/fxfa/parser/cxfa_bind.h"
#include "xfa/fxfa/parser/cxfa_binditems.h"
#include "xfa/fxfa/parser/cxfa_bookend.h"
#include "xfa/fxfa/parser/cxfa_boolean.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_break.h"
#include "xfa/fxfa/parser/cxfa_breakafter.h"
#include "xfa/fxfa/parser/cxfa_breakbefore.h"
#include "xfa/fxfa/parser/cxfa_button.h"
#include "xfa/fxfa/parser/cxfa_cache.h"
#include "xfa/fxfa/parser/cxfa_calculate.h"
#include "xfa/fxfa/parser/cxfa_calendarsymbols.h"
#include "xfa/fxfa/parser/cxfa_caption.h"
#include "xfa/fxfa/parser/cxfa_certificate.h"
#include "xfa/fxfa/parser/cxfa_certificates.h"
#include "xfa/fxfa/parser/cxfa_change.h"
#include "xfa/fxfa/parser/cxfa_checkbutton.h"
#include "xfa/fxfa/parser/cxfa_choicelist.h"
#include "xfa/fxfa/parser/cxfa_color.h"
#include "xfa/fxfa/parser/cxfa_comb.h"
#include "xfa/fxfa/parser/cxfa_command.h"
#include "xfa/fxfa/parser/cxfa_common.h"
#include "xfa/fxfa/parser/cxfa_compress.h"
#include "xfa/fxfa/parser/cxfa_compression.h"
#include "xfa/fxfa/parser/cxfa_compresslogicalstructure.h"
#include "xfa/fxfa/parser/cxfa_compressobjectstream.h"
#include "xfa/fxfa/parser/cxfa_config.h"
#include "xfa/fxfa/parser/cxfa_conformance.h"
#include "xfa/fxfa/parser/cxfa_connect.h"
#include "xfa/fxfa/parser/cxfa_connectionset.h"
#include "xfa/fxfa/parser/cxfa_connectstring.h"
#include "xfa/fxfa/parser/cxfa_contentarea.h"
#include "xfa/fxfa/parser/cxfa_contentcopy.h"
#include "xfa/fxfa/parser/cxfa_copies.h"
#include "xfa/fxfa/parser/cxfa_corner.h"
#include "xfa/fxfa/parser/cxfa_creator.h"
#include "xfa/fxfa/parser/cxfa_currencysymbol.h"
#include "xfa/fxfa/parser/cxfa_currencysymbols.h"
#include "xfa/fxfa/parser/cxfa_currentpage.h"
#include "xfa/fxfa/parser/cxfa_data.h"
#include "xfa/fxfa/parser/cxfa_datagroup.h"
#include "xfa/fxfa/parser/cxfa_datamodel.h"
#include "xfa/fxfa/parser/cxfa_datavalue.h"
#include "xfa/fxfa/parser/cxfa_date.h"
#include "xfa/fxfa/parser/cxfa_datepattern.h"
#include "xfa/fxfa/parser/cxfa_datepatterns.h"
#include "xfa/fxfa/parser/cxfa_datetime.h"
#include "xfa/fxfa/parser/cxfa_datetimeedit.h"
#include "xfa/fxfa/parser/cxfa_datetimesymbols.h"
#include "xfa/fxfa/parser/cxfa_day.h"
#include "xfa/fxfa/parser/cxfa_daynames.h"
#include "xfa/fxfa/parser/cxfa_debug.h"
#include "xfa/fxfa/parser/cxfa_decimal.h"
#include "xfa/fxfa/parser/cxfa_defaulttypeface.h"
#include "xfa/fxfa/parser/cxfa_defaultui.h"
#include "xfa/fxfa/parser/cxfa_delete.h"
#include "xfa/fxfa/parser/cxfa_delta.h"
#include "xfa/fxfa/parser/cxfa_deltas.h"
#include "xfa/fxfa/parser/cxfa_desc.h"
#include "xfa/fxfa/parser/cxfa_destination.h"
#include "xfa/fxfa/parser/cxfa_digestmethod.h"
#include "xfa/fxfa/parser/cxfa_digestmethods.h"
#include "xfa/fxfa/parser/cxfa_documentassembly.h"
#include "xfa/fxfa/parser/cxfa_draw.h"
#include "xfa/fxfa/parser/cxfa_driver.h"
#include "xfa/fxfa/parser/cxfa_dsigdata.h"
#include "xfa/fxfa/parser/cxfa_duplexoption.h"
#include "xfa/fxfa/parser/cxfa_dynamicrender.h"
#include "xfa/fxfa/parser/cxfa_edge.h"
#include "xfa/fxfa/parser/cxfa_effectiveinputpolicy.h"
#include "xfa/fxfa/parser/cxfa_effectiveoutputpolicy.h"
#include "xfa/fxfa/parser/cxfa_embed.h"
#include "xfa/fxfa/parser/cxfa_encoding.h"
#include "xfa/fxfa/parser/cxfa_encodings.h"
#include "xfa/fxfa/parser/cxfa_encrypt.h"
#include "xfa/fxfa/parser/cxfa_encryption.h"
#include "xfa/fxfa/parser/cxfa_encryptionlevel.h"
#include "xfa/fxfa/parser/cxfa_encryptionmethod.h"
#include "xfa/fxfa/parser/cxfa_encryptionmethods.h"
#include "xfa/fxfa/parser/cxfa_enforce.h"
#include "xfa/fxfa/parser/cxfa_equate.h"
#include "xfa/fxfa/parser/cxfa_equaterange.h"
#include "xfa/fxfa/parser/cxfa_era.h"
#include "xfa/fxfa/parser/cxfa_eranames.h"
#include "xfa/fxfa/parser/cxfa_event.h"
#include "xfa/fxfa/parser/cxfa_exclgroup.h"
#include "xfa/fxfa/parser/cxfa_exclude.h"
#include "xfa/fxfa/parser/cxfa_excludens.h"
#include "xfa/fxfa/parser/cxfa_exdata.h"
#include "xfa/fxfa/parser/cxfa_execute.h"
#include "xfa/fxfa/parser/cxfa_exobject.h"
#include "xfa/fxfa/parser/cxfa_extras.h"
#include "xfa/fxfa/parser/cxfa_field.h"
#include "xfa/fxfa/parser/cxfa_fill.h"
#include "xfa/fxfa/parser/cxfa_filter.h"
#include "xfa/fxfa/parser/cxfa_fliplabel.h"
#include "xfa/fxfa/parser/cxfa_float.h"
#include "xfa/fxfa/parser/cxfa_font.h"
#include "xfa/fxfa/parser/cxfa_fontinfo.h"
#include "xfa/fxfa/parser/cxfa_form.h"
#include "xfa/fxfa/parser/cxfa_format.h"
#include "xfa/fxfa/parser/cxfa_formfieldfilling.h"
#include "xfa/fxfa/parser/cxfa_groupparent.h"
#include "xfa/fxfa/parser/cxfa_handler.h"
#include "xfa/fxfa/parser/cxfa_hyphenation.h"
#include "xfa/fxfa/parser/cxfa_ifempty.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_imageedit.h"
#include "xfa/fxfa/parser/cxfa_includexdpcontent.h"
#include "xfa/fxfa/parser/cxfa_incrementalload.h"
#include "xfa/fxfa/parser/cxfa_incrementalmerge.h"
#include "xfa/fxfa/parser/cxfa_insert.h"
#include "xfa/fxfa/parser/cxfa_instancemanager.h"
#include "xfa/fxfa/parser/cxfa_integer.h"
#include "xfa/fxfa/parser/cxfa_interactive.h"
#include "xfa/fxfa/parser/cxfa_issuers.h"
#include "xfa/fxfa/parser/cxfa_items.h"
#include "xfa/fxfa/parser/cxfa_jog.h"
#include "xfa/fxfa/parser/cxfa_keep.h"
#include "xfa/fxfa/parser/cxfa_keyusage.h"
#include "xfa/fxfa/parser/cxfa_labelprinter.h"
#include "xfa/fxfa/parser/cxfa_layout.h"
#include "xfa/fxfa/parser/cxfa_level.h"
#include "xfa/fxfa/parser/cxfa_line.h"
#include "xfa/fxfa/parser/cxfa_linear.h"
#include "xfa/fxfa/parser/cxfa_linearized.h"
#include "xfa/fxfa/parser/cxfa_locale.h"
#include "xfa/fxfa/parser/cxfa_localeset.h"
#include "xfa/fxfa/parser/cxfa_lockdocument.h"
#include "xfa/fxfa/parser/cxfa_log.h"
#include "xfa/fxfa/parser/cxfa_manifest.h"
#include "xfa/fxfa/parser/cxfa_map.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_mdp.h"
#include "xfa/fxfa/parser/cxfa_medium.h"
#include "xfa/fxfa/parser/cxfa_mediuminfo.h"
#include "xfa/fxfa/parser/cxfa_meridiem.h"
#include "xfa/fxfa/parser/cxfa_meridiemnames.h"
#include "xfa/fxfa/parser/cxfa_message.h"
#include "xfa/fxfa/parser/cxfa_messaging.h"
#include "xfa/fxfa/parser/cxfa_mode.h"
#include "xfa/fxfa/parser/cxfa_modifyannots.h"
#include "xfa/fxfa/parser/cxfa_month.h"
#include "xfa/fxfa/parser/cxfa_monthnames.h"
#include "xfa/fxfa/parser/cxfa_msgid.h"
#include "xfa/fxfa/parser/cxfa_nameattr.h"
#include "xfa/fxfa/parser/cxfa_neverembed.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_numberofcopies.h"
#include "xfa/fxfa/parser/cxfa_numberpattern.h"
#include "xfa/fxfa/parser/cxfa_numberpatterns.h"
#include "xfa/fxfa/parser/cxfa_numbersymbol.h"
#include "xfa/fxfa/parser/cxfa_numbersymbols.h"
#include "xfa/fxfa/parser/cxfa_numericedit.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/cxfa_oid.h"
#include "xfa/fxfa/parser/cxfa_oids.h"
#include "xfa/fxfa/parser/cxfa_openaction.h"
#include "xfa/fxfa/parser/cxfa_operation.h"
#include "xfa/fxfa/parser/cxfa_output.h"
#include "xfa/fxfa/parser/cxfa_outputbin.h"
#include "xfa/fxfa/parser/cxfa_outputxsl.h"
#include "xfa/fxfa/parser/cxfa_overflow.h"
#include "xfa/fxfa/parser/cxfa_overprint.h"
#include "xfa/fxfa/parser/cxfa_packet.h"
#include "xfa/fxfa/parser/cxfa_packets.h"
#include "xfa/fxfa/parser/cxfa_pagearea.h"
#include "xfa/fxfa/parser/cxfa_pageoffset.h"
#include "xfa/fxfa/parser/cxfa_pagerange.h"
#include "xfa/fxfa/parser/cxfa_pageset.h"
#include "xfa/fxfa/parser/cxfa_pagination.h"
#include "xfa/fxfa/parser/cxfa_paginationoverride.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_part.h"
#include "xfa/fxfa/parser/cxfa_password.h"
#include "xfa/fxfa/parser/cxfa_passwordedit.h"
#include "xfa/fxfa/parser/cxfa_pattern.h"
#include "xfa/fxfa/parser/cxfa_pcl.h"
#include "xfa/fxfa/parser/cxfa_pdf.h"
#include "xfa/fxfa/parser/cxfa_pdfa.h"
#include "xfa/fxfa/parser/cxfa_permissions.h"
#include "xfa/fxfa/parser/cxfa_picktraybypdfsize.h"
#include "xfa/fxfa/parser/cxfa_picture.h"
#include "xfa/fxfa/parser/cxfa_plaintextmetadata.h"
#include "xfa/fxfa/parser/cxfa_presence.h"
#include "xfa/fxfa/parser/cxfa_present.h"
#include "xfa/fxfa/parser/cxfa_print.h"
#include "xfa/fxfa/parser/cxfa_printername.h"
#include "xfa/fxfa/parser/cxfa_printhighquality.h"
#include "xfa/fxfa/parser/cxfa_printscaling.h"
#include "xfa/fxfa/parser/cxfa_producer.h"
#include "xfa/fxfa/parser/cxfa_proto.h"
#include "xfa/fxfa/parser/cxfa_ps.h"
#include "xfa/fxfa/parser/cxfa_psmap.h"
#include "xfa/fxfa/parser/cxfa_query.h"
#include "xfa/fxfa/parser/cxfa_radial.h"
#include "xfa/fxfa/parser/cxfa_range.h"
#include "xfa/fxfa/parser/cxfa_reason.h"
#include "xfa/fxfa/parser/cxfa_reasons.h"
#include "xfa/fxfa/parser/cxfa_record.h"
#include "xfa/fxfa/parser/cxfa_recordset.h"
#include "xfa/fxfa/parser/cxfa_rectangle.h"
#include "xfa/fxfa/parser/cxfa_ref.h"
#include "xfa/fxfa/parser/cxfa_relevant.h"
#include "xfa/fxfa/parser/cxfa_rename.h"
#include "xfa/fxfa/parser/cxfa_renderpolicy.h"
#include "xfa/fxfa/parser/cxfa_rootelement.h"
#include "xfa/fxfa/parser/cxfa_runscripts.h"
#include "xfa/fxfa/parser/cxfa_script.h"
#include "xfa/fxfa/parser/cxfa_scriptmodel.h"
#include "xfa/fxfa/parser/cxfa_select.h"
#include "xfa/fxfa/parser/cxfa_setproperty.h"
#include "xfa/fxfa/parser/cxfa_severity.h"
#include "xfa/fxfa/parser/cxfa_sharptext.h"
#include "xfa/fxfa/parser/cxfa_sharpxhtml.h"
#include "xfa/fxfa/parser/cxfa_sharpxml.h"
#include "xfa/fxfa/parser/cxfa_signature.h"
#include "xfa/fxfa/parser/cxfa_signatureproperties.h"
#include "xfa/fxfa/parser/cxfa_signdata.h"
#include "xfa/fxfa/parser/cxfa_signing.h"
#include "xfa/fxfa/parser/cxfa_silentprint.h"
#include "xfa/fxfa/parser/cxfa_soapaction.h"
#include "xfa/fxfa/parser/cxfa_soapaddress.h"
#include "xfa/fxfa/parser/cxfa_solid.h"
#include "xfa/fxfa/parser/cxfa_source.h"
#include "xfa/fxfa/parser/cxfa_sourceset.h"
#include "xfa/fxfa/parser/cxfa_speak.h"
#include "xfa/fxfa/parser/cxfa_staple.h"
#include "xfa/fxfa/parser/cxfa_startnode.h"
#include "xfa/fxfa/parser/cxfa_startpage.h"
#include "xfa/fxfa/parser/cxfa_stipple.h"
#include "xfa/fxfa/parser/cxfa_subform.h"
#include "xfa/fxfa/parser/cxfa_subformset.h"
#include "xfa/fxfa/parser/cxfa_subjectdn.h"
#include "xfa/fxfa/parser/cxfa_subjectdns.h"
#include "xfa/fxfa/parser/cxfa_submit.h"
#include "xfa/fxfa/parser/cxfa_submitformat.h"
#include "xfa/fxfa/parser/cxfa_submiturl.h"
#include "xfa/fxfa/parser/cxfa_subsetbelow.h"
#include "xfa/fxfa/parser/cxfa_suppressbanner.h"
#include "xfa/fxfa/parser/cxfa_tagged.h"
#include "xfa/fxfa/parser/cxfa_template.h"
#include "xfa/fxfa/parser/cxfa_templatecache.h"
#include "xfa/fxfa/parser/cxfa_text.h"
#include "xfa/fxfa/parser/cxfa_textedit.h"
#include "xfa/fxfa/parser/cxfa_threshold.h"
#include "xfa/fxfa/parser/cxfa_time.h"
#include "xfa/fxfa/parser/cxfa_timepattern.h"
#include "xfa/fxfa/parser/cxfa_timepatterns.h"
#include "xfa/fxfa/parser/cxfa_timestamp.h"
#include "xfa/fxfa/parser/cxfa_to.h"
#include "xfa/fxfa/parser/cxfa_tooltip.h"
#include "xfa/fxfa/parser/cxfa_trace.h"
#include "xfa/fxfa/parser/cxfa_transform.h"
#include "xfa/fxfa/parser/cxfa_traversal.h"
#include "xfa/fxfa/parser/cxfa_traverse.h"
#include "xfa/fxfa/parser/cxfa_type.h"
#include "xfa/fxfa/parser/cxfa_typeface.h"
#include "xfa/fxfa/parser/cxfa_typefaces.h"
#include "xfa/fxfa/parser/cxfa_ui.h"
#include "xfa/fxfa/parser/cxfa_update.h"
#include "xfa/fxfa/parser/cxfa_uri.h"
#include "xfa/fxfa/parser/cxfa_user.h"
#include "xfa/fxfa/parser/cxfa_validate.h"
#include "xfa/fxfa/parser/cxfa_validateapprovalsignatures.h"
#include "xfa/fxfa/parser/cxfa_validationmessaging.h"
#include "xfa/fxfa/parser/cxfa_value.h"
#include "xfa/fxfa/parser/cxfa_variables.h"
#include "xfa/fxfa/parser/cxfa_version.h"
#include "xfa/fxfa/parser/cxfa_versioncontrol.h"
#include "xfa/fxfa/parser/cxfa_viewerpreferences.h"
#include "xfa/fxfa/parser/cxfa_webclient.h"
#include "xfa/fxfa/parser/cxfa_whitespace.h"
#include "xfa/fxfa/parser/cxfa_window.h"
#include "xfa/fxfa/parser/cxfa_wsdladdress.h"
#include "xfa/fxfa/parser/cxfa_wsdlconnection.h"
#include "xfa/fxfa/parser/cxfa_xdc.h"
#include "xfa/fxfa/parser/cxfa_xdp.h"
#include "xfa/fxfa/parser/cxfa_xfa.h"
#include "xfa/fxfa/parser/cxfa_xmlconnection.h"
#include "xfa/fxfa/parser/cxfa_xsdconnection.h"
#include "xfa/fxfa/parser/cxfa_xsl.h"
#include "xfa/fxfa/parser/cxfa_zpl.h"

namespace {

struct ElementNameInfo {
  uint32_t hash;  // Hashed as wide string.
  XFA_Element element;
};

const ElementNameInfo ElementNameToEnum[] = {
#undef ELEM____
#undef ELEM_HIDDEN____
#define ELEM____(a, b, c, d) {a, XFA_Element::c},
#define ELEM_HIDDEN____(a, b)
#include "xfa/fxfa/parser/elements.inc"
#undef ELEM____
#undef ELEM_HIDDEN____
};

struct AttributeNameInfo {
  uint32_t hash;  // Hashed as wide string.
  XFA_Attribute attribute;
};

const AttributeNameInfo AttributeNameInfoToEnum[] = {
#undef ATTR____
#define ATTR____(a, b, c) {a, XFA_Attribute::c},
#include "xfa/fxfa/parser/attributes.inc"
#undef ATTR____
};

const char* AttributeToNameASCII(XFA_Attribute attr) {
  switch (attr) {
#undef ATTR____
#define ATTR____(a, b, c) \
  case XFA_Attribute::c:  \
    return b;
#include "xfa/fxfa/parser/attributes.inc"
#undef ATTR____
    default:
      NOTREACHED();
      return "";
  }
}

}  // namespace

// static
XFA_Element CXFA_Node::NameToElement(const WideString& name) {
  uint32_t hash = FX_HashCode_GetW(name.AsStringView(), false);
  auto* elem = std::lower_bound(
      std::begin(ElementNameToEnum), std::end(ElementNameToEnum), hash,
      [](const ElementNameInfo& a, uint32_t hash) { return a.hash < hash; });
  if (elem != std::end(ElementNameToEnum) && elem->hash == hash)
    return elem->element;
  return XFA_Element::Unknown;
}

// static
XFA_Attribute CXFA_Node::NameToAttribute(const WideStringView& name) {
  uint32_t hash = FX_HashCode_GetW(name, false);
  auto* elem = std::lower_bound(
      std::begin(AttributeNameInfoToEnum), std::end(AttributeNameInfoToEnum),
      hash,
      [](const AttributeNameInfo& a, uint32_t hash) { return a.hash < hash; });
  if (elem != std::end(AttributeNameInfoToEnum) && elem->hash == hash)
    return elem->attribute;
  return XFA_Attribute::Unknown;
}

// static
std::unique_ptr<CXFA_Node> CXFA_Node::Create(CXFA_Document* doc,
                                             XFA_Element element,
                                             XFA_PacketType packet) {
  std::unique_ptr<CXFA_Node> node;
  switch (element) {
    case XFA_Element::Ps:
      node = pdfium::MakeUnique<CXFA_Ps>(doc, packet);
      break;
    case XFA_Element::To:
      node = pdfium::MakeUnique<CXFA_To>(doc, packet);
      break;
    case XFA_Element::Ui:
      node = pdfium::MakeUnique<CXFA_Ui>(doc, packet);
      break;
    case XFA_Element::RecordSet:
      node = pdfium::MakeUnique<CXFA_RecordSet>(doc, packet);
      break;
    case XFA_Element::SubsetBelow:
      node = pdfium::MakeUnique<CXFA_SubsetBelow>(doc, packet);
      break;
    case XFA_Element::SubformSet:
      node = pdfium::MakeUnique<CXFA_SubformSet>(doc, packet);
      break;
    case XFA_Element::AdobeExtensionLevel:
      node = pdfium::MakeUnique<CXFA_AdobeExtensionLevel>(doc, packet);
      break;
    case XFA_Element::Typeface:
      node = pdfium::MakeUnique<CXFA_Typeface>(doc, packet);
      break;
    case XFA_Element::Break:
      node = pdfium::MakeUnique<CXFA_Break>(doc, packet);
      break;
    case XFA_Element::FontInfo:
      node = pdfium::MakeUnique<CXFA_FontInfo>(doc, packet);
      break;
    case XFA_Element::NumberPattern:
      node = pdfium::MakeUnique<CXFA_NumberPattern>(doc, packet);
      break;
    case XFA_Element::DynamicRender:
      node = pdfium::MakeUnique<CXFA_DynamicRender>(doc, packet);
      break;
    case XFA_Element::PrintScaling:
      node = pdfium::MakeUnique<CXFA_PrintScaling>(doc, packet);
      break;
    case XFA_Element::CheckButton:
      node = pdfium::MakeUnique<CXFA_CheckButton>(doc, packet);
      break;
    case XFA_Element::DatePatterns:
      node = pdfium::MakeUnique<CXFA_DatePatterns>(doc, packet);
      break;
    case XFA_Element::SourceSet:
      node = pdfium::MakeUnique<CXFA_SourceSet>(doc, packet);
      break;
    case XFA_Element::Amd:
      node = pdfium::MakeUnique<CXFA_Amd>(doc, packet);
      break;
    case XFA_Element::Arc:
      node = pdfium::MakeUnique<CXFA_Arc>(doc, packet);
      break;
    case XFA_Element::Day:
      node = pdfium::MakeUnique<CXFA_Day>(doc, packet);
      break;
    case XFA_Element::Era:
      node = pdfium::MakeUnique<CXFA_Era>(doc, packet);
      break;
    case XFA_Element::Jog:
      node = pdfium::MakeUnique<CXFA_Jog>(doc, packet);
      break;
    case XFA_Element::Log:
      node = pdfium::MakeUnique<CXFA_Log>(doc, packet);
      break;
    case XFA_Element::Map:
      node = pdfium::MakeUnique<CXFA_Map>(doc, packet);
      break;
    case XFA_Element::Mdp:
      node = pdfium::MakeUnique<CXFA_Mdp>(doc, packet);
      break;
    case XFA_Element::BreakBefore:
      node = pdfium::MakeUnique<CXFA_BreakBefore>(doc, packet);
      break;
    case XFA_Element::Oid:
      node = pdfium::MakeUnique<CXFA_Oid>(doc, packet);
      break;
    case XFA_Element::Pcl:
      node = pdfium::MakeUnique<CXFA_Pcl>(doc, packet);
      break;
    case XFA_Element::Pdf:
      node = pdfium::MakeUnique<CXFA_Pdf>(doc, packet);
      break;
    case XFA_Element::Ref:
      node = pdfium::MakeUnique<CXFA_Ref>(doc, packet);
      break;
    case XFA_Element::Uri:
      node = pdfium::MakeUnique<CXFA_Uri>(doc, packet);
      break;
    case XFA_Element::Xdc:
      node = pdfium::MakeUnique<CXFA_Xdc>(doc, packet);
      break;
    case XFA_Element::Xdp:
      node = pdfium::MakeUnique<CXFA_Xdp>(doc, packet);
      break;
    case XFA_Element::Xfa:
      node = pdfium::MakeUnique<CXFA_Xfa>(doc, packet);
      break;
    case XFA_Element::Xsl:
      node = pdfium::MakeUnique<CXFA_Xsl>(doc, packet);
      break;
    case XFA_Element::Zpl:
      node = pdfium::MakeUnique<CXFA_Zpl>(doc, packet);
      break;
    case XFA_Element::Cache:
      node = pdfium::MakeUnique<CXFA_Cache>(doc, packet);
      break;
    case XFA_Element::Margin:
      node = pdfium::MakeUnique<CXFA_Margin>(doc, packet);
      break;
    case XFA_Element::KeyUsage:
      node = pdfium::MakeUnique<CXFA_KeyUsage>(doc, packet);
      break;
    case XFA_Element::Exclude:
      node = pdfium::MakeUnique<CXFA_Exclude>(doc, packet);
      break;
    case XFA_Element::ChoiceList:
      node = pdfium::MakeUnique<CXFA_ChoiceList>(doc, packet);
      break;
    case XFA_Element::Level:
      node = pdfium::MakeUnique<CXFA_Level>(doc, packet);
      break;
    case XFA_Element::LabelPrinter:
      node = pdfium::MakeUnique<CXFA_LabelPrinter>(doc, packet);
      break;
    case XFA_Element::CalendarSymbols:
      node = pdfium::MakeUnique<CXFA_CalendarSymbols>(doc, packet);
      break;
    case XFA_Element::Para:
      node = pdfium::MakeUnique<CXFA_Para>(doc, packet);
      break;
    case XFA_Element::Part:
      node = pdfium::MakeUnique<CXFA_Part>(doc, packet);
      break;
    case XFA_Element::Pdfa:
      node = pdfium::MakeUnique<CXFA_Pdfa>(doc, packet);
      break;
    case XFA_Element::Filter:
      node = pdfium::MakeUnique<CXFA_Filter>(doc, packet);
      break;
    case XFA_Element::Present:
      node = pdfium::MakeUnique<CXFA_Present>(doc, packet);
      break;
    case XFA_Element::Pagination:
      node = pdfium::MakeUnique<CXFA_Pagination>(doc, packet);
      break;
    case XFA_Element::Encoding:
      node = pdfium::MakeUnique<CXFA_Encoding>(doc, packet);
      break;
    case XFA_Element::Event:
      node = pdfium::MakeUnique<CXFA_Event>(doc, packet);
      break;
    case XFA_Element::Whitespace:
      node = pdfium::MakeUnique<CXFA_Whitespace>(doc, packet);
      break;
    case XFA_Element::DefaultUi:
      node = pdfium::MakeUnique<CXFA_DefaultUi>(doc, packet);
      break;
    case XFA_Element::DataModel:
      node = pdfium::MakeUnique<CXFA_DataModel>(doc, packet);
      break;
    case XFA_Element::Barcode:
      node = pdfium::MakeUnique<CXFA_Barcode>(doc, packet);
      break;
    case XFA_Element::TimePattern:
      node = pdfium::MakeUnique<CXFA_TimePattern>(doc, packet);
      break;
    case XFA_Element::BatchOutput:
      node = pdfium::MakeUnique<CXFA_BatchOutput>(doc, packet);
      break;
    case XFA_Element::Enforce:
      node = pdfium::MakeUnique<CXFA_Enforce>(doc, packet);
      break;
    case XFA_Element::CurrencySymbols:
      node = pdfium::MakeUnique<CXFA_CurrencySymbols>(doc, packet);
      break;
    case XFA_Element::AddSilentPrint:
      node = pdfium::MakeUnique<CXFA_AddSilentPrint>(doc, packet);
      break;
    case XFA_Element::Rename:
      node = pdfium::MakeUnique<CXFA_Rename>(doc, packet);
      break;
    case XFA_Element::Operation:
      node = pdfium::MakeUnique<CXFA_Operation>(doc, packet);
      break;
    case XFA_Element::Typefaces:
      node = pdfium::MakeUnique<CXFA_Typefaces>(doc, packet);
      break;
    case XFA_Element::SubjectDNs:
      node = pdfium::MakeUnique<CXFA_SubjectDNs>(doc, packet);
      break;
    case XFA_Element::Issuers:
      node = pdfium::MakeUnique<CXFA_Issuers>(doc, packet);
      break;
    case XFA_Element::WsdlConnection:
      node = pdfium::MakeUnique<CXFA_WsdlConnection>(doc, packet);
      break;
    case XFA_Element::Debug:
      node = pdfium::MakeUnique<CXFA_Debug>(doc, packet);
      break;
    case XFA_Element::Delta:
      node = pdfium::MakeUnique<CXFA_Delta>(doc, packet);
      break;
    case XFA_Element::EraNames:
      node = pdfium::MakeUnique<CXFA_EraNames>(doc, packet);
      break;
    case XFA_Element::ModifyAnnots:
      node = pdfium::MakeUnique<CXFA_ModifyAnnots>(doc, packet);
      break;
    case XFA_Element::StartNode:
      node = pdfium::MakeUnique<CXFA_StartNode>(doc, packet);
      break;
    case XFA_Element::Button:
      node = pdfium::MakeUnique<CXFA_Button>(doc, packet);
      break;
    case XFA_Element::Format:
      node = pdfium::MakeUnique<CXFA_Format>(doc, packet);
      break;
    case XFA_Element::Border:
      node = pdfium::MakeUnique<CXFA_Border>(doc, packet);
      break;
    case XFA_Element::Area:
      node = pdfium::MakeUnique<CXFA_Area>(doc, packet);
      break;
    case XFA_Element::Hyphenation:
      node = pdfium::MakeUnique<CXFA_Hyphenation>(doc, packet);
      break;
    case XFA_Element::Text:
      node = pdfium::MakeUnique<CXFA_Text>(doc, packet);
      break;
    case XFA_Element::Time:
      node = pdfium::MakeUnique<CXFA_Time>(doc, packet);
      break;
    case XFA_Element::Type:
      node = pdfium::MakeUnique<CXFA_Type>(doc, packet);
      break;
    case XFA_Element::Overprint:
      node = pdfium::MakeUnique<CXFA_Overprint>(doc, packet);
      break;
    case XFA_Element::Certificates:
      node = pdfium::MakeUnique<CXFA_Certificates>(doc, packet);
      break;
    case XFA_Element::EncryptionMethods:
      node = pdfium::MakeUnique<CXFA_EncryptionMethods>(doc, packet);
      break;
    case XFA_Element::SetProperty:
      node = pdfium::MakeUnique<CXFA_SetProperty>(doc, packet);
      break;
    case XFA_Element::PrinterName:
      node = pdfium::MakeUnique<CXFA_PrinterName>(doc, packet);
      break;
    case XFA_Element::StartPage:
      node = pdfium::MakeUnique<CXFA_StartPage>(doc, packet);
      break;
    case XFA_Element::PageOffset:
      node = pdfium::MakeUnique<CXFA_PageOffset>(doc, packet);
      break;
    case XFA_Element::DateTime:
      node = pdfium::MakeUnique<CXFA_DateTime>(doc, packet);
      break;
    case XFA_Element::Comb:
      node = pdfium::MakeUnique<CXFA_Comb>(doc, packet);
      break;
    case XFA_Element::Pattern:
      node = pdfium::MakeUnique<CXFA_Pattern>(doc, packet);
      break;
    case XFA_Element::IfEmpty:
      node = pdfium::MakeUnique<CXFA_IfEmpty>(doc, packet);
      break;
    case XFA_Element::SuppressBanner:
      node = pdfium::MakeUnique<CXFA_SuppressBanner>(doc, packet);
      break;
    case XFA_Element::OutputBin:
      node = pdfium::MakeUnique<CXFA_OutputBin>(doc, packet);
      break;
    case XFA_Element::Field:
      node = pdfium::MakeUnique<CXFA_Field>(doc, packet);
      break;
    case XFA_Element::Agent:
      node = pdfium::MakeUnique<CXFA_Agent>(doc, packet);
      break;
    case XFA_Element::OutputXSL:
      node = pdfium::MakeUnique<CXFA_OutputXSL>(doc, packet);
      break;
    case XFA_Element::AdjustData:
      node = pdfium::MakeUnique<CXFA_AdjustData>(doc, packet);
      break;
    case XFA_Element::AutoSave:
      node = pdfium::MakeUnique<CXFA_AutoSave>(doc, packet);
      break;
    case XFA_Element::ContentArea:
      node = pdfium::MakeUnique<CXFA_ContentArea>(doc, packet);
      break;
    case XFA_Element::WsdlAddress:
      node = pdfium::MakeUnique<CXFA_WsdlAddress>(doc, packet);
      break;
    case XFA_Element::Solid:
      node = pdfium::MakeUnique<CXFA_Solid>(doc, packet);
      break;
    case XFA_Element::DateTimeSymbols:
      node = pdfium::MakeUnique<CXFA_DateTimeSymbols>(doc, packet);
      break;
    case XFA_Element::EncryptionLevel:
      node = pdfium::MakeUnique<CXFA_EncryptionLevel>(doc, packet);
      break;
    case XFA_Element::Edge:
      node = pdfium::MakeUnique<CXFA_Edge>(doc, packet);
      break;
    case XFA_Element::Stipple:
      node = pdfium::MakeUnique<CXFA_Stipple>(doc, packet);
      break;
    case XFA_Element::Attributes:
      node = pdfium::MakeUnique<CXFA_Attributes>(doc, packet);
      break;
    case XFA_Element::VersionControl:
      node = pdfium::MakeUnique<CXFA_VersionControl>(doc, packet);
      break;
    case XFA_Element::Meridiem:
      node = pdfium::MakeUnique<CXFA_Meridiem>(doc, packet);
      break;
    case XFA_Element::ExclGroup:
      node = pdfium::MakeUnique<CXFA_ExclGroup>(doc, packet);
      break;
    case XFA_Element::ToolTip:
      node = pdfium::MakeUnique<CXFA_ToolTip>(doc, packet);
      break;
    case XFA_Element::Compress:
      node = pdfium::MakeUnique<CXFA_Compress>(doc, packet);
      break;
    case XFA_Element::Reason:
      node = pdfium::MakeUnique<CXFA_Reason>(doc, packet);
      break;
    case XFA_Element::Execute:
      node = pdfium::MakeUnique<CXFA_Execute>(doc, packet);
      break;
    case XFA_Element::ContentCopy:
      node = pdfium::MakeUnique<CXFA_ContentCopy>(doc, packet);
      break;
    case XFA_Element::DateTimeEdit:
      node = pdfium::MakeUnique<CXFA_DateTimeEdit>(doc, packet);
      break;
    case XFA_Element::Config:
      node = pdfium::MakeUnique<CXFA_Config>(doc, packet);
      break;
    case XFA_Element::Image:
      node = pdfium::MakeUnique<CXFA_Image>(doc, packet);
      break;
    case XFA_Element::SharpxHTML:
      node = pdfium::MakeUnique<CXFA_SharpxHTML>(doc, packet);
      break;
    case XFA_Element::NumberOfCopies:
      node = pdfium::MakeUnique<CXFA_NumberOfCopies>(doc, packet);
      break;
    case XFA_Element::BehaviorOverride:
      node = pdfium::MakeUnique<CXFA_BehaviorOverride>(doc, packet);
      break;
    case XFA_Element::TimeStamp:
      node = pdfium::MakeUnique<CXFA_TimeStamp>(doc, packet);
      break;
    case XFA_Element::Month:
      node = pdfium::MakeUnique<CXFA_Month>(doc, packet);
      break;
    case XFA_Element::ViewerPreferences:
      node = pdfium::MakeUnique<CXFA_ViewerPreferences>(doc, packet);
      break;
    case XFA_Element::ScriptModel:
      node = pdfium::MakeUnique<CXFA_ScriptModel>(doc, packet);
      break;
    case XFA_Element::Decimal:
      node = pdfium::MakeUnique<CXFA_Decimal>(doc, packet);
      break;
    case XFA_Element::Subform:
      node = pdfium::MakeUnique<CXFA_Subform>(doc, packet);
      break;
    case XFA_Element::Select:
      node = pdfium::MakeUnique<CXFA_Select>(doc, packet);
      break;
    case XFA_Element::Window:
      node = pdfium::MakeUnique<CXFA_Window>(doc, packet);
      break;
    case XFA_Element::LocaleSet:
      node = pdfium::MakeUnique<CXFA_LocaleSet>(doc, packet);
      break;
    case XFA_Element::Handler:
      node = pdfium::MakeUnique<CXFA_Handler>(doc, packet);
      break;
    case XFA_Element::Presence:
      node = pdfium::MakeUnique<CXFA_Presence>(doc, packet);
      break;
    case XFA_Element::Record:
      node = pdfium::MakeUnique<CXFA_Record>(doc, packet);
      break;
    case XFA_Element::Embed:
      node = pdfium::MakeUnique<CXFA_Embed>(doc, packet);
      break;
    case XFA_Element::Version:
      node = pdfium::MakeUnique<CXFA_Version>(doc, packet);
      break;
    case XFA_Element::Command:
      node = pdfium::MakeUnique<CXFA_Command>(doc, packet);
      break;
    case XFA_Element::Copies:
      node = pdfium::MakeUnique<CXFA_Copies>(doc, packet);
      break;
    case XFA_Element::Staple:
      node = pdfium::MakeUnique<CXFA_Staple>(doc, packet);
      break;
    case XFA_Element::SubmitFormat:
      node = pdfium::MakeUnique<CXFA_SubmitFormat>(doc, packet);
      break;
    case XFA_Element::Boolean:
      node = pdfium::MakeUnique<CXFA_Boolean>(doc, packet);
      break;
    case XFA_Element::Message:
      node = pdfium::MakeUnique<CXFA_Message>(doc, packet);
      break;
    case XFA_Element::Output:
      node = pdfium::MakeUnique<CXFA_Output>(doc, packet);
      break;
    case XFA_Element::PsMap:
      node = pdfium::MakeUnique<CXFA_PsMap>(doc, packet);
      break;
    case XFA_Element::ExcludeNS:
      node = pdfium::MakeUnique<CXFA_ExcludeNS>(doc, packet);
      break;
    case XFA_Element::Assist:
      node = pdfium::MakeUnique<CXFA_Assist>(doc, packet);
      break;
    case XFA_Element::Picture:
      node = pdfium::MakeUnique<CXFA_Picture>(doc, packet);
      break;
    case XFA_Element::Traversal:
      node = pdfium::MakeUnique<CXFA_Traversal>(doc, packet);
      break;
    case XFA_Element::SilentPrint:
      node = pdfium::MakeUnique<CXFA_SilentPrint>(doc, packet);
      break;
    case XFA_Element::WebClient:
      node = pdfium::MakeUnique<CXFA_WebClient>(doc, packet);
      break;
    case XFA_Element::Producer:
      node = pdfium::MakeUnique<CXFA_Producer>(doc, packet);
      break;
    case XFA_Element::Corner:
      node = pdfium::MakeUnique<CXFA_Corner>(doc, packet);
      break;
    case XFA_Element::MsgId:
      node = pdfium::MakeUnique<CXFA_MsgId>(doc, packet);
      break;
    case XFA_Element::Color:
      node = pdfium::MakeUnique<CXFA_Color>(doc, packet);
      break;
    case XFA_Element::Keep:
      node = pdfium::MakeUnique<CXFA_Keep>(doc, packet);
      break;
    case XFA_Element::Query:
      node = pdfium::MakeUnique<CXFA_Query>(doc, packet);
      break;
    case XFA_Element::Insert:
      node = pdfium::MakeUnique<CXFA_Insert>(doc, packet);
      break;
    case XFA_Element::ImageEdit:
      node = pdfium::MakeUnique<CXFA_ImageEdit>(doc, packet);
      break;
    case XFA_Element::Validate:
      node = pdfium::MakeUnique<CXFA_Validate>(doc, packet);
      break;
    case XFA_Element::DigestMethods:
      node = pdfium::MakeUnique<CXFA_DigestMethods>(doc, packet);
      break;
    case XFA_Element::NumberPatterns:
      node = pdfium::MakeUnique<CXFA_NumberPatterns>(doc, packet);
      break;
    case XFA_Element::PageSet:
      node = pdfium::MakeUnique<CXFA_PageSet>(doc, packet);
      break;
    case XFA_Element::Integer:
      node = pdfium::MakeUnique<CXFA_Integer>(doc, packet);
      break;
    case XFA_Element::SoapAddress:
      node = pdfium::MakeUnique<CXFA_SoapAddress>(doc, packet);
      break;
    case XFA_Element::Equate:
      node = pdfium::MakeUnique<CXFA_Equate>(doc, packet);
      break;
    case XFA_Element::FormFieldFilling:
      node = pdfium::MakeUnique<CXFA_FormFieldFilling>(doc, packet);
      break;
    case XFA_Element::PageRange:
      node = pdfium::MakeUnique<CXFA_PageRange>(doc, packet);
      break;
    case XFA_Element::Update:
      node = pdfium::MakeUnique<CXFA_Update>(doc, packet);
      break;
    case XFA_Element::ConnectString:
      node = pdfium::MakeUnique<CXFA_ConnectString>(doc, packet);
      break;
    case XFA_Element::Mode:
      node = pdfium::MakeUnique<CXFA_Mode>(doc, packet);
      break;
    case XFA_Element::Layout:
      node = pdfium::MakeUnique<CXFA_Layout>(doc, packet);
      break;
    case XFA_Element::Sharpxml:
      node = pdfium::MakeUnique<CXFA_Sharpxml>(doc, packet);
      break;
    case XFA_Element::XsdConnection:
      node = pdfium::MakeUnique<CXFA_XsdConnection>(doc, packet);
      break;
    case XFA_Element::Traverse:
      node = pdfium::MakeUnique<CXFA_Traverse>(doc, packet);
      break;
    case XFA_Element::Encodings:
      node = pdfium::MakeUnique<CXFA_Encodings>(doc, packet);
      break;
    case XFA_Element::Template:
      node = pdfium::MakeUnique<CXFA_Template>(doc, packet);
      break;
    case XFA_Element::Acrobat:
      node = pdfium::MakeUnique<CXFA_Acrobat>(doc, packet);
      break;
    case XFA_Element::ValidationMessaging:
      node = pdfium::MakeUnique<CXFA_ValidationMessaging>(doc, packet);
      break;
    case XFA_Element::Signing:
      node = pdfium::MakeUnique<CXFA_Signing>(doc, packet);
      break;
    case XFA_Element::Script:
      node = pdfium::MakeUnique<CXFA_Script>(doc, packet);
      break;
    case XFA_Element::AddViewerPreferences:
      node = pdfium::MakeUnique<CXFA_AddViewerPreferences>(doc, packet);
      break;
    case XFA_Element::AlwaysEmbed:
      node = pdfium::MakeUnique<CXFA_AlwaysEmbed>(doc, packet);
      break;
    case XFA_Element::PasswordEdit:
      node = pdfium::MakeUnique<CXFA_PasswordEdit>(doc, packet);
      break;
    case XFA_Element::NumericEdit:
      node = pdfium::MakeUnique<CXFA_NumericEdit>(doc, packet);
      break;
    case XFA_Element::EncryptionMethod:
      node = pdfium::MakeUnique<CXFA_EncryptionMethod>(doc, packet);
      break;
    case XFA_Element::Change:
      node = pdfium::MakeUnique<CXFA_Change>(doc, packet);
      break;
    case XFA_Element::PageArea:
      node = pdfium::MakeUnique<CXFA_PageArea>(doc, packet);
      break;
    case XFA_Element::SubmitUrl:
      node = pdfium::MakeUnique<CXFA_SubmitUrl>(doc, packet);
      break;
    case XFA_Element::Oids:
      node = pdfium::MakeUnique<CXFA_Oids>(doc, packet);
      break;
    case XFA_Element::Signature:
      node = pdfium::MakeUnique<CXFA_Signature>(doc, packet);
      break;
    case XFA_Element::ADBE_JSConsole:
      node = pdfium::MakeUnique<CXFA_aDBE_JSConsole>(doc, packet);
      break;
    case XFA_Element::Caption:
      node = pdfium::MakeUnique<CXFA_Caption>(doc, packet);
      break;
    case XFA_Element::Relevant:
      node = pdfium::MakeUnique<CXFA_Relevant>(doc, packet);
      break;
    case XFA_Element::FlipLabel:
      node = pdfium::MakeUnique<CXFA_FlipLabel>(doc, packet);
      break;
    case XFA_Element::ExData:
      node = pdfium::MakeUnique<CXFA_ExData>(doc, packet);
      break;
    case XFA_Element::DayNames:
      node = pdfium::MakeUnique<CXFA_DayNames>(doc, packet);
      break;
    case XFA_Element::SoapAction:
      node = pdfium::MakeUnique<CXFA_SoapAction>(doc, packet);
      break;
    case XFA_Element::DefaultTypeface:
      node = pdfium::MakeUnique<CXFA_DefaultTypeface>(doc, packet);
      break;
    case XFA_Element::Manifest:
      node = pdfium::MakeUnique<CXFA_Manifest>(doc, packet);
      break;
    case XFA_Element::Overflow:
      node = pdfium::MakeUnique<CXFA_Overflow>(doc, packet);
      break;
    case XFA_Element::Linear:
      node = pdfium::MakeUnique<CXFA_Linear>(doc, packet);
      break;
    case XFA_Element::CurrencySymbol:
      node = pdfium::MakeUnique<CXFA_CurrencySymbol>(doc, packet);
      break;
    case XFA_Element::Delete:
      node = pdfium::MakeUnique<CXFA_Delete>(doc, packet);
      break;
    case XFA_Element::DigestMethod:
      node = pdfium::MakeUnique<CXFA_DigestMethod>(doc, packet);
      break;
    case XFA_Element::InstanceManager:
      node = pdfium::MakeUnique<CXFA_InstanceManager>(doc, packet);
      break;
    case XFA_Element::EquateRange:
      node = pdfium::MakeUnique<CXFA_EquateRange>(doc, packet);
      break;
    case XFA_Element::Medium:
      node = pdfium::MakeUnique<CXFA_Medium>(doc, packet);
      break;
    case XFA_Element::TextEdit:
      node = pdfium::MakeUnique<CXFA_TextEdit>(doc, packet);
      break;
    case XFA_Element::TemplateCache:
      node = pdfium::MakeUnique<CXFA_TemplateCache>(doc, packet);
      break;
    case XFA_Element::CompressObjectStream:
      node = pdfium::MakeUnique<CXFA_CompressObjectStream>(doc, packet);
      break;
    case XFA_Element::DataValue:
      node = pdfium::MakeUnique<CXFA_DataValue>(doc, packet);
      break;
    case XFA_Element::AccessibleContent:
      node = pdfium::MakeUnique<CXFA_AccessibleContent>(doc, packet);
      break;
    case XFA_Element::IncludeXDPContent:
      node = pdfium::MakeUnique<CXFA_IncludeXDPContent>(doc, packet);
      break;
    case XFA_Element::XmlConnection:
      node = pdfium::MakeUnique<CXFA_XmlConnection>(doc, packet);
      break;
    case XFA_Element::ValidateApprovalSignatures:
      node = pdfium::MakeUnique<CXFA_ValidateApprovalSignatures>(doc, packet);
      break;
    case XFA_Element::SignData:
      node = pdfium::MakeUnique<CXFA_SignData>(doc, packet);
      break;
    case XFA_Element::Packets:
      node = pdfium::MakeUnique<CXFA_Packets>(doc, packet);
      break;
    case XFA_Element::DatePattern:
      node = pdfium::MakeUnique<CXFA_DatePattern>(doc, packet);
      break;
    case XFA_Element::DuplexOption:
      node = pdfium::MakeUnique<CXFA_DuplexOption>(doc, packet);
      break;
    case XFA_Element::Base:
      node = pdfium::MakeUnique<CXFA_Base>(doc, packet);
      break;
    case XFA_Element::Bind:
      node = pdfium::MakeUnique<CXFA_Bind>(doc, packet);
      break;
    case XFA_Element::Compression:
      node = pdfium::MakeUnique<CXFA_Compression>(doc, packet);
      break;
    case XFA_Element::User:
      node = pdfium::MakeUnique<CXFA_User>(doc, packet);
      break;
    case XFA_Element::Rectangle:
      node = pdfium::MakeUnique<CXFA_Rectangle>(doc, packet);
      break;
    case XFA_Element::EffectiveOutputPolicy:
      node = pdfium::MakeUnique<CXFA_EffectiveOutputPolicy>(doc, packet);
      break;
    case XFA_Element::ADBE_JSDebugger:
      node = pdfium::MakeUnique<CXFA_aDBE_JSDebugger>(doc, packet);
      break;
    case XFA_Element::Acrobat7:
      node = pdfium::MakeUnique<CXFA_Acrobat7>(doc, packet);
      break;
    case XFA_Element::Interactive:
      node = pdfium::MakeUnique<CXFA_Interactive>(doc, packet);
      break;
    case XFA_Element::Locale:
      node = pdfium::MakeUnique<CXFA_Locale>(doc, packet);
      break;
    case XFA_Element::CurrentPage:
      node = pdfium::MakeUnique<CXFA_CurrentPage>(doc, packet);
      break;
    case XFA_Element::Data:
      node = pdfium::MakeUnique<CXFA_Data>(doc, packet);
      break;
    case XFA_Element::Date:
      node = pdfium::MakeUnique<CXFA_Date>(doc, packet);
      break;
    case XFA_Element::Desc:
      node = pdfium::MakeUnique<CXFA_Desc>(doc, packet);
      break;
    case XFA_Element::Encrypt:
      node = pdfium::MakeUnique<CXFA_Encrypt>(doc, packet);
      break;
    case XFA_Element::Draw:
      node = pdfium::MakeUnique<CXFA_Draw>(doc, packet);
      break;
    case XFA_Element::Encryption:
      node = pdfium::MakeUnique<CXFA_Encryption>(doc, packet);
      break;
    case XFA_Element::MeridiemNames:
      node = pdfium::MakeUnique<CXFA_MeridiemNames>(doc, packet);
      break;
    case XFA_Element::Messaging:
      node = pdfium::MakeUnique<CXFA_Messaging>(doc, packet);
      break;
    case XFA_Element::Speak:
      node = pdfium::MakeUnique<CXFA_Speak>(doc, packet);
      break;
    case XFA_Element::DataGroup:
      node = pdfium::MakeUnique<CXFA_DataGroup>(doc, packet);
      break;
    case XFA_Element::Common:
      node = pdfium::MakeUnique<CXFA_Common>(doc, packet);
      break;
    case XFA_Element::Sharptext:
      node = pdfium::MakeUnique<CXFA_Sharptext>(doc, packet);
      break;
    case XFA_Element::PaginationOverride:
      node = pdfium::MakeUnique<CXFA_PaginationOverride>(doc, packet);
      break;
    case XFA_Element::Reasons:
      node = pdfium::MakeUnique<CXFA_Reasons>(doc, packet);
      break;
    case XFA_Element::SignatureProperties:
      node = pdfium::MakeUnique<CXFA_SignatureProperties>(doc, packet);
      break;
    case XFA_Element::Threshold:
      node = pdfium::MakeUnique<CXFA_Threshold>(doc, packet);
      break;
    case XFA_Element::AppearanceFilter:
      node = pdfium::MakeUnique<CXFA_AppearanceFilter>(doc, packet);
      break;
    case XFA_Element::Fill:
      node = pdfium::MakeUnique<CXFA_Fill>(doc, packet);
      break;
    case XFA_Element::Font:
      node = pdfium::MakeUnique<CXFA_Font>(doc, packet);
      break;
    case XFA_Element::Form:
      node = pdfium::MakeUnique<CXFA_Form>(doc, packet);
      break;
    case XFA_Element::MediumInfo:
      node = pdfium::MakeUnique<CXFA_MediumInfo>(doc, packet);
      break;
    case XFA_Element::Certificate:
      node = pdfium::MakeUnique<CXFA_Certificate>(doc, packet);
      break;
    case XFA_Element::Password:
      node = pdfium::MakeUnique<CXFA_Password>(doc, packet);
      break;
    case XFA_Element::RunScripts:
      node = pdfium::MakeUnique<CXFA_RunScripts>(doc, packet);
      break;
    case XFA_Element::Trace:
      node = pdfium::MakeUnique<CXFA_Trace>(doc, packet);
      break;
    case XFA_Element::Float:
      node = pdfium::MakeUnique<CXFA_Float>(doc, packet);
      break;
    case XFA_Element::RenderPolicy:
      node = pdfium::MakeUnique<CXFA_RenderPolicy>(doc, packet);
      break;
    case XFA_Element::Destination:
      node = pdfium::MakeUnique<CXFA_Destination>(doc, packet);
      break;
    case XFA_Element::Value:
      node = pdfium::MakeUnique<CXFA_Value>(doc, packet);
      break;
    case XFA_Element::Bookend:
      node = pdfium::MakeUnique<CXFA_Bookend>(doc, packet);
      break;
    case XFA_Element::ExObject:
      node = pdfium::MakeUnique<CXFA_ExObject>(doc, packet);
      break;
    case XFA_Element::OpenAction:
      node = pdfium::MakeUnique<CXFA_OpenAction>(doc, packet);
      break;
    case XFA_Element::NeverEmbed:
      node = pdfium::MakeUnique<CXFA_NeverEmbed>(doc, packet);
      break;
    case XFA_Element::BindItems:
      node = pdfium::MakeUnique<CXFA_BindItems>(doc, packet);
      break;
    case XFA_Element::Calculate:
      node = pdfium::MakeUnique<CXFA_Calculate>(doc, packet);
      break;
    case XFA_Element::Print:
      node = pdfium::MakeUnique<CXFA_Print>(doc, packet);
      break;
    case XFA_Element::Extras:
      node = pdfium::MakeUnique<CXFA_Extras>(doc, packet);
      break;
    case XFA_Element::Proto:
      node = pdfium::MakeUnique<CXFA_Proto>(doc, packet);
      break;
    case XFA_Element::DSigData:
      node = pdfium::MakeUnique<CXFA_DSigData>(doc, packet);
      break;
    case XFA_Element::Creator:
      node = pdfium::MakeUnique<CXFA_Creator>(doc, packet);
      break;
    case XFA_Element::Connect:
      node = pdfium::MakeUnique<CXFA_Connect>(doc, packet);
      break;
    case XFA_Element::Permissions:
      node = pdfium::MakeUnique<CXFA_Permissions>(doc, packet);
      break;
    case XFA_Element::ConnectionSet:
      node = pdfium::MakeUnique<CXFA_ConnectionSet>(doc, packet);
      break;
    case XFA_Element::Submit:
      node = pdfium::MakeUnique<CXFA_Submit>(doc, packet);
      break;
    case XFA_Element::Range:
      node = pdfium::MakeUnique<CXFA_Range>(doc, packet);
      break;
    case XFA_Element::Linearized:
      node = pdfium::MakeUnique<CXFA_Linearized>(doc, packet);
      break;
    case XFA_Element::Packet:
      node = pdfium::MakeUnique<CXFA_Packet>(doc, packet);
      break;
    case XFA_Element::RootElement:
      node = pdfium::MakeUnique<CXFA_RootElement>(doc, packet);
      break;
    case XFA_Element::PlaintextMetadata:
      node = pdfium::MakeUnique<CXFA_PlaintextMetadata>(doc, packet);
      break;
    case XFA_Element::NumberSymbols:
      node = pdfium::MakeUnique<CXFA_NumberSymbols>(doc, packet);
      break;
    case XFA_Element::PrintHighQuality:
      node = pdfium::MakeUnique<CXFA_PrintHighQuality>(doc, packet);
      break;
    case XFA_Element::Driver:
      node = pdfium::MakeUnique<CXFA_Driver>(doc, packet);
      break;
    case XFA_Element::IncrementalLoad:
      node = pdfium::MakeUnique<CXFA_IncrementalLoad>(doc, packet);
      break;
    case XFA_Element::SubjectDN:
      node = pdfium::MakeUnique<CXFA_SubjectDN>(doc, packet);
      break;
    case XFA_Element::CompressLogicalStructure:
      node = pdfium::MakeUnique<CXFA_CompressLogicalStructure>(doc, packet);
      break;
    case XFA_Element::IncrementalMerge:
      node = pdfium::MakeUnique<CXFA_IncrementalMerge>(doc, packet);
      break;
    case XFA_Element::Radial:
      node = pdfium::MakeUnique<CXFA_Radial>(doc, packet);
      break;
    case XFA_Element::Variables:
      node = pdfium::MakeUnique<CXFA_Variables>(doc, packet);
      break;
    case XFA_Element::TimePatterns:
      node = pdfium::MakeUnique<CXFA_TimePatterns>(doc, packet);
      break;
    case XFA_Element::EffectiveInputPolicy:
      node = pdfium::MakeUnique<CXFA_EffectiveInputPolicy>(doc, packet);
      break;
    case XFA_Element::NameAttr:
      node = pdfium::MakeUnique<CXFA_NameAttr>(doc, packet);
      break;
    case XFA_Element::Conformance:
      node = pdfium::MakeUnique<CXFA_Conformance>(doc, packet);
      break;
    case XFA_Element::Transform:
      node = pdfium::MakeUnique<CXFA_Transform>(doc, packet);
      break;
    case XFA_Element::LockDocument:
      node = pdfium::MakeUnique<CXFA_LockDocument>(doc, packet);
      break;
    case XFA_Element::BreakAfter:
      node = pdfium::MakeUnique<CXFA_BreakAfter>(doc, packet);
      break;
    case XFA_Element::Line:
      node = pdfium::MakeUnique<CXFA_Line>(doc, packet);
      break;
    case XFA_Element::Source:
      node = pdfium::MakeUnique<CXFA_Source>(doc, packet);
      break;
    case XFA_Element::Occur:
      node = pdfium::MakeUnique<CXFA_Occur>(doc, packet);
      break;
    case XFA_Element::PickTrayByPDFSize:
      node = pdfium::MakeUnique<CXFA_PickTrayByPDFSize>(doc, packet);
      break;
    case XFA_Element::MonthNames:
      node = pdfium::MakeUnique<CXFA_MonthNames>(doc, packet);
      break;
    case XFA_Element::Severity:
      node = pdfium::MakeUnique<CXFA_Severity>(doc, packet);
      break;
    case XFA_Element::GroupParent:
      node = pdfium::MakeUnique<CXFA_GroupParent>(doc, packet);
      break;
    case XFA_Element::DocumentAssembly:
      node = pdfium::MakeUnique<CXFA_DocumentAssembly>(doc, packet);
      break;
    case XFA_Element::NumberSymbol:
      node = pdfium::MakeUnique<CXFA_NumberSymbol>(doc, packet);
      break;
    case XFA_Element::Tagged:
      node = pdfium::MakeUnique<CXFA_Tagged>(doc, packet);
      break;
    case XFA_Element::Items:
      node = pdfium::MakeUnique<CXFA_Items>(doc, packet);
      break;
    default:
      NOTREACHED();
      return nullptr;
  }
  if (!node || !node->IsValidInPacket(packet))
    return nullptr;
  return node;
}

// static
WideString CXFA_Node::AttributeToName(XFA_Attribute attr) {
  return WideString::FromASCII(AttributeToNameASCII(attr));
}
