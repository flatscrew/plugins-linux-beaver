/*
 * Pippin (2006) for making GEGL
 * Beaver for vibe coding
 * Grok 3 for making the GEGL plugin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

/* Enum for LJC types */
enum_start(gegl_ljc_type)
  enum_value(GEGL_LJC_DEFAULT, "default", N_("Default Lissajous Curve"))
  enum_value(GEGL_LJC_DASHED, "dashed", N_("Dashed Lissajous Curve"))
  enum_value(GEGL_LJC_WAVY, "wavy", N_("Wavy Lissajous Curve"))
  enum_value(GEGL_LJC_OFFSET, "offset", N_("Offset Lissajous Curve"))
  enum_value(GEGL_LJC_TWISTED, "twisted", N_("Twisted Lissajous Curve"))
enum_end(GeglLjcType)

property_enum(ljc_type, _("Curve Type"),
              GeglLjcType, gegl_ljc_type,
              GEGL_LJC_DEFAULT)
    description(_("Select the type of Lissajous curve to render"))

property_boolean(transparent_background, _("Transparent Background"), FALSE)
    description(_("If checked, the background will be transparent; if unchecked, the background color will be used"))

property_color(background_color, _("Background Color"), "#70d0ff")
    description(_("Color of the background"))
    ui_meta("sensitive", "!transparent_background")

property_color(line_color, _("Line Color"), "#ffffff")
    description(_("Color of the Lissajous curve lines"))

property_double(line_thickness, _("Line Thickness"), 15.0)
    description(_("Thickness of the Lissajous curve lines"))
    value_range(11.0, 100.0)
    ui_range(11.0, 60.0)

property_int(a, _("A"), 5)
    description(_("Frequency for x-axis (controls horizontal oscillations)"))
    value_range(1, 8)

property_int(b, _("B"), 4)
    description(_("Frequency for y-axis (controls vertical oscillations)"))
    value_range(1, 8)

property_double(delta, _("Delta"), 0.0)
    description(_("Phase shift for x-axis (in radians)"))
    value_range(0.0, 12.28)

property_double(scale_factor, _("Scale"), 1.0)
    description(_("Zoom level for the Lissajous curve (higher values zoom in, lower values zoom out)"))
    value_range(0.01, 2.0)

property_double(rotation, _("Rotation"), 0.0)
    description(_("Rotate the LJC around its center (in degrees)"))
    value_range(0.0, 360.0)
    ui_meta("direction", "ccw")

property_double(offset_x, _("Offset X"), 0.0)
    description(_("Horizontal offset of the Lissajous curve (normalized, -1.0 to 1.0)"))
    value_range(-1.0, 1.0)
    ui_range(-1.0, 1.0)

property_double(offset_y, _("Offset Y"), 0.0)
    description(_("Vertical offset of the Lissajous curve (normalized, -1.0 to 1.0)"))
    value_range(-1.0, 1.0)
    ui_range(-1.0, 1.0)

property_int(quality, _("Quality of Lines"), 1900)
    description(_("Controls the number of steps for rendering the curve. Higher values fix visual bugs in the lines at the cost of slower rendering speed. When the lines are thin and scale is hgh this should be higher to prevent the known visual bug of bead-like lines"))
    value_range(1800, 3500)

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     ljslines
#define GEGL_OP_C_SOURCE ljs.c

#include <gegl-op.h>

static void
prepare(GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space(operation, "input");
  gegl_operation_set_format(operation, "input", babl_format_with_space("RGBA float", space));
  gegl_operation_set_format(operation, "output", babl_format_with_space("RGBA float", space));
}

