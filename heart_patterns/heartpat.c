/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions for making GEGL
 * Heart Patterns by Grok - Vibe coded by Beaver
 */
#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start(gegl_heart_pattern2521)
  enum_value(HEART_PATTERN_BASIC_GRID, "basic_grid", N_("Basic Grid"))
  enum_value(HEART_PATTERN_OFFSET_GRID, "offset_grid", N_("Offset Grid"))
  enum_value(HEART_PATTERN_HEXAGONAL_GRID, "hexagonal_grid", N_("Hexagonal Grid"))
  enum_value(HEART_PATTERN_DIAGONAL_GRID, "diagonal_grid", N_("Diagonal Grid"))
  enum_value(HEART_PATTERN_WAVE_GRID, "wave_grid", N_("Wave Grid"))
  enum_value(HEART_PATTERN_STAGGERED_SIZES, "staggered_sizes", N_("Staggered Sizes"))
enum_end(GeglHeartPattern2521)

property_enum(pattern, _("Heart Pattern"),
              GeglHeartPattern2521, gegl_heart_pattern2521,
              HEART_PATTERN_DIAGONAL_GRID)
    description(_("Select the heart pattern to use"))

property_int(outline_style, _("Outline Style"), 1)
    description(_("Outline style of the heart outline: 1 (Filled), 2 (Medium Outline), 3 (Thick Outline)"))
    value_range(1, 3)
    ui_range(1, 3)

property_double(size, _("Heart Size"), 50.0)
    description(_("Size of the hearts in pixels"))
    value_range(10.0, 200.0)
    ui_range(10.0, 150.0)

property_double(rotation, _("Rotation"), 0.0)
    value_range(-180.0, 180.0)
    description(_("Rotation angle of the hearts in degrees"))

property_double(pan_x, _("X Pan"), 0.0)
    value_range(-100.0, 100.0)
    description(_("Horizontal offset of the tiled pattern"))

property_double(pan_y, _("Y Pan"), 0.0)
    value_range(-100.0, 100.0)
    description(_("Vertical offset of the tiled pattern"))

property_double(zoom, _("Zoom"), 1.0)
    value_range(0.1, 5.0)
    description(_("Zoom level of the hearts and tiling grid"))

property_double(pointiness, _("Pointiness"), 0.0)
    description(_("Adjusts the sharpness of the heart's bottom point"))
    value_range(-0.5, 0.5)
    ui_range(-0.5, 0.5)

property_double(lobe_roundness, _("Lobe Roundness"), 0.0)
    description(_("Controls the roundness of the heart's lobes"))
    value_range(-0.5, 0.5)
    ui_range(-0.5, 0.5)

property_color(heart_color, _("Heart Color"), "#ff5555")
    description(_("Color of the hearts"))

property_color(bg_color, _("Background Color"), "#76ebff")
    description(_("Color of the background"))
    ui_meta("sensitive", "! transparent_bg")

