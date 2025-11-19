/*
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

enum_start(pattern_type5931)
  enum_value(MICRO_GRID, "micro_grid", "Micro Grid")
  enum_value(CURVED_LATTICE, "curved_lattice", "Curved Lattice")
  enum_value(MINI_WAVES, "mini_waves", "Mini Waves")
  enum_value(ARC_WEAVE, "arc_weave", "Arc Weave")
  enum_value(TRIANGLE_TILES, "triangle_tiles", "Triangle Tiles")
  enum_value(WOVEN_STRIPS, "woven_strips", "Woven Strips")
  enum_value(OFFSET_SQUARES, "offset_squares", "Offset Squares")
  enum_value(GRID_DIAMONDS, "grid_diamonds", "Grid Diamonds")
  enum_value(WAVE_TRIANGLES, "wave_triangles", "Wave Triangles")
  enum_value(SHIFTED_GRID, "shifted_grid", "Shifted Grid")
  enum_value(ZIGZAG_TILES, "zigzag_tiles", "Zigzag Tiles")
  enum_value(DOUBLE_WAVE_GRID, "double_wave_grid", "Double Wave Grid")
  enum_value(CIRCLE_ARCS, "circle_arcs", "Circle Arcs")
  enum_value(DOUBLE_TRIANGLES, "double_triangles", "Double Triangles")
  enum_value(DIAMOND_CROSS, "diamond_cross", "Diamond Cross")
  enum_value(NET, "net", "Net")
  enum_value(CROSSED_SQUARES, "crossed_squares", "Crossed Squares")
  enum_value(TILTED_ZIGZAG, "tilted_zigzag", "Tilted Zigzag")
  enum_value(DIAMOND_LAYERS, "diamond_layers", "Diamond Layers")
  enum_value(LAYERED_SQUARES, "layered_squares", "Layered Squares")
  enum_value(TRIANGLE_WEAVE, "triangle_weave", "Triangle Weave")
  enum_value(FLOWING_CURVES, "flowing_curves", "Flowing Curves")
  enum_value(WAVY_GRID, "wavy_grid", "Wavy Grid")
  enum_value(ARC_FANS, "arc_fans", "Arc Fans")
  enum_value(CURVED_CROSS, "curved_cross", "Curved Cross")
  enum_value(CURVED_DIAMONDS, "curved_diamonds", "Curved Diamonds")
  enum_value(ANGLED_BANDS, "angled_bands", "Angled Bands")
  enum_value(FOLDED_GRID, "folded_grid", "Folded Grid")
  enum_value(JAGGED_LINES, "jagged_lines", "Jagged Lines")
  enum_value(TRIANGLE_BANDS, "triangle_bands", "Triangle Bands")
  enum_value(SKEWED_CROSS, "skewed_cross", "Skewed Cross")
  enum_value(DOT_GRID, "dot_grid", "Dot Grid")
  enum_value(OFFSET_DOTS, "offset_dots", "Offset Dots")
  enum_value(DIAMOND_DOTS, "diamond_dots", "Diamond Dots")
  enum_value(CLUSTERED_DOTS, "clustered_dots", "Clustered Dots")
enum_end(pattern_type5931)

property_enum(pattern, _("Pattern"),
              pattern_type5931, pattern_type5931,
              MICRO_GRID)
    description(_("Select the pattern to render"))

property_double(tile_size, _("Tile Size"), 350.0)
    description(_("Size of the repeating tile in pixels"))
    value_range(50.0, 1500.0)
    ui_range(50.0, 1500.0)

property_double(line_width, _("Line Width"), 2.0)
    description(_("Adjust the thickness of the pattern lines or size of dots"))
    value_range(0.00, 6.0)
    ui_range(0.00, 6.0)

property_double(rotation, _("Rotation"), 0.0)
    description(_("Rotate the pattern (degrees)"))
    value_range(0.0, 360.0)
    ui_range(0.0, 360.0)

property_double(skew_x, _("Skew X"), 0.0)
    description(_("Horizontal skew of the pattern (degrees)"))
    value_range(-45.0, 45.0)
    ui_range(-45.0, 45.0)

property_double(skew_y, _("Skew Y"), 0.0)
    description(_("Vertical skew of the pattern (degrees)"))
    value_range(-45.0, 45.0)
    ui_range(-45.0, 45.0)

property_color(foreground_color, _("Foreground Color"), "#4A90D9")
    description(_("Color of the pattern shapes"))

property_color(background_color, _("Background Color"), "#A3CFFF")
    description(_("Color of the background"))
    ui_meta("visible", "! transparent_background")

property_boolean(transparent_background, _("Transparent Background"), FALSE)
    description(_("Make the background transparent instead of using the background color"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     patterncollection2
#define GEGL_OP_C_SOURCE patterncollection2.c

#include "gegl-op.h"

static void prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
}

static GeglRectangle get_bounding_box (GeglOperation *operation)
{
  return gegl_rectangle_infinite_plane ();
}

static gboolean
process (GeglOperation        *operation,
         GeglOperationContext *context,
         const gchar          *output_prop,
         const GeglRectangle  *result,
         gint                 level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  GeglBuffer *output = gegl_operation_context_get_target (context, output_prop);
  gdouble fg_color[4], bg_color[4];
  gdouble tile_size = o->tile_size;
  GeglRectangle rect = *result;
  gint x, y;

  // Get color values
  gegl_color_get_rgba (o->foreground_color, &fg_color[0], &fg_color[1], &fg_color[2], &fg_color[3]);
  gegl_color_get_rgba (o->background_color, &bg_color[0], &bg_color[1], &bg_color[2], &bg_color[3]);

  // Convert rotation and skew to radians
  gdouble rotation_rad = o->rotation * G_PI / 180.0;
  gdouble cos_rot = cos(rotation_rad);
  gdouble sin_rot = sin(rotation_rad);
  gdouble skew_x_rad = o->skew_x * G_PI / 180.0;
  gdouble skew_y_rad = o->skew_y * G_PI / 180.0;
  gdouble tan_skew_x = tan(skew_x_rad);
  gdouble tan_skew_y = tan(skew_y_rad);

  for (y = rect.y; y < rect.y + rect.height; y++)
  {
    for (x = rect.x; x < rect.x + rect.width; x++)
    {
      gdouble px = (gdouble)x;
      gdouble py = (gdouble)y;

      // Apply inverse skew transformation (shear)
      gdouble px_sheared = px - tan_skew_x * py;
      gdouble py_sheared = py - tan_skew_y * px;

      // Apply rotation to the sheared coordinates around (0, 0)
      gdouble px_rot = px_sheared * cos_rot - py_sheared * sin_rot;
      gdouble py_rot = px_sheared * sin_rot + py_sheared * cos_rot;

      // Map rotated and sheared coordinates to tile space
      gdouble tx = fmod(px_rot, tile_size);
      gdouble ty = fmod(py_rot, tile_size);
      if (tx < 0) tx += tile_size;
      if (ty < 0) ty += tile_size;

      gfloat out[4];
      gboolean in_shape = FALSE;

      switch (o->pattern)
      {
        case MICRO_GRID:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          gdouble wave = sin((cx + cy) / cell_size * 2.0 * G_PI) * (cell_size / 10.0);
          if (fmod(cx + wave, cell_size / 3.0) < line_width ||
              fmod(cy + wave, cell_size / 3.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case CURVED_LATTICE:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave_x = sin((cy / cell_size) * 2.0 * G_PI) * (cell_size / 8.0);
          gdouble wave_y = sin((cx / cell_size) * 2.0 * G_PI) * (cell_size / 8.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - (cell_size / 2.0 + wave_x)) < line_width ||
              fabs(cy - (cell_size / 2.0 + wave_y)) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case MINI_WAVES:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave = sin((cx / cell_size) * 3.0 * G_PI) * (cell_size / 6.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cy - (cell_size / 2.0 + wave)) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case ARC_WEAVE:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble center_x = cell_size / 2.0;
          gdouble center_y = cell_size / 2.0;
          gdouble dx = cx - center_x;
          gdouble dy = cy - center_y;
          gdouble dist = sqrt(dx * dx + dy * dy);
          gdouble angle = atan2(dy, dx);
          gdouble arc = sin(angle * 3.0) * (cell_size / 10.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (dist > cell_size / 3.0 - line_width && dist < cell_size / 3.0 + line_width &&
              fabs(arc) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case TRIANGLE_TILES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cy) < line_width || fabs(cx + cy - cell_size) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case WOVEN_STRIPS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave1 = sin((cx / cell_size) * 2.0 * G_PI) * (cell_size / 8.0);
          gdouble wave2 = sin((cy / cell_size) * 2.0 * G_PI) * (cell_size / 8.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          gdouble over_under = sin((tx / tile_size) * 2.0 * G_PI) * sin((ty / tile_size) * 2.0 * G_PI);
          if (fabs(cx - (cell_size / 2.0 + wave1)) < line_width && over_under > 0)
          {
            in_shape = TRUE;
          }
          if (fabs(cy - (cell_size / 2.0 + wave2)) < line_width && over_under <= 0)
          {
            in_shape = TRUE;
          }
          break;
        }
        case OFFSET_SQUARES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (cx < line_width || cx > cell_size - line_width ||
              cy < line_width || cy > cell_size - line_width)
          {
            in_shape = TRUE;
          }
          else if (cx > cell_size / 4.0 - line_width && cx < cell_size / 4.0 + line_width &&
                   cy > cell_size / 4.0 - line_width && cy < cell_size / 4.0 + line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case GRID_DIAMONDS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) < cell_size / 4.0 + line_width &&
              fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) > cell_size / 4.0 - line_width)
          {
            in_shape = TRUE;
          }
          else if (fabs(cx - cell_size / 2.0) < line_width ||
                   fabs(cy - cell_size / 2.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case WAVE_TRIANGLES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave = sin((cx / cell_size) * 2.0 * G_PI) * (cell_size / 8.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cy + wave) < line_width || fabs(cx + cy - cell_size + wave) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SHIFTED_GRID:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble shift = sin((cy / cell_size) * 2.0 * G_PI) * (cell_size / 8.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fmod(cx + shift, cell_size / 3.0) < line_width ||
              fmod(cy, cell_size / 3.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case ZIGZAG_TILES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble zigzag_x = fabs(fmod(cx, cell_size / 2.0) - cell_size / 4.0);
          gdouble zigzag_y = fabs(fmod(cy, cell_size / 2.0) - cell_size / 4.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - zigzag_y) < line_width || fabs(cy - zigzag_x) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case DOUBLE_WAVE_GRID:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave1 = sin((cx / cell_size) * 2.0 * G_PI) * (cell_size / 10.0);
          gdouble wave2 = sin((cy / cell_size) * 2.0 * G_PI) * (cell_size / 10.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fmod(cx + wave1 + wave2, cell_size / 3.0) < line_width ||
              fmod(cy + wave1 + wave2, cell_size / 3.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case CIRCLE_ARCS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble center_x = cell_size / 2.0;
          gdouble center_y = cell_size / 2.0;
          gdouble dx = cx - center_x;
          gdouble dy = cy - center_y;
          gdouble dist = sqrt(dx * dx + dy * dy);
          gdouble angle = atan2(dy, dx);
          gdouble arc = sin(angle * 4.0) * (cell_size / 12.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (dist > cell_size / 4.0 - line_width && dist < cell_size / 4.0 + line_width &&
              fabs(arc) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case DOUBLE_TRIANGLES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cy) < line_width || fabs(cx + cy - cell_size) < line_width ||
              fabs(cx - cell_size / 2.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case DIAMOND_CROSS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) < cell_size / 4.0 + line_width &&
              fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) > cell_size / 4.0 - line_width ||
              fabs(cx - cell_size / 2.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case NET:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave = sin((cy / cell_size) * 2.0 * G_PI) * (cell_size / 8.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cy + wave) < line_width || fabs(cx + cy - cell_size + wave) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case CROSSED_SQUARES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (cx < line_width || cx > cell_size - line_width ||
              cy < line_width || cy > cell_size - line_width ||
              fabs(cx - cell_size / 2.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case TILTED_ZIGZAG:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble diag = cx + cy;
          gdouble zigzag = fabs(fmod(diag, cell_size / 2.0) - cell_size / 4.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cy - zigzag) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case DIAMOND_LAYERS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave = sin((cx + cy) / cell_size * G_PI) * (cell_size / 12.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if ((fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) < cell_size / 4.0 + line_width + wave &&
               fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) > cell_size / 4.0 - line_width + wave) ||
              (fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) < cell_size / 3.0 + line_width + wave &&
               fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) > cell_size / 3.0 - line_width + wave) ||
              fabs(cx - cell_size / 2.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case LAYERED_SQUARES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave = sin((cx + cy) / cell_size * 2.0 * G_PI) * (cell_size / 12.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (cx + wave < line_width || cx + wave > cell_size - line_width ||
              cy + wave < line_width || cy + wave > cell_size - line_width ||
              (cx + wave > cell_size / 4.0 - line_width && cx + wave < cell_size / 4.0 + line_width &&
               cy + wave > cell_size / 4.0 - line_width && cy + wave < cell_size / 4.0 + line_width) ||
              (cx + wave > cell_size / 3.0 - line_width && cx + wave < cell_size / 3.0 + line_width &&
               cy + wave > cell_size / 3.0 - line_width && cy + wave < cell_size / 3.0 + line_width))
          {
            in_shape = TRUE;
          }
          break;
        }
        case TRIANGLE_WEAVE:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave = sin((cx / cell_size) * 2.0 * G_PI) * (cell_size / 10.0);
          gdouble diag1 = fabs(cx - cy + wave);
          gdouble diag2 = fabs(cx + cy - cell_size + wave);
          gdouble cross = fabs(cx - cell_size / 2.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (diag1 < line_width || diag2 < line_width || cross < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case FLOWING_CURVES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave = sin((cx / cell_size) * 2.0 * G_PI + (cy / cell_size) * G_PI) * (cell_size / 8.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cy - (cell_size / 2.0 + wave)) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case WAVY_GRID:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave_x = sin((cy / cell_size) * 2.0 * G_PI) * (cell_size / 10.0);
          gdouble wave_y = sin((cx / cell_size) * 2.0 * G_PI) * (cell_size / 10.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fmod(cx + wave_x, cell_size / 3.0) < line_width ||
              fmod(cy + wave_y, cell_size / 3.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case ARC_FANS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble center_x = cell_size / 2.0;
          gdouble center_y = cell_size / 2.0;
          gdouble dx = cx - center_x;
          gdouble dy = cy - center_y;
          gdouble dist = sqrt(dx * dx + dy * dy);
          gdouble angle = atan2(dy, dx);
          gdouble arc = sin(angle * 3.0 + (dist / cell_size) * G_PI) * (cell_size / 12.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (dist > cell_size / 3.0 - line_width && dist < cell_size / 3.0 + line_width &&
              fabs(arc) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case CURVED_CROSS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave_x = sin((cy / cell_size) * 2.0 * G_PI) * (cell_size / 10.0);
          gdouble wave_y = sin((cx / cell_size) * 2.0 * G_PI) * (cell_size / 10.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - (cell_size / 2.0 + wave_x)) < line_width ||
              fabs(cy - (cell_size / 2.0 + wave_y)) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case CURVED_DIAMONDS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble wave = sin((cx + cy) / cell_size * G_PI) * (cell_size / 12.0);
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) < cell_size / 4.0 + line_width + wave &&
              fabs(cx - cell_size / 2.0) + fabs(cy - cell_size / 2.0) > cell_size / 4.0 - line_width + wave)
          {
            in_shape = TRUE;
          }
          break;
        }
        case ANGLED_BANDS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble shift = fmod(cx + cy, cell_size / 2.0) < cell_size / 4.0 ? cell_size / 8.0 : -cell_size / 8.0;
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fmod(cx + cy + shift, cell_size / 3.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case FOLDED_GRID:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble fold_x = fmod(cx, cell_size / 2.0) < cell_size / 4.0 ? cell_size / 8.0 : -cell_size / 8.0;
          gdouble fold_y = fmod(cy, cell_size / 2.0) < cell_size / 4.0 ? cell_size / 8.0 : -cell_size / 8.0;
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fmod(cx + fold_x, cell_size / 3.0) < line_width ||
              fmod(cy + fold_y, cell_size / 3.0) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case JAGGED_LINES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble jag = fmod(cx, cell_size / 2.0) < cell_size / 4.0 ? cell_size / 8.0 : -cell_size / 8.0;
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cy - (cell_size / 2.0 + jag)) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case TRIANGLE_BANDS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble shift = fmod(cx, cell_size / 2.0) < cell_size / 4.0 ? cell_size / 8.0 : -cell_size / 8.0;
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - cy + shift) < line_width || fabs(cx + cy - cell_size + shift) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SKEWED_CROSS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble shift = fmod(cx + cy, cell_size / 2.0) < cell_size / 4.0 ? cell_size / 8.0 : -cell_size / 8.0;
          gdouble base_line_width = cell_size * 0.05;
          gdouble line_width = base_line_width * o->line_width;
          if (fabs(cx - (cell_size / 2.0 + shift)) < line_width ||
              fabs(cy - (cell_size / 2.0 + shift)) < line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case DOT_GRID:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble center_x = cell_size / 2.0;
          gdouble center_y = cell_size / 2.0;
          gdouble dx = cx - center_x;
          gdouble dy = cy - center_y;
          gdouble dist = sqrt(dx * dx + dy * dy);
          gdouble base_dot_size = cell_size * 0.1;
          gdouble dot_size = base_dot_size * o->line_width;
          if (dist < dot_size)
          {
            in_shape = TRUE;
          }
          break;
        }
        case OFFSET_DOTS:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble shift = fmod(cx + cy, cell_size / 2.0) < cell_size / 4.0 ? cell_size / 8.0 : -cell_size / 8.0;
          gdouble center_x = cell_size / 2.0 + shift;
          gdouble center_y = cell_size / 2.0 + shift;
          gdouble dx = cx - center_x;
          gdouble dy = cy - center_y;
          gdouble dist = sqrt(dx * dx + dy * dy);
          gdouble base_dot_size = cell_size * 0.1;
          gdouble dot_size = base_dot_size * o->line_width;
          if (dist < dot_size)
          {
            in_shape = TRUE;
          }
          break;
        }
        case DIAMOND_DOTS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble center_x = cell_size / 2.0;
          gdouble center_y = cell_size / 2.0;
          gdouble dx = fabs(cx - center_x);
          gdouble dy = fabs(cy - center_y);
          gdouble dist = dx + dy; // Manhattan distance for diamond shape
          gdouble base_dot_size = cell_size * 0.1;
          gdouble dot_size = base_dot_size * o->line_width;
          if (dist < dot_size)
          {
            in_shape = TRUE;
          }
          break;
        }
        case CLUSTERED_DOTS:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble cx = fmod(tx, cell_size);
          gdouble cy = fmod(ty, cell_size);
          gdouble base_dot_size = cell_size * 0.08;
          gdouble dot_size = base_dot_size * o->line_width;
          gdouble centers[4][2] = {
            {cell_size / 4.0, cell_size / 4.0},
            {3.0 * cell_size / 4.0, cell_size / 4.0},
            {cell_size / 4.0, 3.0 * cell_size / 4.0},
            {3.0 * cell_size / 4.0, 3.0 * cell_size / 4.0}
          };
          for (int i = 0; i < 4; i++)
          {
            gdouble dx = cx - centers[i][0];
            gdouble dy = cy - centers[i][1];
            gdouble dist = sqrt(dx * dx + dy * dy);
            if (dist < dot_size)
            {
              in_shape = TRUE;
              break;
            }
          }
          break;
        }
      }

      if (in_shape)
      {
        out[0] = fg_color[0];
        out[1] = fg_color[1];
        out[2] = fg_color[2];
        out[3] = 1.0;
      }
      else
      {
        if (o->transparent_background)
        {
          out[0] = 0.0;
          out[1] = 0.0;
          out[2] = 0.0;
          out[3] = 0.0;
        }
        else
        {
          out[0] = bg_color[0];
          out[1] = bg_color[1];
          out[2] = bg_color[2];
          out[3] = 1.0;
        }
      }

      gegl_buffer_set (output, GEGL_RECTANGLE (x, y, 1, 1), 0, babl_format ("RGBA float"), out, GEGL_AUTO_ROWSTRIDE);
    }
  }

  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);
  operation_class->prepare = prepare;
  operation_class->get_bounding_box = get_bounding_box;
  operation_class->process = process;

  gegl_operation_class_set_keys (operation_class,
    "name",        "ai/lb:pattern-collection-2",
    "title",       _("Pattern Collection 2"),
    "reference-hash", "seamlesspatterns20225",
    "description", _("Renders a collection of seamless, small-scale geometric patterns with adjustable size, rotation, skew, and colors"),
    NULL);
}

#endif