static void
render_ljc(GeglOperation *operation, gfloat *out, gint width, gint height, const GeglRectangle *roi, gint level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  GeglRectangle *canvas = gegl_operation_source_get_bounding_box(operation, "input");
  gfloat canvas_width = canvas ? canvas->width : roi->width;
  gfloat canvas_height = canvas ? canvas->height : roi->height;

  gfloat bg_rgb[4], line_rgb[4];
  gegl_color_get_pixel(o->background_color, babl_format("RGBA float"), bg_rgb);
  gegl_color_get_pixel(o->line_color, babl_format("RGBA float"), line_rgb);
  bg_rgb[3] = 1.0f;

  for (glong i = 0; i < width * height; i++)
  {
    if (o->transparent_background)
    {
      /* Transparent background: set alpha to 0 */
      out[i * 4 + 0] = 0.0f;
      out[i * 4 + 1] = 0.0f;
      out[i * 4 + 2] = 0.0f;
      out[i * 4 + 3] = 0.0f;
    }
    else
    {
      /* Use background color */
      out[i * 4 + 0] = bg_rgb[0];
      out[i * 4 + 1] = bg_rgb[1];
      out[i * 4 + 2] = bg_rgb[2];
      out[i * 4 + 3] = bg_rgb[3];
    }
  }

  gfloat base_scale = MIN(canvas_width, canvas_height) * 0.48f;
  gfloat scale_x = base_scale * o->scale_factor;
  gfloat scale_y = base_scale * o->scale_factor;

  /* Apply the X and Y offsets to the center of the curve */
  gfloat center_x = (canvas_width / 2.0f) + (o->offset_x * canvas_width / 2.0f);
  gfloat center_y = (canvas_height / 2.0f) + (o->offset_y * canvas_height / 2.0f);

  gfloat effective_half_thickness = MAX(o->line_thickness * 0.5f, 1.5f) * 1.1f;
  gfloat effective_half_thickness_squared = effective_half_thickness * effective_half_thickness;

  /* Adjust parameters based on level (preview mode) */
  const gint steps = (level > 0) ? 300 : o->quality; /* Use quality slider for final renders */
  const gint sub_steps = (level > 0) ? 0 : 2;  /* No sub-steps in preview mode */
  const gint checkerboard_skip = (level > 0) ? 4 : 2; /* Skip more pixels in preview mode */

  gfloat core_half_thickness = effective_half_thickness * 0.8f;
  gfloat fade_range = effective_half_thickness - core_half_thickness;

  /* Convert rotation from degrees to radians */
  gfloat theta = o->rotation * G_PI / 180.0f;
  gfloat cos_theta = cos(theta);
  gfloat sin_theta = sin(theta);

  gfloat *curve_points = g_new(gfloat, steps * 2);

  /* Determine the number of passes (only Offset LJC needs multiple passes) */
  gint num_passes = (o->ljc_type == GEGL_LJC_OFFSET) ? 2 : 1;

  for (gint pass = 0; pass < num_passes; pass++)
  {
    gfloat pass_offset_x = 0.0f;
    gfloat pass_offset_y = 0.0f;
    gfloat pass_alpha = 1.0f;

    if (o->ljc_type == GEGL_LJC_OFFSET)
    {
      if (pass == 0)
      {
        /* First pass: Render the shadow (offset, semi-transparent) */
        pass_offset_x = 0.02f;
        pass_offset_y = 0.02f;
        pass_alpha = 0.5f;
      }
      else
      {
        /* Second pass: Render the main curve */
        pass_offset_x = 0.0f;
        pass_offset_y = 0.0f;
        pass_alpha = 1.0f;
      }
    }

    /* Compute curve points for this pass */
    for (gint i = 0; i < steps; i++)
    {
      gfloat t = (gfloat) i / (steps - 1) * 2.0f * G_PI;
      gfloat nx, ny;

      switch (o->ljc_type)
      {
        case GEGL_LJC_DEFAULT:
        case GEGL_LJC_DASHED: /* Dashed will be handled during rendering */
        case GEGL_LJC_OFFSET: /* Offset uses the same base curve */
          /* Standard Lissajous Curve */
          nx = sin(o->a * t + o->delta);
          ny = sin(o->b * t);
          break;

        case GEGL_LJC_WAVY:
          /* Wavy Lissajous Curve: Add secondary oscillation */
          nx = sin(o->a * t + o->delta) + 0.2f * sin(10.0f * t);
          ny = sin(o->b * t) + 0.2f * sin(10.0f * t + G_PI / 2.0f);
          nx = CLAMP(nx, -1.2f, 1.2f) / 1.2f;
          ny = CLAMP(ny, -1.2f, 1.2f) / 1.2f;
          break;

        case GEGL_LJC_TWISTED:
          /* Twisted Lissajous Curve: Add a sinusoidal phase shift */
          {
            gfloat twist = 0.5f * sin(3.0f * t);
            nx = sin(o->a * t + o->delta + twist);
            ny = sin(o->b * t + twist);
          }
          break;

        default:
          nx = sin(o->a * t + o->delta);
          ny = sin(o->b * t);
          break;
      }

      /* Apply offset for this pass (for Offset LJC) */
      nx += pass_offset_x;
      ny += pass_offset_y;

      /* Apply rotation to the curve points */
      gfloat nx_rotated = nx * cos_theta - ny * sin_theta;
      gfloat ny_rotated = nx * sin_theta + ny * cos_theta;

      curve_points[i * 2 + 0] = nx_rotated;
      curve_points[i * 2 + 1] = ny_rotated;
    }

    /* Adjust bounding box to account for rotation, offsets, and user-defined X/Y offsets */
    gfloat extent = sqrt(scale_x * scale_x + scale_y * scale_y) + effective_half_thickness + (num_passes > 1 ? 0.02f * scale_x : 0.0f);
    gfloat min_x = center_x - extent;
    gfloat max_x = center_x + extent;
    gfloat min_y = center_y - extent;
    gfloat max_y = center_y + extent;

    /* First pass: Compute distances for a checkerboard pattern */
    gfloat *distances = g_new(gfloat, width * height);
    for (gint y = 0; y < height; y++)
    {
      for (gint x = 0; x < width; x++)
      {
        if ((x + y) % checkerboard_skip != 0) /* Skip pixels based on checkerboard pattern */
        {
          distances[y * width + x] = G_MAXFLOAT;
          continue;
        }

        gfloat px = x + roi->x;
        gfloat py = y + roi->y;
        if (px < min_x || px > max_x || py < min_y || py > max_y)
        {
          distances[y * width + x] = G_MAXFLOAT;
          continue;
        }

        gfloat nx = (px - center_x) / scale_x;
        gfloat ny = (py - center_y) / scale_y;

        /* Apply rotation to the pixel coordinates (inverse rotation) */
        gfloat nx_rotated = nx * cos_theta + ny * sin_theta;
        gfloat ny_rotated = -nx * sin_theta + ny * cos_theta;

        gfloat min_dist_squared = G_MAXFLOAT;

        for (gint i = 0; i < steps; i++)
        {
          /* For dashed LJC, skip segments to create dashes */
          if (o->ljc_type == GEGL_LJC_DASHED)
          {
            gint segment = (i % 30); /* Dash pattern: 20 steps on, 10 steps off */
            if (segment >= 20) /* Skip rendering this segment */
              continue;
          }

          gfloat cx1 = curve_points[i * 2 + 0];
          gfloat cy1 = curve_points[i * 2 + 1];
          gfloat cx2 = curve_points[(i + 1) % steps * 2 + 0];
          gfloat cy2 = curve_points[(i + 1) % steps * 2 + 1];

          for (gint s = 0; s <= sub_steps; s++)
          {
            gfloat t = (gfloat) s / sub_steps;
            gfloat cx = cx1 + t * (cx2 - cx1);
            gfloat cy = cy1 + t * (cy2 - cy1);
            gfloat dx = nx_rotated - cx;
            gfloat dy = ny_rotated - cy;
            gfloat dist_squared = dx * dx + dy * dy;
            min_dist_squared = MIN(min_dist_squared, dist_squared);
          }
        }

        distances[y * width + x] = min_dist_squared * (scale_x * scale_x);
      }
    }

    /* Second pass: Interpolate missing pixels and render */
    for (gint y = 0; y < height; y++)
    {
      for (gint x = 0; x < width; x++)
      {
        gfloat dist_squared_pixels;
        if ((x + y) % checkerboard_skip == 0)
        {
          dist_squared_pixels = distances[y * width + x];
        }
        else
        {
          /* Interpolate from neighbors */
          gfloat d0 = (x > 0) ? distances[y * width + (x - 1)] : G_MAXFLOAT;
          gfloat d1 = (x < width - 1) ? distances[y * width + (x + 1)] : G_MAXFLOAT;
          gfloat d2 = (y > 0) ? distances[(y - 1) * width + x] : G_MAXFLOAT;
          gfloat d3 = (y < height - 1) ? distances[(y + 1) * width + x] : G_MAXFLOAT;
          gfloat min_dist = G_MAXFLOAT;
          if (d0 != G_MAXFLOAT) min_dist = MIN(min_dist, d0);
          if (d1 != G_MAXFLOAT) min_dist = MIN(min_dist, d1);
          if (d2 != G_MAXFLOAT) min_dist = MIN(min_dist, d2);
          if (d3 != G_MAXFLOAT) min_dist = MIN(min_dist, d3);
          dist_squared_pixels = min_dist;
        }

        if (dist_squared_pixels <= effective_half_thickness_squared)
        {
          glong idx = (y * width + x) * 4;
          gfloat dist_pixels = sqrt(dist_squared_pixels);
          gfloat alpha;

          if (dist_pixels <= core_half_thickness)
          {
            alpha = pass_alpha;
          }
          else
          {
            gfloat fade_dist = (dist_pixels - core_half_thickness) / fade_range;
            alpha = pass_alpha * (1.0f - fade_dist);
            alpha = CLAMP(alpha, 0.0f, 1.0f);
          }

          /* Blend with existing pixel values for multiple passes */
          gfloat current_alpha = out[idx + 3];
          gfloat new_alpha = alpha + current_alpha * (1.0f - alpha);
          if (new_alpha > 0.0f)
          {
            out[idx + 0] = (line_rgb[0] * alpha + out[idx + 0] * current_alpha * (1.0f - alpha)) / new_alpha;
            out[idx + 1] = (line_rgb[1] * alpha + out[idx + 1] * current_alpha * (1.0f - alpha)) / new_alpha;
            out[idx + 2] = (line_rgb[2] * alpha + out[idx + 2] * current_alpha * (1.0f - alpha)) / new_alpha;
            out[idx + 3] = new_alpha;
          }
        }
      }
    }

    g_free(distances);
  }

  g_free(curve_points);
}

static gboolean
process(GeglOperation *operation,
        void          *in_buf,
        void          *out_buf,
        glong          n_pixels,
        const GeglRectangle *roi,
        gint           level)
{
  gfloat *out = (gfloat *) out_buf;
  render_ljc(operation, out, roi->width, roi->height, roi, level);
  return TRUE;
}

static void
gegl_op_class_init(GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS(klass);
  GeglOperationPointFilterClass *point_filter_class = GEGL_OPERATION_POINT_FILTER_CLASS(klass);

  operation_class->prepare = prepare;
  point_filter_class->process = process;

  gegl_operation_class_set_keys(operation_class,
    "name",        "ai/lb:lissajous-curve",
    "title",       _("Lissajous Curve"),
    "reference-hash", "lissajousgeglcurve1",
    "description", _("Renders a Lissajous curve with customizable line and background colors, adjustable line thickness, parameters for frequency, phase, rotation, and position"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL/Geometric Lines",
    "gimp:menu-label", _("Lissajous Curve..."),
    NULL);
}

#endif
