// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_node.h"

#include <math.h>
#include <stdint.h>

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/cfx_read_only_string_stream.h"
#include "core/fxcrt/cfx_read_only_vector_stream.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_font.h"
#include "fxjs/gc/container_trace.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "fxjs/xfa/cjx_node.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/containers/span.h"
#include "third_party/base/notreached.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fgas/crt/cfgas_decimal.h"
#include "xfa/fgas/crt/locale_iface.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/cxfa_textprovider.h"
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
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_assist.h"
#include "xfa/fxfa/parser/cxfa_attachnodelist.h"
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
#include "xfa/fxfa/parser/cxfa_desc.h"
#include "xfa/fxfa/parser/cxfa_destination.h"
#include "xfa/fxfa/parser/cxfa_digestmethod.h"
#include "xfa/fxfa/parser/cxfa_digestmethods.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_document_builder.h"
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
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_lockdocument.h"
#include "xfa/fxfa/parser/cxfa_log.h"
#include "xfa/fxfa/parser/cxfa_manifest.h"
#include "xfa/fxfa/parser/cxfa_map.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_mdp.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
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
#include "xfa/fxfa/parser/cxfa_nodeiteratortemplate.h"
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
#include "xfa/fxfa/parser/cxfa_stroke.h"
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
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfacontainernode.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfanode.h"
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
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_utils.h"

class CXFA_FieldLayoutData;
class CXFA_ImageEditData;
class CXFA_ImageLayoutData;
class CXFA_TextEditData;
class CXFA_TextLayoutData;

namespace {

constexpr uint8_t kMaxExecuteRecursion = 2;

constexpr uint8_t kInvBase64[128] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255,
    255, 255, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255,
    255, 255, 255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
    10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
    25,  255, 255, 255, 255, 255, 255, 26,  27,  28,  29,  30,  31,  32,  33,
    34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  255, 255, 255, 255, 255,
};

inline uint8_t GetInvBase64(uint8_t x) {
  return (x & 128) == 0 ? kInvBase64[x] : 255;
}

DataVector<uint8_t> XFA_RemoveBase64Whitespace(
    pdfium::span<const uint8_t> spStr) {
  DataVector<uint8_t> result;
  result.reserve(spStr.size());
  for (uint8_t ch : spStr) {
    if (GetInvBase64(ch) != 255 || ch == '=')
      result.push_back(ch);
  }
  return result;
}

DataVector<uint8_t> XFA_Base64Decode(const ByteString& bsStr) {
  DataVector<uint8_t> result;
  if (bsStr.IsEmpty())
    return result;

  DataVector<uint8_t> buffer = XFA_RemoveBase64Whitespace(bsStr.raw_span());
  result.reserve(3 * (buffer.size() / 4));

  uint32_t dwLimb = 0;
  for (size_t i = 0; i + 3 < buffer.size(); i += 4) {
    if (buffer[i] == '=' || buffer[i + 1] == '=' || buffer[i + 2] == '=' ||
        buffer[i + 3] == '=') {
      if (buffer[i] == '=' || buffer[i + 1] == '=') {
        break;
      }
      if (buffer[i + 2] == '=') {
        dwLimb = ((uint32_t)kInvBase64[buffer[i]] << 6) |
                 ((uint32_t)kInvBase64[buffer[i + 1]]);
        result.push_back((uint8_t)(dwLimb >> 4) & 0xFF);
      } else {
        dwLimb = ((uint32_t)kInvBase64[buffer[i]] << 12) |
                 ((uint32_t)kInvBase64[buffer[i + 1]] << 6) |
                 ((uint32_t)kInvBase64[buffer[i + 2]]);
        result.push_back((uint8_t)(dwLimb >> 10) & 0xFF);
        result.push_back((uint8_t)(dwLimb >> 2) & 0xFF);
      }
    } else {
      dwLimb = ((uint32_t)kInvBase64[buffer[i]] << 18) |
               ((uint32_t)kInvBase64[buffer[i + 1]] << 12) |
               ((uint32_t)kInvBase64[buffer[i + 2]] << 6) |
               ((uint32_t)kInvBase64[buffer[i + 3]]);
      result.push_back((uint8_t)(dwLimb >> 16) & 0xff);
      result.push_back((uint8_t)(dwLimb >> 8) & 0xff);
      result.push_back((uint8_t)(dwLimb)&0xff);
    }
  }
  return result;
}

FXCODEC_IMAGE_TYPE XFA_GetImageType(const WideString& wsType) {
  WideString wsContentType(wsType);
  if (wsContentType.EqualsASCIINoCase("image/jpg"))
    return FXCODEC_IMAGE_JPG;

#ifdef PDF_ENABLE_XFA_BMP
  if (wsContentType.EqualsASCIINoCase("image/bmp"))
    return FXCODEC_IMAGE_BMP;
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
  if (wsContentType.EqualsASCIINoCase("image/gif"))
    return FXCODEC_IMAGE_GIF;
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
  if (wsContentType.EqualsASCIINoCase("image/png"))
    return FXCODEC_IMAGE_PNG;
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_TIFF
  if (wsContentType.EqualsASCII("image/tif"))
    return FXCODEC_IMAGE_TIFF;
#endif  // PDF_ENABLE_XFA_TIFF

  return FXCODEC_IMAGE_UNKNOWN;
}

RetainPtr<CFX_DIBitmap> XFA_LoadImageData(CXFA_FFDoc* pDoc,
                                          CXFA_Image* pImage,
                                          bool& bNameImage,
                                          int32_t& iImageXDpi,
                                          int32_t& iImageYDpi) {
  WideString wsHref = pImage->GetHref();
  WideString wsImage = pImage->GetContent();
  if (wsHref.IsEmpty() && wsImage.IsEmpty())
    return nullptr;

  FXCODEC_IMAGE_TYPE type = XFA_GetImageType(pImage->GetContentType());

  RetainPtr<IFX_SeekableReadStream> pImageFileRead;
  if (wsImage.GetLength() > 0) {
    XFA_AttributeValue iEncoding = pImage->GetTransferEncoding();
    if (iEncoding == XFA_AttributeValue::Base64) {
      DataVector<uint8_t> buffer = XFA_Base64Decode(wsImage.ToUTF8());
      if (!buffer.empty()) {
        pImageFileRead =
            pdfium::MakeRetain<CFX_ReadOnlyVectorStream>(std::move(buffer));
      }
    } else {
      pImageFileRead =
          pdfium::MakeRetain<CFX_ReadOnlyStringStream>(wsImage.ToDefANSI());
    }
  } else {
    WideString wsURL = wsHref;
    if (!(wsURL.First(7).EqualsASCII("http://") ||
          wsURL.First(6).EqualsASCII("ftp://"))) {
      RetainPtr<CFX_DIBitmap> pBitmap =
          pDoc->GetPDFNamedImage(wsURL.AsStringView(), iImageXDpi, iImageYDpi);
      if (pBitmap) {
        bNameImage = true;
        return pBitmap;
      }
    }
    pImageFileRead = pDoc->OpenLinkedFile(wsURL);
  }
  if (!pImageFileRead)
    return nullptr;

  bNameImage = false;
  return XFA_LoadImageFromBuffer(std::move(pImageFileRead), type, iImageXDpi,
                                 iImageYDpi);
}

bool SplitDateTime(const WideString& wsDateTime,
                   WideString& wsDate,
                   WideString& wsTime) {
  wsDate.clear();
  wsTime.clear();
  if (wsDateTime.IsEmpty())
    return false;

  auto nSplitIndex = wsDateTime.Find('T');
  if (!nSplitIndex.has_value())
    nSplitIndex = wsDateTime.Find(' ');
  if (!nSplitIndex.has_value())
    return false;

  wsDate = wsDateTime.First(nSplitIndex.value());
  if (!wsDate.IsEmpty()) {
    if (!std::any_of(wsDate.begin(), wsDate.end(),
                     [](wchar_t c) { return FXSYS_IsDecimalDigit(c); })) {
      return false;
    }
  }
  wsTime = wsDateTime.Last(wsDateTime.GetLength() - nSplitIndex.value() - 1);
  if (!wsTime.IsEmpty()) {
    if (!std::any_of(wsTime.begin(), wsTime.end(),
                     [](wchar_t c) { return FXSYS_IsDecimalDigit(c); })) {
      return false;
    }
  }
  return true;
}

// Stack allocated. Using containers of members would be correct here
// if advanced GC worked with STL.
using NodeSet = std::set<cppgc::Member<CXFA_Node>>;
using NodeSetPair = std::pair<NodeSet, NodeSet>;
using NodeSetPairMap = std::map<uint32_t, NodeSetPair>;
using NodeSetPairMapMap = std::map<CXFA_Node*, NodeSetPairMap>;
using NodeVector = std::vector<cppgc::Member<CXFA_Node>>;

NodeVector NodesSortedByDocumentIdx(const NodeSet& rgNodeSet) {
  if (rgNodeSet.empty())
    return NodeVector();

  NodeVector rgNodeArray;
  CXFA_Node* pCommonParent = (*rgNodeSet.begin())->GetParent();
  for (CXFA_Node* pNode = pCommonParent->GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pdfium::Contains(rgNodeSet, pNode))
      rgNodeArray.push_back(pNode);
  }
  return rgNodeArray;
}

NodeSetPair* NodeSetPairForNode(CXFA_Node* pNode, NodeSetPairMapMap* pMap) {
  CXFA_Node* pParentNode = pNode->GetParent();
  uint32_t dwNameHash = pNode->GetNameHash();
  if (!pParentNode || !dwNameHash)
    return nullptr;

  return &((*pMap)[pParentNode][dwNameHash]);
}

void ReorderDataNodes(const NodeSet& sSet1,
                      const NodeSet& sSet2,
                      bool bInsertBefore) {
  NodeSetPairMapMap rgMap;
  for (CXFA_Node* pNode : sSet1) {
    NodeSetPair* pNodeSetPair = NodeSetPairForNode(pNode, &rgMap);
    if (pNodeSetPair)
      pNodeSetPair->first.insert(pNode);
  }
  for (CXFA_Node* pNode : sSet2) {
    NodeSetPair* pNodeSetPair = NodeSetPairForNode(pNode, &rgMap);
    if (pNodeSetPair) {
      if (pdfium::Contains(pNodeSetPair->first, pNode))
        pNodeSetPair->first.erase(pNode);
      else
        pNodeSetPair->second.insert(pNode);
    }
  }
  for (auto& iter1 : rgMap) {
    NodeSetPairMap* pNodeSetPairMap = &iter1.second;
    for (auto& iter2 : *pNodeSetPairMap) {
      NodeSetPair* pNodeSetPair = &iter2.second;
      if (!pNodeSetPair->first.empty() && !pNodeSetPair->second.empty()) {
        NodeVector rgNodeArray1 = NodesSortedByDocumentIdx(pNodeSetPair->first);
        NodeVector rgNodeArray2 =
            NodesSortedByDocumentIdx(pNodeSetPair->second);
        CXFA_Node* pParentNode = nullptr;
        CXFA_Node* pBeforeNode = nullptr;
        if (bInsertBefore) {
          pBeforeNode = rgNodeArray2.front();
          pParentNode = pBeforeNode->GetParent();
        } else {
          CXFA_Node* pLastNode = rgNodeArray2.back();
          pParentNode = pLastNode->GetParent();
          pBeforeNode = pLastNode->GetNextSibling();
        }
        for (auto& pCurNode : rgNodeArray1) {
          pParentNode->RemoveChildAndNotify(pCurNode, true);
          pParentNode->InsertChildAndNotify(pCurNode, pBeforeNode);
        }
      }
    }
    pNodeSetPairMap->clear();
  }
}

float GetEdgeThickness(const std::vector<CXFA_Stroke*>& strokes,
                       bool b3DStyle,
                       int32_t nIndex) {
  float fThickness = 0.0f;
  CXFA_Stroke* stroke = strokes[nIndex * 2 + 1];
  if (stroke->IsVisible()) {
    if (nIndex == 0)
      fThickness += 2.5f;

    fThickness += stroke->GetThickness() * (b3DStyle ? 4 : 2);
  }
  return fThickness;
}

WideString FormatNumStr(const WideString& wsValue, LocaleIface* pLocale) {
  if (wsValue.IsEmpty())
    return WideString();

  WideString wsSrcNum = wsValue;
  WideString wsGroupSymbol = pLocale->GetGroupingSymbol();
  bool bNeg = false;
  if (wsSrcNum[0] == '-') {
    bNeg = true;
    wsSrcNum.Delete(0, 1);
  }

  size_t dot_index = wsSrcNum.Find('.').value_or(wsSrcNum.GetLength());
  if (dot_index == 0)
    return WideString();

  size_t nPos = dot_index % 3;
  WideString wsOutput;
  for (size_t i = 0; i < dot_index; i++) {
    if (i % 3 == nPos && i != 0)
      wsOutput += wsGroupSymbol;

    wsOutput += wsSrcNum[i];
  }
  if (dot_index < wsSrcNum.GetLength()) {
    wsOutput += pLocale->GetDecimalSymbol();
    wsOutput += wsSrcNum.Last(wsSrcNum.GetLength() - dot_index - 1);
  }
  if (bNeg)
    return pLocale->GetMinusSymbol() + wsOutput;

  return wsOutput;
}

CXFA_Node* FindFirstSiblingNamedInList(CXFA_Node* parent,
                                       uint32_t dwNameHash,
                                       Mask<XFA_NodeFilter> dwFilter);
CXFA_Node* FindFirstSiblingOfClassInList(CXFA_Node* parent,
                                         XFA_Element element,
                                         Mask<XFA_NodeFilter> dwFilter);

CXFA_Node* FindFirstSiblingNamed(CXFA_Node* parent, uint32_t dwNameHash) {
  CXFA_Node* result = FindFirstSiblingNamedInList(parent, dwNameHash,
                                                  XFA_NodeFilter::kProperties);
  if (result)
    return result;

  return FindFirstSiblingNamedInList(parent, dwNameHash,
                                     XFA_NodeFilter::kChildren);
}

CXFA_Node* FindFirstSiblingNamedInList(CXFA_Node* parent,
                                       uint32_t dwNameHash,
                                       Mask<XFA_NodeFilter> dwFilter) {
  for (CXFA_Node* child : parent->GetNodeListWithFilter(dwFilter)) {
    if (child->GetNameHash() == dwNameHash)
      return child;

    CXFA_Node* result = FindFirstSiblingNamed(child, dwNameHash);
    if (result)
      return result;
  }
  return nullptr;
}

CXFA_Node* FindFirstSiblingOfClass(CXFA_Node* parent, XFA_Element element) {
  CXFA_Node* result = FindFirstSiblingOfClassInList(
      parent, element, XFA_NodeFilter::kProperties);
  if (result)
    return result;

  return FindFirstSiblingOfClassInList(parent, element,
                                       XFA_NodeFilter::kChildren);
}

CXFA_Node* FindFirstSiblingOfClassInList(CXFA_Node* parent,
                                         XFA_Element element,
                                         Mask<XFA_NodeFilter> dwFilter) {
  for (CXFA_Node* child : parent->GetNodeListWithFilter(dwFilter)) {
    if (child->GetElementType() == element)
      return child;

    CXFA_Node* result = FindFirstSiblingOfClass(child, element);
    if (result)
      return result;
  }
  return nullptr;
}

WideString GetNameExpressionSinglePath(CXFA_Node* pNode) {
  const bool bIsProperty = pNode->IsProperty();
  const bool bIsClassIndex =
      pNode->IsUnnamed() ||
      (bIsProperty && pNode->GetElementType() != XFA_Element::PageSet);
  const wchar_t* pszFormat;
  WideString ws;
  if (bIsClassIndex) {
    pszFormat = L"#%ls[%zu]";
    ws = WideString::FromASCII(pNode->GetClassName());
  } else {
    pszFormat = L"%ls[%zu]";
    ws = pNode->JSObject()->GetCData(XFA_Attribute::Name);
    ws.Replace(L".", L"\\.");
  }

  return WideString::Format(pszFormat, ws.c_str(),
                            pNode->GetIndex(bIsProperty, bIsClassIndex));
}

void TraverseSiblings(CXFA_Node* parent,
                      uint32_t dwNameHash,
                      std::vector<CXFA_Node*>* pSiblings,
                      bool bIsClassName) {
  DCHECK(parent);
  DCHECK(pSiblings);
  for (CXFA_Node* child :
       parent->GetNodeListWithFilter(XFA_NodeFilter::kChildren)) {
    if (child->GetElementType() == XFA_Element::Variables)
      continue;

    if (bIsClassName) {
      if (child->GetClassHashCode() == dwNameHash)
        pSiblings->push_back(child);
    } else {
      if (child->GetNameHash() == dwNameHash)
        pSiblings->push_back(child);
    }
    if (child->IsTransparent() &&
        child->GetElementType() != XFA_Element::PageSet) {
      TraverseSiblings(child, dwNameHash, pSiblings, bIsClassName);
    }
  }
}

void TraversePropertiesOrSiblings(CXFA_Node* parent,
                                  uint32_t dwNameHash,
                                  std::vector<CXFA_Node*>* pSiblings,
                                  bool bIsClassName) {
  DCHECK(parent);
  DCHECK(pSiblings);
  for (CXFA_Node* child :
       parent->GetNodeListWithFilter(XFA_NodeFilter::kProperties)) {
    if (bIsClassName) {
      if (child->GetClassHashCode() == dwNameHash)
        pSiblings->push_back(child);
    } else {
      if (child->GetNameHash() == dwNameHash) {
        if (child->GetElementType() != XFA_Element::PageSet &&
            child->GetElementType() != XFA_Element::Extras &&
            child->GetElementType() != XFA_Element::Items) {
          pSiblings->push_back(child);
        }
      }
    }
    if (child->IsUnnamed() && child->GetElementType() == XFA_Element::PageSet) {
      TraverseSiblings(child, dwNameHash, pSiblings, bIsClassName);
    }
  }
  if (pSiblings->empty())
    TraverseSiblings(parent, dwNameHash, pSiblings, bIsClassName);
}

}  // namespace

class CXFA_WidgetLayoutData
    : public cppgc::GarbageCollected<CXFA_WidgetLayoutData> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  virtual ~CXFA_WidgetLayoutData() = default;

  virtual void Trace(cppgc::Visitor* visitor) const {}

  virtual CXFA_FieldLayoutData* AsFieldLayoutData() { return nullptr; }
  virtual CXFA_ImageLayoutData* AsImageLayoutData() { return nullptr; }
  virtual CXFA_TextLayoutData* AsTextLayoutData() { return nullptr; }

  float GetWidgetHeight() const { return m_fWidgetHeight; }
  void SetWidgetHeight(float height) { m_fWidgetHeight = height; }

 protected:
  CXFA_WidgetLayoutData() = default;

 private:
  float m_fWidgetHeight = -1.0f;
};

class CXFA_TextLayoutData final : public CXFA_WidgetLayoutData {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_TextLayoutData() override = default;

  void Trace(cppgc::Visitor* visitor) const override {
    CXFA_WidgetLayoutData::Trace(visitor);
    visitor->Trace(m_pTextLayout);
    visitor->Trace(m_pTextProvider);
  }

  CXFA_TextLayoutData* AsTextLayoutData() override { return this; }

  CXFA_TextLayout* GetTextLayout() const { return m_pTextLayout; }
  CXFA_TextProvider* GetTextProvider() const { return m_pTextProvider; }

  void LoadText(CXFA_FFDoc* doc, CXFA_Node* pNode) {
    if (m_pTextLayout)
      return;

    m_pTextProvider = cppgc::MakeGarbageCollected<CXFA_TextProvider>(
        doc->GetHeap()->GetAllocationHandle(), pNode,
        CXFA_TextProvider::Type::kText);
    m_pTextLayout = cppgc::MakeGarbageCollected<CXFA_TextLayout>(
        doc->GetHeap()->GetAllocationHandle(), doc, m_pTextProvider);
  }

 private:
  CXFA_TextLayoutData() = default;

  cppgc::Member<CXFA_TextLayout> m_pTextLayout;
  cppgc::Member<CXFA_TextProvider> m_pTextProvider;
};

class CXFA_ImageLayoutData final : public CXFA_WidgetLayoutData {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_ImageLayoutData() override = default;

  CXFA_ImageLayoutData* AsImageLayoutData() override { return this; }

  bool LoadImageData(CXFA_FFDoc* doc, CXFA_Node* pNode) {
    if (m_pDIBitmap)
      return true;

    CXFA_Value* value = pNode->GetFormValueIfExists();
    if (!value)
      return false;

    CXFA_Image* image = value->GetImageIfExists();
    if (!image)
      return false;

    pNode->SetLayoutImage(XFA_LoadImageData(doc, image, m_bNamedImage,
                                            m_iImageXDpi, m_iImageYDpi));
    return !!m_pDIBitmap;
  }

  CFX_Size GetDpi() const { return CFX_Size(m_iImageXDpi, m_iImageYDpi); }
  RetainPtr<CFX_DIBitmap> GetBitmap() { return m_pDIBitmap; }
  void SetBitmap(RetainPtr<CFX_DIBitmap> pBitmap) {
    m_pDIBitmap = std::move(pBitmap);
  }

 private:
  CXFA_ImageLayoutData() = default;

  bool m_bNamedImage = false;
  int32_t m_iImageXDpi = 0;
  int32_t m_iImageYDpi = 0;
  RetainPtr<CFX_DIBitmap> m_pDIBitmap;
};

class CXFA_FieldLayoutData : public CXFA_WidgetLayoutData {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FieldLayoutData() override = default;

  void Trace(cppgc::Visitor* visitor) const override {
    CXFA_WidgetLayoutData::Trace(visitor);
    visitor->Trace(m_pCapTextLayout);
    visitor->Trace(m_pCapTextProvider);
  }
  CXFA_FieldLayoutData* AsFieldLayoutData() override { return this; }

  virtual CXFA_ImageEditData* AsImageEditData() { return nullptr; }
  virtual CXFA_TextEditData* AsTextEditData() { return nullptr; }

  bool LoadCaption(CXFA_FFDoc* doc, CXFA_Node* pNode) {
    if (m_pCapTextLayout)
      return true;
    CXFA_Caption* caption = pNode->GetCaptionIfExists();
    if (!caption || caption->IsHidden())
      return false;

    m_pCapTextProvider = cppgc::MakeGarbageCollected<CXFA_TextProvider>(
        doc->GetHeap()->GetAllocationHandle(), pNode,
        CXFA_TextProvider::Type::kCaption);
    m_pCapTextLayout = cppgc::MakeGarbageCollected<CXFA_TextLayout>(
        doc->GetHeap()->GetAllocationHandle(), doc, m_pCapTextProvider);
    return true;
  }

  cppgc::Member<CXFA_TextLayout> m_pCapTextLayout;
  cppgc::Member<CXFA_TextProvider> m_pCapTextProvider;
  std::unique_ptr<CFDE_TextOut> m_pTextOut;
  std::vector<float> m_FieldSplitArray;

 protected:
  CXFA_FieldLayoutData() = default;
};

class CXFA_TextEditData final : public CXFA_FieldLayoutData {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_TextEditData() override = default;

  CXFA_TextEditData* AsTextEditData() override { return this; }

 protected:
  CXFA_TextEditData() = default;
};

class CXFA_ImageEditData final : public CXFA_FieldLayoutData {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_ImageEditData() override = default;

  CXFA_ImageEditData* AsImageEditData() override { return this; }

  bool LoadImageData(CXFA_FFDoc* doc, CXFA_Node* pNode) {
    if (m_pDIBitmap)
      return true;

    CXFA_Value* value = pNode->GetFormValueIfExists();
    if (!value)
      return false;

    CXFA_Image* image = value->GetImageIfExists();
    if (!image)
      return false;

    pNode->SetEditImage(XFA_LoadImageData(doc, image, m_bNamedImage,
                                          m_iImageXDpi, m_iImageYDpi));
    return !!m_pDIBitmap;
  }

  CFX_Size GetDpi() const { return CFX_Size(m_iImageXDpi, m_iImageYDpi); }
  RetainPtr<CFX_DIBitmap> GetBitmap() { return m_pDIBitmap; }
  void SetBitmap(RetainPtr<CFX_DIBitmap> pBitmap) {
    m_pDIBitmap = std::move(pBitmap);
  }

 private:
  CXFA_ImageEditData() = default;

  bool m_bNamedImage = false;
  int32_t m_iImageXDpi = 0;
  int32_t m_iImageYDpi = 0;
  RetainPtr<CFX_DIBitmap> m_pDIBitmap;
};

CXFA_Node::CXFA_Node(CXFA_Document* pDoc,
                     XFA_PacketType ePacket,
                     Mask<XFA_XDPPACKET> validPackets,
                     XFA_ObjectType oType,
                     XFA_Element eType,
                     pdfium::span<const PropertyData> properties,
                     pdfium::span<const AttributeData> attributes,
                     CJX_Object* js_object)
    : CXFA_Object(pDoc, oType, eType, js_object),
      m_Properties(properties),
      m_Attributes(attributes),
      m_ValidPackets(validPackets),
      m_ePacket(ePacket) {
  DCHECK(m_pDocument);
}

