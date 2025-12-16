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
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions
 * 2025 Beaver modifying mostly Grok's work. Deep Seek helped a little oto
*/
#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start(gegl_circle_patternx)
  enum_value(GEGL_CIRCLE_PATTERN_BASIC_GRID, "basic_grid", N_("Basic Grid"))
  enum_value(GEGL_CIRCLE_PATTERN_OFFSET_GRID, "offset_grid", N_("Offset Grid"))
  enum_value(GEGL_CIRCLE_PATTERN_HEXAGONAL_GRID, "hexagonal_grid", N_("Hexagonal Grid"))
  enum_value(GEGL_CIRCLE_PATTERN_RANDOM_SCATTER, "random_scatter", N_("Random Scatter"))
  enum_value(GEGL_CIRCLE_PATTERN_RIPPLE, "ripple", N_("Ripple Circles"))
  enum_value(GEGL_CIRCLE_PATTERN_DIAGONAL_GRID, "diagonal_grid", N_("Diagonal Grid"))
  enum_value(GEGL_CIRCLE_PATTERN_STAGGERED_SIZES, "staggered_sizes", N_("Staggered Sizes"))
enum_end(GeglCirclePatternx)

property_enum(pattern, _("Circle Pattern"),
              GeglCirclePatternx, gegl_circle_patternx,
              GEGL_CIRCLE_PATTERN_BASIC_GRID)
    description(_("Select the circle pattern style"))

property_double(radius, _("Circle Radius"), 50.0)
    description(_("Radius of the circle in pixels"))
    value_range(10.0, 200.0)
    ui_range(10.0, 100.0)

property_color(circle_color, _("Circle Color"), "#ffffff")
    description(_("Color of the circle (default is white)"))

property_boolean(enable_background, _("Enable Background"), TRUE)
    description(_("Enable or disable the background color; when disabled, background is transparent"))

property_color(bg_color, _("Background Color"), "#a8d5ff")
    description(_("Color of the background (default is light blue)"))
    ui_meta("visible", "enable_background")

property_boolean(draw_outline, _("Draw Outline"), FALSE)
    description(_("Enable to draw an outline around the circle"))

property_double(outline_thickness, _("Outline Thickness"), 2.0)
    description(_("Thickness of the circle outline in pixels"))
    value_range(1.0, 20.0)
    ui_range(1.0, 10.0)
    ui_meta("visible", "draw_outline")

property_color(outline_color, _("Outline Color"), "#000000")
    description(_("Color of the circle outline (default is black)"))
    ui_meta("visible", "draw_outline")

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     circlepatterns
#define GEGL_OP_C_SOURCE circlepatterns.c

#include "gegl-op.h"

static void
prepare(GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space(operation, "input");
  gegl_operation_set_format(operation, "input", babl_format_with_space("RGBA float", space));
  gegl_operation_set_format(operation, "output", babl_format_with_space("RGBA float", space));
}

static GeglRectangle
get_bounding_box(GeglOperation *operation)
{
  GeglRectangle *in_rect = gegl_operation_source_get_bounding_box(operation, "input");
  if (in_rect)
    return *in_rect;
  else
  {
    GeglRectangle result = {0, 0, 0, 0};
    return result;
  }
}

// Simple hash function for pseudo-random numbers
static guint32
hash_position(gint x, gint y)
{
  return (x * 131 + y * 151) ^ (x + y);
}

// Pseudo-random float between 0 and 1 based on position
static gfloat
random_float(gint x, gint y)
{
  return (hash_position(x, y) % 1000) / 1000.0f;
}

