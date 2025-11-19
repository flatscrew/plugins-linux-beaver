/* cstripes.c
 *
 * Copyright (C) 2025 LinuxBeaver and contributors
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
 * Pippin for writing GEGL
 * Grok for Writing this plugin
 * Beaver for directing Grok
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_stripes_num_stripes_0381)
  enum_value (GEGL_STRIPES_NUM_STRIPES_2, "two",   N_("2 Stripes"))
  enum_value (GEGL_STRIPES_NUM_STRIPES_3, "three", N_("3 Stripes"))
  enum_value (GEGL_STRIPES_NUM_STRIPES_4, "four",  N_("4 Stripes"))
  enum_value (GEGL_STRIPES_NUM_STRIPES_5, "five",  N_("5 Stripes"))
  enum_value (GEGL_STRIPES_NUM_STRIPES_6, "six",   N_("6 Stripes"))
  enum_value (GEGL_STRIPES_NUM_STRIPES_7, "seven", N_("7 Stripes"))
  enum_value (GEGL_STRIPES_NUM_STRIPES_8, "eight", N_("8 Stripes"))
enum_end (GeglStripesNumStripes0381)

property_enum (num_stripes, _("Number of Stripes"),
               GeglStripesNumStripes0381, gegl_stripes_num_stripes_0381,
               GEGL_STRIPES_NUM_STRIPES_4)
    description (_("Number of stripes to display (also determines number of colors)"))

property_color (color1, _("Color 1"), "#ff6f61") // Coral
    description (_("First color of the stripes"))

property_color (color2, _("Color 2"), "#26a69a") // Teal
    description (_("Second color of the stripes"))
    ui_meta ("visible", "num_stripes {two,three,four,five,six,seven,eight}")

property_color (color3, _("Color 3"), "#fff176") // Lemon Yellow
    description (_("Third color of the stripes"))
    ui_meta ("visible", "num_stripes {three,four,five,six,seven,eight}")

property_color (color4, _("Color 4"), "#ce93d8") // Lavender
    description (_("Fourth color of the stripes"))
    ui_meta ("visible", "num_stripes {four,five,six,seven,eight}")

property_color (color5, _("Color 5"), "#4caf50") // Emerald Green
    description (_("Fifth color of the stripes"))
    ui_meta ("visible", "num_stripes {five,six,seven,eight}")

property_color (color6, _("Color 6"), "#3f51b5") // Sapphire Blue
    description (_("Sixth color of the stripes"))
    ui_meta ("visible", "num_stripes {six,seven,eight}")

property_color (color7, _("Color 7"), "#ec407a") // Magenta
    description (_("Seventh color of the stripes"))
    ui_meta ("visible", "num_stripes {seven,eight}")

property_color (color8, _("Color 8"), "#37474f") // Charcoal
    description (_("Eighth color of the stripes"))
    ui_meta ("visible", "num_stripes {eight}")

property_double (rotation, _("Rotation"), 0.0)
    description (_("Rotation angle of the stripes in degrees (0 to 360)"))
    value_range (0.0, 360.0)
    ui_range (0.0, 360.0)
    ui_meta ("unit", "degree")
    ui_meta ("direction", "ccw")

property_double (stripe_size, _("Stripe Size"), 1.0)
    description (_("Adjusts the width of the stripes (0.1 = thin, 5.0 = wide)"))
    value_range (0.1, 5.0)
    ui_range (0.1, 5.0)

property_double (x_offset, _("X Offset"), 0.0)
    description (_("Horizontal shift of the stripes (pixels)"))
    value_range (-1000.0, 1000.0)
    ui_range (-500.0, 500.0)
    ui_meta ("unit", "pixel-coordinate")

property_double (y_offset, _("Y Offset"), 0.0)
    description (_("Vertical shift of the stripes (pixels)"))
    value_range (-1000.0, 1000.0)
    ui_range (-500.0, 500.0)
    ui_meta ("unit", "pixel-coordinate")

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     cstripes
#define GEGL_OP_C_SOURCE cstripes.c

#include "gegl-op.h"

static void
prepare (GeglOperation *operation)
{
  const Babl *format = babl_format ("RGBA float");
  gegl_operation_set_format (operation, "input", format);
  gegl_operation_set_format (operation, "output", format);
}

static gboolean
process (GeglOperation       *operation,
         void               *in_buf,
         void               *out_buf,
         glong               n_pixels,
         const GeglRectangle *roi,
         gint                level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  gfloat *out_pixel = (gfloat *) out_buf;
  const Babl *format = babl_format ("RGBA float");

  /* Get colors */
  gfloat c1[4], c2[4], c3[4], c4[4], c5[4], c6[4], c7[4], c8[4];
  gegl_color_get_pixel (o->color1, format, c1);
  gegl_color_get_pixel (o->color2, format, c2);
  gegl_color_get_pixel (o->color3, format, c3);
  gegl_color_get_pixel (o->color4, format, c4);
  gegl_color_get_pixel (o->color5, format, c5);
  gegl_color_get_pixel (o->color6, format, c6);
  gegl_color_get_pixel (o->color7, format, c7);
  gegl_color_get_pixel (o->color8, format, c8);

  /* Get canvas dimensions */
  GeglRectangle *canvas = gegl_operation_source_get_bounding_box (operation, "input");
  gfloat canvas_width = canvas ? canvas->width : roi->width;
  gfloat canvas_height = canvas ? canvas->height : roi->height;
  gfloat cx = canvas_width / 2.0;
  gfloat cy = canvas_height / 2.0;

  /* Get rotation */
  gfloat rotation_rad = o->rotation * G_PI / 180.0;
  gfloat cos_r = cosf (rotation_rad);
  gfloat sin_r = sinf (rotation_rad);

  /* Get number of stripes, stripe size, and offsets */
  gint num_stripes = o->num_stripes;
  gfloat stripe_size = o->stripe_size;
  gfloat x_offset = o->x_offset;
  gfloat y_offset = o->y_offset;

  /* Number of colors equals number of stripes */
  gint num_colors = num_stripes;

  /* Map enum values to integers for num_stripes */
  switch (num_stripes)
  {
    case 0: num_stripes = 2; break; // GEGL_STRIPES_NUM_STRIPES_2
    case 1: num_stripes = 3; break; // GEGL_STRIPES_NUM_STRIPES_3
    case 2: num_stripes = 4; break; // GEGL_STRIPES_NUM_STRIPES_4
    case 3: num_stripes = 5; break; // GEGL_STRIPES_NUM_STRIPES_5
    case 4: num_stripes = 6; break; // GEGL_STRIPES_NUM_STRIPES_6
    case 5: num_stripes = 7; break; // GEGL_STRIPES_NUM_STRIPES_7
    case 6: num_stripes = 8; break; // GEGL_STRIPES_NUM_STRIPES_8
    default: num_stripes = 4; break;
  }
  num_colors = num_stripes; /* Ensure num_colors matches num_stripes after mapping */

  /* Stripe width (base width, not scaled directly by stripe_size) */
  gfloat stripe_width = canvas_width / num_stripes;

  for (glong i = 0; i < n_pixels; i++)
  {
    /* Compute global coordinates with offsets */
    gint x = (i % roi->width) + roi->x;
    gint y = (i / roi->width) + roi->y;

    /* Translate to center and apply offsets */
    gfloat dx = x - cx + x_offset;
    gfloat dy = y - cy + y_offset;

    /* Rotate */
    gfloat rx = dx * cos_r + dy * sin_r;

    /* Map to stripe space, scaling by stripe_size */
    rx += canvas_width / 2.0; /* Shift to ensure positive coordinates */
    gfloat scaled_rx = rx / stripe_size; /* Scale coordinate by stripe_size */
    gfloat stripe_pos = fmodf (scaled_rx, canvas_width);
    if (stripe_pos < 0)
      stripe_pos += canvas_width;

    /* Determine stripe index using floorf for consistent boundaries */
    gint stripe_index = (gint) floorf (stripe_pos / stripe_width);
    gint color_index = stripe_index % num_colors;
    if (color_index < 0)
      color_index += num_colors; /* Ensure positive index */

    /* Assign color */
    gfloat *color;
    switch (color_index)
    {
      case 0: color = c1; break;
      case 1: color = c2; break;
      case 2: color = c3; break;
      case 3: color = c4; break;
      case 4: color = c5; break;
      case 5: color = c6; break;
      case 6: color = c7; break;
      case 7: color = c8; break;
      default: color = c1; break;
    }

    out_pixel[0] = color[0];
    out_pixel[1] = color[1];
    out_pixel[2] = color[2];
    out_pixel[3] = 1.0; /* Full opacity */

    out_pixel += 4;
  }

  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationPointFilterClass *point_filter_class = GEGL_OPERATION_POINT_FILTER_CLASS (klass);

  operation_class->prepare = prepare;
  point_filter_class->process = process;

  gegl_operation_class_set_keys (operation_class,
    "name", "ai/lb:stripes",
    "title", _("Color Stripes"),
    "description", _("Generates fullscreen colored stripes with adjustable count, colors, size, rotation, and position, inspired by vibrant patterns"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Color Stripes..."),
    NULL);
}

#endif
