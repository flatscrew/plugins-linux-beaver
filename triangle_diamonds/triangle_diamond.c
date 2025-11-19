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
 * 2025 Beaver modifying mostly Grok's work. Deep Seek helped a little too
 *
 */
#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start(gegl_pattern_seed5283)
  enum_value(GEGL_GROK2_PATTERN_DIAMOND, "diamond", N_("Original Diamond"))
  enum_value(GEGL_GROK2_PATTERN_HALFWAY_TRIANGLES, "halfway_triangles", N_("Halfway Triangles"))
  enum_value(GEGL_GROK2_PATTERN_NESTED, "nested", N_("Nested Triangles"))
  enum_value(GEGL_GROK2_PATTERN_CHECKERBOARD, "checkerboard", N_("Checkerboard Triangles"))
  enum_value(GEGL_GROK2_PATTERN_SCALED_DIAMONDS, "scaled_diamonds", N_("Scaled Diamonds"))
  enum_value(GEGL_GROK2_PATTERN_UPSIDE_DOWN_NESTED, "upside_down_nested", N_("Upside-Down Nested Triangles"))
  enum_value(GEGL_GROK2_PATTERN_OFFSET_DIAGONAL, "offset_diagonal", N_("Offset Diagonal Grid"))
  enum_value(GEGL_GROK2_PATTERN_DOUBLE_DIAGONAL, "double_diagonal", N_("Double Diagonal"))
  enum_value(GEGL_GROK2_PATTERN_ROTATED_DIAMONDS, "rotated_diamonds", N_("Rotated Diamonds"))
  enum_value(GEGL_GROK2_PATTERN_DIAMOND_WAVES, "diamond_waves", N_("Diamond Waves"))
  enum_value(GEGL_GROK2_PATTERN_DIAMOND_CHECKERBOARD, "diamond_checkerboard", N_("Diamond Checkerboard"))
  enum_value(GEGL_GROK2_PATTERN_DIAMOND_GRADIENT, "diamond_gradient", N_("Diamond Gradient"))
  enum_value(GEGL_GROK2_PATTERN_TRIPLE_DIAGONAL, "triple_diagonal", N_("Triple Diagonal"))
  enum_value(GEGL_GROK2_PATTERN_SHIFTED_DOUBLE_DIAGONAL, "shifted_double_diagonal", N_("Shifted Double Diagonal"))
  enum_value(GEGL_GROK2_PATTERN_DENSE_DOUBLE_DIAGONAL, "dense_double_diagonal", N_("Dense Double Diagonal"))
enum_end(GeglPatternSeed5283)

property_enum(pattern, _("Pattern"),
              GeglPatternSeed5283, gegl_pattern_seed5283,
              GEGL_GROK2_PATTERN_DIAMOND)
    description(_("Select the triangle or diamond pattern to render"))

property_double(size, _("Triangle Size"), 50.0)
    description(_("Size of each triangle in pixels"))
    value_range(10.0, 200.0)
    ui_range(10.0, 100.0)

property_double(rotation, _("Rotation"), 0.0)
    description(_("Rotate the pattern (degrees)"))
    value_range(0.0, 360.0)
    ui_range(0.0, 360.0)

property_double(translate_x, _("X Translation"), 0.0)
    description(_("Move the pattern along the X-axis (pixels)"))
    value_range(-1000.0, 1000.0)
    ui_range(-500.0, 500.0)

property_double(translate_y, _("Y Translation"), 0.0)
    description(_("Move the pattern along the Y-axis (pixels)"))
    value_range(-1000.0, 1000.0)
    ui_range(-500.0, 500.0)

property_color(fg_color, _("Triangle Color"), "#ffffff")
    description(_("Color of the triangles (default is white)"))

property_color(bg_color, _("Background Color"), "#a8d5ff")
    description(_("Color of the background (default is light blue)"))
    ui_meta("visible", "! transparent_background")

property_boolean(transparent_background, _("Transparent Background"), FALSE)
    description(_("Make the background transparent instead of using the background color"))