CXFA_Node::~CXFA_Node() = default;

void CXFA_Node::Trace(cppgc::Visitor* visitor) const {
  CXFA_Object::Trace(visitor);
  GCedTreeNodeMixin<CXFA_Node>::Trace(visitor);
  visitor->Trace(m_pAuxNode);
  ContainerTrace(visitor, binding_nodes_);
  visitor->Trace(m_pLayoutData);
  visitor->Trace(ui_);
}

CXFA_Node* CXFA_Node::Clone(bool bRecursive) {
  CXFA_Node* pClone = m_pDocument->CreateNode(m_ePacket, m_elementType);
  if (!pClone)
    return nullptr;

  JSObject()->MergeAllData(pClone);
  pClone->UpdateNameHash();
  if (IsNeedSavingXMLNode()) {
    CFX_XMLNode* pCloneXML;
    if (IsAttributeInXML()) {
      WideString wsName = JSObject()
                              ->TryAttribute(XFA_Attribute::Name, false)
                              .value_or(WideString());
      auto* pCloneXMLElement =
          GetXMLDocument()->CreateNode<CFX_XMLElement>(wsName);

      WideString wsValue = JSObject()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty()) {
        auto* text = GetXMLDocument()->CreateNode<CFX_XMLText>(wsValue);
        pCloneXMLElement->AppendLastChild(text);
      }

      pCloneXML = pCloneXMLElement;
      pClone->JSObject()->SetEnum(XFA_Attribute::Contains,
                                  XFA_AttributeValue::Unknown, false);
    } else {
      pCloneXML = xml_node_->Clone(GetXMLDocument());
    }
    pClone->SetXMLMappingNode(pCloneXML);
  }
  if (bRecursive) {
    for (CXFA_Node* pChild = GetFirstChild(); pChild;
         pChild = pChild->GetNextSibling()) {
      pClone->InsertChildAndNotify(pChild->Clone(bRecursive), nullptr);
    }
  }
  pClone->SetInitializedFlagAndNotify();
  pClone->SetBindingNode(nullptr);
  return pClone;
}

CXFA_Node* CXFA_Node::GetNextContainerSibling() const {
  for (auto* pNode = GetNextSibling(); pNode; pNode = pNode->GetNextSibling()) {
    if (pNode->GetObjectType() == XFA_ObjectType::ContainerNode)
      return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetPrevContainerSibling() const {
  for (auto* pNode = GetPrevSibling(); pNode; pNode = pNode->GetPrevSibling()) {
    if (pNode->GetObjectType() == XFA_ObjectType::ContainerNode)
      return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetFirstContainerChild() const {
  for (auto* pNode = GetFirstChild(); pNode; pNode = pNode->GetNextSibling()) {
    if (pNode->GetObjectType() == XFA_ObjectType::ContainerNode)
      return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetContainerParent() const {
  for (auto* pNode = GetParent(); pNode; pNode = pNode->GetParent()) {
    if (pNode->GetObjectType() == XFA_ObjectType::ContainerNode)
      return pNode;
  }
  return nullptr;
}

bool CXFA_Node::IsValidInPacket(XFA_PacketType packet) const {
  uint32_t bitflag = 1 << static_cast<uint8_t>(packet);
  return !!(m_ValidPackets & static_cast<XFA_XDPPACKET>(bitflag));
}

const CXFA_Node::PropertyData* CXFA_Node::GetPropertyData(
    XFA_Element property) const {
  DCHECK(property != XFA_Element::Unknown);
  for (const auto& prop : m_Properties) {
    if (prop.property == property)
      return &prop;
  }
  return nullptr;
}

bool CXFA_Node::HasProperty(XFA_Element property) const {
  return !!GetPropertyData(property);
}

bool CXFA_Node::HasPropertyFlag(XFA_Element property,
                                XFA_PropertyFlag flag) const {
  const PropertyData* data = GetPropertyData(property);
  return data && !!(data->flags & flag);
}

uint8_t CXFA_Node::PropertyOccurrenceCount(XFA_Element property) const {
  const PropertyData* data = GetPropertyData(property);
  return data ? data->occurrence_count : 0;
}

std::pair<CXFA_Node*, int32_t> CXFA_Node::GetProperty(
    int32_t index,
    XFA_Element eProperty) const {
  if (index < 0 || index >= PropertyOccurrenceCount(eProperty))
    return {nullptr, 0};

  int32_t iCount = 0;
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() == eProperty) {
      iCount++;
      if (iCount > index)
        return {pNode, iCount};
    }
  }
  return {nullptr, iCount};
}

CXFA_Node* CXFA_Node::GetOrCreateProperty(int32_t index,
                                          XFA_Element eProperty) {
  if (index < 0 || index >= PropertyOccurrenceCount(eProperty))
    return nullptr;

  int32_t iCount = 0;
  CXFA_Node* node;
  std::tie(node, iCount) = GetProperty(index, eProperty);
  if (node)
    return node;

  if (HasPropertyFlag(eProperty, XFA_PropertyFlag::kOneOf)) {
    for (CXFA_Node* pNode = GetFirstChild(); pNode;
         pNode = pNode->GetNextSibling()) {
      if (HasPropertyFlag(pNode->GetElementType(), XFA_PropertyFlag::kOneOf)) {
        return nullptr;
      }
    }
  }

  CXFA_Node* pNewNode = nullptr;
  for (; iCount <= index; ++iCount) {
    pNewNode = GetDocument()->CreateNode(GetPacketType(), eProperty);
    if (!pNewNode)
      return nullptr;

    InsertChildAndNotify(pNewNode, nullptr);
    pNewNode->SetInitializedFlagAndNotify();
  }
  return pNewNode;
}

absl::optional<XFA_Element> CXFA_Node::GetFirstPropertyWithFlag(
    XFA_PropertyFlag flag) const {
  for (const auto& prop : m_Properties) {
    if (prop.flags & flag)
      return prop.property;
  }
  return absl::nullopt;
}

const CXFA_Node::AttributeData* CXFA_Node::GetAttributeData(
    XFA_Attribute attr) const {
  DCHECK(attr != XFA_Attribute::Unknown);
  for (const auto& cur_attr : m_Attributes) {
    if (cur_attr.attribute == attr)
      return &cur_attr;
  }
  return nullptr;
}

bool CXFA_Node::HasAttribute(XFA_Attribute attr) const {
  return !!GetAttributeData(attr);
}

XFA_Attribute CXFA_Node::GetAttribute(size_t i) const {
  return i < m_Attributes.size() ? m_Attributes[i].attribute
                                 : XFA_Attribute::Unknown;
}

XFA_AttributeType CXFA_Node::GetAttributeType(XFA_Attribute type) const {
  const AttributeData* data = GetAttributeData(type);
  return data ? data->type : XFA_AttributeType::CData;
}

std::vector<CXFA_Node*> CXFA_Node::GetNodeListForType(XFA_Element eTypeFilter) {
  std::vector<CXFA_Node*> nodes;
  for (CXFA_Node* pChild = GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (pChild->GetElementType() == eTypeFilter)
      nodes.push_back(pChild);
  }
  return nodes;
}

std::vector<CXFA_Node*> CXFA_Node::GetNodeListWithFilter(
    Mask<XFA_NodeFilter> dwFilter) {
  if (!dwFilter)
    return std::vector<CXFA_Node*>();

  const bool bFilterChildren = !!(dwFilter & XFA_NodeFilter::kChildren);
  const bool bFilterProperties = !!(dwFilter & XFA_NodeFilter::kProperties);
  const bool bFilterOneOfProperties =
      !!(dwFilter & XFA_NodeFilter::kOneOfProperty);

  std::vector<CXFA_Node*> nodes;
  if (bFilterChildren && bFilterProperties && !bFilterOneOfProperties) {
    for (CXFA_Node* pChild = GetFirstChild(); pChild;
         pChild = pChild->GetNextSibling()) {
      nodes.push_back(pChild);
    }
    return nodes;
  }

  for (CXFA_Node* pChild = GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (HasProperty(pChild->GetElementType())) {
      if (bFilterProperties) {
        nodes.push_back(pChild);
      } else if (bFilterOneOfProperties &&
                 HasPropertyFlag(pChild->GetElementType(),
                                 XFA_PropertyFlag::kOneOf)) {
        nodes.push_back(pChild);
      } else if (bFilterChildren &&
                 (pChild->GetElementType() == XFA_Element::Variables ||
                  pChild->GetElementType() == XFA_Element::PageSet)) {
        nodes.push_back(pChild);
      }
    } else if (bFilterChildren) {
      nodes.push_back(pChild);
    }
  }

  if (!bFilterOneOfProperties || !nodes.empty())
    return nodes;

  absl::optional<XFA_Element> property =
      GetFirstPropertyWithFlag(XFA_PropertyFlag::kDefaultOneOf);
  if (!property.has_value())
    return nodes;

  CXFA_Node* pNewNode =
      m_pDocument->CreateNode(GetPacketType(), property.value());
  if (pNewNode) {
    InsertChildAndNotify(pNewNode, nullptr);
    pNewNode->SetInitializedFlagAndNotify();
    nodes.push_back(pNewNode);
  }
  return nodes;
}

CXFA_Node* CXFA_Node::CreateSamePacketNode(XFA_Element eType) {
  CXFA_Node* pNode = m_pDocument->CreateNode(m_ePacket, eType);
  if (!pNode)
    return nullptr;

  pNode->SetInitializedFlagAndNotify();
  return pNode;
}

CXFA_Node* CXFA_Node::CloneTemplateToForm(bool bRecursive) {
  DCHECK_EQ(m_ePacket, XFA_PacketType::Template);
  CXFA_Node* pClone =
      m_pDocument->CreateNode(XFA_PacketType::Form, m_elementType);
  if (!pClone)
    return nullptr;

  pClone->SetTemplateNode(this);
  pClone->UpdateNameHash();
  pClone->SetXMLMappingNode(GetXMLMappingNode());
  if (bRecursive) {
    for (CXFA_Node* pChild = GetFirstChild(); pChild;
         pChild = pChild->GetNextSibling()) {
      pClone->InsertChildAndNotify(pChild->CloneTemplateToForm(bRecursive),
                                   nullptr);
    }
  }
  pClone->SetInitializedFlagAndNotify();
  return pClone;
}

CXFA_Node* CXFA_Node::GetTemplateNodeIfExists() const {
  return m_pAuxNode;
}

void CXFA_Node::SetTemplateNode(CXFA_Node* pTemplateNode) {
  m_pAuxNode = pTemplateNode;
}

CXFA_Node* CXFA_Node::GetBindData() {
  DCHECK_EQ(GetPacketType(), XFA_PacketType::Form);
  return GetBindingNode();
}

std::vector<CXFA_Node*> CXFA_Node::GetBindItemsCopy() const {
  return std::vector<CXFA_Node*>(binding_nodes_.begin(), binding_nodes_.end());
}

void CXFA_Node::AddBindItem(CXFA_Node* pFormNode) {
  DCHECK(pFormNode);

  if (BindsFormItems()) {
    if (!pdfium::Contains(binding_nodes_, pFormNode))
      binding_nodes_.emplace_back(pFormNode);
    return;
  }

  CXFA_Node* pOldFormItem = GetBindingNode();
  if (!pOldFormItem) {
    SetBindingNode(pFormNode);
    return;
  }
  if (pOldFormItem == pFormNode)
    return;

  binding_nodes_.clear();
  binding_nodes_.push_back(pOldFormItem);
  binding_nodes_.push_back(pFormNode);
  m_uNodeFlags |= XFA_NodeFlag::kBindFormItems;
}

bool CXFA_Node::RemoveBindItem(CXFA_Node* pFormNode) {
  if (BindsFormItems()) {
    auto it =
        std::find(binding_nodes_.begin(), binding_nodes_.end(), pFormNode);
    if (it != binding_nodes_.end())
      binding_nodes_.erase(it);

    if (binding_nodes_.size() == 1) {
      m_uNodeFlags.Clear(XFA_NodeFlag::kBindFormItems);
      return true;
    }
    return !binding_nodes_.empty();
  }

  CXFA_Node* pOldFormItem = GetBindingNode();
  if (pOldFormItem != pFormNode)
    return !!pOldFormItem;

  SetBindingNode(nullptr);
  return false;
}

bool CXFA_Node::HasBindItem() const {
  return GetPacketType() == XFA_PacketType::Datasets && GetBindingNode();
}

CXFA_Node* CXFA_Node::GetContainerNode() {
  if (GetPacketType() != XFA_PacketType::Form)
    return nullptr;
  XFA_Element eType = GetElementType();
  if (eType == XFA_Element::ExclGroup)
    return nullptr;
  CXFA_Node* pParentNode = GetParent();
  if (pParentNode && pParentNode->GetElementType() == XFA_Element::ExclGroup)
    return nullptr;

  if (eType == XFA_Element::Field) {
    if (IsChoiceListMultiSelect())
      return nullptr;

    WideString wsPicture = GetPictureContent(XFA_ValuePicture::kDataBind);
    if (!wsPicture.IsEmpty())
      return this;

    CXFA_Node* pDataNode = GetBindData();
    if (!pDataNode)
      return nullptr;

    CXFA_Node* pFieldNode = nullptr;
    for (auto* pFormNode : pDataNode->GetBindItemsCopy()) {
      if (!pFormNode || pFormNode->HasRemovedChildren())
        continue;
      pFieldNode = pFormNode->IsWidgetReady() ? pFormNode : nullptr;
      if (pFieldNode)
        wsPicture = pFieldNode->GetPictureContent(XFA_ValuePicture::kDataBind);
      if (!wsPicture.IsEmpty())
        break;

      pFieldNode = nullptr;
    }
    return pFieldNode;
  }

  CXFA_Node* pGrandNode = pParentNode ? pParentNode->GetParent() : nullptr;
  CXFA_Node* pValueNode =
      (pParentNode && pParentNode->GetElementType() == XFA_Element::Value)
          ? pParentNode
          : nullptr;
  if (!pValueNode) {
    pValueNode =
        (pGrandNode && pGrandNode->GetElementType() == XFA_Element::Value)
            ? pGrandNode
            : nullptr;
  }
  CXFA_Node* pParentOfValueNode =
      pValueNode ? pValueNode->GetParent() : nullptr;
  return pParentOfValueNode ? pParentOfValueNode->GetContainerNode() : nullptr;
}

GCedLocaleIface* CXFA_Node::GetLocale() {
  absl::optional<WideString> localeName = GetLocaleName();
  if (!localeName.has_value())
    return nullptr;
  if (localeName.value().EqualsASCII("ambient"))
    return GetDocument()->GetLocaleMgr()->GetDefLocale();
  return GetDocument()->GetLocaleMgr()->GetLocaleByName(localeName.value());
}

absl::optional<WideString> CXFA_Node::GetLocaleName() {
  CXFA_Node* pForm = ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form));
  if (!pForm)
    return absl::nullopt;

  CXFA_Subform* pTopSubform =
      pForm->GetFirstChildByClass<CXFA_Subform>(XFA_Element::Subform);
  if (!pTopSubform)
    return absl::nullopt;

  absl::optional<WideString> localeName;
  CXFA_Node* pLocaleNode = this;
  do {
    localeName =
        pLocaleNode->JSObject()->TryCData(XFA_Attribute::Locale, false);
    if (localeName.has_value())
      return localeName;

    pLocaleNode = pLocaleNode->GetParent();
  } while (pLocaleNode && pLocaleNode != pTopSubform);

  CXFA_Node* pConfig = ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Config));
  localeName = GetDocument()->GetLocaleMgr()->GetConfigLocaleName(pConfig);
  if (localeName.has_value())
    return localeName;

  if (pTopSubform) {
    localeName =
        pTopSubform->JSObject()->TryCData(XFA_Attribute::Locale, false);
    if (localeName.has_value())
      return localeName;
  }

  LocaleIface* pLocale = GetDocument()->GetLocaleMgr()->GetDefLocale();
  if (!pLocale)
    return absl::nullopt;

  return pLocale->GetName();
}

XFA_AttributeValue CXFA_Node::GetIntact() {
  CXFA_Keep* pKeep = GetFirstChildByClass<CXFA_Keep>(XFA_Element::Keep);
  auto layout = JSObject()->TryEnum(XFA_Attribute::Layout, true);
  XFA_AttributeValue eLayoutType =
      layout.value_or(XFA_AttributeValue::Position);
  if (pKeep) {
    absl::optional<XFA_AttributeValue> intact =
        GetIntactFromKeep(pKeep, eLayoutType);
    if (intact.has_value())
      return intact.value();
  }

  switch (GetElementType()) {
    case XFA_Element::Subform:
      switch (eLayoutType) {
        case XFA_AttributeValue::Position:
        case XFA_AttributeValue::Row:
          return XFA_AttributeValue::ContentArea;
        default:
          return XFA_AttributeValue::None;
      }
    case XFA_Element::Field: {
      CXFA_Node* parent = GetParent();
      if (!parent || parent->GetElementType() == XFA_Element::PageArea)
        return XFA_AttributeValue::ContentArea;
      if (parent->GetIntact() != XFA_AttributeValue::None)
        return XFA_AttributeValue::ContentArea;

      auto value = parent->JSObject()->TryEnum(XFA_Attribute::Layout, true);
      XFA_AttributeValue eParLayout =
          value.value_or(XFA_AttributeValue::Position);
      if (eParLayout == XFA_AttributeValue::Position ||
          eParLayout == XFA_AttributeValue::Row ||
          eParLayout == XFA_AttributeValue::Table) {
        return XFA_AttributeValue::None;
      }

      XFA_VERSION version = m_pDocument->GetCurVersionMode();
      if (eParLayout == XFA_AttributeValue::Tb && version < XFA_VERSION_208) {
        absl::optional<CXFA_Measurement> measureH =
            JSObject()->TryMeasure(XFA_Attribute::H, false);
        if (measureH.has_value())
          return XFA_AttributeValue::ContentArea;
      }
      return XFA_AttributeValue::None;
    }
    case XFA_Element::Draw:
      return XFA_AttributeValue::ContentArea;
    default:
      return XFA_AttributeValue::None;
  }
}

WideString CXFA_Node::GetNameExpression() {
  WideString wsName = GetNameExpressionSinglePath(this);
  CXFA_Node* parent = GetParent();
  while (parent) {
    WideString wsParent = GetNameExpressionSinglePath(parent);
    wsParent += L".";
    wsParent += wsName;
    wsName = std::move(wsParent);
    parent = parent->GetParent();
  }
  return wsName;
}

CXFA_Node* CXFA_Node::GetDataDescriptionNode() {
  if (m_ePacket == XFA_PacketType::Datasets)
    return m_pAuxNode;
  return nullptr;
}

void CXFA_Node::SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode) {
  DCHECK_EQ(m_ePacket, XFA_PacketType::Datasets);
  m_pAuxNode = pDataDescriptionNode;
}

CXFA_Node* CXFA_Node::GetModelNode() {
  switch (GetPacketType()) {
    case XFA_PacketType::Xdp:
      return m_pDocument->GetRoot();
    case XFA_PacketType::Config:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Config));
    case XFA_PacketType::Template:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Template));
    case XFA_PacketType::Form:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Form));
    case XFA_PacketType::Datasets:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Datasets));
    case XFA_PacketType::LocaleSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_LocaleSet));
    case XFA_PacketType::ConnectionSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_ConnectionSet));
    case XFA_PacketType::SourceSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_SourceSet));
    case XFA_PacketType::Xdc:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Xdc));
    default:
      return this;
  }
}

size_t CXFA_Node::CountChildren(XFA_Element eType, bool bOnlyChild) {
  size_t count = 0;
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != eType && eType != XFA_Element::Unknown)
      continue;
    if (bOnlyChild && HasProperty(pNode->GetElementType()))
      continue;
    ++count;
  }
  return count;
}

CXFA_Node* CXFA_Node::GetChildInternal(size_t index,
                                       XFA_Element eType,
                                       bool bOnlyChild) const {
  size_t count = 0;
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != eType && eType != XFA_Element::Unknown)
      continue;
    if (bOnlyChild && HasProperty(pNode->GetElementType()))
      continue;
    if (count == index)
      return pNode;

    ++count;
  }
  return nullptr;
}

bool CXFA_Node::IsAncestorOf(const CXFA_Node* that) const {
  while (that) {
    if (this == that)
      return true;
    that = that->GetParent();
  }
  return false;
}

void CXFA_Node::InsertChildAndNotify(int32_t index, CXFA_Node* pNode) {
  InsertChildAndNotify(pNode, GetNthChild(index));
}

void CXFA_Node::InsertChildAndNotify(CXFA_Node* pNode, CXFA_Node* pBeforeNode) {
  CHECK(!pNode->GetParent());
  CHECK(!pBeforeNode || pBeforeNode->GetParent() == this);
  pNode->ClearFlag(XFA_NodeFlag::kHasRemovedChildren);
  InsertBefore(pNode, pBeforeNode);

  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildAdded(this);

  if (!IsNeedSavingXMLNode() || !pNode->xml_node_)
    return;

  DCHECK(!pNode->xml_node_->GetParent());
  xml_node_->InsertBefore(pNode->xml_node_,
                          pBeforeNode ? pBeforeNode->xml_node_ : nullptr);
}

void CXFA_Node::RemoveChildAndNotify(CXFA_Node* pNode, bool bNotify) {
  CHECK(pNode);
  if (pNode->GetParent() != this)
    return;

  pNode->SetFlag(XFA_NodeFlag::kHasRemovedChildren);
  GCedTreeNodeMixin<CXFA_Node>::RemoveChild(pNode);
  OnRemoved(bNotify);

  if (!IsNeedSavingXMLNode() || !pNode->xml_node_)
    return;

  if (!pNode->IsAttributeInXML()) {
    xml_node_->RemoveChild(pNode->xml_node_);
    return;
  }

  DCHECK_EQ(pNode->xml_node_, xml_node_);
  CFX_XMLElement* pXMLElement = ToXMLElement(pNode->xml_node_);
  if (pXMLElement) {
    WideString wsAttributeName =
        pNode->JSObject()->GetCData(XFA_Attribute::QualifiedName);
    pXMLElement->RemoveAttribute(wsAttributeName);
  }

  WideString wsName = pNode->JSObject()
                          ->TryAttribute(XFA_Attribute::Name, false)
                          .value_or(WideString());

  auto* pNewXMLElement = GetXMLDocument()->CreateNode<CFX_XMLElement>(wsName);
  WideString wsValue = JSObject()->GetCData(XFA_Attribute::Value);
  if (!wsValue.IsEmpty()) {
    auto* text = GetXMLDocument()->CreateNode<CFX_XMLText>(wsValue);
    pNewXMLElement->AppendLastChild(text);
  }
  pNode->xml_node_ = pNewXMLElement;
  pNode->JSObject()->SetEnum(XFA_Attribute::Contains,
                             XFA_AttributeValue::Unknown, false);
}

CXFA_Node* CXFA_Node::GetFirstChildByName(WideStringView wsName) const {
  return GetFirstChildByName(FX_HashCode_GetW(wsName));
}

