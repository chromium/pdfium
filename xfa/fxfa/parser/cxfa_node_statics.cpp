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
  uint32_t hash;
  XFA_Element element;
};

#undef ELEM
#define ELEM(a, b, c) a, c

const ElementNameInfo ElementNameToEnum[] = {
    {ELEM(0x23ee3, L"ps", XFA_Element::Ps)},
    {ELEM(0x25363, L"to", XFA_Element::To)},
    {ELEM(0x2587e, L"ui", XFA_Element::Ui)},
    {ELEM(0x1c648b, L"recordSet", XFA_Element::RecordSet)},
    {ELEM(0x171428f, L"subsetBelow", XFA_Element::SubsetBelow)},
    {ELEM(0x1a0776a, L"subformSet", XFA_Element::SubformSet)},
    {ELEM(0x2340d70, L"adobeExtensionLevel", XFA_Element::AdobeExtensionLevel)},
    {ELEM(0x2c1c7f1, L"typeface", XFA_Element::Typeface)},
    {ELEM(0x5518c25, L"break", XFA_Element::Break)},
    {ELEM(0x5fff523, L"fontInfo", XFA_Element::FontInfo)},
    {ELEM(0x653a227, L"numberPattern", XFA_Element::NumberPattern)},
    {ELEM(0x65b4a05, L"dynamicRender", XFA_Element::DynamicRender)},
    {ELEM(0x7e4362e, L"printScaling", XFA_Element::PrintScaling)},
    {ELEM(0x7fe6d3a, L"checkButton", XFA_Element::CheckButton)},
    {ELEM(0x80cf58f, L"datePatterns", XFA_Element::DatePatterns)},
    {ELEM(0x811929d, L"sourceSet", XFA_Element::SourceSet)},
    {ELEM(0x9f9d612, L"amd", XFA_Element::Amd)},
    {ELEM(0x9f9efb6, L"arc", XFA_Element::Arc)},
    {ELEM(0xa48835e, L"day", XFA_Element::Day)},
    {ELEM(0xa6328b8, L"era", XFA_Element::Era)},
    {ELEM(0xae6a0a0, L"jog", XFA_Element::Jog)},
    {ELEM(0xb1b3d22, L"log", XFA_Element::Log)},
    {ELEM(0xb35439e, L"map", XFA_Element::Map)},
    {ELEM(0xb355301, L"mdp", XFA_Element::Mdp)},
    {ELEM(0xb420438, L"breakBefore", XFA_Element::BreakBefore)},
    {ELEM(0xb6a091c, L"oid", XFA_Element::Oid)},
    {ELEM(0xb84389f, L"pcl", XFA_Element::Pcl)},
    {ELEM(0xb843dba, L"pdf", XFA_Element::Pdf)},
    {ELEM(0xbb8df5d, L"ref", XFA_Element::Ref)},
    {ELEM(0xc080cd0, L"uri", XFA_Element::Uri)},
    {ELEM(0xc56afbf, L"xdc", XFA_Element::Xdc)},
    {ELEM(0xc56afcc, L"xdp", XFA_Element::Xdp)},
    {ELEM(0xc56b9ff, L"xfa", XFA_Element::Xfa)},
    {ELEM(0xc56fcb7, L"xsl", XFA_Element::Xsl)},
    {ELEM(0xc8b89d6, L"zpl", XFA_Element::Zpl)},
    {ELEM(0xc9bae94, L"cache", XFA_Element::Cache)},
    {ELEM(0xcb016be, L"margin", XFA_Element::Margin)},
    {ELEM(0xe1378fe, L"keyUsage", XFA_Element::KeyUsage)},
    {ELEM(0xfe3596a, L"exclude", XFA_Element::Exclude)},
    {ELEM(0x10395ac7, L"choiceList", XFA_Element::ChoiceList)},
    {ELEM(0x1059ec18, L"level", XFA_Element::Level)},
    {ELEM(0x10874804, L"labelPrinter", XFA_Element::LabelPrinter)},
    {ELEM(0x10c40e03, L"calendarSymbols", XFA_Element::CalendarSymbols)},
    {ELEM(0x10f1ea24, L"para", XFA_Element::Para)},
    {ELEM(0x10f1ea37, L"part", XFA_Element::Part)},
    {ELEM(0x1140975b, L"pdfa", XFA_Element::Pdfa)},
    {ELEM(0x1154efe6, L"filter", XFA_Element::Filter)},
    {ELEM(0x13f41de1, L"present", XFA_Element::Present)},
    {ELEM(0x1827e6ea, L"pagination", XFA_Element::Pagination)},
    {ELEM(0x18463707, L"encoding", XFA_Element::Encoding)},
    {ELEM(0x185e41e2, L"event", XFA_Element::Event)},
    {ELEM(0x1adb142d, L"whitespace", XFA_Element::Whitespace)},
    {ELEM(0x1f3f64c3, L"defaultUi", XFA_Element::DefaultUi)},
    {ELEM(0x204e87cb, L"dataModel", XFA_Element::DataModel)},
    {ELEM(0x2057b350, L"barcode", XFA_Element::Barcode)},
    {ELEM(0x20596bad, L"timePattern", XFA_Element::TimePattern)},
    {ELEM(0x210b74d3, L"batchOutput", XFA_Element::BatchOutput)},
    {ELEM(0x212ff0e2, L"enforce", XFA_Element::Enforce)},
    {ELEM(0x21d351b4, L"currencySymbols", XFA_Element::CurrencySymbols)},
    {ELEM(0x21db83c5, L"addSilentPrint", XFA_Element::AddSilentPrint)},
    {ELEM(0x22266258, L"rename", XFA_Element::Rename)},
    {ELEM(0x226ca8f1, L"operation", XFA_Element::Operation)},
    {ELEM(0x23e27b84, L"typefaces", XFA_Element::Typefaces)},
    {ELEM(0x23f4aa75, L"subjectDNs", XFA_Element::SubjectDNs)},
    {ELEM(0x240d5e8e, L"issuers", XFA_Element::Issuers)},
    {ELEM(0x24a52f8a, L"wsdlConnection", XFA_Element::WsdlConnection)},
    {ELEM(0x254ebd07, L"debug", XFA_Element::Debug)},
    {ELEM(0x2655c66a, L"delta", XFA_Element::Delta)},
    {ELEM(0x26c0daec, L"eraNames", XFA_Element::EraNames)},
    {ELEM(0x273ab03b, L"modifyAnnots", XFA_Element::ModifyAnnots)},
    {ELEM(0x27875bb4, L"startNode", XFA_Element::StartNode)},
    {ELEM(0x285d0dbc, L"button", XFA_Element::Button)},
    {ELEM(0x28dee6e9, L"format", XFA_Element::Format)},
    {ELEM(0x2a23349e, L"border", XFA_Element::Border)},
    {ELEM(0x2ae67f19, L"area", XFA_Element::Area)},
    {ELEM(0x2c3c4c67, L"hyphenation", XFA_Element::Hyphenation)},
    {ELEM(0x2d08af85, L"text", XFA_Element::Text)},
    {ELEM(0x2d71b00f, L"time", XFA_Element::Time)},
    {ELEM(0x2f16a382, L"type", XFA_Element::Type)},
    {ELEM(0x2fe057e9, L"overprint", XFA_Element::Overprint)},
    {ELEM(0x302aee16, L"certificates", XFA_Element::Certificates)},
    {ELEM(0x30b227df, L"encryptionMethods", XFA_Element::EncryptionMethods)},
    {ELEM(0x32b900d1, L"setProperty", XFA_Element::SetProperty)},
    {ELEM(0x337d9e45, L"printerName", XFA_Element::PrinterName)},
    {ELEM(0x33edda4b, L"startPage", XFA_Element::StartPage)},
    {ELEM(0x381943e4, L"pageOffset", XFA_Element::PageOffset)},
    {ELEM(0x382106cd, L"dateTime", XFA_Element::DateTime)},
    {ELEM(0x386e7421, L"comb", XFA_Element::Comb)},
    {ELEM(0x390acd9e, L"pattern", XFA_Element::Pattern)},
    {ELEM(0x3942163e, L"ifEmpty", XFA_Element::IfEmpty)},
    {ELEM(0x39944a7b, L"suppressBanner", XFA_Element::SuppressBanner)},
    {ELEM(0x3b3c3dca, L"outputBin", XFA_Element::OutputBin)},
    {ELEM(0x3b8a4024, L"field", XFA_Element::Field)},
    {ELEM(0x3c15352f, L"agent", XFA_Element::Agent)},
    {ELEM(0x3d7e8668, L"outputXSL", XFA_Element::OutputXSL)},
    {ELEM(0x3e1c91c5, L"adjustData", XFA_Element::AdjustData)},
    {ELEM(0x3e7a9408, L"autoSave", XFA_Element::AutoSave)},
    {ELEM(0x3ecead94, L"contentArea", XFA_Element::ContentArea)},
    {ELEM(0x3fadaec0, L"wsdlAddress", XFA_Element::WsdlAddress)},
    {ELEM(0x40623b5b, L"solid", XFA_Element::Solid)},
    {ELEM(0x41f0bd76, L"dateTimeSymbols", XFA_Element::DateTimeSymbols)},
    {ELEM(0x444e7523, L"encryptionLevel", XFA_Element::EncryptionLevel)},
    {ELEM(0x4523af55, L"edge", XFA_Element::Edge)},
    {ELEM(0x45d5e3c1, L"stipple", XFA_Element::Stipple)},
    {ELEM(0x475e4e87, L"attributes", XFA_Element::Attributes)},
    {ELEM(0x487a8c87, L"versionControl", XFA_Element::VersionControl)},
    {ELEM(0x48e5248c, L"meridiem", XFA_Element::Meridiem)},
    {ELEM(0x48f36719, L"exclGroup", XFA_Element::ExclGroup)},
    {ELEM(0x4977356b, L"toolTip", XFA_Element::ToolTip)},
    {ELEM(0x499afecc, L"compress", XFA_Element::Compress)},
    {ELEM(0x4a0c4948, L"reason", XFA_Element::Reason)},
    {ELEM(0x4bdcce13, L"execute", XFA_Element::Execute)},
    {ELEM(0x4c56b216, L"contentCopy", XFA_Element::ContentCopy)},
    {ELEM(0x4cc176d3, L"dateTimeEdit", XFA_Element::DateTimeEdit)},
    {ELEM(0x4e1e39b6, L"config", XFA_Element::Config)},
    {ELEM(0x4e2d6083, L"image", XFA_Element::Image)},
    {ELEM(0x4e814150, L"#xHTML", XFA_Element::SharpxHTML)},
    {ELEM(0x4f2388c1, L"numberOfCopies", XFA_Element::NumberOfCopies)},
    {ELEM(0x4f512e30, L"behaviorOverride", XFA_Element::BehaviorOverride)},
    {ELEM(0x4fdc3454, L"timeStamp", XFA_Element::TimeStamp)},
    {ELEM(0x51d90546, L"month", XFA_Element::Month)},
    {ELEM(0x523437e4, L"viewerPreferences", XFA_Element::ViewerPreferences)},
    {ELEM(0x53abc1c6, L"scriptModel", XFA_Element::ScriptModel)},
    {ELEM(0x54034c2f, L"decimal", XFA_Element::Decimal)},
    {ELEM(0x54202c9e, L"subform", XFA_Element::Subform)},
    {ELEM(0x542c7300, L"select", XFA_Element::Select)},
    {ELEM(0x5436d198, L"window", XFA_Element::Window)},
    {ELEM(0x5473b6dc, L"localeSet", XFA_Element::LocaleSet)},
    {ELEM(0x56ae179e, L"handler", XFA_Element::Handler)},
    {ELEM(0x570ce835, L"presence", XFA_Element::Presence)},
    {ELEM(0x5779d65f, L"record", XFA_Element::Record)},
    {ELEM(0x59c8f27d, L"embed", XFA_Element::Embed)},
    {ELEM(0x5a50e9e6, L"version", XFA_Element::Version)},
    {ELEM(0x5b8383df, L"command", XFA_Element::Command)},
    {ELEM(0x5c43c6c3, L"copies", XFA_Element::Copies)},
    {ELEM(0x5e0c2c49, L"staple", XFA_Element::Staple)},
    {ELEM(0x5e5083dd, L"submitFormat", XFA_Element::SubmitFormat)},
    {ELEM(0x5e8c5d20, L"boolean", XFA_Element::Boolean)},
    {ELEM(0x60490a85, L"message", XFA_Element::Message)},
    {ELEM(0x60d4c8b1, L"output", XFA_Element::Output)},
    {ELEM(0x61810081, L"psMap", XFA_Element::PsMap)},
    {ELEM(0x62bd904b, L"excludeNS", XFA_Element::ExcludeNS)},
    {ELEM(0x669d4f77, L"assist", XFA_Element::Assist)},
    {ELEM(0x67334a1c, L"picture", XFA_Element::Picture)},
    {ELEM(0x67fe7334, L"traversal", XFA_Element::Traversal)},
    {ELEM(0x6894589c, L"silentPrint", XFA_Element::SilentPrint)},
    {ELEM(0x68a16bbd, L"webClient", XFA_Element::WebClient)},
    {ELEM(0x6a4bc084, L"producer", XFA_Element::Producer)},
    {ELEM(0x6a9e04c9, L"corner", XFA_Element::Corner)},
    {ELEM(0x6ccd7274, L"msgId", XFA_Element::MsgId)},
    {ELEM(0x6e67921f, L"color", XFA_Element::Color)},
    {ELEM(0x6ec217a5, L"keep", XFA_Element::Keep)},
    {ELEM(0x6eef1116, L"query", XFA_Element::Query)},
    {ELEM(0x7033bfd5, L"insert", XFA_Element::Insert)},
    {ELEM(0x704af389, L"imageEdit", XFA_Element::ImageEdit)},
    {ELEM(0x7233018a, L"validate", XFA_Element::Validate)},
    {ELEM(0x72ba47b4, L"digestMethods", XFA_Element::DigestMethods)},
    {ELEM(0x72f2aa7a, L"numberPatterns", XFA_Element::NumberPatterns)},
    {ELEM(0x74caed29, L"pageSet", XFA_Element::PageSet)},
    {ELEM(0x7568e6ae, L"integer", XFA_Element::Integer)},
    {ELEM(0x76182db9, L"soapAddress", XFA_Element::SoapAddress)},
    {ELEM(0x773146c5, L"equate", XFA_Element::Equate)},
    {ELEM(0x77d449dd, L"formFieldFilling", XFA_Element::FormFieldFilling)},
    {ELEM(0x7889d68a, L"pageRange", XFA_Element::PageRange)},
    {ELEM(0x7baca2e3, L"update", XFA_Element::Update)},
    {ELEM(0x7ce89001, L"connectString", XFA_Element::ConnectString)},
    {ELEM(0x7d9fd7c5, L"mode", XFA_Element::Mode)},
    {ELEM(0x7e7e845e, L"layout", XFA_Element::Layout)},
    {ELEM(0x7e845c34, L"#xml", XFA_Element::Sharpxml)},
    {ELEM(0x7fb341df, L"xsdConnection", XFA_Element::XsdConnection)},
    {ELEM(0x7ffb51cc, L"traverse", XFA_Element::Traverse)},
    {ELEM(0x80203b5a, L"encodings", XFA_Element::Encodings)},
    {ELEM(0x803550fc, L"template", XFA_Element::Template)},
    {ELEM(0x803d5bbc, L"acrobat", XFA_Element::Acrobat)},
    {ELEM(0x821d6569,
          L"validationMessaging",
          XFA_Element::ValidationMessaging)},
    {ELEM(0x830e688f, L"signing", XFA_Element::Signing)},
    {ELEM(0x83dab9f5, L"script", XFA_Element::Script)},
    {ELEM(0x8411ebcd,
          L"addViewerPreferences",
          XFA_Element::AddViewerPreferences)},
    {ELEM(0x8777642e, L"alwaysEmbed", XFA_Element::AlwaysEmbed)},
    {ELEM(0x877a6b39, L"passwordEdit", XFA_Element::PasswordEdit)},
    {ELEM(0x87e84c99, L"numericEdit", XFA_Element::NumericEdit)},
    {ELEM(0x8852cdec, L"encryptionMethod", XFA_Element::EncryptionMethod)},
    {ELEM(0x891f4606, L"change", XFA_Element::Change)},
    {ELEM(0x89939f36, L"pageArea", XFA_Element::PageArea)},
    {ELEM(0x8a9d6247, L"submitUrl", XFA_Element::SubmitUrl)},
    {ELEM(0x8ad8b90f, L"oids", XFA_Element::Oids)},
    {ELEM(0x8b036f32, L"signature", XFA_Element::Signature)},
    {ELEM(0x8b128efb, L"ADBE_JSConsole", XFA_Element::ADBE_JSConsole)},
    {ELEM(0x8bcfe96e, L"caption", XFA_Element::Caption)},
    {ELEM(0x8e1c2921, L"relevant", XFA_Element::Relevant)},
    {ELEM(0x8e3f0a4b, L"flipLabel", XFA_Element::FlipLabel)},
    {ELEM(0x900280b7, L"exData", XFA_Element::ExData)},
    {ELEM(0x91e80352, L"dayNames", XFA_Element::DayNames)},
    {ELEM(0x93113b11, L"soapAction", XFA_Element::SoapAction)},
    {ELEM(0x938b09f6, L"defaultTypeface", XFA_Element::DefaultTypeface)},
    {ELEM(0x95b37897, L"manifest", XFA_Element::Manifest)},
    {ELEM(0x97b76b54, L"overflow", XFA_Element::Overflow)},
    {ELEM(0x9a57861b, L"linear", XFA_Element::Linear)},
    {ELEM(0x9ad5a821, L"currencySymbol", XFA_Element::CurrencySymbol)},
    {ELEM(0x9c6471b3, L"delete", XFA_Element::Delete)},
    {ELEM(0x9deea61d, L"deltas", XFA_Element::Deltas)},
    {ELEM(0x9e67de21, L"digestMethod", XFA_Element::DigestMethod)},
    {ELEM(0x9f3e9510, L"instanceManager", XFA_Element::InstanceManager)},
    {ELEM(0xa0799892, L"equateRange", XFA_Element::EquateRange)},
    {ELEM(0xa084a381, L"medium", XFA_Element::Medium)},
    {ELEM(0xa1211b8b, L"textEdit", XFA_Element::TextEdit)},
    {ELEM(0xa17008f0, L"templateCache", XFA_Element::TemplateCache)},
    {ELEM(0xa4f7b88f,
          L"compressObjectStream",
          XFA_Element::CompressObjectStream)},
    {ELEM(0xa65f5d17, L"dataValue", XFA_Element::DataValue)},
    {ELEM(0xa6caaa89, L"accessibleContent", XFA_Element::AccessibleContent)},
    {ELEM(0xa94cc00b, L"includeXDPContent", XFA_Element::IncludeXDPContent)},
    {ELEM(0xa9b081a1, L"xmlConnection", XFA_Element::XmlConnection)},
    {ELEM(0xab2a3b74,
          L"validateApprovalSignatures",
          XFA_Element::ValidateApprovalSignatures)},
    {ELEM(0xab8c5a2b, L"signData", XFA_Element::SignData)},
    {ELEM(0xabaa2ceb, L"packets", XFA_Element::Packets)},
    {ELEM(0xadba359c, L"datePattern", XFA_Element::DatePattern)},
    {ELEM(0xae222b2b, L"duplexOption", XFA_Element::DuplexOption)},
    {ELEM(0xb012effb, L"base", XFA_Element::Base)},
    {ELEM(0xb0e5485d, L"bind", XFA_Element::Bind)},
    {ELEM(0xb45d61b2, L"compression", XFA_Element::Compression)},
    {ELEM(0xb563f0ff, L"user", XFA_Element::User)},
    {ELEM(0xb5848ad5, L"rectangle", XFA_Element::Rectangle)},
    {ELEM(0xb6dacb72,
          L"effectiveOutputPolicy",
          XFA_Element::EffectiveOutputPolicy)},
    {ELEM(0xb7d7654d, L"ADBE_JSDebugger", XFA_Element::ADBE_JSDebugger)},
    {ELEM(0xbab37f73, L"acrobat7", XFA_Element::Acrobat7)},
    {ELEM(0xbc70081e, L"interactive", XFA_Element::Interactive)},
    {ELEM(0xbc8fa350, L"locale", XFA_Element::Locale)},
    {ELEM(0xbcd44940, L"currentPage", XFA_Element::CurrentPage)},
    {ELEM(0xbde9abda, L"data", XFA_Element::Data)},
    {ELEM(0xbde9abde, L"date", XFA_Element::Date)},
    {ELEM(0xbe52dfbf, L"desc", XFA_Element::Desc)},
    {ELEM(0xbf4b6405, L"encrypt", XFA_Element::Encrypt)},
    {ELEM(0xbfa87cce, L"draw", XFA_Element::Draw)},
    {ELEM(0xc181ff4b, L"encryption", XFA_Element::Encryption)},
    {ELEM(0xc1970f40, L"meridiemNames", XFA_Element::MeridiemNames)},
    {ELEM(0xc5ad9f5e, L"messaging", XFA_Element::Messaging)},
    {ELEM(0xc69549f4, L"speak", XFA_Element::Speak)},
    {ELEM(0xc7743dc7, L"dataGroup", XFA_Element::DataGroup)},
    {ELEM(0xc7eb20e9, L"common", XFA_Element::Common)},
    {ELEM(0xc85d4528, L"#text", XFA_Element::Sharptext)},
    {ELEM(0xc861556a, L"paginationOverride", XFA_Element::PaginationOverride)},
    {ELEM(0xc903dabb, L"reasons", XFA_Element::Reasons)},
    {ELEM(0xc9a8127f,
          L"signatureProperties",
          XFA_Element::SignatureProperties)},
    {ELEM(0xca010c2d, L"threshold", XFA_Element::Threshold)},
    {ELEM(0xcb4c5e96, L"appearanceFilter", XFA_Element::AppearanceFilter)},
    {ELEM(0xcc92aba7, L"fill", XFA_Element::Fill)},
    {ELEM(0xcd308b77, L"font", XFA_Element::Font)},
    {ELEM(0xcd309ff4, L"form", XFA_Element::Form)},
    {ELEM(0xcebcca2d, L"mediumInfo", XFA_Element::MediumInfo)},
    {ELEM(0xcfe0d643, L"certificate", XFA_Element::Certificate)},
    {ELEM(0xd012c033, L"password", XFA_Element::Password)},
    {ELEM(0xd01604bd, L"runScripts", XFA_Element::RunScripts)},
    {ELEM(0xd1227e6f, L"trace", XFA_Element::Trace)},
    {ELEM(0xd1532876, L"float", XFA_Element::Float)},
    {ELEM(0xd17a6c30, L"renderPolicy", XFA_Element::RenderPolicy)},
    {ELEM(0xd58aa962, L"destination", XFA_Element::Destination)},
    {ELEM(0xd6e27f1d, L"value", XFA_Element::Value)},
    {ELEM(0xd7a14462, L"bookend", XFA_Element::Bookend)},
    {ELEM(0xd8c31254, L"exObject", XFA_Element::ExObject)},
    {ELEM(0xda6a8590, L"openAction", XFA_Element::OpenAction)},
    {ELEM(0xdab4fb7d, L"neverEmbed", XFA_Element::NeverEmbed)},
    {ELEM(0xdb98475f, L"bindItems", XFA_Element::BindItems)},
    {ELEM(0xdbfbe02e, L"calculate", XFA_Element::Calculate)},
    {ELEM(0xdd7676ed, L"print", XFA_Element::Print)},
    {ELEM(0xdde273d7, L"extras", XFA_Element::Extras)},
    {ELEM(0xde146b34, L"proto", XFA_Element::Proto)},
    {ELEM(0xdf059321, L"dSigData", XFA_Element::DSigData)},
    {ELEM(0xdfccf030, L"creator", XFA_Element::Creator)},
    {ELEM(0xdff78c6a, L"connect", XFA_Element::Connect)},
    {ELEM(0xe11a2cbc, L"permissions", XFA_Element::Permissions)},
    {ELEM(0xe14c801c, L"connectionSet", XFA_Element::ConnectionSet)},
    {ELEM(0xe1c83a14, L"submit", XFA_Element::Submit)},
    {ELEM(0xe29821cd, L"range", XFA_Element::Range)},
    {ELEM(0xe38d83c7, L"linearized", XFA_Element::Linearized)},
    {ELEM(0xe3aa2578, L"packet", XFA_Element::Packet)},
    {ELEM(0xe3aa860e, L"rootElement", XFA_Element::RootElement)},
    {ELEM(0xe3e553fa, L"plaintextMetadata", XFA_Element::PlaintextMetadata)},
    {ELEM(0xe3e6e4f2, L"numberSymbols", XFA_Element::NumberSymbols)},
    {ELEM(0xe3f067f6, L"printHighQuality", XFA_Element::PrintHighQuality)},
    {ELEM(0xe3fd078c, L"driver", XFA_Element::Driver)},
    {ELEM(0xe48b34f2, L"incrementalLoad", XFA_Element::IncrementalLoad)},
    {ELEM(0xe550e7c2, L"subjectDN", XFA_Element::SubjectDN)},
    {ELEM(0xe6669a78,
          L"compressLogicalStructure",
          XFA_Element::CompressLogicalStructure)},
    {ELEM(0xe7a7ea02, L"incrementalMerge", XFA_Element::IncrementalMerge)},
    {ELEM(0xe948530d, L"radial", XFA_Element::Radial)},
    {ELEM(0xea8d6999, L"variables", XFA_Element::Variables)},
    {ELEM(0xeaa142c0, L"timePatterns", XFA_Element::TimePatterns)},
    {ELEM(0xeb943a71,
          L"effectiveInputPolicy",
          XFA_Element::EffectiveInputPolicy)},
    {ELEM(0xef04a2bc, L"nameAttr", XFA_Element::NameAttr)},
    {ELEM(0xf07222ab, L"conformance", XFA_Element::Conformance)},
    {ELEM(0xf0aaaadc, L"transform", XFA_Element::Transform)},
    {ELEM(0xf1433e88, L"lockDocument", XFA_Element::LockDocument)},
    {ELEM(0xf54eb997, L"breakAfter", XFA_Element::BreakAfter)},
    {ELEM(0xf616da28, L"line", XFA_Element::Line)},
    {ELEM(0xf7055fb1, L"source", XFA_Element::Source)},
    {ELEM(0xf7eebe1c, L"occur", XFA_Element::Occur)},
    {ELEM(0xf8d10d97, L"pickTrayByPDFSize", XFA_Element::PickTrayByPDFSize)},
    {ELEM(0xf8f19e3a, L"monthNames", XFA_Element::MonthNames)},
    {ELEM(0xf984149b, L"severity", XFA_Element::Severity)},
    {ELEM(0xf9bcb037, L"groupParent", XFA_Element::GroupParent)},
    {ELEM(0xfbc42fff, L"documentAssembly", XFA_Element::DocumentAssembly)},
    {ELEM(0xfc78159f, L"numberSymbol", XFA_Element::NumberSymbol)},
    {ELEM(0xfcbd606c, L"tagged", XFA_Element::Tagged)},
    {ELEM(0xff063802, L"items", XFA_Element::Items)},
};

