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
 * Deep Seek wrote the plugin, Grok made slight modifications and Beaver vibe coded both of them
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_truchet_mode151)
  enum_value (GEGL_TRUCHET_LINES, "lines", N_("Classic Lines"))
  enum_value (GEGL_TRUCHET_ARCS, "arcs", N_("Quarter Arcs"))
  enum_value (GEGL_TRUCHET_LINES_ONLY, "lines_only", N_("Lines"))
  enum_value (GEGL_TRUCHET_WAVES, "waves", N_("Sine Waves"))
  enum_value (GEGL_TRUCHET_CROSSES, "crosses", N_("X Crosses"))
  enum_value (GEGL_TRUCHET_TRIANGLES, "triangles", N_("Triangular"))
  enum_value (GEGL_TRUCHET_DIAMONDS, "diamonds", N_("Diamond Grid"))
  enum_value (GEGL_TRUCHET_KALEIDOSCOPE, "kaleidoscope", N_("Kaleidoscope"))
  enum_value (GEGL_TRUCHET_ZIGZAG, "zigzag", N_("Zigzag"))
  enum_value (GEGL_TRUCHET_SPIRALS, "spirals", N_("Spirals"))
  enum_value (GEGL_TRUCHET_DOTS_AND_LINES, "dots_and_lines", N_("Dots and Lines"))
  enum_value (GEGL_TRUCHET_NESTED_SQUARES, "nested_squares", N_("Nested Squares"))
enum_end (GeglTruchetMode151)

property_enum (mode, _("Pattern Style"), 
              GeglTruchetMode151, gegl_truchet_mode151,
              GEGL_TRUCHET_LINES)
    description (_("Style of Truchet tiles to generate"))

property_seed (seed, _("Random seed"), rand)
    description (_("The random seed for the noise function"))

property_double (tile_size, _("Tile Size"), 60.0)
    value_range (4.0, 300.0)

property_double (thickness, _("Thickness"), 7.0)
    value_range (1.0, 60.0)
    ui_meta ("visible", "!mode {lines}")

property_color (color1, _("Background Color"), "#58b2ff")
    description (_("Background color"))
    ui_meta ("visible", "! transparent_background")

property_color (color2, _("Foreground Color"), "white")
    description (_("Foreground color for the pattern"))

property_boolean (transparent_background, _("Transparent Background"), FALSE)
    description (_("Make the background transparent instead of using the background color"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     truchettiles
#define GEGL_OP_C_SOURCE truchettiles.c

#include "gegl-op.h"

static void prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "input", babl_format ("RGBA float"));
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
}

static void draw_arc(gfloat *out, const gfloat *color,
                    gfloat x, gfloat y, gfloat size,
                    gint quadrant, gfloat thickness)
{
  gfloat center_x = (quadrant % 2) ? size : 0;
  gfloat center_y = (quadrant > 1) ? size : 0;
  gfloat dx = x - center_x;
  gfloat dy = y - center_y;
  gfloat dist = hypotf(dx, dy);
  
  if (fabs(dist - size/2) < thickness/2) {
    memcpy(out, color, 4 * sizeof(gfloat)); // Changed to copy RGBA
  }
}

static void draw_dot(gfloat *out, const gfloat *color,
                     gfloat x, gfloat y, gfloat center_x, gfloat center_y,
                     gfloat radius, gfloat thickness)
{
  gfloat dx = x - center_x;
  gfloat dy = y - center_y;
  gfloat dist = hypotf(dx, dy);
  
  if (dist <= radius + thickness/2 && dist >= radius - thickness/2) {
    memcpy(out, color, 4 * sizeof(gfloat)); // Changed to copy RGBA
  }
}

static void draw_line(gfloat *out, const gfloat *color,
                      gfloat x1, gfloat y1, gfloat x2, gfloat y2,
                      gfloat px, gfloat py, gfloat thickness)
{
  gfloat line_len = hypotf(x2-x1, y2-y1);
  gfloat t = ((px-x1)*(x2-x1) + (py-y1)*(y2-y1)) / (line_len*line_len);
  t = CLAMP(t, 0.0f, 1.0f);
  gfloat projx = x1 + t*(x2-x1);
  gfloat projy = y1 + t*(y2-y1);
  gfloat dist = hypotf(px-projx, py-projy);
  
  if (dist < thickness/2) {
    memcpy(out, color, 4 * sizeof(gfloat)); // Changed to copy RGBA
  }
}