property_boolean(apply_noise_reduction, _("Apply Noise Reduction"), FALSE)
    description(_("Enable noise reduction to smooth triangle edges"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     triangle_diamond
#define GEGL_OP_C_SOURCE triangle_diamond.c

#include "gegl-op.h"

static void
prepare(GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space(operation, "input");
  gegl_operation_set_format(operation, "input", babl_format_with_space("RGBA float", space));
  gegl_operation_set_format(operation, "output", babl_format_with_space("RGBA float", space));
}

static gboolean
point_inside_triangle(gfloat x, gfloat y, gfloat x1, gfloat y1, gfloat x2, gfloat y2, gfloat x3, gfloat y3)
{
  gfloat denom = ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
  if (fabs(denom) < 1e-6) return FALSE;
  gfloat a = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denom;
  gfloat b = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denom;
  gfloat c = 1.0f - a - b;
  return a >= 0 && b >= 0 && c >= 0;
}

static GeglBuffer *
apply_noise_reduction(GeglBuffer *input_buffer, const GeglRectangle *result)
{
  GeglNode *gegl, *source, *noise_reduction, *sink;
  GeglBuffer *output_buffer;

  gegl = gegl_node_new();

  source = gegl_node_new_child(gegl,
                               "operation", "gegl:buffer-source",
                               "buffer", input_buffer,
                               NULL);

  noise_reduction = gegl_node_new_child(gegl,
                                        "operation", "gegl:noise-reduction",
                                        "iterations", 2,  // Reduced from 15 to 2
                                        NULL);

  sink = gegl_node_new_child(gegl,
                             "operation", "gegl:buffer-sink",
                             "buffer", &output_buffer,
                             NULL);

  gegl_node_link_many(source, noise_reduction, sink, NULL);

  gegl_node_process(sink);

  g_object_unref(gegl);

  return output_buffer;
}

static gboolean
process(GeglOperation *operation, void *in_buf, void *out_buf, glong n_pixels, const GeglRectangle *roi, gint level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  gfloat fg_rgb[3], bg_rgb[3];

  gegl_color_get_pixel(o->fg_color, babl_format("RGB float"), fg_rgb);
  gegl_color_get_pixel(o->bg_color, babl_format("RGB float"), bg_rgb);

  gfloat size = o->size;
  gfloat half_size = size / 2.0f;
  gfloat height = size * sin(G_PI / 3.0f);

  // Get rotation and translation properties
  gfloat angle = o->rotation * G_PI / 180.0f; // Convert degrees to radians
  gfloat cos_a = cos(angle);
  gfloat sin_a = sin(angle);
  gfloat translate_x = o->translate_x;
  gfloat translate_y = o->translate_y;

  const Babl *format = babl_format("RGBA float");
  GeglRectangle padded_roi = *roi;
  padded_roi.x -= 16;
  padded_roi.y -= 16;
  padded_roi.width += 32;
  padded_roi.height += 32;
  GeglBuffer *temp_buffer = gegl_buffer_new(&padded_roi, format);
  gfloat *temp_data = (gfloat *) g_malloc((padded_roi.width * padded_roi.height) * 4 * sizeof(gfloat));

  for (gint y = padded_roi.y; y < padded_roi.y + padded_roi.height; y++)
  {
    for (gint x = padded_roi.x; x < padded_roi.x + padded_roi.width; x++)
    {
      gint idx = ((y - padded_roi.y) * padded_roi.width + (x - padded_roi.x)) * 4;
      gboolean inside = FALSE;
      gfloat r = fg_rgb[0], g = fg_rgb[1], b = fg_rgb[2]; // Default to foreground color

      // Apply translation: Shift the coordinates
      gfloat tx = x - translate_x;
      gfloat ty = y - translate_y;

      // Apply rotation: Rotate the translated coordinates around the origin
      gfloat rx = tx * cos_a + ty * sin_a;
      gfloat ry = -tx * sin_a + ty * cos_a;

      // Use the transformed coordinates (rx, ry) in the pattern logic
      switch (o->pattern)
      {
        case GEGL_GROK2_PATTERN_DIAMOND: // Original Diamond
        {
          gfloat grid_x = floor(rx / (size * 1.5f)) * size * 1.5f;
          gfloat grid_y = floor(ry / (height * 2.0f)) * height * 2.0f;
          gfloat x1 = grid_x + half_size;
          gfloat y1 = grid_y;
          gfloat x2 = grid_x;
          gfloat y2 = grid_y + height;
          gfloat x3 = grid_x + size;
          gfloat y3 = grid_y + height;
          gfloat x4 = grid_x + half_size;
          gfloat y4 = grid_y + height * 2.0f;
          gfloat x5 = grid_x;
          gfloat y5 = grid_y + height;
          gfloat x6 = grid_x + size;
          gfloat y6 = grid_y + height;
          inside = point_inside_triangle(rx, ry, x1, y1, x2, y2, x3, y3) ||
                   point_inside_triangle(rx, ry, x4, y4, x5, y5, x6, y6);
          break;
        }
        case GEGL_GROK2_PATTERN_HALFWAY_TRIANGLES: // Halfway Triangles
        {
          gfloat row_height = height * 1.5f;
          gfloat row = ry / row_height;
          gfloat col = rx / size;
          gfloat grid_y = floor(row) * row_height;
          gfloat grid_x = floor(col) * size;
          if ((gint)floor(row) % 2 == 1) grid_x += half_size;
          gboolean upward = ((gint)floor(row) + (gint)floor(col)) % 2 == 0;
          gfloat x1, y1, x2, y2, x3, y3;
          if (upward)
          {
            x1 = grid_x + half_size;
            y1 = grid_y;
            x2 = grid_x;
            y2 = grid_y + height;
            x3 = grid_x + size;
            y3 = grid_y + height;
          }
          else
          {
            x1 = grid_x;
            y1 = grid_y + height;
            x2 = grid_x + size;
            y2 = grid_y + height;
            x3 = grid_x + half_size;
            y3 = grid_y;
          }
          inside = point_inside_triangle(rx, ry, x1, y1, x2, y2, x3, y3);
          break;
        }
        case GEGL_GROK2_PATTERN_NESTED: // Nested Triangles
        {
          gfloat grid_x = floor(rx / size) * size;
          gfloat grid_y = floor(ry / height) * height;
          gfloat x1 = grid_x + half_size;
          gfloat y1 = grid_y;
          gfloat x2 = grid_x;
          gfloat y2 = grid_y + height;
          gfloat x3 = grid_x + size;
          gfloat y3 = grid_y + height;
          inside = point_inside_triangle(rx, ry, x1, y1, x2, y2, x3, y3);
          if (!inside)
          {
            x1 += size / 4.0f;
            y1 += height / 2.0f;
            x2 += size / 4.0f;
            y2 -= height / 2.0f;
            x3 -= size / 4.0f;
            y3 -= height / 2.0f;
            inside = point_inside_triangle(rx, ry, x1, y1, x2, y2, x3, y3);
          }
          break;
        }
        case GEGL_GROK2_PATTERN_CHECKERBOARD: // Checkerboard Triangles
        {
          gfloat grid_x = floor(rx / size) * size;
          gfloat grid_y = floor(ry / height) * height;
          if (((gint)floor(rx / size) + (gint)floor(ry / height)) % 2 == 0)
          {
            gfloat x1 = grid_x + half_size;
            gfloat y1 = grid_y;
            gfloat x2 = grid_x;
            gfloat y2 = grid_y + height;
            gfloat x3 = grid_x + size;
            gfloat y3 = grid_y + height;
            inside = point_inside_triangle(rx, ry, x1, y1, x2, y2, x3, y3);
          }
          break;
        }
        case GEGL_GROK2_PATTERN_SCALED_DIAMONDS: // Scaled Diamonds
        {
          gfloat grid_x = floor(rx / (size * 3.0f)) * size * 3.0f;
          gfloat grid_y = floor(ry / (height * 4.0f)) * height * 4.0f;
          if ((gint)floor(ry / (height * 4.0f)) % 2 == 1)
            grid_x += half_size * 1.5f;
          gfloat x1 = grid_x + size;
          gfloat y1 = grid_y;
          gfloat x2 = grid_x;
          gfloat y2 = grid_y + height * 2.0f;
          gfloat x3 = grid_x + size * 2.0f;
          gfloat y3 = grid_y + height * 2.0f;
          gfloat x4 = grid_x + size;
          gfloat y4 = grid_y + height * 4.0f;
          gfloat x5 = grid_x;
          gfloat y5 = grid_y + height * 2.0f;
          gfloat x6 = grid_x + size * 2.0f;
          gfloat y6 = grid_y + height * 2.0f;
          inside = point_inside_triangle(rx, ry, x1, y1, x2, y2, x3, y3) ||
                   point_inside_triangle(rx, ry, x4, y4, x5, y5, x6, y6);
          break;
        }
        case GEGL_GROK2_PATTERN_UPSIDE_DOWN_NESTED: // Upside-Down Nested Triangles
        {
          gfloat grid_x = floor(rx / size) * size;
          gfloat grid_y = floor(ry / height) * height;
          gfloat x1 = grid_x + half_size;
          gfloat y1 = grid_y + height;
          gfloat x2 = grid_x;
          gfloat y2 = grid_y;
          gfloat x3 = grid_x + size;
          gfloat y3 = grid_y;
          
          inside = point_inside_triangle(rx, ry, x1, y1, x2, y2, x3, y3);
          if (!inside)
          {
            x1 -= size / 4.0f;
            y1 -= height / 2.0f;
            x2 += size / 4.0f;
            y2 += height / 2.0f;
            x3 -= size / 4.0f;
            y3 += height / 2.0f;
            inside = point_inside_triangle(rx, ry, x1, y1, x2, y2, x3, y3);
          }
          break;
        }
        case GEGL_GROK2_PATTERN_OFFSET_DIAGONAL: // Offset Diagonal Grid
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          gfloat grid_row = floor(rotated_y / diagonal_size);
          gfloat offset = (fmod(grid_row, 2.0f) == 0) ? 0 : diagonal_size / 2.0f;
          rotated_x += offset;

          gfloat grid_x = fmod(rotated_x, diagonal_size);
          gfloat grid_y = fmod(rotated_y, diagonal_size);
          if (grid_x < 0) grid_x += diagonal_size;
          if (grid_y < 0) grid_y += diagonal_size;

          gfloat local_x = grid_x - diagonal_size / 2.0f;
          gfloat local_y = grid_y - diagonal_size / 2.0f;

          gfloat manhattan_dist = fabs(local_x) + fabs(local_y);
          inside = manhattan_dist <= diagonal_size / 2.0f;

          break;
        }
        case GEGL_GROK2_PATTERN_DOUBLE_DIAGONAL: // Double Diagonal
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          // First grid
          gfloat grid_x1 = fmod(rotated_x, diagonal_size);
          gfloat grid_y1 = fmod(rotated_y, diagonal_size);
          if (grid_x1 < 0) grid_x1 += diagonal_size;
          if (grid_y1 < 0) grid_y1 += diagonal_size;

          gfloat local_x1 = grid_x1 - diagonal_size / 2.0f;
          gfloat local_y1 = grid_y1 - diagonal_size / 2.0f;
          gfloat manhattan_dist1 = fabs(local_x1) + fabs(local_y1);

          // Second grid, scaled down
          gfloat diagonal_size2 = diagonal_size * 0.5f;
          gfloat grid_x2 = fmod(rotated_x, diagonal_size2);
          gfloat grid_y2 = fmod(rotated_y, diagonal_size2);
          if (grid_x2 < 0) grid_x2 += diagonal_size2;
          if (grid_y2 < 0) grid_y2 += diagonal_size2;

          gfloat local_x2 = grid_x2 - diagonal_size2 / 2.0f;
          gfloat local_y2 = grid_y2 - diagonal_size2 / 2.0f;
          gfloat manhattan_dist2 = fabs(local_x2) + fabs(local_y2);

          inside = (manhattan_dist1 <= diagonal_size / 2.0f) || (manhattan_dist2 <= diagonal_size2 / 2.0f);
          break;
        }
        case GEGL_GROK2_PATTERN_ROTATED_DIAMONDS: // Rotated Diamonds
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          gfloat grid_x = floor(rotated_x / diagonal_size) * diagonal_size;
          gfloat grid_y = floor(rotated_y / diagonal_size) * diagonal_size;

          gfloat center_x = grid_x + diagonal_size / 2.0f;
          gfloat center_y = grid_y + diagonal_size / 2.0f;

          // Rotate each diamond based on its grid position
          gfloat angle = ((gint)(grid_x / diagonal_size) + (gint)(grid_y / diagonal_size)) * 0.3f;
          gfloat cos_a = cos(angle);
          gfloat sin_a = sin(angle);

          gfloat local_x = (rotated_x - center_x) * cos_a + (rotated_y - center_y) * sin_a;
          gfloat local_y = -(rotated_x - center_x) * sin_a + (rotated_y - center_y) * cos_a;

          gfloat manhattan_dist = fabs(local_x) + fabs(local_y);
          inside = manhattan_dist <= diagonal_size / 2.0f;
          break;
        }
        case GEGL_GROK2_PATTERN_DIAMOND_WAVES: // Diamond Waves
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          gfloat grid_x = floor(rotated_x / diagonal_size) * diagonal_size;
          gfloat grid_y = floor(rotated_y / diagonal_size) * diagonal_size;

          // Modulate size with a sine wave
          gfloat wave = 0.7f + 0.3f * sin((grid_x + grid_y) * 0.05f);
          gfloat local_size = diagonal_size * wave;

          gfloat local_x = fmod(rotated_x, diagonal_size) - diagonal_size / 2.0f;
          gfloat local_y = fmod(rotated_y, diagonal_size) - diagonal_size / 2.0f;
          if (local_x < 0) local_x += diagonal_size;
          if (local_y < 0) local_y += diagonal_size;
          local_x -= diagonal_size / 2.0f;
          local_y -= diagonal_size / 2.0f;

          gfloat manhattan_dist = fabs(local_x) + fabs(local_y);
          inside = manhattan_dist <= local_size / 2.0f;
          break;
        }
        case GEGL_GROK2_PATTERN_DIAMOND_CHECKERBOARD: // Diamond Checkerboard
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          gfloat grid_x = floor(rotated_x / diagonal_size);
          gfloat grid_y = floor(rotated_y / diagonal_size);

          if (((gint)grid_x + (gint)grid_y) % 2 == 0)
          {
            gfloat local_x = fmod(rotated_x, diagonal_size) - diagonal_size / 2.0f;
            gfloat local_y = fmod(rotated_y, diagonal_size) - diagonal_size / 2.0f;
            if (local_x < 0) local_x += diagonal_size;
            if (local_y < 0) local_y += diagonal_size;
            local_x -= diagonal_size / 2.0f;
            local_y -= diagonal_size / 2.0f;

            gfloat manhattan_dist = fabs(local_x) + fabs(local_y);
            inside = manhattan_dist <= diagonal_size / 2.0f;
          }
          break;
        }
        case GEGL_GROK2_PATTERN_DIAMOND_GRADIENT: // Diamond Gradient
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          gfloat grid_x = fmod(rotated_x, diagonal_size);
          gfloat grid_y = fmod(rotated_y, diagonal_size);
          if (grid_x < 0) grid_x += diagonal_size;
          if (grid_y < 0) grid_y += diagonal_size;

          gfloat local_x = grid_x - diagonal_size / 2.0f;
          gfloat local_y = grid_y - diagonal_size / 2.0f;

          gfloat manhattan_dist = fabs(local_x) + fabs(local_y);
          inside = manhattan_dist <= diagonal_size / 2.0f;

          if (inside)
          {
            gfloat t = manhattan_dist / (diagonal_size / 2.0f);
            r = (1.0f - t) * fg_rgb[0] + t * bg_rgb[0];
            g = (1.0f - t) * fg_rgb[1] + t * bg_rgb[1];
            b = (1.0f - t) * fg_rgb[2] + t * bg_rgb[2];
          }
          break;
        }
        case GEGL_GROK2_PATTERN_TRIPLE_DIAGONAL: // Triple Diagonal
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          // First grid (1x scale)
          gfloat grid_x1 = fmod(rotated_x, diagonal_size);
          gfloat grid_y1 = fmod(rotated_y, diagonal_size);
          if (grid_x1 < 0) grid_x1 += diagonal_size;
          if (grid_y1 < 0) grid_y1 += diagonal_size;
          gfloat local_x1 = grid_x1 - diagonal_size / 2.0f;
          gfloat local_y1 = grid_y1 - diagonal_size / 2.0f;
          gfloat manhattan_dist1 = fabs(local_x1) + fabs(local_y1);

          // Second grid (0.5x scale)
          gfloat diagonal_size2 = diagonal_size * 0.5f;
          gfloat grid_x2 = fmod(rotated_x, diagonal_size2);
          gfloat grid_y2 = fmod(rotated_y, diagonal_size2);
          if (grid_x2 < 0) grid_x2 += diagonal_size2;
          if (grid_y2 < 0) grid_y2 += diagonal_size2;
          gfloat local_x2 = grid_x2 - diagonal_size2 / 2.0f;
          gfloat local_y2 = grid_y2 - diagonal_size2 / 2.0f;
          gfloat manhattan_dist2 = fabs(local_x2) + fabs(local_y2);

          // Third grid (0.25x scale)
          gfloat diagonal_size3 = diagonal_size * 0.25f;
          gfloat grid_x3 = fmod(rotated_x, diagonal_size3);
          gfloat grid_y3 = fmod(rotated_y, diagonal_size3);
          if (grid_x3 < 0) grid_x3 += diagonal_size3;
          if (grid_y3 < 0) grid_y3 += diagonal_size3;
          gfloat local_x3 = grid_x3 - diagonal_size3 / 2.0f;
          gfloat local_y3 = grid_y3 - diagonal_size3 / 2.0f;
          gfloat manhattan_dist3 = fabs(local_x3) + fabs(local_y3);

          inside = (manhattan_dist1 <= diagonal_size / 2.0f) ||
                   (manhattan_dist2 <= diagonal_size2 / 2.0f) ||
                   (manhattan_dist3 <= diagonal_size3 / 2.0f);
          break;
        }
        case GEGL_GROK2_PATTERN_SHIFTED_DOUBLE_DIAGONAL: // Shifted Double Diagonal
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          // First grid (1x scale)
          gfloat grid_x1 = fmod(rotated_x, diagonal_size);
          gfloat grid_y1 = fmod(rotated_y, diagonal_size);
          if (grid_x1 < 0) grid_x1 += diagonal_size;
          if (grid_y1 < 0) grid_y1 += diagonal_size;
          gfloat local_x1 = grid_x1 - diagonal_size / 2.0f;
          gfloat local_y1 = grid_y1 - diagonal_size / 2.0f;
          gfloat manhattan_dist1 = fabs(local_x1) + fabs(local_y1);

          // Second grid (0.5x scale), shifted diagonally
          gfloat diagonal_size2 = diagonal_size * 0.5f;
          gfloat shifted_x = rotated_x + diagonal_size2 / 4.0f;
          gfloat shifted_y = rotated_y + diagonal_size2 / 4.0f;
          gfloat grid_x2 = fmod(shifted_x, diagonal_size2);
          gfloat grid_y2 = fmod(shifted_y, diagonal_size2);
          if (grid_x2 < 0) grid_x2 += diagonal_size2;
          if (grid_y2 < 0) grid_y2 += diagonal_size2;
          gfloat local_x2 = grid_x2 - diagonal_size2 / 2.0f;
          gfloat local_y2 = grid_y2 - diagonal_size2 / 2.0f;
          gfloat manhattan_dist2 = fabs(local_x2) + fabs(local_y2);

          inside = (manhattan_dist1 <= diagonal_size / 2.0f) || (manhattan_dist2 <= diagonal_size2 / 2.0f);
          break;
        }
        case GEGL_GROK2_PATTERN_DENSE_DOUBLE_DIAGONAL: // Dense Double Diagonal
        {
          gfloat diagonal_size = size * sqrt(2.0f);
          gfloat rotated_x = (rx + ry) / sqrt(2.0f);
          gfloat rotated_y = (ry - rx) / sqrt(2.0f);

          // First grid (1x scale)
          gfloat grid_x1 = fmod(rotated_x, diagonal_size);
          gfloat grid_y1 = fmod(rotated_y, diagonal_size);
          if (grid_x1 < 0) grid_x1 += diagonal_size;
          if (grid_y1 < 0) grid_y1 += diagonal_size;
          gfloat local_x1 = grid_x1 - diagonal_size / 2.0f;
          gfloat local_y1 = grid_y1 - diagonal_size / 2.0f;
          gfloat manhattan_dist1 = fabs(local_x1) + fabs(local_y1);

          // Second grid (0.75x scale for denser overlap)
          gfloat diagonal_size2 = diagonal_size * 0.75f;
          gfloat grid_x2 = fmod(rotated_x, diagonal_size2);
          gfloat grid_y2 = fmod(rotated_y, diagonal_size2);
          if (grid_x2 < 0) grid_x2 += diagonal_size2;
          if (grid_y2 < 0) grid_y2 += diagonal_size2;
          gfloat local_x2 = grid_x2 - diagonal_size2 / 2.0f;
          gfloat local_y2 = grid_y2 - diagonal_size2 / 2.0f;
          gfloat manhattan_dist2 = fabs(local_x2) + fabs(local_y2);

          inside = (manhattan_dist1 <= diagonal_size / 2.0f) || (manhattan_dist2 <= diagonal_size2 / 2.0f);
          break;
        }
      }

      if (inside)
      {
        temp_data[idx + 0] = r;
        temp_data[idx + 1] = g;
        temp_data[idx + 2] = b;
        temp_data[idx + 3] = 1.0f;
      }
      else
      {
        if (o->transparent_background)
        {
          temp_data[idx + 0] = 0.0f;
          temp_data[idx + 1] = 0.0f;
          temp_data[idx + 2] = 0.0f;
          temp_data[idx + 3] = 0.0f;
        }
        else
        {
          temp_data[idx + 0] = bg_rgb[0];
          temp_data[idx + 1] = bg_rgb[1];
          temp_data[idx + 2] = bg_rgb[2];
          temp_data[idx + 3] = 1.0f;
        }
      }
    }
  }

  gegl_buffer_set(temp_buffer, &padded_roi, 0, format, temp_data, GEGL_AUTO_ROWSTRIDE);
  g_free(temp_data);

  if (o->apply_noise_reduction)
  {
    GeglBuffer *smoothed_buffer = apply_noise_reduction(temp_buffer, &padded_roi);
    gegl_buffer_get(smoothed_buffer, roi, 1.0, format, out_buf, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_CLAMP);
    g_object_unref(smoothed_buffer);
  }
  else
  {
    gegl_buffer_get(temp_buffer, roi, 1.0, format, out_buf, GEGL_AUTO_ROWSTRIDE, GEGL_ABYSS_CLAMP);
  }

  g_object_unref(temp_buffer);

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
      "name",        "ai/lb:triangle-diamond",
      "title",       _("Triangle and Diamond Patterns"),
      "reference-hash", "trianglepatterngegl2025",
      "description", _("Renders various triangle and diamond patterns"),
      "gimp:menu-path", "<Image>/Filters/AI GEGL",
      "gimp:menu-label", _("Triangle Diamond Patterns..."),
      NULL);
}

#endif
