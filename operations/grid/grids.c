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
 * Grok, vibe coded with beaver. Fork of GEGL grid from Oyvind Kolas, 
 * Apparently he made both the node chaining area which I always credit him for and the grid
 *
 */
#include "config.h"
#include <glib/gi18n-lib.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start(grok_grid_type)
  enum_value(GROK_GRID_STANDARD, "standard", N_("Standard Grid"))
  enum_value(GROK_GRID_DASHED, "dashed", N_("Dashed Grid"))
  enum_value(GROK_GRID_DIAGONAL, "diagonal", N_("Diagonal Grid"))
  enum_value(GROK_GRID_DOUBLE, "double", N_("Double Grid"))
  enum_value(GROK_GRID_WAVY, "wavy", N_("Wavy Grid"))
  enum_value(GROK_GRID_RANDOM_OFFSET, "random_offset", N_("Random Offset Grid"))
  enum_value(GROK_GRID_GRADIENT, "gradient", N_("Gradient Grid"))
  enum_value(GROK_GRID_NESTED, "nested", N_("Nested Grid"))
enum_end(GrokGridType)

property_enum(grid_type, _("Grid Type"),
              GrokGridType, grok_grid_type,
              GROK_GRID_STANDARD)
    description(_("Select the type of grid to render"))

property_int (x, _("X spacing"), 155)
    description (_("Horizontal spacing between grid lines"))
    value_range (1, 512)
    ui_meta     ("axis", "x")

property_int (y, _("Y spacing"), 155)
    description (_("Vertical spacing between grid lines"))
    value_range (1, 512)
    ui_meta     ("axis", "y")

property_int (width, _("Line width"), 50)
    description (_("Width of the grid lines in pixels"))
    value_range (1, 100)

property_double (rotation, _("Rotation"), 135.0)
    description (_("Rotate the grid (degrees)"))
    value_range (0.0, 360.0)
    ui_range (0.0, 360.0)

property_double (translate_x, _("X Translation"), 0.0)
    description (_("Move the grid along the X-axis (pixels)"))
    value_range (-1000.0, 1000.0)
    ui_range (-500.0, 500.0)

property_double (translate_y, _("Y Translation"), -150.0)
    description (_("Move the grid along the Y-axis (pixels)"))
    value_range (-1000.0, 1000.0)
    ui_range (-500.0, 500.0)

property_color (color, _("Grid Color"), "#ffffff")
    description (_("Color of the grid lines (defaults to white)"))

property_boolean (fill_background, _("Fill Background"), TRUE)
    description (_("Enable to fill the background with a solid color"))

property_color (bg_color, _("Background Color"), "#ff0000")
    description (_("Color of the background when Fill Background is enabled (defaults to red)"))
    ui_meta ("visible", "fill_background")

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     grokgrid
#define GEGL_OP_C_SOURCE grids.c

#include "gegl-op.h"

static void
prepare (GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space (operation, "input");
  gegl_operation_set_format (operation, "input", babl_format_with_space ("RGBA float", space));
  gegl_operation_set_format (operation, "output", babl_format_with_space ("RGBA float", space));
}