static gboolean
process(GeglOperation *operation, void *in_buf, void *out_buf, glong n_pixels, const GeglRectangle *roi, gint level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  gfloat circle_rgb[3], bg_rgb[3], outline_rgb[3];

  gegl_color_get_pixel(o->circle_color, babl_format("RGB float"), circle_rgb);
  if (o->enable_background)
    gegl_color_get_pixel(o->bg_color, babl_format("RGB float"), bg_rgb);
  gegl_color_get_pixel(o->outline_color, babl_format("RGB float"), outline_rgb);

  gfloat radius = o->radius;
  gfloat diameter = 2.0f * radius;
  gfloat outline_thickness = o->outline_thickness;

  // Process the exact ROI without padding
  const Babl *format = babl_format("RGBA float");
  gfloat *out_data = (gfloat *) out_buf;

  for (gint y = roi->y; y < roi->y + roi->height; y++)
  {
    for (gint x = roi->x; x < roi->x + roi->width; x++)
    {
      gint idx = ((y - roi->y) * roi->width + (x - roi->x)) * 4;
      gfloat min_distance = G_MAXFLOAT;
      gfloat current_radius = radius;
      gboolean is_inside = FALSE;
      gboolean is_outline = FALSE;

      switch (o->pattern)
      {
        case GEGL_CIRCLE_PATTERN_BASIC_GRID:
        {
          gfloat grid_x = floor(x / diameter) * diameter + radius;
          gfloat grid_y = floor(y / diameter) * diameter + radius;
          gfloat dx = x - grid_x;
          gfloat dy = y - grid_y;
          gfloat distance = sqrt(dx * dx + dy * dy);
          min_distance = distance;
          current_radius = radius;
          break;
        }
        case GEGL_CIRCLE_PATTERN_OFFSET_GRID:
        {
          gfloat row_height = diameter;
          gfloat row = floor(y / row_height);
          gfloat col = floor(x / diameter);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + radius;
              gfloat grid_x = (col + c) * diameter + radius;
              if ((gint)(row + r) % 2 == 1) grid_x += radius;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance)
              {
                min_distance = distance;
                current_radius = radius;
              }
            }
          }
          break;
        }
        case GEGL_CIRCLE_PATTERN_HEXAGONAL_GRID:
        {
          gfloat row_height = diameter * sqrt(3.0f) / 2.0f;
          gfloat row = floor(y / row_height);
          gfloat col = floor(x / diameter);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + radius;
              gfloat grid_x = (col + c) * diameter + radius;
              if ((gint)(row + r) % 2 == 1) grid_x += radius;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance)
              {
                min_distance = distance;
                current_radius = radius;
              }
            }
          }
          break;
        }
        case GEGL_CIRCLE_PATTERN_RANDOM_SCATTER:
        {
          gfloat grid_x_base = floor(x / diameter) * diameter;
          gfloat grid_y_base = floor(y / diameter) * diameter;
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat base_x = grid_x_base + c * diameter;
              gfloat base_y = grid_y_base + r * diameter;
              gfloat offset_x = (random_float(base_x, base_y) - 0.5f) * radius;
              gfloat offset_y = (random_float(base_y, base_x) - 0.5f) * radius;
              gfloat grid_x = base_x + radius + offset_x;
              gfloat grid_y = base_y + radius + offset_y;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance)
              {
                min_distance = distance;
                current_radius = radius;
              }
            }
          }
          break;
        }
        case GEGL_CIRCLE_PATTERN_RIPPLE:
        {
          gfloat row_height = diameter;
          gfloat col = floor(x / diameter);
          gfloat row = floor(y / row_height);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_x = (col + c) * diameter + radius;
              gfloat grid_y = (row + r) * row_height + radius;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              gfloat wave = sin((col + c) * 0.1f + (row + r) * 0.1f) * 0.3f + 1.0f;
              gfloat modulated_radius = radius * wave;
              if (distance < min_distance)
              {
                min_distance = distance;
                current_radius = modulated_radius;
              }
            }
          }
          break;
        }
        case GEGL_CIRCLE_PATTERN_DIAGONAL_GRID:
        {
          gfloat diag_spacing = diameter * sqrt(2.0f);
          gfloat diag_x = (x + y) / diag_spacing;
          gfloat diag_y = (x - y) / diag_spacing;
          gfloat col = floor(diag_x);
          gfloat row = floor(diag_y);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_diag_x = (col + c) * diag_spacing;
              gfloat grid_diag_y = (row + r) * diag_spacing;
              gfloat grid_x = (grid_diag_x + grid_diag_y) / 2;
              gfloat grid_y = (grid_diag_x - grid_diag_y) / 2;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance)
              {
                min_distance = distance;
                current_radius = radius;
              }
            }
          }
          break;
        }
        case GEGL_CIRCLE_PATTERN_STAGGERED_SIZES:
        {
          gfloat row_height = diameter;
          gfloat row = floor(y / row_height);
          gfloat col = floor(x / diameter);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + radius;
              gfloat grid_x = (col + c) * diameter + radius;
              gfloat circle_radius = radius;
              if ((gint)(col + c) % 2 == 0) circle_radius = radius;
              else circle_radius = radius * 0.5f;
              if ((gint)(row + r) % 2 == 1) circle_radius = radius * 0.75f;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance)
              {
                min_distance = distance;
                current_radius = circle_radius;
              }
            }
          }
          break;
        }
      }

      if (min_distance <= current_radius)
      {
        is_inside = TRUE;
        if (o->draw_outline && min_distance >= (current_radius - outline_thickness))
        {
          is_outline = TRUE;
        }
      }

      if (is_inside)
      {
        if (is_outline)
        {
          out_data[idx + 0] = outline_rgb[0];
          out_data[idx + 1] = outline_rgb[1];
          out_data[idx + 2] = outline_rgb[2];
          out_data[idx + 3] = 1.0f;
        }
        else
        {
          out_data[idx + 0] = circle_rgb[0];
          out_data[idx + 1] = circle_rgb[1];
          out_data[idx + 2] = circle_rgb[2];
          out_data[idx + 3] = 1.0f;
        }
      }
      else
      {
        if (o->enable_background)
        {
          out_data[idx + 0] = bg_rgb[0];
          out_data[idx + 1] = bg_rgb[1];
          out_data[idx + 2] = bg_rgb[2];
          out_data[idx + 3] = 1.0f;  // Opaque background
        }
        else
        {
          out_data[idx + 0] = 0.0f;
          out_data[idx + 1] = 0.0f;
          out_data[idx + 2] = 0.0f;
          out_data[idx + 3] = 0.0f;  // Transparent background
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
  operation_class->get_bounding_box = get_bounding_box;
  point_filter_class->process = process;

  gegl_operation_class_set_keys(operation_class,
      "name",        "ai/lb:circle-patterns",
      "title",       _("Circle Patterns"),
      "reference-hash", "circlepatterngegl2025",
      "description", _("Renders various tiled circle patterns"),
      "gimp:menu-path", "<Image>/Filters/AI GEGL",
      "gimp:menu-label", _("Circle Patterns..."),
      NULL);
}

#endif