CXFA_Node* CXFA_Node::GetFirstChildByName(uint32_t dwNameHash) const {
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetNameHash() == dwNameHash)
      return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetFirstChildByClassInternal(XFA_Element eType) const {
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() == eType)
      return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextSameNameSibling(uint32_t dwNameHash) const {
  for (CXFA_Node* pNode = GetNextSibling(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetNameHash() == dwNameHash)
      return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextSameNameSiblingInternal(
    WideStringView wsNodeName) const {
  return GetNextSameNameSibling(FX_HashCode_GetW(wsNodeName));
}

CXFA_Node* CXFA_Node::GetNextSameClassSiblingInternal(XFA_Element eType) const {
  for (CXFA_Node* pNode = GetNextSibling(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() == eType)
      return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetOneChildNamed(WideStringView wsName) {
  return FindFirstSiblingNamed(this, FX_HashCode_GetW(wsName));
}

CXFA_Node* CXFA_Node::GetOneChildOfClass(WideStringView wsClass) {
  XFA_Element element = XFA_GetElementByName(wsClass);
  if (element == XFA_Element::Unknown)
    return nullptr;

  return FindFirstSiblingOfClass(this, element);
}

std::vector<CXFA_Node*> CXFA_Node::GetSiblings(bool bIsClassName) {
  std::vector<CXFA_Node*> siblings;
  CXFA_Node* parent = GetParent();
  if (!parent)
    return siblings;
  if (!parent->HasProperty(GetElementType())) {
    parent = GetTransparentParent();
    if (!parent)
      return siblings;
  }

  uint32_t dwNameHash = bIsClassName ? GetClassHashCode() : GetNameHash();
  TraversePropertiesOrSiblings(parent, dwNameHash, &siblings, bIsClassName);
  return siblings;
}

size_t CXFA_Node::GetIndex(bool bIsProperty, bool bIsClassIndex) {
  CXFA_Node* parent = GetParent();
  if (!parent)
    return 0;

  if (!bIsProperty) {
    parent = GetTransparentParent();
    if (!parent)
      return 0;
  }
  uint32_t dwHashName = bIsClassIndex ? GetClassHashCode() : GetNameHash();
  std::vector<CXFA_Node*> siblings;
  TraversePropertiesOrSiblings(parent, dwHashName, &siblings, bIsClassIndex);
  for (size_t i = 0; i < siblings.size(); ++i) {
    if (siblings[i] == this)
      return i;
  }
  return 0;
}

size_t CXFA_Node::GetIndexByName() {
  return GetIndex(IsProperty(), /*bIsClassIndex=*/false);
}

size_t CXFA_Node::GetIndexByClassName() {
  return GetIndex(IsProperty(), /*bIsClassIndex=*/true);
}

CXFA_Node* CXFA_Node::GetInstanceMgrOfSubform() {
  CXFA_Node* pInstanceMgr = nullptr;
  if (m_ePacket == XFA_PacketType::Form) {
    CXFA_Node* pParentNode = GetParent();
    if (!pParentNode || pParentNode->GetElementType() == XFA_Element::Area)
      return pInstanceMgr;

    for (CXFA_Node* pNode = GetPrevSibling(); pNode;
         pNode = pNode->GetPrevSibling()) {
      XFA_Element eType = pNode->GetElementType();
      if ((eType == XFA_Element::Subform || eType == XFA_Element::SubformSet) &&
          pNode->m_dwNameHash != m_dwNameHash) {
        break;
      }
      if (eType == XFA_Element::InstanceManager) {
        WideString wsName = JSObject()->GetCData(XFA_Attribute::Name);
        WideString wsInstName =
            pNode->JSObject()->GetCData(XFA_Attribute::Name);
        if (wsInstName.GetLength() > 0 && wsInstName[0] == '_' &&
            wsInstName.Last(wsInstName.GetLength() - 1) == wsName) {
          pInstanceMgr = pNode;
        }
        break;
      }
    }
  }
  return pInstanceMgr;
}

CXFA_Occur* CXFA_Node::GetOccurIfExists() {
  return GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
}

bool CXFA_Node::HasFlag(XFA_NodeFlag dwFlag) const {
  if (m_uNodeFlags & dwFlag)
    return true;
  if (dwFlag == XFA_NodeFlag::kHasRemovedChildren)
    return GetParent() && GetParent()->HasFlag(dwFlag);
  return false;
}

void CXFA_Node::SetInitializedFlagAndNotify() {
  if (!IsInitialized()) {
    CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
    if (pNotify)
      pNotify->OnNodeReady(this);
  }
  m_uNodeFlags |= XFA_NodeFlag::kInitialized;
}

void CXFA_Node::SetFlag(XFA_NodeFlag dwFlag) {
  m_uNodeFlags |= dwFlag;
}

void CXFA_Node::ClearFlag(XFA_NodeFlag dwFlag) {
  m_uNodeFlags.Clear(dwFlag);
}

bool CXFA_Node::IsAttributeInXML() {
  return JSObject()->GetEnum(XFA_Attribute::Contains) ==
         XFA_AttributeValue::MetaData;
}

void CXFA_Node::OnRemoved(bool bNotify) const {
  if (!bNotify)
    return;

  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildRemoved();
}

void CXFA_Node::UpdateNameHash() {
  WideString wsName = JSObject()->GetCData(XFA_Attribute::Name);
  m_dwNameHash = FX_HashCode_GetW(wsName.AsStringView());
}

CFX_XMLNode* CXFA_Node::CreateXMLMappingNode() {
  if (!xml_node_) {
    xml_node_ = GetXMLDocument()->CreateNode<CFX_XMLElement>(
        JSObject()->GetCData(XFA_Attribute::Name));
  }
  return xml_node_;
}

bool CXFA_Node::IsNeedSavingXMLNode() const {
  return xml_node_ && (GetPacketType() == XFA_PacketType::Datasets ||
                       GetElementType() == XFA_Element::Xfa);
}

CXFA_Node* CXFA_Node::GetItemIfExists(int32_t iIndex) {
  int32_t iCount = 0;
  uint32_t dwNameHash = 0;
  for (CXFA_Node* pNode = GetNextSibling(); pNode;
       pNode = pNode->GetNextSibling()) {
    XFA_Element eCurType = pNode->GetElementType();
    if (eCurType == XFA_Element::InstanceManager)
      break;
    if ((eCurType != XFA_Element::Subform) &&
        (eCurType != XFA_Element::SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      WideString wsName = pNode->JSObject()->GetCData(XFA_Attribute::Name);
      WideString wsInstName = JSObject()->GetCData(XFA_Attribute::Name);
      if (wsInstName.GetLength() < 1 || wsInstName[0] != '_' ||
          wsInstName.Last(wsInstName.GetLength() - 1) != wsName) {
        return nullptr;
      }
      dwNameHash = pNode->GetNameHash();
    }
    if (dwNameHash != pNode->GetNameHash())
      break;

    iCount++;
    if (iCount > iIndex)
      return pNode;
  }
  return nullptr;
}

int32_t CXFA_Node::GetCount() {
  int32_t iCount = 0;
  uint32_t dwNameHash = 0;
  for (CXFA_Node* pNode = GetNextSibling(); pNode;
       pNode = pNode->GetNextSibling()) {
    XFA_Element eCurType = pNode->GetElementType();
    if (eCurType == XFA_Element::InstanceManager)
      break;
    if ((eCurType != XFA_Element::Subform) &&
        (eCurType != XFA_Element::SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      WideString wsName = pNode->JSObject()->GetCData(XFA_Attribute::Name);
      WideString wsInstName = JSObject()->GetCData(XFA_Attribute::Name);
      if (wsInstName.GetLength() < 1 || wsInstName[0] != '_' ||
          wsInstName.Last(wsInstName.GetLength() - 1) != wsName) {
        return iCount;
      }
      dwNameHash = pNode->GetNameHash();
    }
    if (dwNameHash != pNode->GetNameHash())
      break;

    iCount++;
  }
  return iCount;
}

void CXFA_Node::InsertItem(CXFA_Node* pNewInstance,
                           int32_t iPos,
                           int32_t iCount,
                           bool bMoveDataBindingNodes) {
  if (iCount < 0)
    iCount = GetCount();
  if (iPos < 0)
    iPos = iCount;
  if (iPos == iCount) {
    CXFA_Node* item = GetItemIfExists(iCount - 1);
    if (!item)
      return;

    CXFA_Node* pNextSibling =
        iCount > 0 ? item->GetNextSibling() : GetNextSibling();
    GetParent()->InsertChildAndNotify(pNewInstance, pNextSibling);
    if (bMoveDataBindingNodes) {
      NodeSet sNew;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorNew(pNewInstance);
      for (CXFA_Node* pNode = sIteratorNew.GetCurrent(); pNode;
           pNode = sIteratorNew.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (pDataNode)
          sNew.insert(pDataNode);
      }
      NodeSet sAfter;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorAfter(pNextSibling);
      for (CXFA_Node* pNode = sIteratorAfter.GetCurrent(); pNode;
           pNode = sIteratorAfter.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (pDataNode)
          sAfter.insert(pDataNode);
      }
      ReorderDataNodes(sNew, sAfter, false);
    }
  } else {
    CXFA_Node* pBeforeInstance = GetItemIfExists(iPos);
    if (!pBeforeInstance) {
      // TODO(dsinclair): What should happen here?
      return;
    }

    GetParent()->InsertChildAndNotify(pNewInstance, pBeforeInstance);
    if (bMoveDataBindingNodes) {
      NodeSet sNew;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorNew(pNewInstance);
      for (CXFA_Node* pNode = sIteratorNew.GetCurrent(); pNode;
           pNode = sIteratorNew.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (pDataNode)
          sNew.insert(pDataNode);
      }
      NodeSet sBefore;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorBefore(pBeforeInstance);
      for (CXFA_Node* pNode = sIteratorBefore.GetCurrent(); pNode;
           pNode = sIteratorBefore.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (pDataNode)
          sBefore.insert(pDataNode);
      }
      ReorderDataNodes(sNew, sBefore, true);
    }
  }
}

void CXFA_Node::RemoveItem(CXFA_Node* pRemoveInstance,
                           bool bRemoveDataBinding) {
  GetParent()->RemoveChildAndNotify(pRemoveInstance, true);
  if (!bRemoveDataBinding)
    return;

  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>
      sIterator(pRemoveInstance);
  for (CXFA_Node* pFormNode = sIterator.GetCurrent(); pFormNode;
       pFormNode = sIterator.MoveToNext()) {
    CXFA_Node* pDataNode = pFormNode->GetBindData();
    if (!pDataNode)
      continue;

    if (!pDataNode->RemoveBindItem(pFormNode)) {
      if (CXFA_Node* pDataParent = pDataNode->GetParent()) {
        pDataParent->RemoveChildAndNotify(pDataNode, true);
      }
    }
    pFormNode->SetBindingNode(nullptr);
  }
}

CXFA_Node* CXFA_Node::CreateInstanceIfPossible(bool bDataMerge) {
  CXFA_Document* pDocument = GetDocument();
  CXFA_Node* pTemplateNode = GetTemplateNodeIfExists();
  if (!pTemplateNode)
    return nullptr;

  CXFA_Node* pFormParent = GetParent();
  CXFA_Node* pDataScope = nullptr;
  for (CXFA_Node* pRootBoundNode = pFormParent;
       pRootBoundNode && pRootBoundNode->IsContainerNode();
       pRootBoundNode = pRootBoundNode->GetParent()) {
    pDataScope = pRootBoundNode->GetBindData();
    if (pDataScope)
      break;
  }
  if (!pDataScope) {
    pDataScope = ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Record));
    DCHECK(pDataScope);
  }

  CXFA_Node* pInstance = pDocument->DataMerge_CopyContainer(
      pTemplateNode, pFormParent, pDataScope, true, bDataMerge, true);
  if (pInstance) {
    pDocument->DataMerge_UpdateBindingRelations(pInstance);
    pFormParent->RemoveChildAndNotify(pInstance, true);
  }
  return pInstance;
}

absl::optional<bool> CXFA_Node::GetDefaultBoolean(XFA_Attribute attr) const {
  absl::optional<void*> value =
      GetDefaultValue(attr, XFA_AttributeType::Boolean);
  if (!value.has_value())
    return absl::nullopt;
  return !!value.value();
}

absl::optional<int32_t> CXFA_Node::GetDefaultInteger(XFA_Attribute attr) const {
  absl::optional<void*> value =
      GetDefaultValue(attr, XFA_AttributeType::Integer);
  if (!value.has_value())
    return absl::nullopt;
  return static_cast<int32_t>(reinterpret_cast<uintptr_t>(value.value()));
}

absl::optional<CXFA_Measurement> CXFA_Node::GetDefaultMeasurement(
    XFA_Attribute attr) const {
  absl::optional<void*> value =
      GetDefaultValue(attr, XFA_AttributeType::Measure);
  if (!value.has_value())
    return absl::nullopt;

  WideString str = WideString(static_cast<const wchar_t*>(value.value()));
  return CXFA_Measurement(str.AsStringView());
}

absl::optional<WideString> CXFA_Node::GetDefaultCData(
    XFA_Attribute attr) const {
  absl::optional<void*> value = GetDefaultValue(attr, XFA_AttributeType::CData);
  if (!value.has_value())
    return absl::nullopt;

  return WideString(static_cast<const wchar_t*>(value.value()));
}

absl::optional<XFA_AttributeValue> CXFA_Node::GetDefaultEnum(
    XFA_Attribute attr) const {
  absl::optional<void*> value = GetDefaultValue(attr, XFA_AttributeType::Enum);
  if (!value.has_value())
    return absl::nullopt;
  return static_cast<XFA_AttributeValue>(
      reinterpret_cast<uintptr_t>(value.value()));
}

absl::optional<void*> CXFA_Node::GetDefaultValue(
    XFA_Attribute attr,
    XFA_AttributeType eType) const {
  const AttributeData* data = GetAttributeData(attr);
  if (!data || data->type != eType)
    return absl::nullopt;
  return data->default_value;
}

void CXFA_Node::SendAttributeChangeMessage(XFA_Attribute eAttribute,
                                           bool bScriptModify) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (GetPacketType() != XFA_PacketType::Form) {
    pNotify->OnValueChanged(this, eAttribute, this, this);
    return;
  }

  bool bNeedFindContainer = false;
  switch (GetElementType()) {
    case XFA_Element::Caption:
      bNeedFindContainer = true;
      pNotify->OnValueChanged(this, eAttribute, this, GetParent());
      break;
    case XFA_Element::Font:
    case XFA_Element::Para: {
      bNeedFindContainer = true;
      CXFA_Node* pParentNode = GetParent();
      if (pParentNode->GetElementType() == XFA_Element::Caption) {
        pNotify->OnValueChanged(this, eAttribute, pParentNode,
                                pParentNode->GetParent());
      } else {
        pNotify->OnValueChanged(this, eAttribute, this, pParentNode);
      }
      break;
    }
    case XFA_Element::Margin: {
      bNeedFindContainer = true;
      CXFA_Node* pParentNode = GetParent();
      XFA_Element eParentType = pParentNode->GetElementType();
      if (pParentNode->IsContainerNode()) {
        pNotify->OnValueChanged(this, eAttribute, this, pParentNode);
      } else if (eParentType == XFA_Element::Caption) {
        pNotify->OnValueChanged(this, eAttribute, pParentNode,
                                pParentNode->GetParent());
      } else {
        CXFA_Node* pNode = pParentNode->GetParent();
        if (pNode && pNode->GetElementType() == XFA_Element::Ui)
          pNotify->OnValueChanged(this, eAttribute, pNode, pNode->GetParent());
      }
      break;
    }
    case XFA_Element::Comb: {
      CXFA_Node* pEditNode = GetParent();
      XFA_Element eUIType = pEditNode->GetElementType();
      if (pEditNode && (eUIType == XFA_Element::DateTimeEdit ||
                        eUIType == XFA_Element::NumericEdit ||
                        eUIType == XFA_Element::TextEdit)) {
        CXFA_Node* pUINode = pEditNode->GetParent();
        if (pUINode) {
          pNotify->OnValueChanged(this, eAttribute, pUINode,
                                  pUINode->GetParent());
        }
      }
      break;
    }
    case XFA_Element::Button:
    case XFA_Element::Barcode:
    case XFA_Element::ChoiceList:
    case XFA_Element::DateTimeEdit:
    case XFA_Element::NumericEdit:
    case XFA_Element::PasswordEdit:
    case XFA_Element::TextEdit: {
      CXFA_Node* pUINode = GetParent();
      if (pUINode) {
        pNotify->OnValueChanged(this, eAttribute, pUINode,
                                pUINode->GetParent());
      }
      break;
    }
    case XFA_Element::CheckButton: {
      bNeedFindContainer = true;
      CXFA_Node* pUINode = GetParent();
      if (pUINode) {
        pNotify->OnValueChanged(this, eAttribute, pUINode,
                                pUINode->GetParent());
      }
      break;
    }
    case XFA_Element::Keep:
    case XFA_Element::Bookend:
    case XFA_Element::Break:
    case XFA_Element::BreakAfter:
    case XFA_Element::BreakBefore:
    case XFA_Element::Overflow:
      bNeedFindContainer = true;
      break;
    case XFA_Element::Area:
    case XFA_Element::Draw:
    case XFA_Element::ExclGroup:
    case XFA_Element::Field:
    case XFA_Element::Subform:
    case XFA_Element::SubformSet:
      pNotify->OnContainerChanged();
      pNotify->OnValueChanged(this, eAttribute, this, this);
      break;
    case XFA_Element::Sharptext:
    case XFA_Element::Sharpxml:
    case XFA_Element::SharpxHTML: {
      CXFA_Node* pTextNode = GetParent();
      if (!pTextNode)
        return;

      CXFA_Node* pValueNode = pTextNode->GetParent();
      if (!pValueNode)
        return;

      XFA_Element eType = pValueNode->GetElementType();
      if (eType == XFA_Element::Value) {
        bNeedFindContainer = true;
        CXFA_Node* pNode = pValueNode->GetParent();
        if (pNode && pNode->IsContainerNode()) {
          if (bScriptModify)
            pValueNode = pNode;

          pNotify->OnValueChanged(this, eAttribute, pValueNode, pNode);
        } else {
          pNotify->OnValueChanged(this, eAttribute, pNode, pNode->GetParent());
        }
      } else {
        if (eType == XFA_Element::Items) {
          CXFA_Node* pNode = pValueNode->GetParent();
          if (pNode && pNode->IsContainerNode()) {
            pNotify->OnValueChanged(this, eAttribute, pValueNode, pNode);
          }
        }
      }
      break;
    }
    default:
      break;
  }

  if (!bNeedFindContainer)
    return;

  CXFA_Node* pParent = this;
  while (pParent && !pParent->IsContainerNode())
    pParent = pParent->GetParent();

  if (pParent)
    pNotify->OnContainerChanged();
}

void CXFA_Node::SyncValue(const WideString& wsValue, bool bNotify) {
  WideString wsFormatValue = wsValue;
  CXFA_Node* pContainerNode = GetContainerNode();
  if (pContainerNode)
    wsFormatValue = pContainerNode->GetFormatDataValue(wsValue);

  JSObject()->SetContent(wsValue, wsFormatValue, bNotify, false, true);
}

WideString CXFA_Node::GetRawValue() const {
  return JSObject()->GetContent(false);
}

int32_t CXFA_Node::GetRotate() const {
  absl::optional<int32_t> degrees =
      JSObject()->TryInteger(XFA_Attribute::Rotate, false);
  return degrees.has_value() ? XFA_MapRotation(degrees.value()) / 90 * 90 : 0;
}

CXFA_Border* CXFA_Node::GetBorderIfExists() const {
  return JSObject()->GetProperty<CXFA_Border>(0, XFA_Element::Border);
}

CXFA_Border* CXFA_Node::GetOrCreateBorderIfPossible() {
  return JSObject()->GetOrCreateProperty<CXFA_Border>(0, XFA_Element::Border);
}

CXFA_Caption* CXFA_Node::GetCaptionIfExists() const {
  return JSObject()->GetProperty<CXFA_Caption>(0, XFA_Element::Caption);
}

CXFA_Font* CXFA_Node::GetOrCreateFontIfPossible() {
  return JSObject()->GetOrCreateProperty<CXFA_Font>(0, XFA_Element::Font);
}

CXFA_Font* CXFA_Node::GetFontIfExists() const {
  return JSObject()->GetProperty<CXFA_Font>(0, XFA_Element::Font);
}

float CXFA_Node::GetFontSize() const {
  CXFA_Font* font = GetFontIfExists();
  float fFontSize = font ? font->GetFontSize() : 10.0f;
  return fFontSize < 0.1f ? 10.0f : fFontSize;
}

float CXFA_Node::GetLineHeight() const {
  float fLineHeight = 0;
  CXFA_Para* para = GetParaIfExists();
  if (para)
    fLineHeight = para->GetLineHeight();

  if (fLineHeight < 1)
    fLineHeight = GetFontSize() * 1.2f;
  return fLineHeight;
}

FX_ARGB CXFA_Node::GetTextColor() const {
  CXFA_Font* font = GetFontIfExists();
  return font ? font->GetColor() : 0xFF000000;
}

CXFA_Margin* CXFA_Node::GetMarginIfExists() const {
  return JSObject()->GetProperty<CXFA_Margin>(0, XFA_Element::Margin);
}

CXFA_Para* CXFA_Node::GetParaIfExists() const {
  return JSObject()->GetProperty<CXFA_Para>(0, XFA_Element::Para);
}

bool CXFA_Node::IsOpenAccess() const {
  for (auto* pNode = this; pNode; pNode = pNode->GetContainerParent()) {
    XFA_AttributeValue iAcc = pNode->JSObject()->GetEnum(XFA_Attribute::Access);
    if (iAcc != XFA_AttributeValue::Open)
      return false;
  }
  return true;
}

CXFA_Value* CXFA_Node::GetDefaultValueIfExists() {
  CXFA_Node* pTemNode = GetTemplateNodeIfExists();
  return pTemNode ? pTemNode->JSObject()->GetProperty<CXFA_Value>(
                        0, XFA_Element::Value)
                  : nullptr;
}

CXFA_Value* CXFA_Node::GetFormValueIfExists() const {
  return JSObject()->GetProperty<CXFA_Value>(0, XFA_Element::Value);
}

CXFA_Calculate* CXFA_Node::GetCalculateIfExists() const {
  return JSObject()->GetProperty<CXFA_Calculate>(0, XFA_Element::Calculate);
}

CXFA_Validate* CXFA_Node::GetValidateIfExists() const {
  return JSObject()->GetProperty<CXFA_Validate>(0, XFA_Element::Validate);
}

CXFA_Validate* CXFA_Node::GetOrCreateValidateIfPossible() {
  return JSObject()->GetOrCreateProperty<CXFA_Validate>(0,
                                                        XFA_Element::Validate);
}

CXFA_Bind* CXFA_Node::GetBindIfExists() const {
  return JSObject()->GetProperty<CXFA_Bind>(0, XFA_Element::Bind);
}

absl::optional<XFA_AttributeValue> CXFA_Node::GetIntactFromKeep(
    const CXFA_Keep* pKeep,
    XFA_AttributeValue eLayoutType) const {
  absl::optional<XFA_AttributeValue> intact =
      pKeep->JSObject()->TryEnum(XFA_Attribute::Intact, false);
  if (!intact.has_value())
    return absl::nullopt;

  if (intact.value() != XFA_AttributeValue::None ||
      eLayoutType != XFA_AttributeValue::Row ||
      m_pDocument->GetCurVersionMode() >= XFA_VERSION_208) {
    return intact;
  }

  CXFA_Node* pPreviewRow = GetPrevContainerSibling();
  if (!pPreviewRow || pPreviewRow->JSObject()->GetEnum(XFA_Attribute::Layout) !=
                          XFA_AttributeValue::Row) {
    return intact;
  }

  absl::optional<XFA_AttributeValue> value =
      pKeep->JSObject()->TryEnum(XFA_Attribute::Previous, false);
  if (value == XFA_AttributeValue::ContentArea ||
      value == XFA_AttributeValue::PageArea) {
    return XFA_AttributeValue::ContentArea;
  }

  CXFA_Keep* pNode =
      pPreviewRow->GetFirstChildByClass<CXFA_Keep>(XFA_Element::Keep);
  if (!pNode)
    return intact;

  absl::optional<XFA_AttributeValue> ret =
      pNode->JSObject()->TryEnum(XFA_Attribute::Next, false);
  if (ret == XFA_AttributeValue::ContentArea ||
      ret == XFA_AttributeValue::PageArea) {
    return XFA_AttributeValue::ContentArea;
  }
  return intact;
}

absl::optional<float> CXFA_Node::TryWidth() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::W);
}

absl::optional<float> CXFA_Node::TryHeight() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::H);
}

absl::optional<float> CXFA_Node::TryMinWidth() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::MinW);
}

absl::optional<float> CXFA_Node::TryMinHeight() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::MinH);
}

absl::optional<float> CXFA_Node::TryMaxWidth() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::MaxW);
}

absl::optional<float> CXFA_Node::TryMaxHeight() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::MaxH);
}

CXFA_Node* CXFA_Node::GetExclGroupIfExists() {
  CXFA_Node* pExcl = GetParent();
  if (!pExcl || pExcl->GetElementType() != XFA_Element::ExclGroup)
    return nullptr;
  return pExcl;
}

XFA_EventError CXFA_Node::ProcessEvent(CXFA_FFDocView* pDocView,
                                       XFA_AttributeValue iActivity,
                                       CXFA_EventParam* pEventParam) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EventError::kNotExist;

  std::vector<CXFA_Event*> eventArray =
      GetEventByActivity(iActivity, pEventParam->m_bIsFormReady);
  bool first = true;
  XFA_EventError iRet = XFA_EventError::kNotExist;
  for (CXFA_Event* event : eventArray) {
    XFA_EventError result =
        ProcessEventInternal(pDocView, iActivity, event, pEventParam);
    if (first || result == XFA_EventError::kSuccess)
      iRet = result;
    first = false;
  }
  return iRet;
}

XFA_EventError CXFA_Node::ProcessEventInternal(CXFA_FFDocView* pDocView,
                                               XFA_AttributeValue iActivity,
                                               CXFA_Event* event,
                                               CXFA_EventParam* pEventParam) {
  if (!event)
    return XFA_EventError::kNotExist;

  switch (event->GetEventType()) {
    case XFA_Element::Execute:
      break;
    case XFA_Element::Script:
      if (iActivity == XFA_AttributeValue::DocClose) {
        // Too late, scripting engine already gone.
        return XFA_EventError::kNotExist;
      }
      return ExecuteScript(pDocView, event->GetScriptIfExists(), pEventParam);
    case XFA_Element::SignData:
      break;
    case XFA_Element::Submit: {
// TODO(crbug.com/867485): Submit is disabled for now. Fix it and reenable this
// code.
#ifdef PDF_XFA_ELEMENT_SUBMIT_ENABLED
      CXFA_Submit* submit = event->GetSubmitIfExists();
      if (!submit)
        return XFA_EventError::kNotExist;
      return pDocView->GetDoc()->GetDocEnvironment()->Submit(pDocView->GetDoc(),
                                                             submit);
#else
      return XFA_EventError::kDisabled;
#endif  // PDF_XFA_ELEMENT_SUBMIT_ENABLED
    }
    default:
      break;
  }
  return XFA_EventError::kNotExist;
}