static void draw_zigzag(gfloat *out, const gfloat *color,
                        gfloat x, gfloat y, gfloat size,
                        gboolean vertical, gfloat thickness)
{
  if (vertical) {
    gfloat step = size / 4;
    for (gint i = 0; i < 4; i++) {
      gfloat y1 = i * step;
      gfloat y2 = (i + 1) * step;
      gfloat x1 = (i % 2 == 0) ? size / 4 : size * 3 / 4;
      gfloat x2 = (i % 2 == 0) ? size * 3 / 4 : size / 4;
      draw_line(out, color, x1, y1, x2, y2, x, y, thickness);
    }
  } else {
    gfloat step = size / 4;
    for (gint i = 0; i < 4; i++) {
      gfloat x1 = i * step;
      gfloat x2 = (i + 1) * step;
      gfloat y1 = (i % 2 == 0) ? size / 4 : size * 3 / 4;
      gfloat y2 = (i % 2 == 0) ? size * 3 / 4 : size / 4;
      draw_line(out, color, x1, y1, x2, y2, x, y, thickness);
    }
  }
}

static void draw_spiral(gfloat *out, const gfloat *color,
                        gfloat x, gfloat y, gfloat size,
                        gboolean clockwise, gfloat thickness)
{
  gfloat center_x = size / 2;
  gfloat center_y = size / 2;
  gfloat dx = x - center_x;
  gfloat dy = y - center_y;
  gfloat dist = hypotf(dx, dy);
  gfloat angle = atan2(dy, dx);
  if (angle < 0) angle += 2 * M_PI;
  
  gfloat max_radius = size / 2;
  gfloat spiral_factor = max_radius / (2 * M_PI);
  gfloat expected_radius = angle * spiral_factor;
  if (clockwise) {
    angle = 2 * M_PI - angle;
    expected_radius = angle * spiral_factor;
  }
  
  if (fabs(dist - expected_radius) < thickness / 2 && dist <= max_radius) {
    memcpy(out, color, 4 * sizeof(gfloat)); // Changed to copy RGBA
  }
}

static void draw_rotated_square(gfloat *out, const gfloat *color,
                                gfloat x, gfloat y, gfloat size,
                                gfloat center_x, gfloat center_y,
                                gfloat square_size, gfloat thickness,
                                gfloat angle)
{
  gfloat dx = x - center_x;
  gfloat dy = y - center_y;
  gfloat rotated_x = dx * cos(-angle) - dy * sin(-angle);
  gfloat rotated_y = dx * sin(-angle) + dy * cos(-angle);
  rotated_x += center_x;
  rotated_y += center_y;
  
  gfloat half_size = square_size / 2;
  gfloat dist_x = fabs(rotated_x - center_x);
  gfloat dist_y = fabs(rotated_y - center_y);
  
  if ((dist_x <= half_size + thickness/2 && dist_x >= half_size - thickness/2 && dist_y <= half_size) ||
      (dist_y <= half_size + thickness/2 && dist_y >= half_size - thickness/2 && dist_x <= half_size)) {
    memcpy(out, color, 4 * sizeof(gfloat)); // Changed to copy RGBA
  }
}