#undef ELEM

struct AttributeNameInfo {
  uint32_t hash;
  XFA_Attribute attribute;
};

#undef NAME
#define NAME(a, b, c) a, c

const AttributeNameInfo AttributeNameInfoToEnum[] = {
    {NAME(0x68, L"h", XFA_Attribute::H)},
    {NAME(0x77, L"w", XFA_Attribute::W)},
    {NAME(0x78, L"x", XFA_Attribute::X)},
    {NAME(0x79, L"y", XFA_Attribute::Y)},
    {NAME(0x21aed, L"id", XFA_Attribute::Id)},
    {NAME(0x25363, L"to", XFA_Attribute::To)},
    {NAME(0xcb0ac9, L"lineThrough", XFA_Attribute::LineThrough)},
    {NAME(0x2282c73, L"hAlign", XFA_Attribute::HAlign)},
    {NAME(0x2c1c7f1, L"typeface", XFA_Attribute::Typeface)},
    {NAME(0x3106c3a, L"beforeTarget", XFA_Attribute::BeforeTarget)},
    {NAME(0x31b19c1, L"name", XFA_Attribute::Name)},
    {NAME(0x3848b3f, L"next", XFA_Attribute::Next)},
    {NAME(0x43e349b, L"dataRowCount", XFA_Attribute::DataRowCount)},
    {NAME(0x5518c25, L"break", XFA_Attribute::Break)},
    {NAME(0x5ce6195, L"vScrollPolicy", XFA_Attribute::VScrollPolicy)},
    {NAME(0x8c74ae9,
          L"fontHorizontalScale",
          XFA_Attribute::FontHorizontalScale)},
    {NAME(0x8d4f1c7, L"textIndent", XFA_Attribute::TextIndent)},
    {NAME(0x97c1c65, L"context", XFA_Attribute::Context)},
    {NAME(0x9876578, L"trayOut", XFA_Attribute::TrayOut)},
    {NAME(0xa2e3514, L"cap", XFA_Attribute::Cap)},
    {NAME(0xb3543a6, L"max", XFA_Attribute::Max)},
    {NAME(0xb356ca4, L"min", XFA_Attribute::Min)},
    {NAME(0xbb8df5d, L"ref", XFA_Attribute::Ref)},
    {NAME(0xbb8f3df, L"rid", XFA_Attribute::Rid)},
    {NAME(0xc080cd3, L"url", XFA_Attribute::Url)},
    {NAME(0xc0811ed, L"use", XFA_Attribute::Use)},
    {NAME(0xcfea02e, L"leftInset", XFA_Attribute::LeftInset)},
    {NAME(0x1026c59d, L"widows", XFA_Attribute::Widows)},
    {NAME(0x1059ec18, L"level", XFA_Attribute::Level)},
    {NAME(0x1356caf8, L"bottomInset", XFA_Attribute::BottomInset)},
    {NAME(0x13a08bdb, L"overflowTarget", XFA_Attribute::OverflowTarget)},
    {NAME(0x1414d431, L"allowMacro", XFA_Attribute::AllowMacro)},
    {NAME(0x14a32d52, L"pagePosition", XFA_Attribute::PagePosition)},
    {NAME(0x1517dfa1, L"columnWidths", XFA_Attribute::ColumnWidths)},
    {NAME(0x169134a1, L"overflowLeader", XFA_Attribute::OverflowLeader)},
    {NAME(0x1b8dce3e, L"action", XFA_Attribute::Action)},
    {NAME(0x1e459b8f, L"nonRepudiation", XFA_Attribute::NonRepudiation)},
    {NAME(0x1ec8ab2c, L"rate", XFA_Attribute::Rate)},
    {NAME(0x1ef3a64a, L"allowRichText", XFA_Attribute::AllowRichText)},
    {NAME(0x2038c9b2, L"role", XFA_Attribute::Role)},
    {NAME(0x20914367, L"overflowTrailer", XFA_Attribute::OverflowTrailer)},
    {NAME(0x226ca8f1, L"operation", XFA_Attribute::Operation)},
    {NAME(0x24d85167, L"timeout", XFA_Attribute::Timeout)},
    {NAME(0x25764436, L"topInset", XFA_Attribute::TopInset)},
    {NAME(0x25839852, L"access", XFA_Attribute::Access)},
    {NAME(0x268b7ec1, L"commandType", XFA_Attribute::CommandType)},
    {NAME(0x28dee6e9, L"format", XFA_Attribute::Format)},
    {NAME(0x28e17e91, L"dataPrep", XFA_Attribute::DataPrep)},
    {NAME(0x292b88fe, L"widgetData", XFA_Attribute::WidgetData)},
    {NAME(0x29418bb7, L"abbr", XFA_Attribute::Abbr)},
    {NAME(0x2a82d99c, L"marginRight", XFA_Attribute::MarginRight)},
    {NAME(0x2b5df51e, L"dataDescription", XFA_Attribute::DataDescription)},
    {NAME(0x2bb3f470, L"encipherOnly", XFA_Attribute::EncipherOnly)},
    {NAME(0x2cd79033, L"kerningMode", XFA_Attribute::KerningMode)},
    {NAME(0x2ee7678f, L"rotate", XFA_Attribute::Rotate)},
    {NAME(0x2f105f72,
          L"wordCharacterCount",
          XFA_Attribute::WordCharacterCount)},
    {NAME(0x2f16a382, L"type", XFA_Attribute::Type)},
    {NAME(0x34ae103c, L"reserve", XFA_Attribute::Reserve)},
    {NAME(0x3650557e, L"textLocation", XFA_Attribute::TextLocation)},
    {NAME(0x39cdb0a2, L"priority", XFA_Attribute::Priority)},
    {NAME(0x3a0273a6, L"underline", XFA_Attribute::Underline)},
    {NAME(0x3b582286, L"moduleWidth", XFA_Attribute::ModuleWidth)},
    {NAME(0x3d123c26, L"hyphenate", XFA_Attribute::Hyphenate)},
    {NAME(0x3e7af94f, L"listen", XFA_Attribute::Listen)},
    {NAME(0x4156ee3f, L"delimiter", XFA_Attribute::Delimiter)},
    {NAME(0x42fed1fd, L"contentType", XFA_Attribute::ContentType)},
    {NAME(0x453eaf38, L"startNew", XFA_Attribute::StartNew)},
    {NAME(0x45a6daf8, L"eofAction", XFA_Attribute::EofAction)},
    {NAME(0x47cfa43a, L"allowNeutral", XFA_Attribute::AllowNeutral)},
    {NAME(0x47d03490, L"connection", XFA_Attribute::Connection)},
    {NAME(0x4873c601, L"baselineShift", XFA_Attribute::BaselineShift)},
    {NAME(0x4b319767, L"overlinePeriod", XFA_Attribute::OverlinePeriod)},
    {NAME(0x4b8bc840, L"fracDigits", XFA_Attribute::FracDigits)},
    {NAME(0x4ef3d02c, L"orientation", XFA_Attribute::Orientation)},
    {NAME(0x4fdc3454, L"timeStamp", XFA_Attribute::TimeStamp)},
    {NAME(0x52666f1c, L"printCheckDigit", XFA_Attribute::PrintCheckDigit)},
    {NAME(0x534729c9, L"marginLeft", XFA_Attribute::MarginLeft)},
    {NAME(0x5392ea58, L"stroke", XFA_Attribute::Stroke)},
    {NAME(0x5404d6df, L"moduleHeight", XFA_Attribute::ModuleHeight)},
    {NAME(0x54fa722c, L"transferEncoding", XFA_Attribute::TransferEncoding)},
    {NAME(0x552d9ad5, L"usage", XFA_Attribute::Usage)},
    {NAME(0x570ce835, L"presence", XFA_Attribute::Presence)},
    {NAME(0x5739d1ff, L"radixOffset", XFA_Attribute::RadixOffset)},
    {NAME(0x577682ac, L"preserve", XFA_Attribute::Preserve)},
    {NAME(0x58be2870, L"aliasNode", XFA_Attribute::AliasNode)},
    {NAME(0x5a32e493, L"multiLine", XFA_Attribute::MultiLine)},
    {NAME(0x5a50e9e6, L"version", XFA_Attribute::Version)},
    {NAME(0x5ab23b6c, L"startChar", XFA_Attribute::StartChar)},
    {NAME(0x5b707a35, L"scriptTest", XFA_Attribute::ScriptTest)},
    {NAME(0x5c054755, L"startAngle", XFA_Attribute::StartAngle)},
    {NAME(0x5ec958c0, L"cursorType", XFA_Attribute::CursorType)},
    {NAME(0x5f760b50, L"digitalSignature", XFA_Attribute::DigitalSignature)},
    {NAME(0x60a61edd, L"codeType", XFA_Attribute::CodeType)},
    {NAME(0x60d4c8b1, L"output", XFA_Attribute::Output)},
    {NAME(0x64110ab5, L"bookendTrailer", XFA_Attribute::BookendTrailer)},
    {NAME(0x65e30c67, L"imagingBBox", XFA_Attribute::ImagingBBox)},
    {NAME(0x66539c48, L"excludeInitialCap", XFA_Attribute::ExcludeInitialCap)},
    {NAME(0x66642f8f, L"force", XFA_Attribute::Force)},
    {NAME(0x69aa2292, L"crlSign", XFA_Attribute::CrlSign)},
    {NAME(0x6a3405dd, L"previous", XFA_Attribute::Previous)},
    {NAME(0x6a95c976,
          L"pushCharacterCount",
          XFA_Attribute::PushCharacterCount)},
    {NAME(0x6b6ddcfb, L"nullTest", XFA_Attribute::NullTest)},
    {NAME(0x6cfa828a, L"runAt", XFA_Attribute::RunAt)},
    {NAME(0x731e0665, L"spaceBelow", XFA_Attribute::SpaceBelow)},
    {NAME(0x74788f8b, L"sweepAngle", XFA_Attribute::SweepAngle)},
    {NAME(0x78bff531, L"numberOfCells", XFA_Attribute::NumberOfCells)},
    {NAME(0x79543055, L"letterSpacing", XFA_Attribute::LetterSpacing)},
    {NAME(0x79975f2b, L"lockType", XFA_Attribute::LockType)},
    {NAME(0x7a0cc471, L"passwordChar", XFA_Attribute::PasswordChar)},
    {NAME(0x7a7cc341, L"vAlign", XFA_Attribute::VAlign)},
    {NAME(0x7b29630a, L"sourceBelow", XFA_Attribute::SourceBelow)},
    {NAME(0x7b95e661, L"inverted", XFA_Attribute::Inverted)},
    {NAME(0x7c2fd80b, L"mark", XFA_Attribute::Mark)},
    {NAME(0x7c2ff6ae, L"maxH", XFA_Attribute::MaxH)},
    {NAME(0x7c2ff6bd, L"maxW", XFA_Attribute::MaxW)},
    {NAME(0x7c732a66, L"truncate", XFA_Attribute::Truncate)},
    {NAME(0x7d02356c, L"minH", XFA_Attribute::MinH)},
    {NAME(0x7d02357b, L"minW", XFA_Attribute::MinW)},
    {NAME(0x7d0b5fca, L"initial", XFA_Attribute::Initial)},
    {NAME(0x7d9fd7c5, L"mode", XFA_Attribute::Mode)},
    {NAME(0x7e7e845e, L"layout", XFA_Attribute::Layout)},
    {NAME(0x7f6fd3d7, L"server", XFA_Attribute::Server)},
    {NAME(0x824f21b7, L"embedPDF", XFA_Attribute::EmbedPDF)},
    {NAME(0x8340ea66, L"oddOrEven", XFA_Attribute::OddOrEven)},
    {NAME(0x836d4d7c, L"tabDefault", XFA_Attribute::TabDefault)},
    {NAME(0x8855805f, L"contains", XFA_Attribute::Contains)},
    {NAME(0x8a692521, L"rightInset", XFA_Attribute::RightInset)},
    {NAME(0x8af2e657, L"maxChars", XFA_Attribute::MaxChars)},
    {NAME(0x8b90e1f2, L"open", XFA_Attribute::Open)},
    {NAME(0x8c99377e, L"relation", XFA_Attribute::Relation)},
    {NAME(0x8d181d61, L"wideNarrowRatio", XFA_Attribute::WideNarrowRatio)},
    {NAME(0x8e1c2921, L"relevant", XFA_Attribute::Relevant)},
    {NAME(0x8e29d794, L"signatureType", XFA_Attribute::SignatureType)},
    {NAME(0x8ec6204c, L"lineThroughPeriod", XFA_Attribute::LineThroughPeriod)},
    {NAME(0x8ed182d1, L"shape", XFA_Attribute::Shape)},
    {NAME(0x8fa01790, L"tabStops", XFA_Attribute::TabStops)},
    {NAME(0x8fc36c0a, L"outputBelow", XFA_Attribute::OutputBelow)},
    {NAME(0x9041d4b0, L"short", XFA_Attribute::Short)},
    {NAME(0x907c7719, L"fontVerticalScale", XFA_Attribute::FontVerticalScale)},
    {NAME(0x94446dcc, L"thickness", XFA_Attribute::Thickness)},
    {NAME(0x957fa006, L"commitOn", XFA_Attribute::CommitOn)},
    {NAME(0x982bd892,
          L"remainCharacterCount",
          XFA_Attribute::RemainCharacterCount)},
    {NAME(0x98fd4d81, L"keyAgreement", XFA_Attribute::KeyAgreement)},
    {NAME(0x99800d7a,
          L"errorCorrectionLevel",
          XFA_Attribute::ErrorCorrectionLevel)},
    {NAME(0x9a63da3d, L"upsMode", XFA_Attribute::UpsMode)},
    {NAME(0x9cc17d75, L"mergeMode", XFA_Attribute::MergeMode)},
    {NAME(0x9d833d75, L"circular", XFA_Attribute::Circular)},
    {NAME(0x9d8ee204, L"psName", XFA_Attribute::PsName)},
    {NAME(0x9dcc3ab3, L"trailer", XFA_Attribute::Trailer)},
    {NAME(0xa0933954, L"unicodeRange", XFA_Attribute::UnicodeRange)},
    {NAME(0xa1b0d2f5, L"executeType", XFA_Attribute::ExecuteType)},
    {NAME(0xa25a883d, L"duplexImposition", XFA_Attribute::DuplexImposition)},
    {NAME(0xa42ca1b7, L"trayIn", XFA_Attribute::TrayIn)},
    {NAME(0xa433f001, L"bindingNode", XFA_Attribute::BindingNode)},
    {NAME(0xa5340ff5, L"bofAction", XFA_Attribute::BofAction)},
    {NAME(0xa5b410cf, L"save", XFA_Attribute::Save)},
    {NAME(0xa6118c89, L"targetType", XFA_Attribute::TargetType)},
    {NAME(0xa66404cb, L"keyEncipherment", XFA_Attribute::KeyEncipherment)},
    {NAME(0xa6710262,
          L"credentialServerPolicy",
          XFA_Attribute::CredentialServerPolicy)},
    {NAME(0xa686975b, L"size", XFA_Attribute::Size)},
    {NAME(0xa85e74f3, L"initialNumber", XFA_Attribute::InitialNumber)},
    {NAME(0xabef37e3, L"slope", XFA_Attribute::Slope)},
    {NAME(0xabfa6c4f, L"cSpace", XFA_Attribute::CSpace)},
    {NAME(0xac06e2b0, L"colSpan", XFA_Attribute::ColSpan)},
    {NAME(0xadc4c77b, L"binding", XFA_Attribute::Binding)},
    {NAME(0xaf754613, L"checksum", XFA_Attribute::Checksum)},
    {NAME(0xb045fbc5, L"charEncoding", XFA_Attribute::CharEncoding)},
    {NAME(0xb0e5485d, L"bind", XFA_Attribute::Bind)},
    {NAME(0xb12128b7, L"textEntry", XFA_Attribute::TextEntry)},
    {NAME(0xb373a862, L"archive", XFA_Attribute::Archive)},
    {NAME(0xb598a1f7, L"uuid", XFA_Attribute::Uuid)},
    {NAME(0xb5e49bf2, L"posture", XFA_Attribute::Posture)},
    {NAME(0xb6b44172, L"after", XFA_Attribute::After)},
    {NAME(0xb716467b, L"orphans", XFA_Attribute::Orphans)},
    {NAME(0xbc0c4695, L"qualifiedName", XFA_Attribute::QualifiedName)},
    {NAME(0xbc254332, L"usehref", XFA_Attribute::Usehref)},
    {NAME(0xbc8fa350, L"locale", XFA_Attribute::Locale)},
    {NAME(0xbd6e1d88, L"weight", XFA_Attribute::Weight)},
    {NAME(0xbd96a0e9, L"underlinePeriod", XFA_Attribute::UnderlinePeriod)},
    {NAME(0xbde9abda, L"data", XFA_Attribute::Data)},
    {NAME(0xbe52dfbf, L"desc", XFA_Attribute::Desc)},
    {NAME(0xbe9ba472, L"numbered", XFA_Attribute::Numbered)},
    {NAME(0xc035c6b1, L"dataColumnCount", XFA_Attribute::DataColumnCount)},
    {NAME(0xc0ec9fa4, L"overline", XFA_Attribute::Overline)},
    {NAME(0xc2ba0923, L"urlPolicy", XFA_Attribute::UrlPolicy)},
    {NAME(0xc2bd40fd, L"anchorType", XFA_Attribute::AnchorType)},
    {NAME(0xc39a88bd, L"labelRef", XFA_Attribute::LabelRef)},
    {NAME(0xc3c1442f, L"bookendLeader", XFA_Attribute::BookendLeader)},
    {NAME(0xc4547a08, L"maxLength", XFA_Attribute::MaxLength)},
    {NAME(0xc4fed09b, L"accessKey", XFA_Attribute::AccessKey)},
    {NAME(0xc5762157, L"cursorLocation", XFA_Attribute::CursorLocation)},
    {NAME(0xc860f30a, L"delayedOpen", XFA_Attribute::DelayedOpen)},
    {NAME(0xc8da4da7, L"target", XFA_Attribute::Target)},
    {NAME(0xca5dc27c, L"dataEncipherment", XFA_Attribute::DataEncipherment)},
    {NAME(0xcb150479, L"afterTarget", XFA_Attribute::AfterTarget)},
    {NAME(0xcbcaf66d, L"leader", XFA_Attribute::Leader)},
    {NAME(0xcca7897e, L"picker", XFA_Attribute::Picker)},
    {NAME(0xcd7f7b54, L"from", XFA_Attribute::From)},
    {NAME(0xcea5e62c, L"baseProfile", XFA_Attribute::BaseProfile)},
    {NAME(0xd171b240, L"aspect", XFA_Attribute::Aspect)},
    {NAME(0xd3c84d25, L"rowColumnRatio", XFA_Attribute::RowColumnRatio)},
    {NAME(0xd4b01921, L"lineHeight", XFA_Attribute::LineHeight)},
    {NAME(0xd4cc53f8, L"highlight", XFA_Attribute::Highlight)},
    {NAME(0xd50f903a, L"valueRef", XFA_Attribute::ValueRef)},
    {NAME(0xd52482e0, L"maxEntries", XFA_Attribute::MaxEntries)},
    {NAME(0xd57c513c, L"dataLength", XFA_Attribute::DataLength)},
    {NAME(0xd6128d8d, L"activity", XFA_Attribute::Activity)},
    {NAME(0xd6a39990, L"input", XFA_Attribute::Input)},
    {NAME(0xd6e27f1d, L"value", XFA_Attribute::Value)},
    {NAME(0xd70798c2, L"blankOrNotBlank", XFA_Attribute::BlankOrNotBlank)},
    {NAME(0xd861f8af, L"addRevocationInfo", XFA_Attribute::AddRevocationInfo)},
    {NAME(0xd8f982bf, L"genericFamily", XFA_Attribute::GenericFamily)},
    {NAME(0xd996fa9b, L"hand", XFA_Attribute::Hand)},
    {NAME(0xdb55fec5, L"href", XFA_Attribute::Href)},
    {NAME(0xdc75676c, L"textEncoding", XFA_Attribute::TextEncoding)},
    {NAME(0xde7f92ba, L"leadDigits", XFA_Attribute::LeadDigits)},
    {NAME(0xe11a2cbc, L"permissions", XFA_Attribute::Permissions)},
    {NAME(0xe18b5659, L"spaceAbove", XFA_Attribute::SpaceAbove)},
    {NAME(0xe1a26b56, L"codeBase", XFA_Attribute::CodeBase)},
    {NAME(0xe349d044, L"stock", XFA_Attribute::Stock)},
    {NAME(0xe372ae97, L"isNull", XFA_Attribute::IsNull)},
    {NAME(0xe4c3a5e5, L"restoreState", XFA_Attribute::RestoreState)},
    {NAME(0xe5c96d6a, L"excludeAllCaps", XFA_Attribute::ExcludeAllCaps)},
    {NAME(0xe64b1129, L"formatTest", XFA_Attribute::FormatTest)},
    {NAME(0xe6f99487, L"hScrollPolicy", XFA_Attribute::HScrollPolicy)},
    {NAME(0xe8dddf50, L"join", XFA_Attribute::Join)},
    {NAME(0xe8f118a8, L"keyCertSign", XFA_Attribute::KeyCertSign)},
    {NAME(0xe948b9a8, L"radius", XFA_Attribute::Radius)},
    {NAME(0xe996b2fe, L"sourceAbove", XFA_Attribute::SourceAbove)},
    {NAME(0xea7090a0, L"override", XFA_Attribute::Override)},
    {NAME(0xeb091003, L"classId", XFA_Attribute::ClassId)},
    {NAME(0xeb511b54, L"disable", XFA_Attribute::Disable)},
    {NAME(0xeda9017a, L"scope", XFA_Attribute::Scope)},
    {NAME(0xf197844d, L"match", XFA_Attribute::Match)},
    {NAME(0xf2009339, L"placement", XFA_Attribute::Placement)},
    {NAME(0xf4ffce73, L"before", XFA_Attribute::Before)},
    {NAME(0xf531b059, L"writingScript", XFA_Attribute::WritingScript)},
    {NAME(0xf575ca75, L"endChar", XFA_Attribute::EndChar)},
    {NAME(0xf6b47749, L"lock", XFA_Attribute::Lock)},
    {NAME(0xf6b4afb0, L"long", XFA_Attribute::Long)},
    {NAME(0xf6b59543, L"intact", XFA_Attribute::Intact)},
    {NAME(0xf889e747, L"xdpContent", XFA_Attribute::XdpContent)},
    {NAME(0xfea53ec6, L"decipherOnly", XFA_Attribute::DecipherOnly)},
};