XFA_EventError CXFA_Node::ProcessCalculate(CXFA_FFDocView* pDocView) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EventError::kNotExist;

  CXFA_Calculate* calc = GetCalculateIfExists();
  if (!calc)
    return XFA_EventError::kNotExist;
  if (IsUserInteractive())
    return XFA_EventError::kDisabled;

  CXFA_EventParam EventParam;
  EventParam.m_eType = XFA_EVENT_Calculate;
  EventParam.m_bTargeted = false;
  XFA_EventError iRet =
      ExecuteScript(pDocView, calc->GetScriptIfExists(), &EventParam);
  if (iRet != XFA_EventError::kSuccess)
    return iRet;

  if (GetRawValue() != EventParam.m_wsResult) {
    SetValue(XFA_ValuePicture::kRaw, EventParam.m_wsResult);
    pDocView->UpdateUIDisplay(this, nullptr);
  }
  return XFA_EventError::kSuccess;
}

void CXFA_Node::ProcessScriptTestValidate(CXFA_FFDocView* pDocView,
                                          CXFA_Validate* validate,
                                          bool bVersionFlag) {
  CXFA_FFApp::CallbackIface* pAppProvider =
      pDocView->GetDoc()->GetApp()->GetAppProvider();
  if (!pAppProvider)
    return;

  WideString wsTitle = pAppProvider->GetAppTitle();
  WideString wsScriptMsg = validate->GetScriptMessageText();
  if (validate->GetScriptTest() == XFA_AttributeValue::Warning) {
    if (IsUserInteractive())
      return;
    if (wsScriptMsg.IsEmpty())
      wsScriptMsg = GetValidateMessage(false, bVersionFlag);

    if (bVersionFlag) {
      pAppProvider->MsgBox(wsScriptMsg, wsTitle,
                           static_cast<uint32_t>(AlertIcon::kWarning),
                           static_cast<uint32_t>(AlertButton::kOK));
      return;
    }
    if (pAppProvider->MsgBox(wsScriptMsg, wsTitle,
                             static_cast<uint32_t>(AlertIcon::kWarning),
                             static_cast<uint32_t>(AlertButton::kYesNo)) ==
        static_cast<uint32_t>(AlertReturn::kYes)) {
      SetFlag(XFA_NodeFlag::kUserInteractive);
    }
    return;
  }

  if (wsScriptMsg.IsEmpty())
    wsScriptMsg = GetValidateMessage(true, bVersionFlag);
  pAppProvider->MsgBox(wsScriptMsg, wsTitle,
                       static_cast<uint32_t>(AlertIcon::kError),
                       static_cast<uint32_t>(AlertButton::kOK));
}

XFA_EventError CXFA_Node::ProcessFormatTestValidate(CXFA_FFDocView* pDocView,
                                                    CXFA_Validate* validate,
                                                    bool bVersionFlag) {
  WideString wsPicture = validate->GetPicture();
  if (wsPicture.IsEmpty())
    return XFA_EventError::kNotExist;

  WideString wsRawValue = GetRawValue();
  if (wsRawValue.IsEmpty())
    return XFA_EventError::kError;

  GCedLocaleIface* pLocale = GetLocale();
  if (!pLocale)
    return XFA_EventError::kNotExist;

  CXFA_LocaleValue lcValue = XFA_GetLocaleValue(this);
  if (lcValue.ValidateValue(lcValue.GetValue(), wsPicture, pLocale, nullptr))
    return XFA_EventError::kSuccess;

  CXFA_FFApp::CallbackIface* pAppProvider =
      pDocView->GetDoc()->GetApp()->GetAppProvider();
  if (!pAppProvider)
    return XFA_EventError::kNotExist;

  WideString wsFormatMsg = validate->GetFormatMessageText();
  WideString wsTitle = pAppProvider->GetAppTitle();
  if (validate->GetFormatTest() == XFA_AttributeValue::Error) {
    if (wsFormatMsg.IsEmpty())
      wsFormatMsg = GetValidateMessage(true, bVersionFlag);
    pAppProvider->MsgBox(wsFormatMsg, wsTitle,
                         static_cast<uint32_t>(AlertIcon::kError),
                         static_cast<uint32_t>(AlertButton::kOK));
    return XFA_EventError::kError;
  }

  if (wsFormatMsg.IsEmpty())
    wsFormatMsg = GetValidateMessage(false, bVersionFlag);

  if (bVersionFlag) {
    pAppProvider->MsgBox(wsFormatMsg, wsTitle,
                         static_cast<uint32_t>(AlertIcon::kWarning),
                         static_cast<uint32_t>(AlertButton::kOK));
    return XFA_EventError::kError;
  }

  if (pAppProvider->MsgBox(wsFormatMsg, wsTitle,
                           static_cast<uint32_t>(AlertIcon::kWarning),
                           static_cast<uint32_t>(AlertButton::kYesNo)) ==
      static_cast<uint32_t>(AlertReturn::kYes)) {
    SetFlag(XFA_NodeFlag::kUserInteractive);
  }

  return XFA_EventError::kError;
}

XFA_EventError CXFA_Node::ProcessNullTestValidate(CXFA_FFDocView* pDocView,
                                                  CXFA_Validate* validate,
                                                  int32_t iFlags,
                                                  bool bVersionFlag) {
  if (!GetValue(XFA_ValuePicture::kRaw).IsEmpty())
    return XFA_EventError::kSuccess;
  if (m_bIsNull && m_bPreNull)
    return XFA_EventError::kSuccess;

  XFA_AttributeValue eNullTest = validate->GetNullTest();
  WideString wsNullMsg = validate->GetNullMessageText();
  if (iFlags & 0x01) {
    if (eNullTest == XFA_AttributeValue::Disabled)
      return XFA_EventError::kSuccess;

    if (!wsNullMsg.IsEmpty())
      pDocView->AddNullTestMsg(wsNullMsg);
    return XFA_EventError::kError;
  }
  if (wsNullMsg.IsEmpty() && bVersionFlag &&
      eNullTest != XFA_AttributeValue::Disabled) {
    return XFA_EventError::kError;
  }
  CXFA_FFApp::CallbackIface* pAppProvider =
      pDocView->GetDoc()->GetApp()->GetAppProvider();
  if (!pAppProvider)
    return XFA_EventError::kNotExist;

  WideString wsCaptionName;
  WideString wsTitle = pAppProvider->GetAppTitle();
  switch (eNullTest) {
    case XFA_AttributeValue::Error: {
      if (wsNullMsg.IsEmpty()) {
        wsCaptionName = GetValidateCaptionName(bVersionFlag);
        wsNullMsg = wsCaptionName + L" cannot be blank.";
      }
      pAppProvider->MsgBox(wsNullMsg, wsTitle,
                           static_cast<uint32_t>(AlertIcon::kStatus),
                           static_cast<uint32_t>(AlertButton::kOK));
      return XFA_EventError::kError;
    }
    case XFA_AttributeValue::Warning: {
      if (IsUserInteractive())
        return XFA_EventError::kSuccess;

      if (wsNullMsg.IsEmpty()) {
        wsCaptionName = GetValidateCaptionName(bVersionFlag);
        wsNullMsg = wsCaptionName +
                    L" cannot be blank. To ignore validations for " +
                    wsCaptionName + L", click Ignore.";
      }
      if (pAppProvider->MsgBox(wsNullMsg, wsTitle,
                               static_cast<uint32_t>(AlertIcon::kWarning),
                               static_cast<uint32_t>(AlertButton::kYesNo)) ==
          static_cast<uint32_t>(AlertReturn::kYes)) {
        SetFlag(XFA_NodeFlag::kUserInteractive);
      }
      return XFA_EventError::kError;
    }
    case XFA_AttributeValue::Disabled:
    default:
      break;
  }
  return XFA_EventError::kSuccess;
}

XFA_EventError CXFA_Node::ProcessValidate(CXFA_FFDocView* pDocView,
                                          int32_t iFlags) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EventError::kNotExist;

  CXFA_Validate* validate = GetValidateIfExists();
  if (!validate)
    return XFA_EventError::kNotExist;

  const bool bInitDoc = validate->NeedsInitApp();
  const bool bStatus =
      pDocView->GetLayoutStatus() != CXFA_FFDocView::LayoutStatus::kEnd;

  XFA_EventError iFormat = XFA_EventError::kNotExist;
  XFA_EventError iRet = XFA_EventError::kNotExist;
  CXFA_Script* script = validate->GetScriptIfExists();
  bool bRet = false;
  bool hasBoolResult = (bInitDoc || bStatus) && GetRawValue().IsEmpty();
  if (script) {
    CXFA_EventParam eParam;
    eParam.m_eType = XFA_EVENT_Validate;
    std::tie(iRet, bRet) = ExecuteBoolScript(pDocView, script, &eParam);
  }

  XFA_VERSION version = pDocView->GetDoc()->GetXFADoc()->GetCurVersionMode();
  bool bVersionFlag = version < XFA_VERSION_208;

  if (bInitDoc) {
    validate->ClearFlag(XFA_NodeFlag::kNeedsInitApp);
  } else {
    iFormat = ProcessFormatTestValidate(pDocView, validate, bVersionFlag);
    if (!bVersionFlag)
      bVersionFlag = pDocView->GetDoc()->GetXFADoc()->is_scripting();
    XFA_EventErrorAccumulate(
        &iRet,
        ProcessNullTestValidate(pDocView, validate, iFlags, bVersionFlag));
  }
  if (iRet == XFA_EventError::kSuccess && iFormat != XFA_EventError::kSuccess &&
      hasBoolResult && !bRet) {
    ProcessScriptTestValidate(pDocView, validate, bVersionFlag);
  }
  XFA_EventErrorAccumulate(&iRet, iFormat);
  return iRet;
}

WideString CXFA_Node::GetValidateCaptionName(bool bVersionFlag) {
  WideString wsCaptionName;

  if (!bVersionFlag) {
    CXFA_Caption* caption = GetCaptionIfExists();
    if (caption) {
      CXFA_Value* capValue = caption->GetValueIfExists();
      if (capValue) {
        CXFA_Text* captionText = capValue->GetTextIfExists();
        if (captionText)
          wsCaptionName = captionText->GetContent();
      }
    }
  }
  if (!wsCaptionName.IsEmpty())
    return wsCaptionName;
  return JSObject()->GetCData(XFA_Attribute::Name);
}

WideString CXFA_Node::GetValidateMessage(bool bError, bool bVersionFlag) {
  WideString wsCaptionName = GetValidateCaptionName(bVersionFlag);
  if (bVersionFlag)
    return wsCaptionName + L" validation failed";
  WideString result =
      L"The value you entered for " + wsCaptionName + L" is invalid.";
  if (!bError) {
    result +=
        L" To ignore validations for " + wsCaptionName + L", click Ignore.";
  }
  return result;
}

XFA_EventError CXFA_Node::ExecuteScript(CXFA_FFDocView* pDocView,
                                        CXFA_Script* script,
                                        CXFA_EventParam* pEventParam) {
  return ExecuteBoolScript(pDocView, script, pEventParam).first;
}

std::pair<XFA_EventError, bool> CXFA_Node::ExecuteBoolScript(
    CXFA_FFDocView* pDocView,
    CXFA_Script* script,
    CXFA_EventParam* pEventParam) {
  if (m_ExecuteRecursionDepth > kMaxExecuteRecursion)
    return {XFA_EventError::kSuccess, false};

  DCHECK(pEventParam);
  if (!script)
    return {XFA_EventError::kNotExist, false};
  if (script->GetRunAt() == XFA_AttributeValue::Server)
    return {XFA_EventError::kDisabled, false};

  WideString wsExpression = script->GetExpression();
  if (wsExpression.IsEmpty())
    return {XFA_EventError::kNotExist, false};

  CXFA_Script::Type eScriptType = script->GetContentType();
  if (eScriptType == CXFA_Script::Type::Unknown)
    return {XFA_EventError::kSuccess, false};

  CXFA_FFDoc* pDoc = pDocView->GetDoc();
  CFXJSE_Engine* pContext = pDoc->GetXFADoc()->GetScriptContext();
  CFXJSE_Engine::EventParamScope paramScope(
      pContext, pEventParam->m_bTargeted ? this : nullptr, pEventParam);
  pContext->SetRunAtType(script->GetRunAt());

  std::vector<cppgc::Persistent<CXFA_Node>> refNodes;
  if (pEventParam->m_eType == XFA_EVENT_InitCalculate ||
      pEventParam->m_eType == XFA_EVENT_Calculate) {
    pContext->SetNodesOfRunScript(&refNodes);
  }

  auto pTmpRetValue = std::make_unique<CFXJSE_Value>();
  bool bRet = false;
  {
    AutoRestorer<uint8_t> restorer(&m_ExecuteRecursionDepth);
    ++m_ExecuteRecursionDepth;
    bRet = pContext->RunScript(eScriptType, wsExpression.AsStringView(),
                               pTmpRetValue.get(), this);
  }

  XFA_EventError iRet = XFA_EventError::kError;
  if (bRet) {
    iRet = XFA_EventError::kSuccess;
    if (pEventParam->m_eType == XFA_EVENT_Calculate ||
        pEventParam->m_eType == XFA_EVENT_InitCalculate) {
      if (!pTmpRetValue->IsUndefined(pContext->GetIsolate())) {
        if (!pTmpRetValue->IsNull(pContext->GetIsolate()))
          pEventParam->m_wsResult =
              pTmpRetValue->ToWideString(pContext->GetIsolate());

        iRet = XFA_EventError::kSuccess;
      } else {
        iRet = XFA_EventError::kError;
      }
      if (pEventParam->m_eType == XFA_EVENT_InitCalculate) {
        if ((iRet == XFA_EventError::kSuccess) &&
            (GetRawValue() != pEventParam->m_wsResult)) {
          SetValue(XFA_ValuePicture::kRaw, pEventParam->m_wsResult);
          pDocView->AddValidateNode(this);
        }
      }
      for (CXFA_Node* pRefNode : refNodes) {
        if (pRefNode == this)
          continue;

        CJX_Object::CalcData* pGlobalData =
            pRefNode->JSObject()->GetOrCreateCalcData(pDoc->GetHeap());
        if (!pdfium::Contains(pGlobalData->m_Globals, this))
          pGlobalData->m_Globals.push_back(this);
      }
    }
  }
  pContext->SetNodesOfRunScript(nullptr);

  return {iRet, pTmpRetValue->IsBoolean(pContext->GetIsolate()) &&
                    pTmpRetValue->ToBoolean(pContext->GetIsolate())};
}

std::pair<XFA_FFWidgetType, CXFA_Ui*>
CXFA_Node::CreateChildUIAndValueNodesIfNeeded() {
  XFA_Element eType = GetElementType();
  DCHECK(eType == XFA_Element::Field || eType == XFA_Element::Draw);

  // Both Field and Draw have a UI property. We should always be able to
  // retrieve or create the UI element. If we can't something is wrong.
  CXFA_Ui* pUI = JSObject()->GetOrCreateProperty<CXFA_Ui>(0, XFA_Element::Ui);
  DCHECK(pUI);

  CXFA_Node* pUIChild = nullptr;
  // Search through the children of the UI node to see if we have any of our
  // One-Of entries. If so, that is the node associated with our UI.
  for (CXFA_Node* pChild = pUI->GetFirstChild(); pChild;
       pChild = pChild->GetNextSibling()) {
    if (pUI->IsAOneOfChild(pChild)) {
      pUIChild = pChild;
      break;
    }
  }

  XFA_FFWidgetType widget_type = XFA_FFWidgetType::kNone;
  XFA_Element expected_ui_child_type = XFA_Element::Unknown;

  // Both Field and Draw nodes have a Value child. So, we should either always
  // have it, or always create it. If we don't get the Value child for some
  // reason something has gone really wrong.
  CXFA_Value* value =
      JSObject()->GetOrCreateProperty<CXFA_Value>(0, XFA_Element::Value);
  DCHECK(value);

  // The Value nodes only have One-Of children. So, if we have a first child
  // that child must be the type we want to use.
  CXFA_Node* child = value->GetFirstChild();
  if (child) {
    switch (child->GetElementType()) {
      case XFA_Element::Boolean:
        expected_ui_child_type = XFA_Element::CheckButton;
        break;
      case XFA_Element::Integer:
      case XFA_Element::Decimal:
      case XFA_Element::Float:
        expected_ui_child_type = XFA_Element::NumericEdit;
        break;
      case XFA_Element::ExData:
      case XFA_Element::Text:
        expected_ui_child_type = XFA_Element::TextEdit;
        widget_type = XFA_FFWidgetType::kText;
        break;
      case XFA_Element::Date:
      case XFA_Element::Time:
      case XFA_Element::DateTime:
        expected_ui_child_type = XFA_Element::DateTimeEdit;
        break;
      case XFA_Element::Image:
        expected_ui_child_type = XFA_Element::ImageEdit;
        widget_type = XFA_FFWidgetType::kImage;
        break;
      case XFA_Element::Arc:
        expected_ui_child_type = XFA_Element::DefaultUi;
        widget_type = XFA_FFWidgetType::kArc;
        break;
      case XFA_Element::Line:
        expected_ui_child_type = XFA_Element::DefaultUi;
        widget_type = XFA_FFWidgetType::kLine;
        break;
      case XFA_Element::Rectangle:
        expected_ui_child_type = XFA_Element::DefaultUi;
        widget_type = XFA_FFWidgetType::kRectangle;
        break;
      default:
        break;
    }
  }

  if (eType == XFA_Element::Draw) {
    if (pUIChild && pUIChild->GetElementType() == XFA_Element::TextEdit) {
      widget_type = XFA_FFWidgetType::kText;
    } else if (pUIChild &&
               pUIChild->GetElementType() == XFA_Element::ImageEdit) {
      widget_type = XFA_FFWidgetType::kImage;
    } else if (widget_type == XFA_FFWidgetType::kNone) {
      widget_type = XFA_FFWidgetType::kText;
    }
  } else if (eType == XFA_Element::Field) {
    if (pUIChild && pUIChild->GetElementType() == XFA_Element::DefaultUi) {
      widget_type = XFA_FFWidgetType::kTextEdit;
    } else if (pUIChild) {
      widget_type = pUIChild->GetDefaultFFWidgetType();
    } else if (expected_ui_child_type == XFA_Element::Unknown) {
      widget_type = XFA_FFWidgetType::kTextEdit;
    }
  } else {
    NOTREACHED_NORETURN();
  }

  if (!pUIChild) {
    if (expected_ui_child_type == XFA_Element::Unknown)
      expected_ui_child_type = XFA_Element::TextEdit;
    pUIChild = pUI->JSObject()->GetOrCreateProperty<CXFA_Node>(
        0, expected_ui_child_type);
  }

  CreateValueNodeIfNeeded(value, pUIChild);
  return {widget_type, pUI};
}

XFA_FFWidgetType CXFA_Node::GetDefaultFFWidgetType() const {
  NOTREACHED_NORETURN();
}

CXFA_Node* CXFA_Node::CreateUINodeIfNeeded(CXFA_Ui* ui, XFA_Element type) {
  return ui->JSObject()->GetOrCreateProperty<CXFA_Node>(0, type);
}

void CXFA_Node::CreateValueNodeIfNeeded(CXFA_Value* value,
                                        CXFA_Node* pUIChild) {
  // Value nodes only have one child. If we have one already we're done.
  if (value->GetFirstChild())
    return;

  // Create the Value node for our UI if needed.
  XFA_Element valueType = pUIChild->GetValueNodeType();
  if (pUIChild->GetElementType() == XFA_Element::CheckButton) {
    CXFA_Items* pItems = GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (pItems) {
      CXFA_Node* pItem =
          pItems->GetChild<CXFA_Node>(0, XFA_Element::Unknown, false);
      if (pItem)
        valueType = pItem->GetElementType();
    }
  }
  value->JSObject()->GetOrCreateProperty<CXFA_Node>(0, valueType);
}

XFA_Element CXFA_Node::GetValueNodeType() const {
  return XFA_Element::Text;
}

CXFA_Node* CXFA_Node::GetUIChildNode() {
  DCHECK(HasCreatedUIWidget());

  if (ff_widget_type_ != XFA_FFWidgetType::kNone)
    return ui_ ? ui_->GetFirstChild() : nullptr;

  XFA_Element type = GetElementType();
  if (type == XFA_Element::Field || type == XFA_Element::Draw) {
    std::tie(ff_widget_type_, ui_) = CreateChildUIAndValueNodesIfNeeded();
  } else if (type == XFA_Element::Subform) {
    ff_widget_type_ = XFA_FFWidgetType::kSubform;
  } else if (type == XFA_Element::ExclGroup) {
    ff_widget_type_ = XFA_FFWidgetType::kExclGroup;
  } else {
    NOTREACHED_NORETURN();
  }
  return ui_ ? ui_->GetFirstChild() : nullptr;
}

XFA_FFWidgetType CXFA_Node::GetFFWidgetType() {
  GetUIChildNode();
  return ff_widget_type_;
}

CXFA_Border* CXFA_Node::GetUIBorder() {
  CXFA_Node* pUIChild = GetUIChildNode();
  return pUIChild ? pUIChild->JSObject()->GetProperty<CXFA_Border>(
                        0, XFA_Element::Border)
                  : nullptr;
}

CFX_RectF CXFA_Node::GetUIMargin() {
  CXFA_Node* pUIChild = GetUIChildNode();
  if (!pUIChild)
    return CFX_RectF();

  CXFA_Margin* mgUI =
      pUIChild->JSObject()->GetProperty<CXFA_Margin>(0, XFA_Element::Margin);
  if (!mgUI)
    return CFX_RectF();

  CXFA_Border* border = GetUIBorder();
  if (border && border->GetPresence() != XFA_AttributeValue::Visible)
    return CFX_RectF();

  absl::optional<float> left = mgUI->TryLeftInset();
  absl::optional<float> top = mgUI->TryTopInset();
  absl::optional<float> right = mgUI->TryRightInset();
  absl::optional<float> bottom = mgUI->TryBottomInset();
  if (border) {
    bool bVisible = false;
    float fThickness = 0;
    XFA_AttributeValue iType = XFA_AttributeValue::Unknown;
    std::tie(iType, bVisible, fThickness) = border->Get3DStyle();
    if (!left.has_value() || !top.has_value() || !right.has_value() ||
        !bottom.has_value()) {
      std::vector<CXFA_Stroke*> strokes = border->GetStrokes();
      if (!top.has_value())
        top = GetEdgeThickness(strokes, bVisible, 0);
      if (!right.has_value())
        right = GetEdgeThickness(strokes, bVisible, 1);
      if (!bottom.has_value())
        bottom = GetEdgeThickness(strokes, bVisible, 2);
      if (!left.has_value())
        left = GetEdgeThickness(strokes, bVisible, 3);
    }
  }
  return CFX_RectF(left.value_or(0.0), top.value_or(0.0), right.value_or(0.0),
                   bottom.value_or(0.0));
}

std::vector<CXFA_Event*> CXFA_Node::GetEventByActivity(
    XFA_AttributeValue iActivity,
    bool bIsFormReady) {
  std::vector<CXFA_Event*> events;
  for (CXFA_Node* node : GetNodeListForType(XFA_Element::Event)) {
    auto* event = static_cast<CXFA_Event*>(node);
    if (event->GetActivity() != iActivity)
      continue;

    if (iActivity != XFA_AttributeValue::Ready) {
      events.push_back(event);
      continue;
    }

    WideString wsRef = event->GetRef();
    if (bIsFormReady) {
      if (wsRef == WideStringView(L"$form"))
        events.push_back(event);
      continue;
    }

    if (wsRef == WideStringView(L"$layout"))
      events.push_back(event);
  }
  return events;
}

