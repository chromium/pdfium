
//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.3
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Line dash generator
//
//----------------------------------------------------------------------------

#include <cmath>

#include "agg_shorten_path.h"
#include "agg_vcgen_dash.h"
#include "core/fxcrt/check_op.h"

namespace pdfium
{
namespace agg
{
vcgen_dash::vcgen_dash() :
    m_total_dash_len(0),
    m_num_dashes(0),
    m_dash_start(0),
    m_shorten(0),
    m_curr_dash_start(0),
    m_curr_dash(0),
    m_src_vertices(),
    m_closed(0),
    m_status(initial),
    m_src_vertex(0)
{
}
void vcgen_dash::remove_all_dashes()
{
    m_total_dash_len = 0;
    m_num_dashes = 0;
    m_curr_dash_start = 0;
    m_curr_dash = 0;
}
void vcgen_dash::add_dash(float dash_len, float gap_len)
{
    if(m_num_dashes < max_dashes) {
        m_total_dash_len += dash_len + gap_len;
        m_dashes[m_num_dashes++] = dash_len;
        m_dashes[m_num_dashes++] = gap_len;
    }
}
void vcgen_dash::dash_start(float ds)
{
  CHECK_GT(m_total_dash_len, 0);
  // According to ISO 32000-2:2020 section 8.4.3.6:
  // If the dash phase is negative, it shall be incremented by twice the sum of
  // all lengths in the dash array until it is positive.
  if (ds < 0.0f) {
    float dash_len_sum = m_total_dash_len * 2;
    ds += ceil(-ds / dash_len_sum) * dash_len_sum;
  }
  CHECK_GE(ds, 0);
  m_dash_start = ds;
  calc_dash_start(ds);
}
void vcgen_dash::calc_dash_start(float ds)
{
    DCHECK_GT(m_total_dash_len, 0);
    ds -= floor(ds / m_total_dash_len) * m_total_dash_len;
    m_curr_dash = 0;
    m_curr_dash_start = 0;
    while(ds > 0) {
        if(ds > m_dashes[m_curr_dash]) {
            ds -= m_dashes[m_curr_dash];
            ++m_curr_dash;
            m_curr_dash_start = 0;
            if(m_curr_dash >= m_num_dashes) {
                m_curr_dash = 0;
            }
        } else {
            m_curr_dash_start = ds;
            ds = 0;
        }
    }
}
void vcgen_dash::remove_all()
{
    m_status = initial;
    m_src_vertices.remove_all();
    m_closed = 0;
}
void vcgen_dash::add_vertex(float x, float y, unsigned cmd)
{
    m_status = initial;
    if(is_move_to(cmd)) {
        m_src_vertices.modify_last(vertex_dist(x, y));
    } else {
        if(is_vertex(cmd)) {
            m_src_vertices.add(vertex_dist(x, y));
        } else {
            m_closed = get_close_flag(cmd);
        }
    }
}
void vcgen_dash::rewind(unsigned)
{
    if(m_status == initial) {
        m_src_vertices.close(m_closed != 0);
        shorten_path(m_src_vertices, m_shorten, m_closed);
    }
    m_status = ready;
    m_src_vertex = 0;
}
unsigned vcgen_dash::vertex(float* x, float* y)
{
    unsigned cmd = path_cmd_move_to;
    while(!is_stop(cmd)) {
        switch(m_status) {
            case initial:
                rewind(0);
            case ready:
                if(m_num_dashes < 2 || m_src_vertices.size() < 2) {
                    cmd = path_cmd_stop;
                    break;
                }
                m_status = polyline;
                m_src_vertex = 1;
                m_v1 = &m_src_vertices[0];
                m_v2 = &m_src_vertices[1];
                m_curr_rest = m_v1->dist;
                *x = m_v1->x;
                *y = m_v1->y;
                if(m_dash_start >= 0) {
                    calc_dash_start(m_dash_start);
                }
                return path_cmd_move_to;
            case polyline: {
                    float dash_rest = m_dashes[m_curr_dash] - m_curr_dash_start;
                    unsigned cmd = (m_curr_dash & 1) ?
                                   path_cmd_move_to :
                                   path_cmd_line_to;
                    if(m_curr_rest > dash_rest) {
                        m_curr_rest -= dash_rest;
                        ++m_curr_dash;
                        if(m_curr_dash >= m_num_dashes) {
                            m_curr_dash = 0;
                        }
                        m_curr_dash_start = 0;
                        *x = m_v2->x - (m_v2->x - m_v1->x) * m_curr_rest / m_v1->dist;
                        *y = m_v2->y - (m_v2->y - m_v1->y) * m_curr_rest / m_v1->dist;
                    } else {
                        m_curr_dash_start += m_curr_rest;
                        *x = m_v2->x;
                        *y = m_v2->y;
                        ++m_src_vertex;
                        m_v1 = m_v2;
                        m_curr_rest = m_v1->dist;
                        if(m_closed) {
                            if(m_src_vertex > m_src_vertices.size()) {
                                m_status = stop;
                            } else {
                                m_v2 = &m_src_vertices
                                       [
                                           (m_src_vertex >= m_src_vertices.size()) ? 0 :
                                           m_src_vertex
                                       ];
                            }
                        } else {
                            if(m_src_vertex >= m_src_vertices.size()) {
                                m_status = stop;
                            } else {
                                m_v2 = &m_src_vertices[m_src_vertex];
                            }
                        }
                    }
                    return cmd;
                }
                break;
            case stop:
                cmd = path_cmd_stop;
                break;
        }
    }
    return path_cmd_stop;
}
}
}  // namespace pdfium