#undef NAME

const char* AttributeToNameASCII(XFA_Attribute attr) {
  switch (attr) {
    case XFA_Attribute::H:
      return "h";
    case XFA_Attribute::W:
      return "w";
    case XFA_Attribute::X:
      return "x";
    case XFA_Attribute::Y:
      return "y";
    case XFA_Attribute::Id:
      return "id";
    case XFA_Attribute::To:
      return "to";
    case XFA_Attribute::LineThrough:
      return "lineThrough";
    case XFA_Attribute::HAlign:
      return "hAlign";
    case XFA_Attribute::Typeface:
      return "typeface";
    case XFA_Attribute::BeforeTarget:
      return "beforeTarget";
    case XFA_Attribute::Name:
      return "name";
    case XFA_Attribute::Next:
      return "next";
    case XFA_Attribute::DataRowCount:
      return "dataRowCount";
    case XFA_Attribute::Break:
      return "break";
    case XFA_Attribute::VScrollPolicy:
      return "vScrollPolicy";
    case XFA_Attribute::FontHorizontalScale:
      return "fontHorizontalScale";
    case XFA_Attribute::TextIndent:
      return "textIndent";
    case XFA_Attribute::Context:
      return "context";
    case XFA_Attribute::TrayOut:
      return "trayOut";
    case XFA_Attribute::Cap:
      return "cap";
    case XFA_Attribute::Max:
      return "max";
    case XFA_Attribute::Min:
      return "min";
    case XFA_Attribute::Ref:
      return "ref";
    case XFA_Attribute::Rid:
      return "rid";
    case XFA_Attribute::Url:
      return "url";
    case XFA_Attribute::Use:
      return "use";
    case XFA_Attribute::LeftInset:
      return "leftInset";
    case XFA_Attribute::Widows:
      return "widows";
    case XFA_Attribute::Level:
      return "level";
    case XFA_Attribute::BottomInset:
      return "bottomInset";
    case XFA_Attribute::OverflowTarget:
      return "overflowTarget";
    case XFA_Attribute::AllowMacro:
      return "allowMacro";
    case XFA_Attribute::PagePosition:
      return "pagePosition";
    case XFA_Attribute::ColumnWidths:
      return "columnWidths";
    case XFA_Attribute::OverflowLeader:
      return "overflowLeader";
    case XFA_Attribute::Action:
      return "action";
    case XFA_Attribute::NonRepudiation:
      return "nonRepudiation";
    case XFA_Attribute::Rate:
      return "rate";
    case XFA_Attribute::AllowRichText:
      return "allowRichText";
    case XFA_Attribute::Role:
      return "role";
    case XFA_Attribute::OverflowTrailer:
      return "overflowTrailer";
    case XFA_Attribute::Operation:
      return "operation";
    case XFA_Attribute::Timeout:
      return "timeout";
    case XFA_Attribute::TopInset:
      return "topInset";
    case XFA_Attribute::Access:
      return "access";
    case XFA_Attribute::CommandType:
      return "commandType";
    case XFA_Attribute::Format:
      return "format";
    case XFA_Attribute::DataPrep:
      return "dataPrep";
    case XFA_Attribute::WidgetData:
      return "widgetData";
    case XFA_Attribute::Abbr:
      return "abbr";
    case XFA_Attribute::MarginRight:
      return "marginRight";
    case XFA_Attribute::DataDescription:
      return "dataDescription";
    case XFA_Attribute::EncipherOnly:
      return "encipherOnly";
    case XFA_Attribute::KerningMode:
      return "kerningMode";
    case XFA_Attribute::Rotate:
      return "rotate";
    case XFA_Attribute::WordCharacterCount:
      return "wordCharacterCount";
    case XFA_Attribute::Type:
      return "type";
    case XFA_Attribute::Reserve:
      return "reserve";
    case XFA_Attribute::TextLocation:
      return "textLocation";
    case XFA_Attribute::Priority:
      return "priority";
    case XFA_Attribute::Underline:
      return "underline";
    case XFA_Attribute::ModuleWidth:
      return "moduleWidth";
    case XFA_Attribute::Hyphenate:
      return "hyphenate";
    case XFA_Attribute::Listen:
      return "listen";
    case XFA_Attribute::Delimiter:
      return "delimiter";
    case XFA_Attribute::ContentType:
      return "contentType";
    case XFA_Attribute::StartNew:
      return "startNew";
    case XFA_Attribute::EofAction:
      return "eofAction";
    case XFA_Attribute::AllowNeutral:
      return "allowNeutral";
    case XFA_Attribute::Connection:
      return "connection";
    case XFA_Attribute::BaselineShift:
      return "baselineShift";
    case XFA_Attribute::OverlinePeriod:
      return "overlinePeriod";
    case XFA_Attribute::FracDigits:
      return "fracDigits";
    case XFA_Attribute::Orientation:
      return "orientation";
    case XFA_Attribute::TimeStamp:
      return "timeStamp";
    case XFA_Attribute::PrintCheckDigit:
      return "printCheckDigit";
    case XFA_Attribute::MarginLeft:
      return "marginLeft";
    case XFA_Attribute::Stroke:
      return "stroke";
    case XFA_Attribute::ModuleHeight:
      return "moduleHeight";
    case XFA_Attribute::TransferEncoding:
      return "transferEncoding";
    case XFA_Attribute::Usage:
      return "usage";
    case XFA_Attribute::Presence:
      return "presence";
    case XFA_Attribute::RadixOffset:
      return "radixOffset";
    case XFA_Attribute::Preserve:
      return "preserve";
    case XFA_Attribute::AliasNode:
      return "aliasNode";
    case XFA_Attribute::MultiLine:
      return "multiLine";
    case XFA_Attribute::Version:
      return "version";
    case XFA_Attribute::StartChar:
      return "startChar";
    case XFA_Attribute::ScriptTest:
      return "scriptTest";
    case XFA_Attribute::StartAngle:
      return "startAngle";
    case XFA_Attribute::CursorType:
      return "cursorType";
    case XFA_Attribute::DigitalSignature:
      return "digitalSignature";
    case XFA_Attribute::CodeType:
      return "codeType";
    case XFA_Attribute::Output:
      return "output";
    case XFA_Attribute::BookendTrailer:
      return "bookendTrailer";
    case XFA_Attribute::ImagingBBox:
      return "imagingBBox";
    case XFA_Attribute::ExcludeInitialCap:
      return "excludeInitialCap";
    case XFA_Attribute::Force:
      return "force";
    case XFA_Attribute::CrlSign:
      return "crlSign";
    case XFA_Attribute::Previous:
      return "previous";
    case XFA_Attribute::PushCharacterCount:
      return "pushCharacterCount";
    case XFA_Attribute::NullTest:
      return "nullTest";
    case XFA_Attribute::RunAt:
      return "runAt";
    case XFA_Attribute::SpaceBelow:
      return "spaceBelow";
    case XFA_Attribute::SweepAngle:
      return "sweepAngle";
    case XFA_Attribute::NumberOfCells:
      return "numberOfCells";
    case XFA_Attribute::LetterSpacing:
      return "letterSpacing";
    case XFA_Attribute::LockType:
      return "lockType";
    case XFA_Attribute::PasswordChar:
      return "passwordChar";
    case XFA_Attribute::VAlign:
      return "vAlign";
    case XFA_Attribute::SourceBelow:
      return "sourceBelow";
    case XFA_Attribute::Inverted:
      return "inverted";
    case XFA_Attribute::Mark:
      return "mark";
    case XFA_Attribute::MaxH:
      return "maxH";
    case XFA_Attribute::MaxW:
      return "maxW";
    case XFA_Attribute::Truncate:
      return "truncate";
    case XFA_Attribute::MinH:
      return "minH";
    case XFA_Attribute::MinW:
      return "minW";
    case XFA_Attribute::Initial:
      return "initial";
    case XFA_Attribute::Mode:
      return "mode";
    case XFA_Attribute::Layout:
      return "layout";
    case XFA_Attribute::Server:
      return "server";
    case XFA_Attribute::EmbedPDF:
      return "embedPDF";
    case XFA_Attribute::OddOrEven:
      return "oddOrEven";
    case XFA_Attribute::TabDefault:
      return "tabDefault";
    case XFA_Attribute::Contains:
      return "contains";
    case XFA_Attribute::RightInset:
      return "rightInset";
    case XFA_Attribute::MaxChars:
      return "maxChars";
    case XFA_Attribute::Open:
      return "open";
    case XFA_Attribute::Relation:
      return "relation";
    case XFA_Attribute::WideNarrowRatio:
      return "wideNarrowRatio";
    case XFA_Attribute::Relevant:
      return "relevant";
    case XFA_Attribute::SignatureType:
      return "signatureType";
    case XFA_Attribute::LineThroughPeriod:
      return "lineThroughPeriod";
    case XFA_Attribute::Shape:
      return "shape";
    case XFA_Attribute::TabStops:
      return "tabStops";
    case XFA_Attribute::OutputBelow:
      return "outputBelow";
    case XFA_Attribute::Short:
      return "short";
    case XFA_Attribute::FontVerticalScale:
      return "fontVerticalScale";
    case XFA_Attribute::Thickness:
      return "thickness";
    case XFA_Attribute::CommitOn:
      return "commitOn";
    case XFA_Attribute::RemainCharacterCount:
      return "remainCharacterCount";
    case XFA_Attribute::KeyAgreement:
      return "keyAgreement";
    case XFA_Attribute::ErrorCorrectionLevel:
      return "errorCorrectionLevel";
    case XFA_Attribute::UpsMode:
      return "upsMode";
    case XFA_Attribute::MergeMode:
      return "mergeMode";
    case XFA_Attribute::Circular:
      return "circular";
    case XFA_Attribute::PsName:
      return "psName";
    case XFA_Attribute::Trailer:
      return "trailer";
    case XFA_Attribute::UnicodeRange:
      return "unicodeRange";
    case XFA_Attribute::ExecuteType:
      return "executeType";
    case XFA_Attribute::DuplexImposition:
      return "duplexImposition";
    case XFA_Attribute::TrayIn:
      return "trayIn";
    case XFA_Attribute::BindingNode:
      return "bindingNode";
    case XFA_Attribute::BofAction:
      return "bofAction";
    case XFA_Attribute::Save:
      return "save";
    case XFA_Attribute::TargetType:
      return "targetType";
    case XFA_Attribute::KeyEncipherment:
      return "keyEncipherment";
    case XFA_Attribute::CredentialServerPolicy:
      return "credentialServerPolicy";
    case XFA_Attribute::Size:
      return "size";
    case XFA_Attribute::InitialNumber:
      return "initialNumber";
    case XFA_Attribute::Slope:
      return "slope";
    case XFA_Attribute::CSpace:
      return "cSpace";
    case XFA_Attribute::ColSpan:
      return "colSpan";
    case XFA_Attribute::Binding:
      return "binding";
    case XFA_Attribute::Checksum:
      return "checksum";
    case XFA_Attribute::CharEncoding:
      return "charEncoding";
    case XFA_Attribute::Bind:
      return "bind";
    case XFA_Attribute::TextEntry:
      return "textEntry";
    case XFA_Attribute::Archive:
      return "archive";
    case XFA_Attribute::Uuid:
      return "uuid";
    case XFA_Attribute::Posture:
      return "posture";
    case XFA_Attribute::After:
      return "after";
    case XFA_Attribute::Orphans:
      return "orphans";
    case XFA_Attribute::QualifiedName:
      return "qualifiedName";
    case XFA_Attribute::Usehref:
      return "usehref";
    case XFA_Attribute::Locale:
      return "locale";
    case XFA_Attribute::Weight:
      return "weight";
    case XFA_Attribute::UnderlinePeriod:
      return "underlinePeriod";
    case XFA_Attribute::Data:
      return "data";
    case XFA_Attribute::Desc:
      return "desc";
    case XFA_Attribute::Numbered:
      return "numbered";
    case XFA_Attribute::DataColumnCount:
      return "dataColumnCount";
    case XFA_Attribute::Overline:
      return "overline";
    case XFA_Attribute::UrlPolicy:
      return "urlPolicy";
    case XFA_Attribute::AnchorType:
      return "anchorType";
    case XFA_Attribute::LabelRef:
      return "labelRef";
    case XFA_Attribute::BookendLeader:
      return "bookendLeader";
    case XFA_Attribute::MaxLength:
      return "maxLength";
    case XFA_Attribute::AccessKey:
      return "accessKey";
    case XFA_Attribute::CursorLocation:
      return "cursorLocation";
    case XFA_Attribute::DelayedOpen:
      return "delayedOpen";
    case XFA_Attribute::Target:
      return "target";
    case XFA_Attribute::DataEncipherment:
      return "dataEncipherment";
    case XFA_Attribute::AfterTarget:
      return "afterTarget";
    case XFA_Attribute::Leader:
      return "leader";
    case XFA_Attribute::Picker:
      return "picker";
    case XFA_Attribute::From:
      return "from";
    case XFA_Attribute::BaseProfile:
      return "baseProfile";
    case XFA_Attribute::Aspect:
      return "aspect";
    case XFA_Attribute::RowColumnRatio:
      return "rowColumnRatio";
    case XFA_Attribute::LineHeight:
      return "lineHeight";
    case XFA_Attribute::Highlight:
      return "highlight";
    case XFA_Attribute::ValueRef:
      return "valueRef";
    case XFA_Attribute::MaxEntries:
      return "maxEntries";
    case XFA_Attribute::DataLength:
      return "dataLength";
    case XFA_Attribute::Activity:
      return "activity";
    case XFA_Attribute::Input:
      return "input";
    case XFA_Attribute::Value:
      return "value";
    case XFA_Attribute::BlankOrNotBlank:
      return "blankOrNotBlank";
    case XFA_Attribute::AddRevocationInfo:
      return "addRevocationInfo";
    case XFA_Attribute::GenericFamily:
      return "genericFamily";
    case XFA_Attribute::Hand:
      return "hand";
    case XFA_Attribute::Href:
      return "href";
    case XFA_Attribute::TextEncoding:
      return "textEncoding";
    case XFA_Attribute::LeadDigits:
      return "leadDigits";
    case XFA_Attribute::Permissions:
      return "permissions";
    case XFA_Attribute::SpaceAbove:
      return "spaceAbove";
    case XFA_Attribute::CodeBase:
      return "codeBase";
    case XFA_Attribute::Stock:
      return "stock";
    case XFA_Attribute::IsNull:
      return "isNull";
    case XFA_Attribute::RestoreState:
      return "restoreState";
    case XFA_Attribute::ExcludeAllCaps:
      return "excludeAllCaps";
    case XFA_Attribute::FormatTest:
      return "formatTest";
    case XFA_Attribute::HScrollPolicy:
      return "hScrollPolicy";
    case XFA_Attribute::Join:
      return "join";
    case XFA_Attribute::KeyCertSign:
      return "keyCertSign";
    case XFA_Attribute::Radius:
      return "radius";
    case XFA_Attribute::SourceAbove:
      return "sourceAbove";
    case XFA_Attribute::Override:
      return "override";
    case XFA_Attribute::ClassId:
      return "classId";
    case XFA_Attribute::Disable:
      return "disable";
    case XFA_Attribute::Scope:
      return "scope";
    case XFA_Attribute::Match:
      return "match";
    case XFA_Attribute::Placement:
      return "placement";
    case XFA_Attribute::Before:
      return "before";
    case XFA_Attribute::WritingScript:
      return "writingScript";
    case XFA_Attribute::EndChar:
      return "endChar";
    case XFA_Attribute::Lock:
      return "lock";
    case XFA_Attribute::Long:
      return "long";
    case XFA_Attribute::Intact:
      return "intact";
    case XFA_Attribute::XdpContent:
      return "xdpContent";
    case XFA_Attribute::DecipherOnly:
      return "decipherOnly";
    default:
      NOTREACHED();
      break;
  }
  return "";
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
  return WideString::FromASCII(ByteStringView(AttributeToNameASCII(attr)));
}