void CXFA_Node::ResetData() {
  WideString wsValue;
  switch (GetFFWidgetType()) {
    case XFA_FFWidgetType::kImageEdit: {
      CXFA_Value* imageValue = GetDefaultValueIfExists();
      CXFA_Image* image = imageValue ? imageValue->GetImageIfExists() : nullptr;
      WideString wsContentType, wsHref;
      if (image) {
        wsValue = image->GetContent();
        wsContentType = image->GetContentType();
        wsHref = image->GetHref();
      }
      SetImageEdit(wsContentType, wsHref, wsValue);
      break;
    }
    case XFA_FFWidgetType::kExclGroup: {
      CXFA_Node* pNextChild = GetFirstContainerChild();
      while (pNextChild) {
        CXFA_Node* pChild = pNextChild;
        if (!pChild->IsWidgetReady())
          continue;

        bool done = false;
        if (wsValue.IsEmpty()) {
          CXFA_Value* defValue = pChild->GetDefaultValueIfExists();
          if (defValue) {
            wsValue = defValue->GetChildValueContent();
            SetValue(XFA_ValuePicture::kRaw, wsValue);
            pChild->SetValue(XFA_ValuePicture::kRaw, wsValue);
            done = true;
          }
        }
        if (!done) {
          CXFA_Items* pItems =
              pChild->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
          if (!pItems)
            continue;

          WideString itemText;
          if (pItems->CountChildren(XFA_Element::Unknown, false) > 1) {
            itemText =
                pItems->GetChild<CXFA_Node>(1, XFA_Element::Unknown, false)
                    ->JSObject()
                    ->GetContent(false);
          }
          pChild->SetValue(XFA_ValuePicture::kRaw, itemText);
        }
        pNextChild = pChild->GetNextContainerSibling();
      }
      break;
    }
    case XFA_FFWidgetType::kChoiceList:
      ClearAllSelections();
      [[fallthrough]];
    default: {
      CXFA_Value* defValue = GetDefaultValueIfExists();
      if (defValue)
        wsValue = defValue->GetChildValueContent();

      SetValue(XFA_ValuePicture::kRaw, wsValue);
      break;
    }
  }
}

void CXFA_Node::SetImageEdit(const WideString& wsContentType,
                             const WideString& wsHref,
                             const WideString& wsData) {
  CXFA_Value* formValue = GetFormValueIfExists();
  CXFA_Image* image = formValue ? formValue->GetImageIfExists() : nullptr;
  if (image) {
    image->SetContentType(WideString(wsContentType));
    image->SetHref(wsHref);
  }

  JSObject()->SetContent(wsData, GetFormatDataValue(wsData), true, false, true);

  CXFA_Node* pBind = GetBindData();
  if (!pBind) {
    if (image)
      image->SetTransferEncoding(XFA_AttributeValue::Base64);
    return;
  }
  pBind->JSObject()->SetCData(XFA_Attribute::ContentType, wsContentType);
  CXFA_Node* pHrefNode = pBind->GetFirstChild();
  if (pHrefNode) {
    pHrefNode->JSObject()->SetCData(XFA_Attribute::Value, wsHref);
    return;
  }
  CFX_XMLElement* pElement = ToXMLElement(pBind->GetXMLMappingNode());
  pElement->SetAttribute(L"href", wsHref);
}

void CXFA_Node::CalcCaptionSize(CXFA_FFDoc* doc, CFX_SizeF* pszCap) {
  CXFA_Caption* caption = GetCaptionIfExists();
  if (!caption || !caption->IsVisible())
    return;

  LoadCaption(doc);

  const float fCapReserve = caption->GetReserve();
  const XFA_AttributeValue iCapPlacement = caption->GetPlacementType();
  const bool bReserveExit = fCapReserve > 0.01;
  const bool bVert = iCapPlacement == XFA_AttributeValue::Top ||
                     iCapPlacement == XFA_AttributeValue::Bottom;
  CXFA_TextLayout* pCapTextLayout =
      m_pLayoutData->AsFieldLayoutData()->m_pCapTextLayout;
  if (pCapTextLayout) {
    if (!bVert && GetFFWidgetType() != XFA_FFWidgetType::kButton)
      pszCap->width = fCapReserve;

    CFX_SizeF minSize;
    *pszCap = pCapTextLayout->CalcSize(minSize, *pszCap);
    if (bReserveExit)
      bVert ? pszCap->height = fCapReserve : pszCap->width = fCapReserve;
  } else {
    float fFontSize = 10.0f;
    CXFA_Font* font = caption->GetFontIfExists();
    if (font) {
      fFontSize = font->GetFontSize();
    } else {
      CXFA_Font* widgetfont = GetFontIfExists();
      if (widgetfont)
        fFontSize = widgetfont->GetFontSize();
    }

    if (bVert) {
      pszCap->height = fCapReserve > 0 ? fCapReserve : fFontSize;
    } else {
      pszCap->width = fCapReserve > 0 ? fCapReserve : 0;
      pszCap->height = fFontSize;
    }
  }

  CXFA_Margin* captionMargin = caption->GetMarginIfExists();
  if (!captionMargin)
    return;

  float fLeftInset = captionMargin->GetLeftInset();
  float fTopInset = captionMargin->GetTopInset();
  float fRightInset = captionMargin->GetRightInset();
  float fBottomInset = captionMargin->GetBottomInset();
  if (bReserveExit) {
    bVert ? (pszCap->width += fLeftInset + fRightInset)
          : (pszCap->height += fTopInset + fBottomInset);
  } else {
    pszCap->width += fLeftInset + fRightInset;
    pszCap->height += fTopInset + fBottomInset;
  }
}

bool CXFA_Node::CalculateFieldAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize) {
  CFX_SizeF szCap;
  CalcCaptionSize(doc, &szCap);

  CFX_RectF rtUIMargin = GetUIMargin();
  pSize->width += rtUIMargin.left + rtUIMargin.width;
  pSize->height += rtUIMargin.top + rtUIMargin.height;
  if (szCap.width > 0 && szCap.height > 0) {
    CXFA_Caption* caption = GetCaptionIfExists();
    XFA_AttributeValue placement = caption
                                       ? caption->GetPlacementType()
                                       : CXFA_Caption::kDefaultPlacementType;
    switch (placement) {
      case XFA_AttributeValue::Left:
      case XFA_AttributeValue::Right:
      case XFA_AttributeValue::Inline: {
        pSize->width += szCap.width;
        pSize->height = std::max(pSize->height, szCap.height);
      } break;
      case XFA_AttributeValue::Top:
      case XFA_AttributeValue::Bottom: {
        pSize->height += szCap.height;
        pSize->width = std::max(pSize->width, szCap.width);
        break;
      }
      default:
        break;
    }
  }
  return CalculateWidgetAutoSize(pSize);
}

bool CXFA_Node::CalculateWidgetAutoSize(CFX_SizeF* pSize) {
  CXFA_Margin* margin = GetMarginIfExists();
  if (margin) {
    pSize->width += margin->GetLeftInset() + margin->GetRightInset();
    pSize->height += margin->GetTopInset() + margin->GetBottomInset();
  }

  CXFA_Para* para = GetParaIfExists();
  if (para)
    pSize->width += para->GetMarginLeft() + para->GetTextIndent();

  absl::optional<float> width = TryWidth();
  if (width.has_value()) {
    pSize->width = width.value();
  } else {
    absl::optional<float> min = TryMinWidth();
    if (min.has_value())
      pSize->width = std::max(pSize->width, min.value());

    absl::optional<float> max = TryMaxWidth();
    if (max.has_value() && max.value() > 0)
      pSize->width = std::min(pSize->width, max.value());
  }

  absl::optional<float> height = TryHeight();
  if (height.has_value()) {
    pSize->height = height.value();
  } else {
    absl::optional<float> min = TryMinHeight();
    if (min.has_value())
      pSize->height = std::max(pSize->height, min.value());

    absl::optional<float> max = TryMaxHeight();
    if (max.has_value() && max.value() > 0)
      pSize->height = std::min(pSize->height, max.value());
  }
  return true;
}

void CXFA_Node::CalculateTextContentSize(CXFA_FFDoc* doc, CFX_SizeF* pSize) {
  float fFontSize = GetFontSize();
  WideString wsText = GetValue(XFA_ValuePicture::kDisplay);
  if (wsText.IsEmpty()) {
    pSize->height += fFontSize;
    return;
  }

  if (wsText.Back() == L'\n')
    wsText += L'\n';

  CXFA_FieldLayoutData* layoutData = m_pLayoutData->AsFieldLayoutData();
  if (!layoutData->m_pTextOut) {
    layoutData->m_pTextOut = std::make_unique<CFDE_TextOut>();
    CFDE_TextOut* pTextOut = layoutData->m_pTextOut.get();
    pTextOut->SetFont(GetFGASFont(doc));
    pTextOut->SetFontSize(fFontSize);
    pTextOut->SetLineBreakTolerance(fFontSize * 0.2f);
    pTextOut->SetLineSpace(GetLineHeight());

    FDE_TextStyle dwStyles;
    dwStyles.last_line_height_ = true;
    if (GetFFWidgetType() == XFA_FFWidgetType::kTextEdit && IsMultiLine())
      dwStyles.line_wrap_ = true;

    pTextOut->SetStyles(dwStyles);
  }
  layoutData->m_pTextOut->CalcLogicSize(wsText.AsStringView(), pSize);
}

bool CXFA_Node::CalculateTextEditAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize) {
  if (pSize->width > 0) {
    CFX_SizeF szOrz = *pSize;
    CFX_SizeF szCap;
    CalcCaptionSize(doc, &szCap);
    bool bCapExit = szCap.width > 0.01 && szCap.height > 0.01;
    XFA_AttributeValue iCapPlacement = XFA_AttributeValue::Unknown;
    if (bCapExit) {
      CXFA_Caption* caption = GetCaptionIfExists();
      iCapPlacement = caption ? caption->GetPlacementType()
                              : CXFA_Caption::kDefaultPlacementType;
      switch (iCapPlacement) {
        case XFA_AttributeValue::Left:
        case XFA_AttributeValue::Right:
        case XFA_AttributeValue::Inline: {
          pSize->width -= szCap.width;
          break;
        }
        default:
          break;
      }
    }
    CFX_RectF rtUIMargin = GetUIMargin();
    pSize->width -= rtUIMargin.left + rtUIMargin.width;
    CXFA_Margin* margin = GetMarginIfExists();
    if (margin)
      pSize->width -= margin->GetLeftInset() + margin->GetRightInset();

    CalculateTextContentSize(doc, pSize);
    pSize->height += rtUIMargin.top + rtUIMargin.height;
    if (bCapExit) {
      switch (iCapPlacement) {
        case XFA_AttributeValue::Left:
        case XFA_AttributeValue::Right:
        case XFA_AttributeValue::Inline: {
          pSize->height = std::max(pSize->height, szCap.height);
        } break;
        case XFA_AttributeValue::Top:
        case XFA_AttributeValue::Bottom: {
          pSize->height += szCap.height;
          break;
        }
        default:
          break;
      }
    }
    pSize->width = szOrz.width;
    return CalculateWidgetAutoSize(pSize);
  }
  CalculateTextContentSize(doc, pSize);
  return CalculateFieldAutoSize(doc, pSize);
}

bool CXFA_Node::CalculateCheckButtonAutoSize(CXFA_FFDoc* doc,
                                             CFX_SizeF* pSize) {
  float fCheckSize = GetCheckButtonSize();
  *pSize = CFX_SizeF(fCheckSize, fCheckSize);
  return CalculateFieldAutoSize(doc, pSize);
}

bool CXFA_Node::CalculatePushButtonAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize) {
  CalcCaptionSize(doc, pSize);
  return CalculateWidgetAutoSize(pSize);
}

CFX_SizeF CXFA_Node::CalculateImageSize(float img_width,
                                        float img_height,
                                        const CFX_Size& dpi) {
  CFX_RectF rtImage(0, 0, XFA_UnitPx2Pt(img_width, dpi.width),
                    XFA_UnitPx2Pt(img_height, dpi.height));

  CFX_RectF rtFit;
  absl::optional<float> width = TryWidth();
  if (width.has_value()) {
    rtFit.width = width.value();
    GetWidthWithoutMargin(rtFit.width);
  } else {
    rtFit.width = rtImage.width;
  }

  absl::optional<float> height = TryHeight();
  if (height.has_value()) {
    rtFit.height = height.value();
    GetHeightWithoutMargin(rtFit.height);
  } else {
    rtFit.height = rtImage.height;
  }

  return rtFit.Size();
}

bool CXFA_Node::CalculateImageAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize) {
  if (!GetLayoutImage())
    LoadLayoutImage(doc);

  pSize->clear();
  RetainPtr<CFX_DIBitmap> pBitmap = GetLayoutImage();
  if (!pBitmap)
    return CalculateWidgetAutoSize(pSize);

  *pSize = CalculateImageSize(pBitmap->GetWidth(), pBitmap->GetHeight(),
                              GetLayoutImageDpi());
  return CalculateWidgetAutoSize(pSize);
}

bool CXFA_Node::CalculateImageEditAutoSize(CXFA_FFDoc* doc, CFX_SizeF* pSize) {
  if (!GetEditImage())
    LoadEditImage(doc);

  pSize->clear();
  RetainPtr<CFX_DIBitmap> pBitmap = GetEditImage();
  if (!pBitmap)
    return CalculateFieldAutoSize(doc, pSize);

  *pSize = CalculateImageSize(pBitmap->GetWidth(), pBitmap->GetHeight(),
                              GetEditImageDpi());
  return CalculateFieldAutoSize(doc, pSize);
}

bool CXFA_Node::LoadLayoutImage(CXFA_FFDoc* doc) {
  InitLayoutData(doc);
  return m_pLayoutData->AsImageLayoutData()->LoadImageData(doc, this);
}

bool CXFA_Node::LoadEditImage(CXFA_FFDoc* doc) {
  InitLayoutData(doc);
  return m_pLayoutData->AsFieldLayoutData()->AsImageEditData()->LoadImageData(
      doc, this);
}

CFX_Size CXFA_Node::GetLayoutImageDpi() const {
  return m_pLayoutData->AsImageLayoutData()->GetDpi();
}

CFX_Size CXFA_Node::GetEditImageDpi() const {
  CXFA_ImageEditData* pData =
      m_pLayoutData->AsFieldLayoutData()->AsImageEditData();
  return pData->GetDpi();
}

float CXFA_Node::CalculateWidgetAutoWidth(float fWidthCalc) {
  CXFA_Margin* margin = GetMarginIfExists();
  if (margin)
    fWidthCalc += margin->GetLeftInset() + margin->GetRightInset();

  absl::optional<float> min = TryMinWidth();
  if (min.has_value())
    fWidthCalc = std::max(fWidthCalc, min.value());

  absl::optional<float> max = TryMaxWidth();
  if (max.has_value() && max.value() > 0)
    fWidthCalc = std::min(fWidthCalc, max.value());

  return fWidthCalc;
}

float CXFA_Node::GetWidthWithoutMargin(float fWidthCalc) const {
  CXFA_Margin* margin = GetMarginIfExists();
  if (margin)
    fWidthCalc -= margin->GetLeftInset() + margin->GetRightInset();
  return fWidthCalc;
}

float CXFA_Node::CalculateWidgetAutoHeight(float fHeightCalc) {
  CXFA_Margin* margin = GetMarginIfExists();
  if (margin)
    fHeightCalc += margin->GetTopInset() + margin->GetBottomInset();

  absl::optional<float> min = TryMinHeight();
  if (min.has_value())
    fHeightCalc = std::max(fHeightCalc, min.value());

  absl::optional<float> max = TryMaxHeight();
  if (max.has_value() && max.value() > 0)
    fHeightCalc = std::min(fHeightCalc, max.value());

  return fHeightCalc;
}

float CXFA_Node::GetHeightWithoutMargin(float fHeightCalc) const {
  CXFA_Margin* margin = GetMarginIfExists();
  if (margin)
    fHeightCalc -= margin->GetTopInset() + margin->GetBottomInset();
  return fHeightCalc;
}

void CXFA_Node::StartWidgetLayout(CXFA_FFDoc* doc,
                                  float* pCalcWidth,
                                  float* pCalcHeight) {
  InitLayoutData(doc);

  if (GetFFWidgetType() == XFA_FFWidgetType::kText) {
    m_pLayoutData->SetWidgetHeight(TryHeight().value_or(-1));
    StartTextLayout(doc, pCalcWidth, pCalcHeight);
    return;
  }
  if (*pCalcWidth > 0 && *pCalcHeight > 0)
    return;

  m_pLayoutData->SetWidgetHeight(-1.0f);
  float fWidth = 0;
  if (*pCalcWidth > 0 && *pCalcHeight < 0) {
    absl::optional<float> height = TryHeight();
    if (height.has_value()) {
      *pCalcHeight = height.value();
    } else {
      CFX_SizeF size = CalculateAccWidthAndHeight(doc, *pCalcWidth);
      *pCalcWidth = size.width;
      *pCalcHeight = size.height;
    }
    m_pLayoutData->SetWidgetHeight(*pCalcHeight);
    return;
  }
  if (*pCalcWidth < 0 && *pCalcHeight < 0) {
    absl::optional<float> height;
    absl::optional<float> width = TryWidth();
    if (width.has_value()) {
      fWidth = width.value();
      height = TryHeight();
      if (height.has_value())
        *pCalcHeight = height.value();
    }
    if (!width.has_value() || !height.has_value()) {
      CFX_SizeF size = CalculateAccWidthAndHeight(doc, fWidth);
      *pCalcWidth = size.width;
      *pCalcHeight = size.height;
    } else {
      *pCalcWidth = fWidth;
    }
  }
  m_pLayoutData->SetWidgetHeight(*pCalcHeight);
}

CFX_SizeF CXFA_Node::CalculateAccWidthAndHeight(CXFA_FFDoc* doc, float fWidth) {
  CFX_SizeF sz(fWidth, m_pLayoutData->GetWidgetHeight());
  switch (GetFFWidgetType()) {
    case XFA_FFWidgetType::kBarcode:
    case XFA_FFWidgetType::kChoiceList:
    case XFA_FFWidgetType::kSignature:
      CalculateFieldAutoSize(doc, &sz);
      break;
    case XFA_FFWidgetType::kImageEdit:
      CalculateImageEditAutoSize(doc, &sz);
      break;
    case XFA_FFWidgetType::kButton:
      CalculatePushButtonAutoSize(doc, &sz);
      break;
    case XFA_FFWidgetType::kCheckButton:
      CalculateCheckButtonAutoSize(doc, &sz);
      break;
    case XFA_FFWidgetType::kDateTimeEdit:
    case XFA_FFWidgetType::kNumericEdit:
    case XFA_FFWidgetType::kPasswordEdit:
    case XFA_FFWidgetType::kTextEdit:
      CalculateTextEditAutoSize(doc, &sz);
      break;
    case XFA_FFWidgetType::kImage:
      CalculateImageAutoSize(doc, &sz);
      break;
    case XFA_FFWidgetType::kArc:
    case XFA_FFWidgetType::kLine:
    case XFA_FFWidgetType::kRectangle:
    case XFA_FFWidgetType::kSubform:
    case XFA_FFWidgetType::kExclGroup:
      CalculateWidgetAutoSize(&sz);
      break;
    case XFA_FFWidgetType::kText:
    case XFA_FFWidgetType::kNone:
      break;
  }
  m_pLayoutData->SetWidgetHeight(sz.height);
  return sz;
}

absl::optional<float> CXFA_Node::FindSplitPos(CXFA_FFDocView* pDocView,
                                              size_t szBlockIndex,
                                              float fCalcHeight) {
  if (!HasCreatedUIWidget())
    return absl::nullopt;

  if (GetFFWidgetType() == XFA_FFWidgetType::kSubform)
    return absl::nullopt;

  switch (GetFFWidgetType()) {
    case XFA_FFWidgetType::kText:
    case XFA_FFWidgetType::kTextEdit:
    case XFA_FFWidgetType::kNumericEdit:
    case XFA_FFWidgetType::kPasswordEdit:
      break;
    default:
      return 0.0f;
  }

  float fTopInset = 0;
  float fBottomInset = 0;
  if (szBlockIndex == 0) {
    CXFA_Margin* margin = GetMarginIfExists();
    if (margin) {
      fTopInset = margin->GetTopInset();
      fBottomInset = margin->GetBottomInset();
    }

    CFX_RectF rtUIMargin = GetUIMargin();
    fTopInset += rtUIMargin.top;
    fBottomInset += rtUIMargin.width;
  }
  if (GetFFWidgetType() == XFA_FFWidgetType::kText) {
    float fHeight = fCalcHeight;
    if (szBlockIndex == 0) {
      fCalcHeight -= fTopInset;
      fCalcHeight = std::max(fCalcHeight, 0.0f);
    }
    CXFA_TextLayout* pTextLayout =
        m_pLayoutData->AsTextLayoutData()->GetTextLayout();
    fCalcHeight = pTextLayout->DoSplitLayout(
        szBlockIndex, fCalcHeight,
        m_pLayoutData->GetWidgetHeight() - fTopInset);
    if (fCalcHeight != 0) {
      if (szBlockIndex == 0)
        fCalcHeight += fTopInset;
      if (fabs(fHeight - fCalcHeight) < kXFAWidgetPrecision)
        return absl::nullopt;
    }
    return fCalcHeight;
  }

  XFA_AttributeValue iCapPlacement = XFA_AttributeValue::Unknown;
  float fCapReserve = 0;
  if (szBlockIndex == 0) {
    CXFA_Caption* caption = GetCaptionIfExists();
    if (caption && !caption->IsHidden()) {
      iCapPlacement = caption->GetPlacementType();
      fCapReserve = caption->GetReserve();
    }
    if (iCapPlacement == XFA_AttributeValue::Top &&
        fCalcHeight < fCapReserve + fTopInset) {
      return 0.0f;
    }
    if (iCapPlacement == XFA_AttributeValue::Bottom &&
        m_pLayoutData->GetWidgetHeight() - fCapReserve - fBottomInset) {
      return 0.0f;
    }
    if (iCapPlacement != XFA_AttributeValue::Top)
      fCapReserve = 0;
  }
  CXFA_FieldLayoutData* pFieldData = m_pLayoutData->AsFieldLayoutData();
  int32_t iLinesCount = 0;
  float fHeight = m_pLayoutData->GetWidgetHeight();
  if (GetValue(XFA_ValuePicture::kDisplay).IsEmpty()) {
    iLinesCount = 1;
  } else {
    if (!pFieldData->m_pTextOut) {
      CFX_SizeF size = CalculateAccWidthAndHeight(pDocView->GetDoc(),
                                                  TryWidth().value_or(0));
      fHeight = size.height;
    }

    iLinesCount = pFieldData->m_pTextOut->GetTotalLines();
  }
  std::vector<float>* pFieldArray = &pFieldData->m_FieldSplitArray;
  size_t szFieldSplitCount = pFieldArray->size();
  if (szFieldSplitCount < szBlockIndex * 3)
    return absl::nullopt;

  for (size_t i = 0; i < szBlockIndex * 3; i += 3) {
    iLinesCount -= static_cast<int32_t>((*pFieldArray)[i + 1]);
    fHeight -= (*pFieldArray)[i + 2];
  }
  if (iLinesCount == 0)
    return absl::nullopt;

  float fLineHeight = GetLineHeight();
  float fFontSize = GetFontSize();
  float fTextHeight = iLinesCount * fLineHeight - fLineHeight + fFontSize;
  float fSpaceAbove = 0;
  float fStartOffset = 0;
  if (fHeight > 0.1f && szBlockIndex == 0) {
    fStartOffset = fTopInset;
    fHeight -= (fTopInset + fBottomInset);
    CXFA_Para* para = GetParaIfExists();
    if (para) {
      fSpaceAbove = para->GetSpaceAbove();
      float fSpaceBelow = para->GetSpaceBelow();
      fHeight -= (fSpaceAbove + fSpaceBelow);
      switch (para->GetVerticalAlign()) {
        case XFA_AttributeValue::Top:
          fStartOffset += fSpaceAbove;
          break;
        case XFA_AttributeValue::Middle:
          fStartOffset += ((fHeight - fTextHeight) / 2 + fSpaceAbove);
          break;
        case XFA_AttributeValue::Bottom:
          fStartOffset += (fHeight - fTextHeight + fSpaceAbove);
          break;
        default:
          NOTREACHED_NORETURN();
      }
    }
    if (fStartOffset < 0.1f)
      fStartOffset = 0;
  }
  if (szBlockIndex > 0) {
    size_t i = szBlockIndex - 1;
    fStartOffset = (*pFieldArray)[i * 3] - (*pFieldArray)[i * 3 + 2];
    if (fStartOffset < 0.1f)
      fStartOffset = 0;
  }
  if (szFieldSplitCount / 3 == (szBlockIndex + 1))
    (*pFieldArray)[0] = fStartOffset;
  else
    pFieldArray->push_back(fStartOffset);

  XFA_VERSION version = pDocView->GetDoc()->GetXFADoc()->GetCurVersionMode();
  bool bCanSplitNoContent = false;
  auto value = GetParent()->JSObject()->TryEnum(XFA_Attribute::Layout, true);
  XFA_AttributeValue eLayoutMode = value.value_or(XFA_AttributeValue::Position);
  if ((eLayoutMode == XFA_AttributeValue::Position ||
       eLayoutMode == XFA_AttributeValue::Tb ||
       eLayoutMode == XFA_AttributeValue::Row ||
       eLayoutMode == XFA_AttributeValue::Table) &&
      version > XFA_VERSION_208) {
    bCanSplitNoContent = true;
  }
  if ((eLayoutMode == XFA_AttributeValue::Tb ||
       eLayoutMode == XFA_AttributeValue::Row ||
       eLayoutMode == XFA_AttributeValue::Table) &&
      version <= XFA_VERSION_208) {
    if (fStartOffset >= fCalcHeight)
      return 0.0f;

    bCanSplitNoContent = true;
  }
  if (!bCanSplitNoContent ||
      fCalcHeight - fTopInset - fSpaceAbove < fLineHeight) {
    return 0.0f;
  }

  if (fStartOffset + kXFAWidgetPrecision >= fCalcHeight) {
    if (szFieldSplitCount / 3 == (szBlockIndex + 1)) {
      (*pFieldArray)[szBlockIndex * 3 + 1] = 0;
      (*pFieldArray)[szBlockIndex * 3 + 2] = fCalcHeight;
    } else {
      pFieldArray->push_back(0);
      pFieldArray->push_back(fCalcHeight);
    }
    return absl::nullopt;
  }

  if (fCalcHeight - fStartOffset < fLineHeight) {
    fCalcHeight = fStartOffset;
    if (szFieldSplitCount / 3 == (szBlockIndex + 1)) {
      (*pFieldArray)[szBlockIndex * 3 + 1] = 0;
      (*pFieldArray)[szBlockIndex * 3 + 2] = fCalcHeight;
    } else {
      pFieldArray->push_back(0);
      pFieldArray->push_back(fCalcHeight);
    }
    return fCalcHeight;
  }

  float fTextNum =
      fCalcHeight + kXFAWidgetPrecision - fCapReserve - fStartOffset;
  int32_t iLineNum = static_cast<int32_t>(
      (fTextNum + (fLineHeight - fFontSize)) / fLineHeight);
  if (iLineNum >= iLinesCount) {
    if (fCalcHeight - fStartOffset - fTextHeight >= fFontSize) {
      if (szFieldSplitCount / 3 == (szBlockIndex + 1)) {
        (*pFieldArray)[szBlockIndex * 3 + 1] = iLinesCount;
        (*pFieldArray)[szBlockIndex * 3 + 2] = fCalcHeight;
      } else {
        pFieldArray->push_back(iLinesCount);
        pFieldArray->push_back(fCalcHeight);
      }
      return absl::nullopt;
    }
    if (fHeight - fStartOffset - fTextHeight < fFontSize) {
      iLineNum -= 1;
      if (iLineNum == 0)
        return 0.0f;
    } else {
      iLineNum = static_cast<int32_t>(fTextNum / fLineHeight);
    }
  }
  if (iLineNum <= 0)
    return 0.0f;

  float fSplitHeight = iLineNum * fLineHeight + fCapReserve + fStartOffset;
  if (szFieldSplitCount / 3 == (szBlockIndex + 1)) {
    (*pFieldArray)[szBlockIndex * 3 + 1] = iLineNum;
    (*pFieldArray)[szBlockIndex * 3 + 2] = fSplitHeight;
  } else {
    pFieldArray->push_back(iLineNum);
    pFieldArray->push_back(fSplitHeight);
  }
  if (fabs(fSplitHeight - fCalcHeight) < kXFAWidgetPrecision)
    return absl::nullopt;
  return fSplitHeight;
}

