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
 * 2025 Grok and DeepSeek with Beaver's help 
* 
* */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_checkerboard_colorsgrokdeepseek)
  enum_value (GEGL_CHECKERBOARD_2,  "two",   N_("2 Colors"))
  enum_value (GEGL_CHECKERBOARD_3,  "three", N_("3 Colors"))
  enum_value (GEGL_CHECKERBOARD_4,  "four",  N_("4 Colors"))
  enum_value (GEGL_CHECKERBOARD_5,  "five",  N_("5 Colors"))
  enum_value (GEGL_CHECKERBOARD_6,  "six",   N_("6 Colors"))
  enum_value (GEGL_CHECKERBOARD_7,  "seven", N_("7 Colors"))
  enum_value (GEGL_CHECKERBOARD_8,  "eight", N_("8 Colors"))
enum_end (GeglCheckerboardColorsgrokdeepseek)

property_enum (num_colors, _("Number of Colors"),
               GeglCheckerboardColorsgrokdeepseek, gegl_checkerboard_colorsgrokdeepseek,
               GEGL_CHECKERBOARD_3)
    description (_("Number of colors in the checkerboard pattern"))

property_color (pattern_color1, _("Pattern Color 1"), "#3a6dff")
    description (_("First color for the pattern"))

property_color (pattern_color2, _("Pattern Color 2"), "#3b4dc2")
    description (_("Second color for the pattern"))
    ui_meta ("visible", "num_colors {two,three,four,five,six,seven,eight}")

property_color (pattern_color3, _("Pattern Color 3"), "#5acbff")
    description (_("Third color for the pattern"))
    ui_meta ("visible", "num_colors {three,four,five,six,seven,eight}")

property_color (pattern_color4, _("Pattern Color 4"), "#8c7dff")
    description (_("Fourth color for the pattern"))
    ui_meta ("visible", "num_colors {four,five,six,seven,eight}")

property_color (pattern_color5, _("Pattern Color 5"), "#f7b5fa")
    description (_("Fifth color for the pattern"))
    ui_meta ("visible", "num_colors {five,six,seven,eight}")

property_color (pattern_color6, _("Pattern Color 6"), "#f443ff")
    description (_("Sixth color for the pattern"))
    ui_meta ("visible", "num_colors {six,seven,eight}")

property_color (pattern_color7, _("Pattern Color 7"), "#ff0000")
    description (_("Seventh color for the pattern"))
    ui_meta ("visible", "num_colors {seven,eight}")

property_color (pattern_color8, _("Pattern Color 8"), "#ffffff")
    description (_("Eighth color for the pattern"))
    ui_meta ("visible", "num_colors {eight}")

property_double (pattern_size, _("Pattern Size"), 50.0)
    description (_("Size of each element in pixels"))
    value_range (10.0, 300.0)

property_double (rotation, _("Pattern Rotation Angle"), 0.0)
    description (_("Rotation angle of the pattern in degrees"))
    value_range (0.0, 360.0)
    ui_meta ("unit", "degree")
    ui_meta ("direction", "ccw")

#else

#define GEGL_OP_FILTER
#define GEGL_OP_NAME     checkers
#define GEGL_OP_C_SOURCE checkers.c

#include "gegl-op.h"

static void prepare (GeglOperation *operation)
{
  const Babl *format = babl_format ("RGBA float");
  gegl_operation_set_format (operation, "output", format);
}

static GeglRectangle get_bounding_box (GeglOperation *operation)
{
  return gegl_rectangle_infinite_plane ();
}