property_boolean(transparent_bg, _("Transparent Background"), FALSE)
    description(_("Enable to make the background transparent"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     heartpat
#define GEGL_OP_C_SOURCE heartpat.c

#include "gegl-op.h"

// Function to check if a point (px, py) is inside a heart centered at (cx, cy)
static gboolean
point_in_heart(gdouble px, gdouble py, gdouble cx, gdouble cy, gdouble size,
               gdouble outline_style_ratio, gdouble rotation,
               gdouble pointiness, gdouble lobe_roundness)
{
  // Translate the point relative to the heart center
  px -= cx;
  py -= cy;

  // Apply rotation
  gdouble rot_rad = rotation * G_PI / 180.0;
  gdouble temp_x = px * cos(rot_rad) + py * sin(rot_rad);
  gdouble temp_y = -px * sin(rot_rad) + py * cos(rot_rad);
  px = temp_x;
  py = temp_y;

  // Scale the point coordinates to the heart's unit size
  px /= size;
  py /= size;

  // Normalize coordinates for the implicit equation
  px *= 2.0;  // Adjust scaling to make hearts larger
  py *= 2.0;
  // Flip y to correct orientation (y increases downward in image coordinates)
  py = -py;
  // Center the heart vertically
  py += 0.5;

  // Implicit heart equation: (x^2 + y^2 - 1)^3 - x^2 * y^3 <= 0
  // Modified to account for pointiness and lobe_roundness
  gdouble x2 = px * px;
  gdouble y2 = py * py;
  gdouble term1 = (x2 + y2 - 1.0);
  gdouble term1_cubed = term1 * term1 * term1;
  gdouble y_adjusted = py * (1.0 + lobe_roundness * 0.5);  // Adjust y for lobe roundness
  gdouble term2 = x2 * pow(y_adjusted, 3.0) * (1.0 - pointiness * 0.5);  // Adjust for pointiness
  gdouble value = term1_cubed - term2;

  // Check if the point is inside the heart
  gboolean is_inside = (value <= 0);

  // Apply outline_style for outline styles
  if (outline_style_ratio < 1.0 && is_inside)
  {
    // outline_style ratio 1.0 = filled, 0.0 = thick outline
    gdouble scale = outline_style_ratio;  // Scale down the shape to create an inner boundary
    gdouble px_inner = px / scale;
    gdouble py_inner = py / scale;
    gdouble x2_inner = px_inner * px_inner;
    gdouble y2_inner = py_inner * py_inner;
    gdouble term1_inner = (x2_inner + y2_inner - 1.0);
    gdouble term1_cubed_inner = term1_inner * term1_inner * term1_inner;
    gdouble y_adjusted_inner = py_inner * (1.0 + lobe_roundness * 0.5);
    gdouble term2_inner = x2_inner * pow(y_adjusted_inner, 3.0) * (1.0 - pointiness * 0.5);
    gdouble inner_value = term1_cubed_inner - term2_inner;

    // For outline mode, point is inside the outline if it's inside the outer shape
    // but outside the inner shape
    is_inside = (inner_value > 0);
  }

  return is_inside;
}

static void
prepare(GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space(operation, "input");
  gegl_operation_set_format(operation, "input", babl_format_with_space("RGBA float", space));
  gegl_operation_set_format(operation, "output", babl_format_with_space("RGBA float", space));
}

static gboolean
process(GeglOperation *operation, void *in_buf, void *out_buf, glong n_pixels, const GeglRectangle *roi, gint level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  gfloat heart_rgb[3], bg_rgb[3];

  gegl_color_get_pixel(o->heart_color, babl_format("RGB float"), heart_rgb);
  gegl_color_get_pixel(o->bg_color, babl_format("RGB float"), bg_rgb);

  gfloat base_size = o->size * o->zoom;  // Size of hearts adjusted by zoom
  gfloat visual_spacing = base_size * 1.5;  // Spacing for grid patterns

  // Map outline_style (1 to 3) to a ratio (1.0 = filled, 0.6 = thick outline)
  gdouble outline_style_ratio;
  switch (o->outline_style)
  {
    case 1: outline_style_ratio = 1.0; break;  // Filled
    case 2: outline_style_ratio = 0.8; break;  // Medium outline
    case 3: outline_style_ratio = 0.6; break;  // Thick outline
    default: outline_style_ratio = 0.8; break;
  }

  // Cast out_buf to a float array for direct pixel manipulation
  gfloat *out_data = (gfloat *) out_buf;

  for (gint y = roi->y; y < roi->y + roi->height; y++)
  {
    for (gint x = roi->x; x < roi->x + roi->width; x++)
    {
      // Calculate the index into out_buf
      gint idx = ((y - roi->y) * roi->width + (x - roi->x)) * 4;
      gfloat min_distance = G_MAXFLOAT;
      gboolean is_inside = FALSE;

      // Apply panning
      gfloat px = x - o->pan_x;
      gfloat py = y - o->pan_y;

      switch (o->pattern)
      {
        case HEART_PATTERN_BASIC_GRID:
        {
          gfloat grid_x = floor(px / visual_spacing) * visual_spacing + visual_spacing / 2;
          gfloat grid_y = floor(py / visual_spacing) * visual_spacing + visual_spacing / 2;
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat center_x = grid_x + c * visual_spacing + o->pan_x;
              gfloat center_y = grid_y + r * visual_spacing + o->pan_y;
              gfloat dx = x - center_x;
              gfloat dy = y - center_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_heart(x, y, center_x, center_y, base_size, outline_style_ratio, o->rotation, o->pointiness, o->lobe_roundness))
              {
                min_distance = distance;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case HEART_PATTERN_OFFSET_GRID:
        {
          gfloat row_height = visual_spacing;
          gfloat row = floor(py / row_height);
          gfloat col = floor(px / visual_spacing);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + visual_spacing / 2 + o->pan_y;
              gfloat grid_x = (col + c) * visual_spacing + visual_spacing / 2 + o->pan_x;
              if ((gint)(row + r) % 2 == 1) grid_x += visual_spacing / 2;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_heart(x, y, grid_x, grid_y, base_size, outline_style_ratio, o->rotation, o->pointiness, o->lobe_roundness))
              {
                min_distance = distance;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case HEART_PATTERN_HEXAGONAL_GRID:
        {
          gfloat row_height = visual_spacing * sqrt(3.0f) / 2.0f;
          gfloat row = floor(py / row_height);
          gfloat col = floor(px / visual_spacing);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + visual_spacing / 2 + o->pan_y;
              gfloat grid_x = (col + c) * visual_spacing + visual_spacing / 2 + o->pan_x;
              if ((gint)(row + r) % 2 == 1) grid_x += visual_spacing / 2;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_heart(x, y, grid_x, grid_y, base_size, outline_style_ratio, o->rotation, o->pointiness, o->lobe_roundness))
              {
                min_distance = distance;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case HEART_PATTERN_DIAGONAL_GRID:
        {
          gfloat diag_spacing = visual_spacing * sqrt(2.0f);
          gfloat diag_x = (px + py) / diag_spacing;
          gfloat diag_y = (px - py) / diag_spacing;
          gfloat col = floor(diag_x);
          gfloat row = floor(diag_y);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_diag_x = (col + c) * diag_spacing;
              gfloat grid_diag_y = (row + r) * diag_spacing;
              gfloat grid_x = (grid_diag_x + grid_diag_y) / 2 + o->pan_x;
              gfloat grid_y = (grid_diag_x - grid_diag_y) / 2 + o->pan_y;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_heart(x, y, grid_x, grid_y, base_size, outline_style_ratio, o->rotation, o->pointiness, o->lobe_roundness))
              {
                min_distance = distance;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case HEART_PATTERN_WAVE_GRID:
        {
          gfloat grid_x = floor(px / visual_spacing) * visual_spacing + visual_spacing / 2;
          gfloat grid_y = floor(py / visual_spacing) * visual_spacing + visual_spacing / 2;
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat center_x = grid_x + c * visual_spacing + sin((grid_y + r * visual_spacing) * 0.05f) * visual_spacing / 2 + o->pan_x;
              gfloat center_y = grid_y + r * visual_spacing + sin((grid_x + c * visual_spacing) * 0.05f) * visual_spacing / 2 + o->pan_y;
              gfloat dx = x - center_x;
              gfloat dy = y - center_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_heart(x, y, center_x, center_y, base_size, outline_style_ratio, o->rotation, o->pointiness, o->lobe_roundness))
              {
                min_distance = distance;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case HEART_PATTERN_STAGGERED_SIZES:
        {
          gfloat row_height = visual_spacing;
          gfloat row = floor(py / row_height);
          gfloat col = floor(px / visual_spacing);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + visual_spacing / 2 + o->pan_y;
              gfloat grid_x = (col + c) * visual_spacing + visual_spacing / 2 + o->pan_x;
              gfloat heart_size = (gint)(col + c) % 2 == 0 ? base_size : base_size * 0.5f;
              if ((gint)(row + r) % 2 == 1) heart_size = base_size * 0.75f;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_heart(x, y, grid_x, grid_y, heart_size, outline_style_ratio, o->rotation, o->pointiness, o->lobe_roundness))
              {
                min_distance = distance;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
      }

      if (is_inside)
      {
        out_data[idx + 0] = heart_rgb[0];
        out_data[idx + 1] = heart_rgb[1];
        out_data[idx + 2] = heart_rgb[2];
        out_data[idx + 3] = 1.0f;
      }
      else
      {
        if (o->transparent_bg)
        {
          out_data[idx + 0] = 0.0f;
          out_data[idx + 1] = 0.0f;
          out_data[idx + 2] = 0.0f;
          out_data[idx + 3] = 0.0f;  // Fully transparent background
        }
        else
        {
          out_data[idx + 0] = bg_rgb[0];
          out_data[idx + 1] = bg_rgb[1];
          out_data[idx + 2] = bg_rgb[2];
          out_data[idx + 3] = 1.0f;  // Opaque background color
        }
      }
    }
  }

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
      "name",        "ai/lb:heart-patterns",
      "title",       _("Heart Patterns"),
      "reference-hash", "heartpatterngegl2025",
      "description", _("Generates a tiled background of hearts with customizable patterns, rotation, panning, and zoom"),
      "gimp:menu-path", "<Image>/Filters/AI GEGL",
      "gimp:menu-label", _("Heart Patterns..."),
      NULL);
}

#endif