void CXFA_Node::InitLayoutData(CXFA_FFDoc* doc) {
  if (m_pLayoutData)
    return;

  switch (GetFFWidgetType()) {
    case XFA_FFWidgetType::kText:
      m_pLayoutData = cppgc::MakeGarbageCollected<CXFA_TextLayoutData>(
          doc->GetHeap()->GetAllocationHandle());
      return;
    case XFA_FFWidgetType::kTextEdit:
      m_pLayoutData = cppgc::MakeGarbageCollected<CXFA_TextEditData>(
          doc->GetHeap()->GetAllocationHandle());
      return;
    case XFA_FFWidgetType::kImage:
      m_pLayoutData = cppgc::MakeGarbageCollected<CXFA_ImageLayoutData>(
          doc->GetHeap()->GetAllocationHandle());
      return;
    case XFA_FFWidgetType::kImageEdit:
      m_pLayoutData = cppgc::MakeGarbageCollected<CXFA_ImageEditData>(
          doc->GetHeap()->GetAllocationHandle());
      return;
    default:
      break;
  }
  if (GetElementType() == XFA_Element::Field) {
    m_pLayoutData = cppgc::MakeGarbageCollected<CXFA_FieldLayoutData>(
        doc->GetHeap()->GetAllocationHandle());
    return;
  }
  m_pLayoutData = cppgc::MakeGarbageCollected<CXFA_WidgetLayoutData>(
      doc->GetHeap()->GetAllocationHandle());
}

void CXFA_Node::StartTextLayout(CXFA_FFDoc* doc,
                                float* pCalcWidth,
                                float* pCalcHeight) {
  InitLayoutData(doc);

  CXFA_TextLayoutData* pTextLayoutData = m_pLayoutData->AsTextLayoutData();
  pTextLayoutData->LoadText(doc, this);

  CXFA_TextLayout* pTextLayout = pTextLayoutData->GetTextLayout();
  float fTextHeight = 0;
  if (*pCalcWidth > 0 && *pCalcHeight > 0) {
    float fWidth = GetWidthWithoutMargin(*pCalcWidth);
    pTextLayout->StartLayout(fWidth);
    fTextHeight = *pCalcHeight;
    fTextHeight = GetHeightWithoutMargin(fTextHeight);
    pTextLayout->DoLayout(fTextHeight);
    return;
  }
  if (*pCalcWidth > 0 && *pCalcHeight < 0) {
    float fWidth = GetWidthWithoutMargin(*pCalcWidth);
    pTextLayout->StartLayout(fWidth);
  }
  if (*pCalcWidth < 0 && *pCalcHeight < 0) {
    absl::optional<float> width = TryWidth();
    if (width.has_value()) {
      pTextLayout->StartLayout(GetWidthWithoutMargin(width.value()));
      *pCalcWidth = width.value();
    } else {
      float fMaxWidth = CalculateWidgetAutoWidth(pTextLayout->StartLayout(-1));
      pTextLayout->StartLayout(GetWidthWithoutMargin(fMaxWidth));
      *pCalcWidth = fMaxWidth;
    }
  }
  if (m_pLayoutData->GetWidgetHeight() < 0) {
    m_pLayoutData->SetWidgetHeight(
        CalculateWidgetAutoHeight(pTextLayout->GetLayoutHeight()));
  }
  fTextHeight = m_pLayoutData->GetWidgetHeight();
  fTextHeight = GetHeightWithoutMargin(fTextHeight);
  pTextLayout->DoLayout(fTextHeight);
  *pCalcHeight = m_pLayoutData->GetWidgetHeight();
}

bool CXFA_Node::LoadCaption(CXFA_FFDoc* doc) {
  InitLayoutData(doc);
  return m_pLayoutData->AsFieldLayoutData()->LoadCaption(doc, this);
}

CXFA_TextLayout* CXFA_Node::GetCaptionTextLayout() {
  return m_pLayoutData ? m_pLayoutData->AsFieldLayoutData()->m_pCapTextLayout
                       : nullptr;
}

CXFA_TextLayout* CXFA_Node::GetTextLayout() {
  return m_pLayoutData ? m_pLayoutData->AsTextLayoutData()->GetTextLayout()
                       : nullptr;
}

RetainPtr<CFX_DIBitmap> CXFA_Node::GetLayoutImage() {
  return m_pLayoutData ? m_pLayoutData->AsImageLayoutData()->GetBitmap()
                       : nullptr;
}

RetainPtr<CFX_DIBitmap> CXFA_Node::GetEditImage() {
  return m_pLayoutData ? m_pLayoutData->AsFieldLayoutData()
                             ->AsImageEditData()
                             ->GetBitmap()
                       : nullptr;
}

void CXFA_Node::SetLayoutImage(RetainPtr<CFX_DIBitmap> newImage) {
  CXFA_ImageLayoutData* pData = m_pLayoutData->AsImageLayoutData();
  if (pData->GetBitmap() != newImage)
    pData->SetBitmap(std::move(newImage));
}

void CXFA_Node::SetEditImage(RetainPtr<CFX_DIBitmap> newImage) {
  CXFA_ImageEditData* pData =
      m_pLayoutData->AsFieldLayoutData()->AsImageEditData();
  if (pData->GetBitmap() != newImage)
    pData->SetBitmap(std::move(newImage));
}

RetainPtr<CFGAS_GEFont> CXFA_Node::GetFGASFont(CXFA_FFDoc* doc) {
  WideString wsFontName = L"Courier";
  uint32_t dwFontStyle = 0;
  CXFA_Font* font = GetFontIfExists();
  if (font) {
    if (font->IsBold())
      dwFontStyle |= FXFONT_FORCE_BOLD;
    if (font->IsItalic())
      dwFontStyle |= FXFONT_ITALIC;

    wsFontName = font->GetTypeface();
  }
  return doc->GetApp()->GetXFAFontMgr()->GetFont(doc, wsFontName, dwFontStyle);
}

bool CXFA_Node::HasButtonRollover() const {
  const auto* pItems = GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return false;

  for (CXFA_Node* pText = pItems->GetFirstChild(); pText;
       pText = pText->GetNextSibling()) {
    if (pText->JSObject()
            ->GetCData(XFA_Attribute::Name)
            .EqualsASCII("rollover")) {
      return !pText->JSObject()->GetContent(false).IsEmpty();
    }
  }
  return false;
}

bool CXFA_Node::HasButtonDown() const {
  const auto* pItems = GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return false;

  for (CXFA_Node* pText = pItems->GetFirstChild(); pText;
       pText = pText->GetNextSibling()) {
    if (pText->JSObject()->GetCData(XFA_Attribute::Name).EqualsASCII("down")) {
      return !pText->JSObject()->GetContent(false).IsEmpty();
    }
  }
  return false;
}

bool CXFA_Node::IsRadioButton() {
  CXFA_Node* pParent = GetParent();
  return pParent && pParent->GetElementType() == XFA_Element::ExclGroup;
}

float CXFA_Node::GetCheckButtonSize() {
  CXFA_Node* pUIChild = GetUIChildNode();
  if (pUIChild) {
    return pUIChild->JSObject()->GetMeasureInUnit(XFA_Attribute::Size,
                                                  XFA_Unit::Pt);
  }
  return CXFA_Measurement(10, XFA_Unit::Pt).ToUnit(XFA_Unit::Pt);
}

XFA_CheckState CXFA_Node::GetCheckState() {
  WideString wsValue = GetRawValue();
  if (wsValue.IsEmpty())
    return XFA_CheckState::kOff;

  auto* pItems = GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return XFA_CheckState::kOff;

  CXFA_Node* pText = pItems->GetFirstChild();
  int32_t i = 0;
  while (pText) {
    absl::optional<WideString> wsContent =
        pText->JSObject()->TryContent(false, true);
    if (wsContent == wsValue)
      return static_cast<XFA_CheckState>(i);

    i++;
    pText = pText->GetNextSibling();
  }
  return XFA_CheckState::kOff;
}

void CXFA_Node::SetCheckState(XFA_CheckState eCheckState) {
  CXFA_Node* node = GetExclGroupIfExists();
  if (!node) {
    CXFA_Items* pItems = GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItems)
      return;

    int32_t i = -1;
    CXFA_Node* pText = pItems->GetFirstChild();
    WideString wsContent;
    while (pText) {
      i++;
      if (i == static_cast<int32_t>(eCheckState)) {
        wsContent = pText->JSObject()->GetContent(false);
        break;
      }
      pText = pText->GetNextSibling();
    }
    SyncValue(wsContent, true);

    return;
  }

  WideString wsValue;
  if (eCheckState != XFA_CheckState::kOff) {
    if (CXFA_Items* pItems =
            GetChild<CXFA_Items>(0, XFA_Element::Items, false)) {
      CXFA_Node* pText = pItems->GetFirstChild();
      if (pText)
        wsValue = pText->JSObject()->GetContent(false);
    }
  }
  CXFA_Node* pChild = node->GetFirstChild();
  for (; pChild; pChild = pChild->GetNextSibling()) {
    if (pChild->GetElementType() != XFA_Element::Field)
      continue;

    CXFA_Items* pItem =
        pChild->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItem)
      continue;

    CXFA_Node* pItemchild = pItem->GetFirstChild();
    if (!pItemchild)
      continue;

    WideString text = pItemchild->JSObject()->GetContent(false);
    WideString wsChildValue = text;
    if (wsValue != text) {
      pItemchild = pItemchild->GetNextSibling();
      if (pItemchild)
        wsChildValue = pItemchild->JSObject()->GetContent(false);
      else
        wsChildValue.clear();
    }
    pChild->SyncValue(wsChildValue, true);
  }
  node->SyncValue(wsValue, true);
}

CXFA_Node* CXFA_Node::GetSelectedMember() {
  CXFA_Node* pSelectedMember = nullptr;
  WideString wsState = GetRawValue();
  if (wsState.IsEmpty())
    return pSelectedMember;

  for (CXFA_Node* pNode = ToNode(GetFirstChild()); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetCheckState() == XFA_CheckState::kOn) {
      pSelectedMember = pNode;
      break;
    }
  }
  return pSelectedMember;
}

CXFA_Node* CXFA_Node::SetSelectedMember(WideStringView wsName) {
  uint32_t nameHash = FX_HashCode_GetW(wsName);
  for (CXFA_Node* pNode = ToNode(GetFirstChild()); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetNameHash() == nameHash) {
      pNode->SetCheckState(XFA_CheckState::kOn);
      return pNode;
    }
  }
  return nullptr;
}

void CXFA_Node::SetSelectedMemberByValue(WideStringView wsValue,
                                         bool bNotify,
                                         bool bScriptModify,
                                         bool bSyncData) {
  WideString wsExclGroup;
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != XFA_Element::Field)
      continue;

    CXFA_Items* pItem =
        pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItem)
      continue;

    CXFA_Node* pItemchild = pItem->GetFirstChild();
    if (!pItemchild)
      continue;

    WideString wsChildValue = pItemchild->JSObject()->GetContent(false);
    if (wsValue != wsChildValue) {
      pItemchild = pItemchild->GetNextSibling();
      if (pItemchild)
        wsChildValue = pItemchild->JSObject()->GetContent(false);
      else
        wsChildValue.clear();
    } else {
      wsExclGroup = wsValue;
    }
    pNode->JSObject()->SetContent(wsChildValue, wsChildValue, bNotify,
                                  bScriptModify, false);
  }
  JSObject()->SetContent(wsExclGroup, wsExclGroup, bNotify, bScriptModify,
                         bSyncData);
}

CXFA_Node* CXFA_Node::GetExclGroupFirstMember() {
  CXFA_Node* pNode = GetFirstChild();
  while (pNode) {
    if (pNode->GetElementType() == XFA_Element::Field)
      return pNode;

    pNode = pNode->GetNextSibling();
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetExclGroupNextMember(CXFA_Node* pNode) {
  if (!pNode)
    return nullptr;

  CXFA_Node* pNodeField = pNode->GetNextSibling();
  while (pNodeField) {
    if (pNodeField->GetElementType() == XFA_Element::Field)
      return pNodeField;

    pNodeField = pNodeField->GetNextSibling();
  }
  return nullptr;
}

bool CXFA_Node::IsChoiceListCommitOnSelect() {
  CXFA_Node* pUIChild = GetUIChildNode();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::CommitOn) ==
           XFA_AttributeValue::Select;
  }
  return true;
}

bool CXFA_Node::IsChoiceListAllowTextEntry() {
  CXFA_Node* pUIChild = GetUIChildNode();
  return pUIChild && pUIChild->JSObject()->GetBoolean(XFA_Attribute::TextEntry);
}

bool CXFA_Node::IsChoiceListMultiSelect() {
  CXFA_Node* pUIChild = GetUIChildNode();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Open) ==
           XFA_AttributeValue::MultiSelect;
  }
  return false;
}

bool CXFA_Node::IsListBox() {
  CXFA_Node* pUIChild = GetUIChildNode();
  if (!pUIChild)
    return false;

  XFA_AttributeValue attr = pUIChild->JSObject()->GetEnum(XFA_Attribute::Open);
  return attr == XFA_AttributeValue::Always ||
         attr == XFA_AttributeValue::MultiSelect;
}

size_t CXFA_Node::CountChoiceListItems(bool bSaveValue) {
  std::vector<CXFA_Node*> pItems;
  int32_t iCount = 0;
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != XFA_Element::Items)
      continue;
    iCount++;
    pItems.push_back(pNode);
    if (iCount == 2)
      break;
  }
  if (iCount == 0)
    return 0;

  CXFA_Node* pItem = pItems[0];
  if (iCount > 1) {
    bool bItemOneHasSave =
        pItems[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        pItems[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItem = pItems[1];
  }
  return pItem->CountChildren(XFA_Element::Unknown, false);
}

absl::optional<WideString> CXFA_Node::GetChoiceListItem(int32_t nIndex,
                                                        bool bSaveValue) {
  std::vector<CXFA_Node*> pItemsArray;
  int32_t iCount = 0;
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != XFA_Element::Items)
      continue;

    ++iCount;
    pItemsArray.push_back(pNode);
    if (iCount == 2)
      break;
  }
  if (iCount == 0)
    return absl::nullopt;

  CXFA_Node* pItems = pItemsArray[0];
  if (iCount > 1) {
    bool bItemOneHasSave =
        pItemsArray[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        pItemsArray[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItems = pItemsArray[1];
  }
  if (!pItems)
    return absl::nullopt;

  CXFA_Node* pItem =
      pItems->GetChild<CXFA_Node>(nIndex, XFA_Element::Unknown, false);
  if (!pItem)
    return absl::nullopt;

  return pItem->JSObject()->GetContent(false);
}

std::vector<WideString> CXFA_Node::GetChoiceListItems(bool bSaveValue) {
  std::vector<CXFA_Node*> items;
  for (CXFA_Node* pNode = GetFirstChild(); pNode && items.size() < 2;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() == XFA_Element::Items)
      items.push_back(pNode);
  }
  if (items.empty())
    return std::vector<WideString>();

  CXFA_Node* pItem = items.front();
  if (items.size() > 1) {
    bool bItemOneHasSave =
        items[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        items[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItem = items[1];
  }

  std::vector<WideString> wsTextArray;
  for (CXFA_Node* pNode = pItem->GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    wsTextArray.emplace_back(pNode->JSObject()->GetContent(false));
  }
  return wsTextArray;
}

int32_t CXFA_Node::CountSelectedItems() {
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  if (IsListBox() || !IsChoiceListAllowTextEntry())
    return fxcrt::CollectionSize<int32_t>(wsValueArray);

  int32_t iSelected = 0;
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  for (const auto& value : wsValueArray) {
    if (pdfium::Contains(wsSaveTextArray, value))
      iSelected++;
  }
  return iSelected;
}

int32_t CXFA_Node::GetSelectedItem(int32_t nIndex) {
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  if (!fxcrt::IndexInBounds(wsValueArray, nIndex))
    return -1;

  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  auto it = std::find(wsSaveTextArray.begin(), wsSaveTextArray.end(),
                      wsValueArray[nIndex]);
  return it != wsSaveTextArray.end()
             ? pdfium::base::checked_cast<int32_t>(it - wsSaveTextArray.begin())
             : -1;
}

std::vector<int32_t> CXFA_Node::GetSelectedItems() {
  std::vector<int32_t> iSelArray;
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  for (const auto& value : wsValueArray) {
    auto it = std::find(wsSaveTextArray.begin(), wsSaveTextArray.end(), value);
    if (it != wsSaveTextArray.end()) {
      iSelArray.push_back(
          pdfium::base::checked_cast<int32_t>(it - wsSaveTextArray.begin()));
    }
  }
  return iSelArray;
}

std::vector<WideString> CXFA_Node::GetSelectedItemsValue() {
  WideString wsValue = GetRawValue();
  if (IsChoiceListMultiSelect())
    return fxcrt::Split(wsValue, L'\n');

  std::vector<WideString> wsSelTextArray;
  wsSelTextArray.push_back(wsValue);
  return wsSelTextArray;
}

bool CXFA_Node::GetItemState(int32_t nIndex) {
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  return fxcrt::IndexInBounds(wsSaveTextArray, nIndex) &&
         pdfium::Contains(GetSelectedItemsValue(), wsSaveTextArray[nIndex]);
}

void CXFA_Node::SetItemState(int32_t nIndex,
                             bool bSelected,
                             bool bNotify,
                             bool bScriptModify) {
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  if (!fxcrt::IndexInBounds(wsSaveTextArray, nIndex))
    return;

  int32_t iSel = -1;
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  auto value_iter = std::find(wsValueArray.begin(), wsValueArray.end(),
                              wsSaveTextArray[nIndex]);
  if (value_iter != wsValueArray.end()) {
    iSel =
        pdfium::base::checked_cast<int32_t>(value_iter - wsValueArray.begin());
  }
  if (IsChoiceListMultiSelect()) {
    if (bSelected) {
      if (iSel < 0) {
        WideString wsValue = GetRawValue();
        if (!wsValue.IsEmpty()) {
          wsValue += L"\n";
        }
        wsValue += wsSaveTextArray[nIndex];
        JSObject()->SetContent(wsValue, wsValue, bNotify, bScriptModify, true);
      }
    } else if (iSel >= 0) {
      std::vector<int32_t> iSelArray = GetSelectedItems();
      auto selected_iter =
          std::find(iSelArray.begin(), iSelArray.end(), nIndex);
      if (selected_iter != iSelArray.end())
        iSelArray.erase(selected_iter);
      SetSelectedItems(iSelArray, bNotify, bScriptModify, true);
    }
  } else {
    if (bSelected) {
      if (iSel < 0) {
        WideString wsSaveText = wsSaveTextArray[nIndex];
        JSObject()->SetContent(wsSaveText, GetFormatDataValue(wsSaveText),
                               bNotify, bScriptModify, true);
      }
    } else if (iSel >= 0) {
      JSObject()->SetContent(WideString(), WideString(), bNotify, bScriptModify,
                             true);
    }
  }
}

void CXFA_Node::SetSelectedItems(const std::vector<int32_t>& iSelArray,
                                 bool bNotify,
                                 bool bScriptModify,
                                 bool bSyncData) {
  WideString wsValue;
  int32_t iSize = fxcrt::CollectionSize<int32_t>(iSelArray);
  if (iSize >= 1) {
    std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
    WideString wsItemValue;
    for (int32_t i = 0; i < iSize; i++) {
      wsItemValue = (iSize == 1) ? wsSaveTextArray[iSelArray[i]]
                                 : wsSaveTextArray[iSelArray[i]] + L"\n";
      wsValue += wsItemValue;
    }
  }
  WideString wsFormat(wsValue);
  if (!IsChoiceListMultiSelect())
    wsFormat = GetFormatDataValue(wsValue);

  JSObject()->SetContent(wsValue, wsFormat, bNotify, bScriptModify, bSyncData);
}

void CXFA_Node::ClearAllSelections() {
  CXFA_Node* pBind = GetBindData();
  if (!pBind || !IsChoiceListMultiSelect()) {
    SyncValue(WideString(), false);
    return;
  }

  while (CXFA_Node* pChildNode = pBind->GetFirstChild())
    pBind->RemoveChildAndNotify(pChildNode, true);
}

void CXFA_Node::InsertItem(const WideString& wsLabel,
                           const WideString& wsValue,
                           bool bNotify) {
  int32_t nIndex = -1;
  WideString wsNewValue(wsValue);
  if (wsNewValue.IsEmpty())
    wsNewValue = wsLabel;

  std::vector<CXFA_Node*> listitems;
  for (CXFA_Node* pItem = GetFirstChild(); pItem;
       pItem = pItem->GetNextSibling()) {
    if (pItem->GetElementType() == XFA_Element::Items)
      listitems.push_back(pItem);
  }
  if (listitems.empty()) {
    CXFA_Node* pItems = CreateSamePacketNode(XFA_Element::Items);
    InsertChildAndNotify(-1, pItems);
    InsertListTextItem(pItems, wsLabel, nIndex);
    CXFA_Node* pSaveItems = CreateSamePacketNode(XFA_Element::Items);
    InsertChildAndNotify(-1, pSaveItems);
    pSaveItems->JSObject()->SetBoolean(XFA_Attribute::Save, true, false);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  } else if (listitems.size() > 1) {
    for (int32_t i = 0; i < 2; i++) {
      CXFA_Node* pNode = listitems[i];
      bool bHasSave = pNode->JSObject()->GetBoolean(XFA_Attribute::Save);
      if (bHasSave)
        InsertListTextItem(pNode, wsNewValue, nIndex);
      else
        InsertListTextItem(pNode, wsLabel, nIndex);
    }
  } else {
    CXFA_Node* pNode = listitems[0];
    pNode->JSObject()->SetBoolean(XFA_Attribute::Save, false, false);
    pNode->JSObject()->SetEnum(XFA_Attribute::Presence,
                               XFA_AttributeValue::Visible, false);
    CXFA_Node* pSaveItems = CreateSamePacketNode(XFA_Element::Items);
    InsertChildAndNotify(-1, pSaveItems);
    pSaveItems->JSObject()->SetBoolean(XFA_Attribute::Save, true, false);
    pSaveItems->JSObject()->SetEnum(XFA_Attribute::Presence,
                                    XFA_AttributeValue::Hidden, false);
    CXFA_Node* pListNode = pNode->GetFirstChild();
    int32_t i = 0;
    while (pListNode) {
      InsertListTextItem(pSaveItems, pListNode->JSObject()->GetContent(false),
                         i);
      ++i;

      pListNode = pListNode->GetNextSibling();
    }
    InsertListTextItem(pNode, wsLabel, nIndex);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  }
  if (bNotify)
    GetDocument()->GetNotify()->OnWidgetListItemAdded(this, wsLabel, nIndex);
}

WideString CXFA_Node::GetItemLabel(WideStringView wsValue) const {
  std::vector<CXFA_Node*> listitems;
  CXFA_Node* pItems = GetFirstChild();
  for (; pItems; pItems = pItems->GetNextSibling()) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;
    listitems.push_back(pItems);
  }

  if (listitems.size() <= 1)
    return WideString(wsValue);

  CXFA_Node* pLabelItems = listitems[0];
  bool bSave = pLabelItems->JSObject()->GetBoolean(XFA_Attribute::Save);
  CXFA_Node* pSaveItems = nullptr;
  if (bSave) {
    pSaveItems = pLabelItems;
    pLabelItems = listitems[1];
  } else {
    pSaveItems = listitems[1];
  }

  int32_t iCount = 0;
  int32_t iSearch = -1;
  for (CXFA_Node* pChildItem = pSaveItems->GetFirstChild(); pChildItem;
       pChildItem = pChildItem->GetNextSibling()) {
    if (pChildItem->JSObject()->GetContent(false) == wsValue) {
      iSearch = iCount;
      break;
    }
    iCount++;
  }
  if (iSearch < 0)
    return WideString();

  CXFA_Node* pText =
      pLabelItems->GetChild<CXFA_Node>(iSearch, XFA_Element::Unknown, false);
  return pText ? pText->JSObject()->GetContent(false) : WideString();
}

WideString CXFA_Node::GetItemValue(WideStringView wsLabel) {
  int32_t iCount = 0;
  std::vector<CXFA_Node*> listitems;
  for (CXFA_Node* pItems = GetFirstChild(); pItems;
       pItems = pItems->GetNextSibling()) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;
    iCount++;
    listitems.push_back(pItems);
  }
  if (iCount <= 1)
    return WideString(wsLabel);

  CXFA_Node* pLabelItems = listitems[0];
  bool bSave = pLabelItems->JSObject()->GetBoolean(XFA_Attribute::Save);
  CXFA_Node* pSaveItems = nullptr;
  if (bSave) {
    pSaveItems = pLabelItems;
    pLabelItems = listitems[1];
  } else {
    pSaveItems = listitems[1];
  }
  iCount = 0;

  int32_t iSearch = -1;
  WideString wsContent;
  CXFA_Node* pChildItem = pLabelItems->GetFirstChild();
  for (; pChildItem; pChildItem = pChildItem->GetNextSibling()) {
    if (pChildItem->JSObject()->GetContent(false) == wsLabel) {
      iSearch = iCount;
      break;
    }
    iCount++;
  }
  if (iSearch < 0)
    return WideString();

  CXFA_Node* pText =
      pSaveItems->GetChild<CXFA_Node>(iSearch, XFA_Element::Unknown, false);
  return pText ? pText->JSObject()->GetContent(false) : WideString();
}