static gboolean
process (GeglOperation       *operation,
         GeglBuffer          *input,
         GeglBuffer          *output,
         const GeglRectangle *result,
         gint                 level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  const GeglRectangle *rect = result;
  gint x, y;
  gfloat output_pixel[4];

  gdouble size = o->pattern_size;
  gdouble angle = o->rotation * G_PI / 180.0;
  gdouble cos_angle = cos(angle);
  gdouble sin_angle = sin(angle);

  gdouble c1_r, c1_g, c1_b, c1_a;
  gdouble c2_r, c2_g, c2_b, c2_a;
  gdouble c3_r, c3_g, c3_b, c3_a;
  gdouble c4_r, c4_g, c4_b, c4_a;
  gdouble c5_r, c5_g, c5_b, c5_a;
  gdouble c6_r, c6_g, c6_b, c6_a;
  gdouble c7_r, c7_g, c7_b, c7_a;
  gdouble c8_r, c8_g, c8_b, c8_a;

  gegl_color_get_rgba (o->pattern_color1, &c1_r, &c1_g, &c1_b, &c1_a);
  gegl_color_get_rgba (o->pattern_color2, &c2_r, &c2_g, &c2_b, &c2_a);
  gegl_color_get_rgba (o->pattern_color3, &c3_r, &c3_g, &c3_b, &c3_a);
  gegl_color_get_rgba (o->pattern_color4, &c4_r, &c4_g, &c4_b, &c4_a);
  gegl_color_get_rgba (o->pattern_color5, &c5_r, &c5_g, &c5_b, &c5_a);
  gegl_color_get_rgba (o->pattern_color6, &c6_r, &c6_g, &c6_b, &c6_a);
  gegl_color_get_rgba (o->pattern_color7, &c7_r, &c7_g, &c7_b, &c7_a);
  gegl_color_get_rgba (o->pattern_color8, &c8_r, &c8_g, &c8_b, &c8_a);

  gint num_colors;
  switch (o->num_colors)
  {
    case GEGL_CHECKERBOARD_2: num_colors = 2; break;
    case GEGL_CHECKERBOARD_3: num_colors = 3; break;
    case GEGL_CHECKERBOARD_4: num_colors = 4; break;
    case GEGL_CHECKERBOARD_5: num_colors = 5; break;
    case GEGL_CHECKERBOARD_6: num_colors = 6; break;
    case GEGL_CHECKERBOARD_7: num_colors = 7; break;
    case GEGL_CHECKERBOARD_8: num_colors = 8; break;
    default: num_colors = 3; break;
  }

  for (y = rect->y; y < rect->y + rect->height; y++)
  {
    for (x = rect->x; x < rect->x + rect->width; x++)
    {
      gdouble rx = x * cos_angle - y * sin_angle;
      gdouble ry = x * sin_angle + y * cos_angle;
      
      gint u = (gint)floor(rx / size);
      gint v = (gint)floor(ry / size);

      gint index = (abs(u) + abs(v)) % num_colors;

      switch (index)
      {
        case 0:
          output_pixel[0] = (gfloat)c1_r;
          output_pixel[1] = (gfloat)c1_g;
          output_pixel[2] = (gfloat)c1_b;
          break;
        case 1:
          output_pixel[0] = (gfloat)c2_r;
          output_pixel[1] = (gfloat)c2_g;
          output_pixel[2] = (gfloat)c2_b;
          break;
        case 2:
          output_pixel[0] = (gfloat)c3_r;
          output_pixel[1] = (gfloat)c3_g;
          output_pixel[2] = (gfloat)c3_b;
          break;
        case 3:
          output_pixel[0] = (gfloat)c4_r;
          output_pixel[1] = (gfloat)c4_g;
          output_pixel[2] = (gfloat)c4_b;
          break;
        case 4:
          output_pixel[0] = (gfloat)c5_r;
          output_pixel[1] = (gfloat)c5_g;
          output_pixel[2] = (gfloat)c5_b;
          break;
        case 5:
          output_pixel[0] = (gfloat)c6_r;
          output_pixel[1] = (gfloat)c6_g;
          output_pixel[2] = (gfloat)c6_b;
          break;
        case 6:
          output_pixel[0] = (gfloat)c7_r;
          output_pixel[1] = (gfloat)c7_g;
          output_pixel[2] = (gfloat)c7_b;
          break;
        case 7:
          output_pixel[0] = (gfloat)c8_r;
          output_pixel[1] = (gfloat)c8_g;
          output_pixel[2] = (gfloat)c8_b;
          break;
      }
      output_pixel[3] = (gfloat)c1_a;

      GeglRectangle pixel_rect = {x, y, 1, 1};
      gegl_buffer_set (output, &pixel_rect, 0, babl_format ("RGBA float"), 
                      output_pixel, GEGL_AUTO_ROWSTRIDE);
    }
  }

  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationFilterClass *filter_class = GEGL_OPERATION_FILTER_CLASS (klass);

  operation_class->prepare = prepare;
  operation_class->get_bounding_box = get_bounding_box;
  filter_class->process = process;

  gegl_operation_class_set_keys (operation_class,
    "name",        "ai/lb:checkerboard",
    "title",       _("Checkerboard Patterns"),
    "reference-hash", "grokanddeepseekcheckerboard",
    "description", _("Generates a checkerboard pattern with 2 to 8 configurable colors, size, and rotation angle"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Checkerboard Patterns..."),
    NULL);
}

#endif