#ifndef NDEBUG
// static
WideString CXFA_Node::ElementToName(XFA_Element attr) {
  switch (attr) {
    case XFA_Element::Ps:
      return L"ps";
    case XFA_Element::To:
      return L"to";
    case XFA_Element::Ui:
      return L"ui";
    case XFA_Element::RecordSet:
      return L"recordSet";
    case XFA_Element::SubsetBelow:
      return L"subsetBelow";
    case XFA_Element::SubformSet:
      return L"subformSet";
    case XFA_Element::AdobeExtensionLevel:
      return L"adobeExtensionLevel";
    case XFA_Element::Typeface:
      return L"typeface";
    case XFA_Element::Break:
      return L"break";
    case XFA_Element::FontInfo:
      return L"fontInfo";
    case XFA_Element::NumberPattern:
      return L"numberPattern";
    case XFA_Element::DynamicRender:
      return L"dynamicRender";
    case XFA_Element::PrintScaling:
      return L"printScaling";
    case XFA_Element::CheckButton:
      return L"checkButton";
    case XFA_Element::DatePatterns:
      return L"datePatterns";
    case XFA_Element::SourceSet:
      return L"sourceSet";
    case XFA_Element::Amd:
      return L"amd";
    case XFA_Element::Arc:
      return L"arc";
    case XFA_Element::Day:
      return L"day";
    case XFA_Element::Era:
      return L"era";
    case XFA_Element::Jog:
      return L"jog";
    case XFA_Element::Log:
      return L"log";
    case XFA_Element::Map:
      return L"map";
    case XFA_Element::Mdp:
      return L"mdp";
    case XFA_Element::BreakBefore:
      return L"breakBefore";
    case XFA_Element::Oid:
      return L"oid";
    case XFA_Element::Pcl:
      return L"pcl";
    case XFA_Element::Pdf:
      return L"pdf";
    case XFA_Element::Ref:
      return L"ref";
    case XFA_Element::Uri:
      return L"uri";
    case XFA_Element::Xdc:
      return L"xdc";
    case XFA_Element::Xdp:
      return L"xdp";
    case XFA_Element::Xfa:
      return L"xfa";
    case XFA_Element::Xsl:
      return L"xsl";
    case XFA_Element::Zpl:
      return L"zpl";
    case XFA_Element::Cache:
      return L"cache";
    case XFA_Element::Margin:
      return L"margin";
    case XFA_Element::KeyUsage:
      return L"keyUsage";
    case XFA_Element::Exclude:
      return L"exclude";
    case XFA_Element::ChoiceList:
      return L"choiceList";
    case XFA_Element::Level:
      return L"level";
    case XFA_Element::LabelPrinter:
      return L"labelPrinter";
    case XFA_Element::CalendarSymbols:
      return L"calendarSymbols";
    case XFA_Element::Para:
      return L"para";
    case XFA_Element::Part:
      return L"part";
    case XFA_Element::Pdfa:
      return L"pdfa";
    case XFA_Element::Filter:
      return L"filter";
    case XFA_Element::Present:
      return L"present";
    case XFA_Element::Pagination:
      return L"pagination";
    case XFA_Element::Encoding:
      return L"encoding";
    case XFA_Element::Event:
      return L"event";
    case XFA_Element::Whitespace:
      return L"whitespace";
    case XFA_Element::DefaultUi:
      return L"defaultUi";
    case XFA_Element::DataModel:
      return L"dataModel";
    case XFA_Element::Barcode:
      return L"barcode";
    case XFA_Element::TimePattern:
      return L"timePattern";
    case XFA_Element::BatchOutput:
      return L"batchOutput";
    case XFA_Element::Enforce:
      return L"enforce";
    case XFA_Element::CurrencySymbols:
      return L"currencySymbols";
    case XFA_Element::AddSilentPrint:
      return L"addSilentPrint";
    case XFA_Element::Rename:
      return L"rename";
    case XFA_Element::Operation:
      return L"operation";
    case XFA_Element::Typefaces:
      return L"typefaces";
    case XFA_Element::SubjectDNs:
      return L"subjectDNs";
    case XFA_Element::Issuers:
      return L"issuers";
    case XFA_Element::WsdlConnection:
      return L"wsdlConnection";
    case XFA_Element::Debug:
      return L"debug";
    case XFA_Element::Delta:
      return L"delta";
    case XFA_Element::EraNames:
      return L"eraNames";
    case XFA_Element::ModifyAnnots:
      return L"modifyAnnots";
    case XFA_Element::StartNode:
      return L"startNode";
    case XFA_Element::Button:
      return L"button";
    case XFA_Element::Format:
      return L"format";
    case XFA_Element::Border:
      return L"border";
    case XFA_Element::Area:
      return L"area";
    case XFA_Element::Hyphenation:
      return L"hyphenation";
    case XFA_Element::Text:
      return L"text";
    case XFA_Element::Time:
      return L"time";
    case XFA_Element::Type:
      return L"type";
    case XFA_Element::Overprint:
      return L"overprint";
    case XFA_Element::Certificates:
      return L"certificates";
    case XFA_Element::EncryptionMethods:
      return L"encryptionMethods";
    case XFA_Element::SetProperty:
      return L"setProperty";
    case XFA_Element::PrinterName:
      return L"printerName";
    case XFA_Element::StartPage:
      return L"startPage";
    case XFA_Element::PageOffset:
      return L"pageOffset";
    case XFA_Element::DateTime:
      return L"dateTime";
    case XFA_Element::Comb:
      return L"comb";
    case XFA_Element::Pattern:
      return L"pattern";
    case XFA_Element::IfEmpty:
      return L"ifEmpty";
    case XFA_Element::SuppressBanner:
      return L"suppressBanner";
    case XFA_Element::OutputBin:
      return L"outputBin";
    case XFA_Element::Field:
      return L"field";
    case XFA_Element::Agent:
      return L"agent";
    case XFA_Element::OutputXSL:
      return L"outputXSL";
    case XFA_Element::AdjustData:
      return L"adjustData";
    case XFA_Element::AutoSave:
      return L"autoSave";
    case XFA_Element::ContentArea:
      return L"contentArea";
    case XFA_Element::WsdlAddress:
      return L"wsdlAddress";
    case XFA_Element::Solid:
      return L"solid";
    case XFA_Element::DateTimeSymbols:
      return L"dateTimeSymbols";
    case XFA_Element::EncryptionLevel:
      return L"encryptionLevel";
    case XFA_Element::Edge:
      return L"edge";
    case XFA_Element::Stipple:
      return L"stipple";
    case XFA_Element::Attributes:
      return L"attributes";
    case XFA_Element::VersionControl:
      return L"versionControl";
    case XFA_Element::Meridiem:
      return L"meridiem";
    case XFA_Element::ExclGroup:
      return L"exclGroup";
    case XFA_Element::ToolTip:
      return L"toolTip";
    case XFA_Element::Compress:
      return L"compress";
    case XFA_Element::Reason:
      return L"reason";
    case XFA_Element::Execute:
      return L"execute";
    case XFA_Element::ContentCopy:
      return L"contentCopy";
    case XFA_Element::DateTimeEdit:
      return L"dateTimeEdit";
    case XFA_Element::Config:
      return L"config";
    case XFA_Element::Image:
      return L"image";
    case XFA_Element::SharpxHTML:
      return L"#xHTML";
    case XFA_Element::NumberOfCopies:
      return L"numberOfCopies";
    case XFA_Element::BehaviorOverride:
      return L"behaviorOverride";
    case XFA_Element::TimeStamp:
      return L"timeStamp";
    case XFA_Element::Month:
      return L"month";
    case XFA_Element::ViewerPreferences:
      return L"viewerPreferences";
    case XFA_Element::ScriptModel:
      return L"scriptModel";
    case XFA_Element::Decimal:
      return L"decimal";
    case XFA_Element::Subform:
      return L"subform";
    case XFA_Element::Select:
      return L"select";
    case XFA_Element::Window:
      return L"window";
    case XFA_Element::LocaleSet:
      return L"localeSet";
    case XFA_Element::Handler:
      return L"handler";
    case XFA_Element::Presence:
      return L"presence";
    case XFA_Element::Record:
      return L"record";
    case XFA_Element::Embed:
      return L"embed";
    case XFA_Element::Version:
      return L"version";
    case XFA_Element::Command:
      return L"command";
    case XFA_Element::Copies:
      return L"copies";
    case XFA_Element::Staple:
      return L"staple";
    case XFA_Element::SubmitFormat:
      return L"submitFormat";
    case XFA_Element::Boolean:
      return L"boolean";
    case XFA_Element::Message:
      return L"message";
    case XFA_Element::Output:
      return L"output";
    case XFA_Element::PsMap:
      return L"psMap";
    case XFA_Element::ExcludeNS:
      return L"excludeNS";
    case XFA_Element::Assist:
      return L"assist";
    case XFA_Element::Picture:
      return L"picture";
    case XFA_Element::Traversal:
      return L"traversal";
    case XFA_Element::SilentPrint:
      return L"silentPrint";
    case XFA_Element::WebClient:
      return L"webClient";
    case XFA_Element::Producer:
      return L"producer";
    case XFA_Element::Corner:
      return L"corner";
    case XFA_Element::MsgId:
      return L"msgId";
    case XFA_Element::Color:
      return L"color";
    case XFA_Element::Keep:
      return L"keep";
    case XFA_Element::Query:
      return L"query";
    case XFA_Element::Insert:
      return L"insert";
    case XFA_Element::ImageEdit:
      return L"imageEdit";
    case XFA_Element::Validate:
      return L"validate";
    case XFA_Element::DigestMethods:
      return L"digestMethods";
    case XFA_Element::NumberPatterns:
      return L"numberPatterns";
    case XFA_Element::PageSet:
      return L"pageSet";
    case XFA_Element::Integer:
      return L"integer";
    case XFA_Element::SoapAddress:
      return L"soapAddress";
    case XFA_Element::Equate:
      return L"equate";
    case XFA_Element::FormFieldFilling:
      return L"formFieldFilling";
    case XFA_Element::PageRange:
      return L"pageRange";
    case XFA_Element::Update:
      return L"update";
    case XFA_Element::ConnectString:
      return L"connectString";
    case XFA_Element::Mode:
      return L"mode";
    case XFA_Element::Layout:
      return L"layout";
    case XFA_Element::Sharpxml:
      return L"#xml";
    case XFA_Element::XsdConnection:
      return L"xsdConnection";
    case XFA_Element::Traverse:
      return L"traverse";
    case XFA_Element::Encodings:
      return L"encodings";
    case XFA_Element::Template:
      return L"template";
    case XFA_Element::Acrobat:
      return L"acrobat";
    case XFA_Element::ValidationMessaging:
      return L"validationMessaging";
    case XFA_Element::Signing:
      return L"signing";
    case XFA_Element::Script:
      return L"script";
    case XFA_Element::AddViewerPreferences:
      return L"addViewerPreferences";
    case XFA_Element::AlwaysEmbed:
      return L"alwaysEmbed";
    case XFA_Element::PasswordEdit:
      return L"passwordEdit";
    case XFA_Element::NumericEdit:
      return L"numericEdit";
    case XFA_Element::EncryptionMethod:
      return L"encryptionMethod";
    case XFA_Element::Change:
      return L"change";
    case XFA_Element::PageArea:
      return L"pageArea";
    case XFA_Element::SubmitUrl:
      return L"submitUrl";
    case XFA_Element::Oids:
      return L"oids";
    case XFA_Element::Signature:
      return L"signature";
    case XFA_Element::ADBE_JSConsole:
      return L"ADBE_JSConsole";
    case XFA_Element::Caption:
      return L"caption";
    case XFA_Element::Relevant:
      return L"relevant";
    case XFA_Element::FlipLabel:
      return L"flipLabel";
    case XFA_Element::ExData:
      return L"exData";
    case XFA_Element::DayNames:
      return L"dayNames";
    case XFA_Element::SoapAction:
      return L"soapAction";
    case XFA_Element::DefaultTypeface:
      return L"defaultTypeface";
    case XFA_Element::Manifest:
      return L"manifest";
    case XFA_Element::Overflow:
      return L"overflow";
    case XFA_Element::Linear:
      return L"linear";
    case XFA_Element::CurrencySymbol:
      return L"currencySymbol";
    case XFA_Element::Delete:
      return L"delete";
    case XFA_Element::Deltas:
      return L"deltas";
    case XFA_Element::DigestMethod:
      return L"digestMethod";
    case XFA_Element::InstanceManager:
      return L"instanceManager";
    case XFA_Element::EquateRange:
      return L"equateRange";
    case XFA_Element::Medium:
      return L"medium";
    case XFA_Element::TextEdit:
      return L"textEdit";
    case XFA_Element::TemplateCache:
      return L"templateCache";
    case XFA_Element::CompressObjectStream:
      return L"compressObjectStream";
    case XFA_Element::DataValue:
      return L"dataValue";
    case XFA_Element::AccessibleContent:
      return L"accessibleContent";
    case XFA_Element::IncludeXDPContent:
      return L"includeXDPContent";
    case XFA_Element::XmlConnection:
      return L"xmlConnection";
    case XFA_Element::ValidateApprovalSignatures:
      return L"validateApprovalSignatures";
    case XFA_Element::SignData:
      return L"signData";
    case XFA_Element::Packets:
      return L"packets";
    case XFA_Element::DatePattern:
      return L"datePattern";
    case XFA_Element::DuplexOption:
      return L"duplexOption";
    case XFA_Element::Base:
      return L"base";
    case XFA_Element::Bind:
      return L"bind";
    case XFA_Element::Compression:
      return L"compression";
    case XFA_Element::User:
      return L"user";
    case XFA_Element::Rectangle:
      return L"rectangle";
    case XFA_Element::EffectiveOutputPolicy:
      return L"effectiveOutputPolicy";
    case XFA_Element::ADBE_JSDebugger:
      return L"ADBE_JSDebugger";
    case XFA_Element::Acrobat7:
      return L"acrobat7";
    case XFA_Element::Interactive:
      return L"interactive";
    case XFA_Element::Locale:
      return L"locale";
    case XFA_Element::CurrentPage:
      return L"currentPage";
    case XFA_Element::Data:
      return L"data";
    case XFA_Element::Date:
      return L"date";
    case XFA_Element::Desc:
      return L"desc";
    case XFA_Element::Encrypt:
      return L"encrypt";
    case XFA_Element::Draw:
      return L"draw";
    case XFA_Element::Encryption:
      return L"encryption";
    case XFA_Element::MeridiemNames:
      return L"meridiemNames";
    case XFA_Element::Messaging:
      return L"messaging";
    case XFA_Element::Speak:
      return L"speak";
    case XFA_Element::DataGroup:
      return L"dataGroup";
    case XFA_Element::Common:
      return L"common";
    case XFA_Element::Sharptext:
      return L"#text";
    case XFA_Element::PaginationOverride:
      return L"paginationOverride";
    case XFA_Element::Reasons:
      return L"reasons";
    case XFA_Element::SignatureProperties:
      return L"signatureProperties";
    case XFA_Element::Threshold:
      return L"threshold";
    case XFA_Element::AppearanceFilter:
      return L"appearanceFilter";
    case XFA_Element::Fill:
      return L"fill";
    case XFA_Element::Font:
      return L"font";
    case XFA_Element::Form:
      return L"form";
    case XFA_Element::MediumInfo:
      return L"mediumInfo";
    case XFA_Element::Certificate:
      return L"certificate";
    case XFA_Element::Password:
      return L"password";
    case XFA_Element::RunScripts:
      return L"runScripts";
    case XFA_Element::Trace:
      return L"trace";
    case XFA_Element::Float:
      return L"float";
    case XFA_Element::RenderPolicy:
      return L"renderPolicy";
    case XFA_Element::Destination:
      return L"destination";
    case XFA_Element::Value:
      return L"value";
    case XFA_Element::Bookend:
      return L"bookend";
    case XFA_Element::ExObject:
      return L"exObject";
    case XFA_Element::OpenAction:
      return L"openAction";
    case XFA_Element::NeverEmbed:
      return L"neverEmbed";
    case XFA_Element::BindItems:
      return L"bindItems";
    case XFA_Element::Calculate:
      return L"calculate";
    case XFA_Element::Print:
      return L"print";
    case XFA_Element::Extras:
      return L"extras";
    case XFA_Element::Proto:
      return L"proto";
    case XFA_Element::DSigData:
      return L"dSigData";
    case XFA_Element::Creator:
      return L"creator";
    case XFA_Element::Connect:
      return L"connect";
    case XFA_Element::Permissions:
      return L"permissions";
    case XFA_Element::ConnectionSet:
      return L"connectionSet";
    case XFA_Element::Submit:
      return L"submit";
    case XFA_Element::Range:
      return L"range";
    case XFA_Element::Linearized:
      return L"linearized";
    case XFA_Element::Packet:
      return L"packet";
    case XFA_Element::RootElement:
      return L"rootElement";
    case XFA_Element::PlaintextMetadata:
      return L"plaintextMetadata";
    case XFA_Element::NumberSymbols:
      return L"numberSymbols";
    case XFA_Element::PrintHighQuality:
      return L"printHighQuality";
    case XFA_Element::Driver:
      return L"driver";
    case XFA_Element::IncrementalLoad:
      return L"incrementalLoad";
    case XFA_Element::SubjectDN:
      return L"subjectDN";
    case XFA_Element::CompressLogicalStructure:
      return L"compressLogicalStructure";
    case XFA_Element::IncrementalMerge:
      return L"incrementalMerge";
    case XFA_Element::Radial:
      return L"radial";
    case XFA_Element::Variables:
      return L"variables";
    case XFA_Element::TimePatterns:
      return L"timePatterns";
    case XFA_Element::EffectiveInputPolicy:
      return L"effectiveInputPolicy";
    case XFA_Element::NameAttr:
      return L"nameAttr";
    case XFA_Element::Conformance:
      return L"conformance";
    case XFA_Element::Transform:
      return L"transform";
    case XFA_Element::LockDocument:
      return L"lockDocument";
    case XFA_Element::BreakAfter:
      return L"breakAfter";
    case XFA_Element::Line:
      return L"line";
    case XFA_Element::Source:
      return L"source";
    case XFA_Element::Occur:
      return L"occur";
    case XFA_Element::PickTrayByPDFSize:
      return L"pickTrayByPDFSize";
    case XFA_Element::MonthNames:
      return L"monthNames";
    case XFA_Element::Severity:
      return L"severity";
    case XFA_Element::GroupParent:
      return L"groupParent";
    case XFA_Element::DocumentAssembly:
      return L"documentAssembly";
    case XFA_Element::NumberSymbol:
      return L"numberSymbol";
    case XFA_Element::Tagged:
      return L"tagged";
    case XFA_Element::Items:
      return L"items";

    default:
      NOTREACHED();
      break;
  }
  return L"";
}
#endif  // NDEBUG