bool CXFA_Node::DeleteItem(int32_t nIndex, bool bNotify, bool bScriptModify) {
  bool bSetValue = false;
  CXFA_Node* pItems = GetFirstChild();
  for (; pItems; pItems = pItems->GetNextSibling()) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;

    if (nIndex < 0) {
      while (CXFA_Node* pNode = pItems->GetFirstChild()) {
        pItems->RemoveChildAndNotify(pNode, true);
      }
    } else {
      if (!bSetValue && pItems->JSObject()->GetBoolean(XFA_Attribute::Save)) {
        SetItemState(nIndex, false, true, bScriptModify);
        bSetValue = true;
      }
      int32_t i = 0;
      CXFA_Node* pNode = pItems->GetFirstChild();
      while (pNode) {
        if (i == nIndex) {
          pItems->RemoveChildAndNotify(pNode, true);
          break;
        }
        i++;
        pNode = pNode->GetNextSibling();
      }
    }
  }
  if (bNotify)
    GetDocument()->GetNotify()->OnWidgetListItemRemoved(this, nIndex);
  return true;
}

bool CXFA_Node::IsHorizontalScrollPolicyOff() {
  CXFA_Node* pUIChild = GetUIChildNode();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::HScrollPolicy) ==
           XFA_AttributeValue::Off;
  }
  return false;
}

bool CXFA_Node::IsVerticalScrollPolicyOff() {
  CXFA_Node* pUIChild = GetUIChildNode();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::VScrollPolicy) ==
           XFA_AttributeValue::Off;
  }
  return false;
}

absl::optional<int32_t> CXFA_Node::GetNumberOfCells() {
  CXFA_Node* pUIChild = GetUIChildNode();
  if (!pUIChild)
    return absl::nullopt;

  CXFA_Comb* pNode = pUIChild->GetChild<CXFA_Comb>(0, XFA_Element::Comb, false);
  if (!pNode)
    return absl::nullopt;

  return pNode->JSObject()->GetInteger(XFA_Attribute::NumberOfCells);
}

bool CXFA_Node::IsMultiLine() {
  CXFA_Node* pUIChild = GetUIChildNode();
  return pUIChild && pUIChild->JSObject()->GetBoolean(XFA_Attribute::MultiLine);
}

std::pair<XFA_Element, int32_t> CXFA_Node::GetMaxChars() const {
  const auto* pNode = GetChild<CXFA_Value>(0, XFA_Element::Value, false);
  if (pNode) {
    if (CXFA_Node* pChild = pNode->GetFirstChild()) {
      switch (pChild->GetElementType()) {
        case XFA_Element::Text:
          return {XFA_Element::Text,
                  pChild->JSObject()->GetInteger(XFA_Attribute::MaxChars)};
        case XFA_Element::ExData: {
          int32_t iMax =
              pChild->JSObject()->GetInteger(XFA_Attribute::MaxLength);
          return {XFA_Element::ExData, iMax < 0 ? 0 : iMax};
        }
        default:
          break;
      }
    }
  }
  return {XFA_Element::Unknown, 0};
}

int32_t CXFA_Node::GetFracDigits() const {
  const auto* pNode = GetChild<CXFA_Value>(0, XFA_Element::Value, false);
  if (!pNode)
    return -1;

  const auto* pChild =
      pNode->GetChild<CXFA_Decimal>(0, XFA_Element::Decimal, false);
  if (!pChild)
    return -1;

  return pChild->JSObject()
      ->TryInteger(XFA_Attribute::FracDigits, true)
      .value_or(-1);
}

int32_t CXFA_Node::GetLeadDigits() const {
  const auto* pNode = GetChild<CXFA_Value>(0, XFA_Element::Value, false);
  if (!pNode)
    return -1;

  const auto* pChild =
      pNode->GetChild<CXFA_Decimal>(0, XFA_Element::Decimal, false);
  if (!pChild)
    return -1;

  return pChild->JSObject()
      ->TryInteger(XFA_Attribute::LeadDigits, true)
      .value_or(-1);
}

bool CXFA_Node::SetValue(XFA_ValuePicture eValueType,
                         const WideString& wsValue) {
  if (wsValue.IsEmpty()) {
    SyncValue(wsValue, true);
    return true;
  }

  SetPreNull(IsNull());
  SetIsNull(false);

  WideString wsNewText(wsValue);
  WideString wsPicture = GetPictureContent(eValueType);
  bool bValidate = true;
  bool bSyncData = false;
  CXFA_Node* pNode = GetUIChildNode();
  if (!pNode)
    return true;

  XFA_Element eType = pNode->GetElementType();
  if (!wsPicture.IsEmpty()) {
    CXFA_LocaleMgr* pLocaleMgr = GetDocument()->GetLocaleMgr();
    GCedLocaleIface* pLocale = GetLocale();
    CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
    bValidate =
        widgetValue.ValidateValue(wsValue, wsPicture, pLocale, &wsPicture);
    if (bValidate) {
      widgetValue = CXFA_LocaleValue(widgetValue.GetType(), wsNewText,
                                     wsPicture, pLocale, pLocaleMgr);
      wsNewText = widgetValue.GetValue();
      if (eType == XFA_Element::NumericEdit)
        wsNewText = NumericLimit(wsNewText);

      bSyncData = true;
    }
  } else if (eType == XFA_Element::NumericEdit) {
    if (!wsNewText.EqualsASCII("0"))
      wsNewText = NumericLimit(wsNewText);

    bSyncData = true;
  }
  if (eType != XFA_Element::NumericEdit || bSyncData)
    SyncValue(wsNewText, true);

  return bValidate;
}

WideString CXFA_Node::GetPictureContent(XFA_ValuePicture ePicture) {
  if (ePicture == XFA_ValuePicture::kRaw)
    return WideString();

  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
  switch (ePicture) {
    case XFA_ValuePicture::kDisplay: {
      auto* pFormat = GetChild<CXFA_Format>(0, XFA_Element::Format, false);
      if (pFormat) {
        auto* pPicture =
            pFormat->GetChild<CXFA_Picture>(0, XFA_Element::Picture, false);
        if (pPicture) {
          absl::optional<WideString> picture =
              pPicture->JSObject()->TryContent(false, true);
          if (picture.has_value())
            return picture.value();
        }
      }

      LocaleIface* pLocale = GetLocale();
      if (!pLocale)
        return WideString();

      switch (widgetValue.GetType()) {
        case CXFA_LocaleValue::ValueType::kDate:
          return pLocale->GetDatePattern(
              LocaleIface::DateTimeSubcategory::kMedium);
        case CXFA_LocaleValue::ValueType::kTime:
          return pLocale->GetTimePattern(
              LocaleIface::DateTimeSubcategory::kMedium);
        case CXFA_LocaleValue::ValueType::kDateTime:
          return pLocale->GetDatePattern(
                     LocaleIface::DateTimeSubcategory::kMedium) +
                 L"T" +
                 pLocale->GetTimePattern(
                     LocaleIface::DateTimeSubcategory::kMedium);
        case CXFA_LocaleValue::ValueType::kDecimal:
        case CXFA_LocaleValue::ValueType::kFloat:
        default:
          return WideString();
      }
    }
    case XFA_ValuePicture::kEdit: {
      CXFA_Ui* pUI = GetChild<CXFA_Ui>(0, XFA_Element::Ui, false);
      if (pUI) {
        if (CXFA_Picture* pPicture =
                pUI->GetChild<CXFA_Picture>(0, XFA_Element::Picture, false)) {
          absl::optional<WideString> picture =
              pPicture->JSObject()->TryContent(false, true);
          if (picture.has_value())
            return picture.value();
        }
      }

      LocaleIface* pLocale = GetLocale();
      if (!pLocale)
        return WideString();

      switch (widgetValue.GetType()) {
        case CXFA_LocaleValue::ValueType::kDate:
          return pLocale->GetDatePattern(
              LocaleIface::DateTimeSubcategory::kShort);
        case CXFA_LocaleValue::ValueType::kTime:
          return pLocale->GetTimePattern(
              LocaleIface::DateTimeSubcategory::kShort);
        case CXFA_LocaleValue::ValueType::kDateTime:
          return pLocale->GetDatePattern(
                     LocaleIface::DateTimeSubcategory::kShort) +
                 L"T" +
                 pLocale->GetTimePattern(
                     LocaleIface::DateTimeSubcategory::kShort);
        default:
          return WideString();
      }
    }
    case XFA_ValuePicture::kDataBind: {
      CXFA_Bind* bind = GetBindIfExists();
      if (bind)
        return bind->GetPicture();
      break;
    }
    default:
      break;
  }
  return WideString();
}

WideString CXFA_Node::GetValue(XFA_ValuePicture eValueType) {
  WideString wsValue = JSObject()->GetContent(false);

  if (eValueType == XFA_ValuePicture::kDisplay)
    wsValue = GetItemLabel(wsValue.AsStringView());

  WideString wsPicture = GetPictureContent(eValueType);
  CXFA_Node* pNode = GetUIChildNode();
  if (!pNode)
    return wsValue;

  switch (pNode->GetElementType()) {
    case XFA_Element::ChoiceList: {
      if (eValueType == XFA_ValuePicture::kDisplay) {
        int32_t iSelItemIndex = GetSelectedItem(0);
        if (iSelItemIndex >= 0) {
          wsValue =
              GetChoiceListItem(iSelItemIndex, false).value_or(WideString());
          wsPicture.clear();
        }
      }
      break;
    }
    case XFA_Element::NumericEdit:
      if (eValueType != XFA_ValuePicture::kRaw && wsPicture.IsEmpty()) {
        LocaleIface* pLocale = GetLocale();
        if (eValueType == XFA_ValuePicture::kDisplay && pLocale)
          wsValue = FormatNumStr(NormalizeNumStr(wsValue), pLocale);
      }
      break;
    default:
      break;
  }
  if (wsPicture.IsEmpty())
    return wsValue;

  GCedLocaleIface* pLocale = GetLocale();
  if (pLocale) {
    CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
    CXFA_LocaleMgr* pLocaleMgr = GetDocument()->GetLocaleMgr();
    switch (widgetValue.GetType()) {
      case CXFA_LocaleValue::ValueType::kDate: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue date(CXFA_LocaleValue::ValueType::kDate, wsDate,
                                pLocaleMgr);
          if (date.FormatPatterns(wsValue, wsPicture, pLocale, eValueType))
            return wsValue;
        }
        break;
      }
      case CXFA_LocaleValue::ValueType::kTime: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue time(CXFA_LocaleValue::ValueType::kTime, wsTime,
                                pLocaleMgr);
          if (time.FormatPatterns(wsValue, wsPicture, pLocale, eValueType))
            return wsValue;
        }
        break;
      }
      default:
        break;
    }
    widgetValue.FormatPatterns(wsValue, wsPicture, pLocale, eValueType);
  }
  return wsValue;
}

WideString CXFA_Node::GetNormalizeDataValue(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return WideString();

  WideString wsPicture = GetPictureContent(XFA_ValuePicture::kDataBind);
  if (wsPicture.IsEmpty())
    return wsValue;

  CXFA_LocaleMgr* pLocaleMgr = GetDocument()->GetLocaleMgr();
  GCedLocaleIface* pLocale = GetLocale();
  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(this);
  if (widgetValue.ValidateValue(wsValue, wsPicture, pLocale, &wsPicture)) {
    widgetValue = CXFA_LocaleValue(widgetValue.GetType(), wsValue, wsPicture,
                                   pLocale, pLocaleMgr);
    return widgetValue.GetValue();
  }
  return wsValue;
}

WideString CXFA_Node::GetFormatDataValue(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return WideString();

  WideString wsPicture = GetPictureContent(XFA_ValuePicture::kDataBind);
  if (wsPicture.IsEmpty())
    return wsValue;

  WideString wsFormattedValue = wsValue;
  GCedLocaleIface* pLocale = GetLocale();
  if (pLocale) {
    CXFA_Value* pNodeValue = GetChild<CXFA_Value>(0, XFA_Element::Value, false);
    if (!pNodeValue)
      return wsValue;

    CXFA_Node* pValueChild = pNodeValue->GetFirstChild();
    if (!pValueChild)
      return wsValue;

    CXFA_LocaleValue::ValueType iVTType =
        XFA_GetLocaleValueType(pValueChild->GetElementType());
    CXFA_LocaleMgr* pLocaleMgr = GetDocument()->GetLocaleMgr();
    CXFA_LocaleValue widgetValue(iVTType, wsValue, pLocaleMgr);
    switch (widgetValue.GetType()) {
      case CXFA_LocaleValue::ValueType::kDate: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue date(CXFA_LocaleValue::ValueType::kDate, wsDate,
                                pLocaleMgr);
          if (date.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                                  XFA_ValuePicture::kDataBind)) {
            return wsFormattedValue;
          }
        }
        break;
      }
      case CXFA_LocaleValue::ValueType::kTime: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue time(CXFA_LocaleValue::ValueType::kTime, wsTime,
                                pLocaleMgr);
          if (time.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                                  XFA_ValuePicture::kDataBind)) {
            return wsFormattedValue;
          }
        }
        break;
      }
      default:
        break;
    }
    widgetValue.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                               XFA_ValuePicture::kDataBind);
  }
  return wsFormattedValue;
}

WideString CXFA_Node::NormalizeNumStr(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return WideString();

  WideString wsOutput = wsValue;
  wsOutput.TrimLeft('0');

  if (!wsOutput.IsEmpty() && wsOutput.Contains('.') && GetFracDigits() != -1) {
    wsOutput.TrimRight(L"0");
    wsOutput.TrimRight(L".");
  }
  if (wsOutput.IsEmpty() || wsOutput[0] == '.')
    wsOutput.InsertAtFront('0');

  return wsOutput;
}

void CXFA_Node::InsertListTextItem(CXFA_Node* pItems,
                                   const WideString& wsText,
                                   int32_t nIndex) {
  CXFA_Node* pText = pItems->CreateSamePacketNode(XFA_Element::Text);
  pItems->InsertChildAndNotify(nIndex, pText);
  pText->JSObject()->SetContent(wsText, wsText, false, false, false);
}

WideString CXFA_Node::NumericLimit(const WideString& wsValue) {
  int32_t iLead = GetLeadDigits();
  int32_t iTread = GetFracDigits();
  if (iLead == -1 && iTread == -1)
    return wsValue;

  int32_t iCount = pdfium::base::checked_cast<int32_t>(wsValue.GetLength());
  if (iCount == 0)
    return wsValue;

  WideString wsRet;
  int32_t i = 0;
  if (wsValue[i] == L'-') {
    wsRet += L'-';
    i++;
  }

  int32_t iLead2 = 0;
  int32_t iTread2 = -1;
  for (; i < iCount; i++) {
    wchar_t wc = wsValue[i];
    if (FXSYS_IsDecimalDigit(wc)) {
      if (iLead >= 0) {
        iLead2++;
        if (iLead2 > iLead)
          return L"0";
      } else if (iTread2 >= 0) {
        iTread2++;
        if (iTread2 > iTread) {
          if (iTread != -1) {
            CFGAS_Decimal wsDeci = CFGAS_Decimal(wsValue.AsStringView());
            wsDeci.SetScale(iTread);
            wsRet = wsDeci.ToWideString();
          }
          return wsRet;
        }
      }
    } else if (wc == L'.') {
      iTread2 = 0;
      iLead = -1;
    }
    wsRet += wc;
  }
  return wsRet;
}

bool CXFA_Node::IsTransparent() const {
  XFA_Element type = GetElementType();
  return type == XFA_Element::SubformSet || type == XFA_Element::Area ||
         type == XFA_Element::Proto || (IsUnnamed() && IsContainerNode());
}

bool CXFA_Node::IsProperty() const {
  CXFA_Node* parent = GetParent();
  return parent && parent->HasProperty(GetElementType());
}

bool CXFA_Node::PresenceRequiresSpace() const {
  auto value = JSObject()->TryEnum(XFA_Attribute::Presence, true);
  XFA_AttributeValue ePresence = value.value_or(XFA_AttributeValue::Visible);
  return ePresence == XFA_AttributeValue::Visible ||
         ePresence == XFA_AttributeValue::Invisible;
}

void CXFA_Node::SetBindingNode(CXFA_Node* node) {
  binding_nodes_.clear();
  if (node)
    binding_nodes_.emplace_back(node);
}

void CXFA_Node::SetNodeAndDescendantsUnused() {
  CXFA_NodeIterator sIterator(this);
  for (CXFA_Node* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    pNode->SetFlag(XFA_NodeFlag::kUnusedNode);
  }
}

void CXFA_Node::SetToXML(const WideString& value) {
  auto* pNode = GetXMLMappingNode();
  switch (pNode->GetType()) {
    case CFX_XMLNode::Type::kElement: {
      auto* elem = static_cast<CFX_XMLElement*>(pNode);
      if (IsAttributeInXML()) {
        elem->SetAttribute(JSObject()->GetCData(XFA_Attribute::QualifiedName),
                           value);
        return;
      }

      bool bDeleteChildren = true;
      if (GetPacketType() == XFA_PacketType::Datasets) {
        for (CXFA_Node* pChildDataNode = GetFirstChild(); pChildDataNode;
             pChildDataNode = pChildDataNode->GetNextSibling()) {
          if (pChildDataNode->HasBindItems()) {
            bDeleteChildren = false;
            break;
          }
        }
      }
      if (bDeleteChildren)
        elem->RemoveAllChildren();

      auto* text = GetXMLDocument()->CreateNode<CFX_XMLText>(value);
      elem->AppendLastChild(text);
      break;
    }
    case CFX_XMLNode::Type::kText:
      ToXMLText(GetXMLMappingNode())->SetText(value);
      break;
    default:
      NOTREACHED_NORETURN();
  }
}

CXFA_Node* CXFA_Node::GetTransparentParent() {
  CXFA_Node* parent = GetParent();
  while (parent) {
    XFA_Element type = parent->GetElementType();
    if (type == XFA_Element::Variables ||
        (type != XFA_Element::SubformSet && !parent->IsUnnamed())) {
      return parent;
    }
    parent = parent->GetParent();
  }
  return nullptr;
}

CFX_XMLDocument* CXFA_Node::GetXMLDocument() const {
  return GetDocument()->GetNotify()->GetFFDoc()->GetXMLDocument();
}