static gboolean process (GeglOperation *operation,
                        void *in_buf,
                        void *out_buf,
                        glong n_pixels,
                        const GeglRectangle *roi,
                        gint level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  gfloat *out = out_buf;
  gfloat bg_color[4], fg_color[4];
  
  gegl_color_get_pixel (o->color1, babl_format ("RGBA float"), bg_color);
  gegl_color_get_pixel (o->color2, babl_format ("RGBA float"), fg_color);
  
  GRand *rand = g_rand_new_with_seed(o->seed);
  
  for (glong i = 0; i < n_pixels; i++)
  {
    gint x = (i % roi->width) + roi->x;
    gint y = (i / roi->width) + roi->y;
    gint tile_x = x / o->tile_size;
    gint tile_y = y / o->tile_size;
    gfloat local_x = fmod(x, o->tile_size);
    gfloat local_y = fmod(y, o->tile_size);
    gfloat norm_x = local_x / o->tile_size;
    gfloat norm_y = local_y / o->tile_size;
    
    g_rand_set_seed(rand, tile_x * 1000 + tile_y + o->seed);
    gboolean flip = g_rand_int_range(rand, 0, 2);
    
    if (o->transparent_background)
    {
      out[0] = 0.0f;
      out[1] = 0.0f;
      out[2] = 0.0f;
      out[3] = 0.0f;
    }
    else
    {
      memcpy(out, bg_color, 4 * sizeof(gfloat));
    }
    
    switch (o->mode) {
      case GEGL_TRUCHET_LINES:
        if ((flip && (local_x > local_y)) || (!flip && (local_x < local_y))) {
          memcpy(out, fg_color, 4 * sizeof(gfloat)); // Changed to copy RGBA
        }
        break;
        
      case GEGL_TRUCHET_ARCS:
        if (flip) {
          draw_arc(out, fg_color, local_x, local_y, o->tile_size, 0, o->thickness);
          draw_arc(out, fg_color, local_x, local_y, o->tile_size, 3, o->thickness);
        } else {
          draw_arc(out, fg_color, local_x, local_y, o->tile_size, 1, o->thickness);
          draw_arc(out, fg_color, local_x, local_y, o->tile_size, 2, o->thickness);
        }
        break;
        
      case GEGL_TRUCHET_LINES_ONLY:
        if (flip) {
          draw_line(out, fg_color, o->tile_size/2, o->tile_size/2, 0, 0, local_x, local_y, o->thickness);
          draw_line(out, fg_color, o->tile_size/2, o->tile_size/2, o->tile_size, o->tile_size, local_x, local_y, o->thickness);
        } else {
          draw_line(out, fg_color, o->tile_size/2, o->tile_size/2, o->tile_size, 0, local_x, local_y, o->thickness);
          draw_line(out, fg_color, o->tile_size/2, o->tile_size/2, 0, o->tile_size, local_x, local_y, o->thickness);
        }
        break;
        
      case GEGL_TRUCHET_WAVES:
        if (flip) {
          draw_line(out, fg_color, 
                   0, o->tile_size/2 + sin(norm_x * M_PI * 2) * o->tile_size/4,
                   o->tile_size, o->tile_size/2 + sin(norm_x * M_PI * 2) * o->tile_size/4,
                   local_x, local_y, o->thickness);
        } else {
          draw_line(out, fg_color, 
                   o->tile_size/2 + sin(norm_y * M_PI * 2) * o->tile_size/4, 0,
                   o->tile_size/2 + sin(norm_y * M_PI * 2) * o->tile_size/4, o->tile_size,
                   local_x, local_y, o->thickness);
        }
        break;
        
      case GEGL_TRUCHET_CROSSES:
        if (flip) {
          draw_line(out, fg_color, 
                   o->tile_size/4, o->tile_size/4,
                   o->tile_size*3/4, o->tile_size*3/4,
                   local_x, local_y, o->thickness);
          draw_line(out, fg_color, 
                   o->tile_size*3/4, o->tile_size/4,
                   o->tile_size/4, o->tile_size*3/4,
                   local_x, local_y, o->thickness);
        } else {
          draw_line(out, fg_color, 
                   o->tile_size/2, o->tile_size/4,
                   o->tile_size/2, o->tile_size*3/4,
                   local_x, local_y, o->thickness);
          draw_line(out, fg_color, 
                   o->tile_size/4, o->tile_size/2,
                   o->tile_size*3/4, o->tile_size/2,
                   local_x, local_y, o->thickness);
        }
        break;
        
      case GEGL_TRUCHET_TRIANGLES:
        if (flip) {
          draw_line(out, fg_color, 0, 0, o->tile_size, o->tile_size, local_x, local_y, o->thickness);
          draw_line(out, fg_color, 0, o->tile_size, o->tile_size, 0, local_x, local_y, o->thickness);
        } else {
          draw_line(out, fg_color, 0, 0, o->tile_size, 0, local_x, local_y, o->thickness);
          draw_line(out, fg_color, 0, 0, 0, o->tile_size, local_x, local_y, o->thickness);
        }
        break;

      case GEGL_TRUCHET_DIAMONDS:
        if (flip) {
          draw_line(out, fg_color, o->tile_size/2, 0, o->tile_size, o->tile_size/2, local_x, local_y, o->thickness);
          draw_line(out, fg_color, o->tile_size, o->tile_size/2, o->tile_size/2, o->tile_size, local_x, local_y, o->thickness);
          draw_line(out, fg_color, o->tile_size/2, o->tile_size, 0, o->tile_size/2, local_x, local_y, o->thickness);
          draw_line(out, fg_color, 0, o->tile_size/2, o->tile_size/2, 0, local_x, local_y, o->thickness);
        } else {
          draw_line(out, fg_color, o->tile_size/4, o->tile_size/4, o->tile_size*3/4, o->tile_size*3/4, local_x, local_y, o->thickness);
          draw_line(out, fg_color, o->tile_size*3/4, o->tile_size/4, o->tile_size/4, o->tile_size*3/4, local_x, local_y, o->thickness);
        }
        break;
        
      case GEGL_TRUCHET_KALEIDOSCOPE:
        for (int i = 0; i < 4; i++) {
          float angle = i * M_PI / 2 + (flip ? M_PI/4 : 0);
          float start_x = o->tile_size/2 + cos(angle) * o->tile_size/4;
          float start_y = o->tile_size/2 + sin(angle) * o->tile_size/4;
          float end_x = o->tile_size/2 + cos(angle + M_PI/2) * o->tile_size/2;
          float end_y = o->tile_size/2 + sin(angle + M_PI/2) * o->tile_size/2;
          draw_line(out, fg_color, start_x, start_y, end_x, end_y, local_x, local_y, o->thickness);
        }
        break;
        
      case GEGL_TRUCHET_ZIGZAG:
        if (flip) {
          draw_zigzag(out, fg_color, local_x, local_y, o->tile_size, TRUE, o->thickness);
        } else {
          draw_zigzag(out, fg_color, local_x, local_y, o->tile_size, FALSE, o->thickness);
        }
        break;
        
      case GEGL_TRUCHET_SPIRALS:
        draw_spiral(out, fg_color, local_x, local_y, o->tile_size, flip, o->thickness);
        break;
        
      case GEGL_TRUCHET_DOTS_AND_LINES:
        draw_dot(out, fg_color, local_x, local_y, o->tile_size/2, o->tile_size/2, o->tile_size/8, o->thickness);
        if (flip) {
          draw_line(out, fg_color, o->tile_size/2, o->tile_size/2, 0, 0, local_x, local_y, o->thickness);
          draw_line(out, fg_color, o->tile_size/2, o->tile_size/2, o->tile_size, o->tile_size, local_x, local_y, o->thickness);
        } else {
          draw_line(out, fg_color, o->tile_size/2, o->tile_size/2, o->tile_size, 0, local_x, local_y, o->thickness);
          draw_line(out, fg_color, o->tile_size/2, o->tile_size/2, 0, o->tile_size, local_x, local_y, o->thickness);
        }
        break;
        
      case GEGL_TRUCHET_NESTED_SQUARES:
        gfloat center = o->tile_size / 2;
        gfloat angle = flip ? M_PI / 4 : -M_PI / 4;
        draw_rotated_square(out, fg_color, local_x, local_y, o->tile_size, center, center, o->tile_size * 0.8, o->thickness, angle);
        draw_rotated_square(out, fg_color, local_x, local_y, o->tile_size, center, center, o->tile_size * 0.6, o->thickness, angle);
        draw_rotated_square(out, fg_color, local_x, local_y, o->tile_size, center, center, o->tile_size * 0.4, o->thickness, angle);
        break;
    }
    
    out += 4;
  }
  
  g_rand_free (rand);
  return TRUE;
}

static void gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationPointFilterClass *point_filter_class = GEGL_OPERATION_POINT_FILTER_CLASS (klass);

  operation_class->prepare = prepare;
  point_filter_class->process = process;

  gegl_operation_class_set_keys (operation_class,
    "name",        "ai/lb:truchet-tiles",
    "title",       _("Truchet Tiles"),
    "description", _("Generates artistic Truchet tile patterns with multiple sophisticated designs"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Truchet Tiles..."),
    NULL);
}

#endif