static gboolean
process (GeglOperation       *operation,
         void                *in_buf,
         void                *out_buf,
         glong                n_pixels,
         const GeglRectangle *roi,
         gint                 level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  gfloat *out_pixel = out_buf;

  gfloat grid_rgb[3];
  gegl_color_get_pixel (o->color, babl_format ("RGB float"), grid_rgb);

  gfloat bg_rgb[3] = {0.0f, 0.0f, 0.0f}; // Default to black in case bg_color isn't used
  if (o->fill_background)
    {
      gegl_color_get_pixel (o->bg_color, babl_format ("RGB float"), bg_rgb);
    }

  // Get rotation and translation properties
  gfloat angle = o->rotation * G_PI / 180.0f; // Convert degrees to radians
  gfloat cos_a = cos(angle);
  gfloat sin_a = sin(angle);
  gfloat translate_x = o->translate_x;
  gfloat translate_y = o->translate_y;

  // For random offset grid, initialize a simple pseudo-random seed based on position
  // We'll use a basic linear congruential generator for simplicity
  guint seed = 12345; // Fixed seed for consistent results

  gint x = roi->x;
  gint y = roi->y;

  for (y = roi->y; y < roi->y + roi->height; y++)
    {
      for (x = roi->x; x < roi->x + roi->width; x++)
        {
          // Apply translation: Shift the coordinates
          gfloat tx = x - translate_x;
          gfloat ty = y - translate_y;

          // Apply rotation: Rotate the translated coordinates around the origin
          gfloat rx = tx * cos_a + ty * sin_a;
          gfloat ry = -tx * sin_a + ty * cos_a;

          // Compute grid positions using the transformed coordinates
          gfloat fx = fmodf (rx, o->x);
          gfloat fy = fmodf (ry, o->y);

          // Handle negative remainders
          if (fx < 0) fx += o->x;
          if (fy < 0) fy += o->y;

          // Default to background color or transparent
          gfloat r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;
          if (o->fill_background)
            {
              r = bg_rgb[0];
              g = bg_rgb[1];
              b = bg_rgb[2];
              a = 1.0f;
            }

          // Determine if the pixel is part of the grid based on the grid type
          gboolean is_grid_pixel = FALSE;

          switch (o->grid_type)
            {
              case GROK_GRID_STANDARD: // Standard Grid (Default)
                {
                  is_grid_pixel = (fx <= o->width || fy <= o->width);
                  break;
                }
              case GROK_GRID_DASHED: // Dashed Grid
                {
                  // Create a dashed effect by modulating the grid lines
                  gfloat dash_length = o->x * 0.3f; // Dash length is 30% of spacing
                  gfloat dash_fx = fmodf(rx, dash_length);
                  gfloat dash_fy = fmodf(ry, dash_length);
                  if (dash_fx < 0) dash_fx += dash_length;
                  if (dash_fy < 0) dash_fy += dash_length;
                  is_grid_pixel = (fx <= o->width && dash_fx <= dash_length * 0.5f) ||
                                  (fy <= o->width && dash_fy <= dash_length * 0.5f);
                  break;
                }
              case GROK_GRID_DIAGONAL: // Diagonal Grid
                {
                  // Add diagonal lines at 45 degrees
                  gfloat diag1 = fmodf(rx + ry, o->x);
                  gfloat diag2 = fmodf(rx - ry, o->x);
                  if (diag1 < 0) diag1 += o->x;
                  if (diag2 < 0) diag2 += o->x;
                  is_grid_pixel = (fx <= o->width || fy <= o->width ||
                                   diag1 <= o->width || diag2 <= o->width);
                  break;
                }
              case GROK_GRID_DOUBLE: // Double Grid
                {
                  // Overlay a second grid with half the spacing
                  gfloat fx2 = fmodf(rx, o->x / 2.0f);
                  gfloat fy2 = fmodf(ry, o->y / 2.0f);
                  if (fx2 < 0) fx2 += o->x / 2.0f;
                  if (fy2 < 0) fy2 += o->y / 2.0f;
                  is_grid_pixel = (fx <= o->width || fy <= o->width ||
                                   fx2 <= o->width / 2.0f || fy2 <= o->width / 2.0f);
                  break;
                }
              case GROK_GRID_WAVY: // Wavy Grid
                {
                  // Modulate the grid lines with a sine wave
                  gfloat wave_amplitude = o->width * 0.5f;
                  gfloat wave_freq = 0.05f;
                  gfloat offset_x = wave_amplitude * sinf(wave_freq * ry);
                  gfloat offset_y = wave_amplitude * sinf(wave_freq * rx);
                  gfloat fx_wavy = fmodf(rx + offset_x, o->x);
                  gfloat fy_wavy = fmodf(ry + offset_y, o->y);
                  if (fx_wavy < 0) fx_wavy += o->x;
                  if (fy_wavy < 0) fy_wavy += o->y;
                  is_grid_pixel = (fx_wavy <= o->width || fy_wavy <= o->width);
                  break;
                }
              case GROK_GRID_RANDOM_OFFSET: // Random Offset Grid
                {
                  // Use a simple pseudo-random offset for each grid line
                  seed = (seed * 1103515245 + 12345) & 0x7fffffff;
                  gfloat rand_offset = ((gfloat)(seed % 1000) / 1000.0f - 0.5f) * o->width * 0.5f;
                  gfloat fx_offset = fmodf(rx + rand_offset, o->x);
                  gfloat fy_offset = fmodf(ry + rand_offset, o->y);
                  if (fx_offset < 0) fx_offset += o->x;
                  if (fy_offset < 0) fy_offset += o->y;
                  is_grid_pixel = (fx_offset <= o->width || fy_offset <= o->width);
                  break;
                }
              case GROK_GRID_GRADIENT: // Gradient Grid
                {
                  // Fade grid lines from foreground to background color based on distance
                  is_grid_pixel = (fx <= o->width || fy <= o->width);
                  if (is_grid_pixel)
                    {
                      gfloat dist = fminf(fx, fy) / o->width;
                      r = (1.0f - dist) * grid_rgb[0] + dist * bg_rgb[0];
                      g = (1.0f - dist) * grid_rgb[1] + dist * bg_rgb[1];
                      b = (1.0f - dist) * grid_rgb[2] + dist * bg_rgb[2];
                      a = 1.0f;
                    }
                  break;
                }
              case GROK_GRID_NESTED: // Nested Grid
                {
                  // Add smaller grids within each cell
                  gfloat sub_x = o->x / 3.0f;
                  gfloat sub_y = o->y / 3.0f;
                  gfloat fx_sub = fmodf(rx, sub_x);
                  gfloat fy_sub = fmodf(ry, sub_y);
                  if (fx_sub < 0) fx_sub += sub_x;
                  if (fy_sub < 0) fy_sub += sub_y;
                  is_grid_pixel = (fx <= o->width || fy <= o->width ||
                                   fx_sub <= o->width / 3.0f || fy_sub <= o->width / 3.0f);
                  break;
                }
            }

          // Set the pixel color
          if (is_grid_pixel)
            {
              // For Gradient Grid, colors are already computed
              if (o->grid_type != GROK_GRID_GRADIENT)
                {
                  r = grid_rgb[0];
                  g = grid_rgb[1];
                  b = grid_rgb[2];
                  a = 1.0f;
                }
            }

          out_pixel[0] = r;
          out_pixel[1] = g;
          out_pixel[2] = b;
          out_pixel[3] = a;
          out_pixel += 4;
        }
    }
  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass           *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationPointFilterClass *point_filter_class = GEGL_OPERATION_POINT_FILTER_CLASS (klass);

  operation_class->prepare = prepare;
  point_filter_class->process = process;

  gegl_operation_class_set_keys (operation_class,
      "name",        "ai/lb:grid",
      "title",       _("Grid"),
      "reference-hash", "grok2grid2025",
      "description", _("Renders a grid with configurable spacing, line width, color, rotation, translations, optional background fill, and various grid styles"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL/",
    "gimp:menu-label", _("Grid..."),
      NULL);
}

#endif