// static
CXFA_Node* CXFA_Node::Create(CXFA_Document* doc,
                             XFA_Element element,
                             XFA_PacketType packet) {
  CXFA_Node* node = nullptr;
  switch (element) {
    case XFA_Element::Ps:
      node = cppgc::MakeGarbageCollected<CXFA_Ps>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::To:
      node = cppgc::MakeGarbageCollected<CXFA_To>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Ui:
      node = cppgc::MakeGarbageCollected<CXFA_Ui>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::RecordSet:
      node = cppgc::MakeGarbageCollected<CXFA_RecordSet>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SubsetBelow:
      node = cppgc::MakeGarbageCollected<CXFA_SubsetBelow>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SubformSet:
      node = cppgc::MakeGarbageCollected<CXFA_SubformSet>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::AdobeExtensionLevel:
      node = cppgc::MakeGarbageCollected<CXFA_AdobeExtensionLevel>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Typeface:
      node = cppgc::MakeGarbageCollected<CXFA_Typeface>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Break:
      node = cppgc::MakeGarbageCollected<CXFA_Break>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::FontInfo:
      node = cppgc::MakeGarbageCollected<CXFA_FontInfo>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::NumberPattern:
      node = cppgc::MakeGarbageCollected<CXFA_NumberPattern>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DynamicRender:
      node = cppgc::MakeGarbageCollected<CXFA_DynamicRender>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PrintScaling:
      node = cppgc::MakeGarbageCollected<CXFA_PrintScaling>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::CheckButton:
      node = cppgc::MakeGarbageCollected<CXFA_CheckButton>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DatePatterns:
      node = cppgc::MakeGarbageCollected<CXFA_DatePatterns>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SourceSet:
      node = cppgc::MakeGarbageCollected<CXFA_SourceSet>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Amd:
      node = cppgc::MakeGarbageCollected<CXFA_Amd>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Arc:
      node = cppgc::MakeGarbageCollected<CXFA_Arc>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Day:
      node = cppgc::MakeGarbageCollected<CXFA_Day>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Era:
      node = cppgc::MakeGarbageCollected<CXFA_Era>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Jog:
      node = cppgc::MakeGarbageCollected<CXFA_Jog>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Log:
      node = cppgc::MakeGarbageCollected<CXFA_Log>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Map:
      node = cppgc::MakeGarbageCollected<CXFA_Map>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Mdp:
      node = cppgc::MakeGarbageCollected<CXFA_Mdp>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::BreakBefore:
      node = cppgc::MakeGarbageCollected<CXFA_BreakBefore>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Oid:
      node = cppgc::MakeGarbageCollected<CXFA_Oid>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Pcl:
      node = cppgc::MakeGarbageCollected<CXFA_Pcl>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Pdf:
      node = cppgc::MakeGarbageCollected<CXFA_Pdf>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Ref:
      node = cppgc::MakeGarbageCollected<CXFA_Ref>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Uri:
      node = cppgc::MakeGarbageCollected<CXFA_Uri>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Xdc:
      node = cppgc::MakeGarbageCollected<CXFA_Xdc>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Xdp:
      node = cppgc::MakeGarbageCollected<CXFA_Xdp>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Xfa:
      node = cppgc::MakeGarbageCollected<CXFA_Xfa>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Xsl:
      node = cppgc::MakeGarbageCollected<CXFA_Xsl>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Zpl:
      node = cppgc::MakeGarbageCollected<CXFA_Zpl>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Cache:
      node = cppgc::MakeGarbageCollected<CXFA_Cache>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Margin:
      node = cppgc::MakeGarbageCollected<CXFA_Margin>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::KeyUsage:
      node = cppgc::MakeGarbageCollected<CXFA_KeyUsage>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Exclude:
      node = cppgc::MakeGarbageCollected<CXFA_Exclude>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ChoiceList:
      node = cppgc::MakeGarbageCollected<CXFA_ChoiceList>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Level:
      node = cppgc::MakeGarbageCollected<CXFA_Level>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::LabelPrinter:
      node = cppgc::MakeGarbageCollected<CXFA_LabelPrinter>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::CalendarSymbols:
      node = cppgc::MakeGarbageCollected<CXFA_CalendarSymbols>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Para:
      node = cppgc::MakeGarbageCollected<CXFA_Para>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Part:
      node = cppgc::MakeGarbageCollected<CXFA_Part>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Pdfa:
      node = cppgc::MakeGarbageCollected<CXFA_Pdfa>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Filter:
      node = cppgc::MakeGarbageCollected<CXFA_Filter>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Present:
      node = cppgc::MakeGarbageCollected<CXFA_Present>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Pagination:
      node = cppgc::MakeGarbageCollected<CXFA_Pagination>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Encoding:
      node = cppgc::MakeGarbageCollected<CXFA_Encoding>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Event:
      node = cppgc::MakeGarbageCollected<CXFA_Event>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Whitespace:
      node = cppgc::MakeGarbageCollected<CXFA_Whitespace>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DefaultUi:
      node = cppgc::MakeGarbageCollected<CXFA_DefaultUi>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DataModel:
      node = cppgc::MakeGarbageCollected<CXFA_DataModel>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Barcode:
      node = cppgc::MakeGarbageCollected<CXFA_Barcode>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::TimePattern:
      node = cppgc::MakeGarbageCollected<CXFA_TimePattern>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::BatchOutput:
      node = cppgc::MakeGarbageCollected<CXFA_BatchOutput>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Enforce:
      node = cppgc::MakeGarbageCollected<CXFA_Enforce>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::CurrencySymbols:
      node = cppgc::MakeGarbageCollected<CXFA_CurrencySymbols>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::AddSilentPrint:
      node = cppgc::MakeGarbageCollected<CXFA_AddSilentPrint>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Rename:
      node = cppgc::MakeGarbageCollected<CXFA_Rename>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Operation:
      node = cppgc::MakeGarbageCollected<CXFA_Operation>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Typefaces:
      node = cppgc::MakeGarbageCollected<CXFA_Typefaces>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SubjectDNs:
      node = cppgc::MakeGarbageCollected<CXFA_SubjectDNs>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Issuers:
      node = cppgc::MakeGarbageCollected<CXFA_Issuers>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::WsdlConnection:
      node = cppgc::MakeGarbageCollected<CXFA_WsdlConnection>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Debug:
      node = cppgc::MakeGarbageCollected<CXFA_Debug>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Delta:
      node = cppgc::MakeGarbageCollected<CXFA_Delta>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::EraNames:
      node = cppgc::MakeGarbageCollected<CXFA_EraNames>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ModifyAnnots:
      node = cppgc::MakeGarbageCollected<CXFA_ModifyAnnots>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::StartNode:
      node = cppgc::MakeGarbageCollected<CXFA_StartNode>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Button:
      node = cppgc::MakeGarbageCollected<CXFA_Button>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Format:
      node = cppgc::MakeGarbageCollected<CXFA_Format>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Border:
      node = cppgc::MakeGarbageCollected<CXFA_Border>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Area:
      node = cppgc::MakeGarbageCollected<CXFA_Area>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Hyphenation:
      node = cppgc::MakeGarbageCollected<CXFA_Hyphenation>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Text:
      node = cppgc::MakeGarbageCollected<CXFA_Text>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Time:
      node = cppgc::MakeGarbageCollected<CXFA_Time>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Type:
      node = cppgc::MakeGarbageCollected<CXFA_Type>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Overprint:
      node = cppgc::MakeGarbageCollected<CXFA_Overprint>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Certificates:
      node = cppgc::MakeGarbageCollected<CXFA_Certificates>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::EncryptionMethods:
      node = cppgc::MakeGarbageCollected<CXFA_EncryptionMethods>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SetProperty:
      node = cppgc::MakeGarbageCollected<CXFA_SetProperty>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PrinterName:
      node = cppgc::MakeGarbageCollected<CXFA_PrinterName>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::StartPage:
      node = cppgc::MakeGarbageCollected<CXFA_StartPage>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PageOffset:
      node = cppgc::MakeGarbageCollected<CXFA_PageOffset>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DateTime:
      node = cppgc::MakeGarbageCollected<CXFA_DateTime>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Comb:
      node = cppgc::MakeGarbageCollected<CXFA_Comb>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Pattern:
      node = cppgc::MakeGarbageCollected<CXFA_Pattern>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::IfEmpty:
      node = cppgc::MakeGarbageCollected<CXFA_IfEmpty>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SuppressBanner:
      node = cppgc::MakeGarbageCollected<CXFA_SuppressBanner>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::OutputBin:
      node = cppgc::MakeGarbageCollected<CXFA_OutputBin>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Field:
      node = cppgc::MakeGarbageCollected<CXFA_Field>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Agent:
      node = cppgc::MakeGarbageCollected<CXFA_Agent>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::OutputXSL:
      node = cppgc::MakeGarbageCollected<CXFA_OutputXSL>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::AdjustData:
      node = cppgc::MakeGarbageCollected<CXFA_AdjustData>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::AutoSave:
      node = cppgc::MakeGarbageCollected<CXFA_AutoSave>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ContentArea:
      node = cppgc::MakeGarbageCollected<CXFA_ContentArea>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::WsdlAddress:
      node = cppgc::MakeGarbageCollected<CXFA_WsdlAddress>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Solid:
      node = cppgc::MakeGarbageCollected<CXFA_Solid>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DateTimeSymbols:
      node = cppgc::MakeGarbageCollected<CXFA_DateTimeSymbols>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::EncryptionLevel:
      node = cppgc::MakeGarbageCollected<CXFA_EncryptionLevel>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Edge:
      node = cppgc::MakeGarbageCollected<CXFA_Edge>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Stipple:
      node = cppgc::MakeGarbageCollected<CXFA_Stipple>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Attributes:
      node = cppgc::MakeGarbageCollected<CXFA_Attributes>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::VersionControl:
      node = cppgc::MakeGarbageCollected<CXFA_VersionControl>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Meridiem:
      node = cppgc::MakeGarbageCollected<CXFA_Meridiem>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ExclGroup:
      node = cppgc::MakeGarbageCollected<CXFA_ExclGroup>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ToolTip:
      node = cppgc::MakeGarbageCollected<CXFA_ToolTip>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Compress:
      node = cppgc::MakeGarbageCollected<CXFA_Compress>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Reason:
      node = cppgc::MakeGarbageCollected<CXFA_Reason>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Execute:
      node = cppgc::MakeGarbageCollected<CXFA_Execute>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ContentCopy:
      node = cppgc::MakeGarbageCollected<CXFA_ContentCopy>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DateTimeEdit:
      node = cppgc::MakeGarbageCollected<CXFA_DateTimeEdit>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Config:
      node = cppgc::MakeGarbageCollected<CXFA_Config>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Image:
      node = cppgc::MakeGarbageCollected<CXFA_Image>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SharpxHTML:
      node = cppgc::MakeGarbageCollected<CXFA_SharpxHTML>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::NumberOfCopies:
      node = cppgc::MakeGarbageCollected<CXFA_NumberOfCopies>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::BehaviorOverride:
      node = cppgc::MakeGarbageCollected<CXFA_BehaviorOverride>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::TimeStamp:
      node = cppgc::MakeGarbageCollected<CXFA_TimeStamp>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Month:
      node = cppgc::MakeGarbageCollected<CXFA_Month>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ViewerPreferences:
      node = cppgc::MakeGarbageCollected<CXFA_ViewerPreferences>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ScriptModel:
      node = cppgc::MakeGarbageCollected<CXFA_ScriptModel>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Decimal:
      node = cppgc::MakeGarbageCollected<CXFA_Decimal>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Subform:
      node = cppgc::MakeGarbageCollected<CXFA_Subform>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Select:
      node = cppgc::MakeGarbageCollected<CXFA_Select>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Window:
      node = cppgc::MakeGarbageCollected<CXFA_Window>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::LocaleSet:
      node = cppgc::MakeGarbageCollected<CXFA_LocaleSet>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Handler:
      node = cppgc::MakeGarbageCollected<CXFA_Handler>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Presence:
      node = cppgc::MakeGarbageCollected<CXFA_Presence>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Record:
      node = cppgc::MakeGarbageCollected<CXFA_Record>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Embed:
      node = cppgc::MakeGarbageCollected<CXFA_Embed>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Version:
      node = cppgc::MakeGarbageCollected<CXFA_Version>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Command:
      node = cppgc::MakeGarbageCollected<CXFA_Command>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Copies:
      node = cppgc::MakeGarbageCollected<CXFA_Copies>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Staple:
      node = cppgc::MakeGarbageCollected<CXFA_Staple>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SubmitFormat:
      node = cppgc::MakeGarbageCollected<CXFA_SubmitFormat>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Boolean:
      node = cppgc::MakeGarbageCollected<CXFA_Boolean>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Message:
      node = cppgc::MakeGarbageCollected<CXFA_Message>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Output:
      node = cppgc::MakeGarbageCollected<CXFA_Output>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PsMap:
      node = cppgc::MakeGarbageCollected<CXFA_PsMap>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ExcludeNS:
      node = cppgc::MakeGarbageCollected<CXFA_ExcludeNS>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Assist:
      node = cppgc::MakeGarbageCollected<CXFA_Assist>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Picture:
      node = cppgc::MakeGarbageCollected<CXFA_Picture>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Traversal:
      node = cppgc::MakeGarbageCollected<CXFA_Traversal>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SilentPrint:
      node = cppgc::MakeGarbageCollected<CXFA_SilentPrint>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::WebClient:
      node = cppgc::MakeGarbageCollected<CXFA_WebClient>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Producer:
      node = cppgc::MakeGarbageCollected<CXFA_Producer>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Corner:
      node = cppgc::MakeGarbageCollected<CXFA_Corner>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::MsgId:
      node = cppgc::MakeGarbageCollected<CXFA_MsgId>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Color:
      node = cppgc::MakeGarbageCollected<CXFA_Color>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Keep:
      node = cppgc::MakeGarbageCollected<CXFA_Keep>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Query:
      node = cppgc::MakeGarbageCollected<CXFA_Query>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Insert:
      node = cppgc::MakeGarbageCollected<CXFA_Insert>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ImageEdit:
      node = cppgc::MakeGarbageCollected<CXFA_ImageEdit>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Validate:
      node = cppgc::MakeGarbageCollected<CXFA_Validate>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DigestMethods:
      node = cppgc::MakeGarbageCollected<CXFA_DigestMethods>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::NumberPatterns:
      node = cppgc::MakeGarbageCollected<CXFA_NumberPatterns>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PageSet:
      node = cppgc::MakeGarbageCollected<CXFA_PageSet>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Integer:
      node = cppgc::MakeGarbageCollected<CXFA_Integer>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SoapAddress:
      node = cppgc::MakeGarbageCollected<CXFA_SoapAddress>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Equate:
      node = cppgc::MakeGarbageCollected<CXFA_Equate>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::FormFieldFilling:
      node = cppgc::MakeGarbageCollected<CXFA_FormFieldFilling>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PageRange:
      node = cppgc::MakeGarbageCollected<CXFA_PageRange>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Update:
      node = cppgc::MakeGarbageCollected<CXFA_Update>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ConnectString:
      node = cppgc::MakeGarbageCollected<CXFA_ConnectString>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Mode:
      node = cppgc::MakeGarbageCollected<CXFA_Mode>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Layout:
      node = cppgc::MakeGarbageCollected<CXFA_Layout>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Sharpxml:
      node = cppgc::MakeGarbageCollected<CXFA_Sharpxml>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::XsdConnection:
      node = cppgc::MakeGarbageCollected<CXFA_XsdConnection>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Traverse:
      node = cppgc::MakeGarbageCollected<CXFA_Traverse>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Encodings:
      node = cppgc::MakeGarbageCollected<CXFA_Encodings>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Template:
      node = cppgc::MakeGarbageCollected<CXFA_Template>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Acrobat:
      node = cppgc::MakeGarbageCollected<CXFA_Acrobat>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ValidationMessaging:
      node = cppgc::MakeGarbageCollected<CXFA_ValidationMessaging>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Signing:
      node = cppgc::MakeGarbageCollected<CXFA_Signing>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Script:
      node = cppgc::MakeGarbageCollected<CXFA_Script>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::AddViewerPreferences:
      node = cppgc::MakeGarbageCollected<CXFA_AddViewerPreferences>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::AlwaysEmbed:
      node = cppgc::MakeGarbageCollected<CXFA_AlwaysEmbed>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PasswordEdit:
      node = cppgc::MakeGarbageCollected<CXFA_PasswordEdit>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::NumericEdit:
      node = cppgc::MakeGarbageCollected<CXFA_NumericEdit>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::EncryptionMethod:
      node = cppgc::MakeGarbageCollected<CXFA_EncryptionMethod>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Change:
      node = cppgc::MakeGarbageCollected<CXFA_Change>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PageArea:
      node = cppgc::MakeGarbageCollected<CXFA_PageArea>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SubmitUrl:
      node = cppgc::MakeGarbageCollected<CXFA_SubmitUrl>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Oids:
      node = cppgc::MakeGarbageCollected<CXFA_Oids>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Signature:
      node = cppgc::MakeGarbageCollected<CXFA_Signature>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ADBE_JSConsole:
      node = cppgc::MakeGarbageCollected<CXFA_ADBE_JSConsole>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Caption:
      node = cppgc::MakeGarbageCollected<CXFA_Caption>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Relevant:
      node = cppgc::MakeGarbageCollected<CXFA_Relevant>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::FlipLabel:
      node = cppgc::MakeGarbageCollected<CXFA_FlipLabel>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ExData:
      node = cppgc::MakeGarbageCollected<CXFA_ExData>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DayNames:
      node = cppgc::MakeGarbageCollected<CXFA_DayNames>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SoapAction:
      node = cppgc::MakeGarbageCollected<CXFA_SoapAction>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DefaultTypeface:
      node = cppgc::MakeGarbageCollected<CXFA_DefaultTypeface>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Manifest:
      node = cppgc::MakeGarbageCollected<CXFA_Manifest>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Overflow:
      node = cppgc::MakeGarbageCollected<CXFA_Overflow>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Linear:
      node = cppgc::MakeGarbageCollected<CXFA_Linear>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::CurrencySymbol:
      node = cppgc::MakeGarbageCollected<CXFA_CurrencySymbol>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Delete:
      node = cppgc::MakeGarbageCollected<CXFA_Delete>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DigestMethod:
      node = cppgc::MakeGarbageCollected<CXFA_DigestMethod>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::InstanceManager:
      node = cppgc::MakeGarbageCollected<CXFA_InstanceManager>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::EquateRange:
      node = cppgc::MakeGarbageCollected<CXFA_EquateRange>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Medium:
      node = cppgc::MakeGarbageCollected<CXFA_Medium>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::TextEdit:
      node = cppgc::MakeGarbageCollected<CXFA_TextEdit>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::TemplateCache:
      node = cppgc::MakeGarbageCollected<CXFA_TemplateCache>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::CompressObjectStream:
      node = cppgc::MakeGarbageCollected<CXFA_CompressObjectStream>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DataValue:
      node = cppgc::MakeGarbageCollected<CXFA_DataValue>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::AccessibleContent:
      node = cppgc::MakeGarbageCollected<CXFA_AccessibleContent>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::IncludeXDPContent:
      node = cppgc::MakeGarbageCollected<CXFA_IncludeXDPContent>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::XmlConnection:
      node = cppgc::MakeGarbageCollected<CXFA_XmlConnection>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ValidateApprovalSignatures:
      node = cppgc::MakeGarbageCollected<CXFA_ValidateApprovalSignatures>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SignData:
      node = cppgc::MakeGarbageCollected<CXFA_SignData>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Packets:
      node = cppgc::MakeGarbageCollected<CXFA_Packets>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DatePattern:
      node = cppgc::MakeGarbageCollected<CXFA_DatePattern>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DuplexOption:
      node = cppgc::MakeGarbageCollected<CXFA_DuplexOption>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Base:
      node = cppgc::MakeGarbageCollected<CXFA_Base>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Bind:
      node = cppgc::MakeGarbageCollected<CXFA_Bind>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Compression:
      node = cppgc::MakeGarbageCollected<CXFA_Compression>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::User:
      node = cppgc::MakeGarbageCollected<CXFA_User>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Rectangle:
      node = cppgc::MakeGarbageCollected<CXFA_Rectangle>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::EffectiveOutputPolicy:
      node = cppgc::MakeGarbageCollected<CXFA_EffectiveOutputPolicy>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ADBE_JSDebugger:
      node = cppgc::MakeGarbageCollected<CXFA_ADBE_JSDebugger>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Acrobat7:
      node = cppgc::MakeGarbageCollected<CXFA_Acrobat7>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Interactive:
      node = cppgc::MakeGarbageCollected<CXFA_Interactive>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Locale:
      node = cppgc::MakeGarbageCollected<CXFA_Locale>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::CurrentPage:
      node = cppgc::MakeGarbageCollected<CXFA_CurrentPage>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Data:
      node = cppgc::MakeGarbageCollected<CXFA_Data>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Date:
      node = cppgc::MakeGarbageCollected<CXFA_Date>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Desc:
      node = cppgc::MakeGarbageCollected<CXFA_Desc>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Encrypt:
      node = cppgc::MakeGarbageCollected<CXFA_Encrypt>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Draw:
      node = cppgc::MakeGarbageCollected<CXFA_Draw>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Encryption:
      node = cppgc::MakeGarbageCollected<CXFA_Encryption>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::MeridiemNames:
      node = cppgc::MakeGarbageCollected<CXFA_MeridiemNames>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Messaging:
      node = cppgc::MakeGarbageCollected<CXFA_Messaging>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Speak:
      node = cppgc::MakeGarbageCollected<CXFA_Speak>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DataGroup:
      node = cppgc::MakeGarbageCollected<CXFA_DataGroup>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Common:
      node = cppgc::MakeGarbageCollected<CXFA_Common>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Sharptext:
      node = cppgc::MakeGarbageCollected<CXFA_Sharptext>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PaginationOverride:
      node = cppgc::MakeGarbageCollected<CXFA_PaginationOverride>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Reasons:
      node = cppgc::MakeGarbageCollected<CXFA_Reasons>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SignatureProperties:
      node = cppgc::MakeGarbageCollected<CXFA_SignatureProperties>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Threshold:
      node = cppgc::MakeGarbageCollected<CXFA_Threshold>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::AppearanceFilter:
      node = cppgc::MakeGarbageCollected<CXFA_AppearanceFilter>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Fill:
      node = cppgc::MakeGarbageCollected<CXFA_Fill>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Font:
      node = cppgc::MakeGarbageCollected<CXFA_Font>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Form:
      node = cppgc::MakeGarbageCollected<CXFA_Form>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::MediumInfo:
      node = cppgc::MakeGarbageCollected<CXFA_MediumInfo>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Certificate:
      node = cppgc::MakeGarbageCollected<CXFA_Certificate>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Password:
      node = cppgc::MakeGarbageCollected<CXFA_Password>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::RunScripts:
      node = cppgc::MakeGarbageCollected<CXFA_RunScripts>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Trace:
      node = cppgc::MakeGarbageCollected<CXFA_Trace>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Float:
      node = cppgc::MakeGarbageCollected<CXFA_Float>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::RenderPolicy:
      node = cppgc::MakeGarbageCollected<CXFA_RenderPolicy>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Destination:
      node = cppgc::MakeGarbageCollected<CXFA_Destination>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Value:
      node = cppgc::MakeGarbageCollected<CXFA_Value>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Bookend:
      node = cppgc::MakeGarbageCollected<CXFA_Bookend>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ExObject:
      node = cppgc::MakeGarbageCollected<CXFA_ExObject>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::OpenAction:
      node = cppgc::MakeGarbageCollected<CXFA_OpenAction>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::NeverEmbed:
      node = cppgc::MakeGarbageCollected<CXFA_NeverEmbed>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::BindItems:
      node = cppgc::MakeGarbageCollected<CXFA_BindItems>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Calculate:
      node = cppgc::MakeGarbageCollected<CXFA_Calculate>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Print:
      node = cppgc::MakeGarbageCollected<CXFA_Print>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Extras:
      node = cppgc::MakeGarbageCollected<CXFA_Extras>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Proto:
      node = cppgc::MakeGarbageCollected<CXFA_Proto>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DSigData:
      node = cppgc::MakeGarbageCollected<CXFA_DSigData>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Creator:
      node = cppgc::MakeGarbageCollected<CXFA_Creator>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Connect:
      node = cppgc::MakeGarbageCollected<CXFA_Connect>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Permissions:
      node = cppgc::MakeGarbageCollected<CXFA_Permissions>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::ConnectionSet:
      node = cppgc::MakeGarbageCollected<CXFA_ConnectionSet>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Submit:
      node = cppgc::MakeGarbageCollected<CXFA_Submit>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Range:
      node = cppgc::MakeGarbageCollected<CXFA_Range>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Linearized:
      node = cppgc::MakeGarbageCollected<CXFA_Linearized>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Packet:
      node = cppgc::MakeGarbageCollected<CXFA_Packet>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::RootElement:
      node = cppgc::MakeGarbageCollected<CXFA_RootElement>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PlaintextMetadata:
      node = cppgc::MakeGarbageCollected<CXFA_PlaintextMetadata>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::NumberSymbols:
      node = cppgc::MakeGarbageCollected<CXFA_NumberSymbols>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PrintHighQuality:
      node = cppgc::MakeGarbageCollected<CXFA_PrintHighQuality>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Driver:
      node = cppgc::MakeGarbageCollected<CXFA_Driver>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::IncrementalLoad:
      node = cppgc::MakeGarbageCollected<CXFA_IncrementalLoad>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::SubjectDN:
      node = cppgc::MakeGarbageCollected<CXFA_SubjectDN>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::CompressLogicalStructure:
      node = cppgc::MakeGarbageCollected<CXFA_CompressLogicalStructure>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::IncrementalMerge:
      node = cppgc::MakeGarbageCollected<CXFA_IncrementalMerge>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Radial:
      node = cppgc::MakeGarbageCollected<CXFA_Radial>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Variables:
      node = cppgc::MakeGarbageCollected<CXFA_Variables>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::TimePatterns:
      node = cppgc::MakeGarbageCollected<CXFA_TimePatterns>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::EffectiveInputPolicy:
      node = cppgc::MakeGarbageCollected<CXFA_EffectiveInputPolicy>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::NameAttr:
      node = cppgc::MakeGarbageCollected<CXFA_NameAttr>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Conformance:
      node = cppgc::MakeGarbageCollected<CXFA_Conformance>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Transform:
      node = cppgc::MakeGarbageCollected<CXFA_Transform>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::LockDocument:
      node = cppgc::MakeGarbageCollected<CXFA_LockDocument>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::BreakAfter:
      node = cppgc::MakeGarbageCollected<CXFA_BreakAfter>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Line:
      node = cppgc::MakeGarbageCollected<CXFA_Line>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Source:
      node = cppgc::MakeGarbageCollected<CXFA_Source>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Occur:
      node = cppgc::MakeGarbageCollected<CXFA_Occur>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::PickTrayByPDFSize:
      node = cppgc::MakeGarbageCollected<CXFA_PickTrayByPDFSize>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::MonthNames:
      node = cppgc::MakeGarbageCollected<CXFA_MonthNames>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Severity:
      node = cppgc::MakeGarbageCollected<CXFA_Severity>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::GroupParent:
      node = cppgc::MakeGarbageCollected<CXFA_GroupParent>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DocumentAssembly:
      node = cppgc::MakeGarbageCollected<CXFA_DocumentAssembly>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::NumberSymbol:
      node = cppgc::MakeGarbageCollected<CXFA_NumberSymbol>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Tagged:
      node = cppgc::MakeGarbageCollected<CXFA_Tagged>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::Items:
      node = cppgc::MakeGarbageCollected<CXFA_Items>(
          doc->GetHeap()->GetAllocationHandle(), doc, packet);
      break;
    case XFA_Element::DataWindow:
    case XFA_Element::Deltas:
    case XFA_Element::EventPseudoModel:
    case XFA_Element::HostPseudoModel:
    case XFA_Element::LayoutPseudoModel:
    case XFA_Element::List:
    case XFA_Element::ListDuplicate:
    case XFA_Element::LogPseudoModel:
    case XFA_Element::Model:
    case XFA_Element::Node:
    case XFA_Element::NodeWithDesc:
    case XFA_Element::NodeWithUse:
    case XFA_Element::NodeWithValue:
    case XFA_Element::Object:
    case XFA_Element::SignaturePseudoModel:
    case XFA_Element::Tree:
    case XFA_Element::TreeList:
    case XFA_Element::Unknown:
      // These defined elements can not be made from an XML parse. Some are
      // not CXFA_Node sub-classes, some are only used as intermediate classes,
      // and so forth.
      return nullptr;
  }
  if (!node || !node->IsValidInPacket(packet))
    return nullptr;
  return node;
}
