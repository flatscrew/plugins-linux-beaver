/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions
 * 2025 Beaver modifying mostly Grok's work. Deep Seek helped a little too
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <math.h>
#include <gegl.h>
#include <gegl-plugin.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_concentric_num_colors2341)
  enum_value (gegl_concentric_num_colors2341_2, "two",   N_("2 Colors"))
  enum_value (gegl_concentric_num_colors2341_3, "three", N_("3 Colors"))
  enum_value (gegl_concentric_num_colors2341_4, "four",  N_("4 Colors"))
  enum_value (gegl_concentric_num_colors2341_5, "five",  N_("5 Colors"))
  enum_value (gegl_concentric_num_colors2341_6, "six",   N_("6 Colors"))
  enum_value (gegl_concentric_num_colors2341_7, "seven", N_("7 Colors"))
  enum_value (gegl_concentric_num_colors2341_8, "eight", N_("8 Colors"))
enum_end (GeglConcentricNumColors)

enum_start (gegl_concentric_shape_type3501)
  enum_value (GEGL_CONCENTRIC_SHAPE_CIRCLE,  "circle",  N_("Circles"))
  enum_value (GEGL_CONCENTRIC_SHAPE_SQUARE,  "square",  N_("Squares"))
  enum_value (GEGL_CONCENTRIC_SHAPE_DIAMOND, "diamond", N_("Diamonds"))
  enum_value (GEGL_CONCENTRIC_SHAPE_HEXAGON, "hexagon", N_("Hexagons"))
  enum_value (GEGL_CONCENTRIC_SHAPE_OCTAGON, "octagon", N_("Octagons"))
  enum_value (GEGL_CONCENTRIC_SHAPE_DECAGON, "decagon", N_("Decagons"))
enum_end (GeglConcentricShapeType3501)

property_enum (num_colors, _("Number of Colors"),
               GeglConcentricNumColors, gegl_concentric_num_colors2341,
               gegl_concentric_num_colors2341_5)
    description (_("Number of colors to use in the concentric shapes"))

property_enum (shape_type, _("Shape Type"),
               GeglConcentricShapeType3501, gegl_concentric_shape_type3501,
               GEGL_CONCENTRIC_SHAPE_CIRCLE)
    description (_("Type of shape to use for concentric patterns"))

property_color (color1, _("Shape Color 1"), "#ff6179")
    description (_("First color of the concentric shapes"))

property_color (color2, _("Shape Color 2"), "#ffffff")
    description (_("Second color of the concentric shapes"))

property_color (color3, _("Shape Color 3"), "#c22485")
    description (_("Third color of the concentric shapes"))
    ui_meta ("visible", "num_colors {three,four,five,six,seven,eight}")

property_color (color4, _("Shape Color 4"), "#94000b")
    description (_("Fourth color of the concentric shapes"))
    ui_meta ("visible", "num_colors {four,five,six,seven,eight}")

property_color (color5, _("Shape Color 5"), "#f99dff")
    description (_("Fifth color of the concentric shapes"))
    ui_meta ("visible", "num_colors {five,six,seven,eight}")

property_color (color6, _("Shape Color 6"), "#c65fff")
    description (_("Sixth color of the concentric shapes"))
    ui_meta ("visible", "num_colors {six,seven,eight}")

property_color (color7, _("Shape Color 7"), "#ffda9e")
    description (_("Seventh color of the concentric shapes"))
    ui_meta ("visible", "num_colors {seven,eight}")

property_color (color8, _("Shape Color 8"), "#80ffe6")
    description (_("Eighth color of the concentric shapes"))
    ui_meta ("visible", "num_colors {eight}")

property_color (bg_color, _("Background Color"), "black")
    description (_("Color of the background (not visible due to full shape coverage)"))
    ui_meta     ("role", "output-extent")

property_double (shape_spacing, _("Shape Spacing"), 70.0)
    description (_("Spacing between concentric shapes"))
    value_range (10.0, 150.0)
    ui_range (10.0, 130.0)

property_double (shape_thickness, _("Shape Thickness"), 1.0)
    description (_("Thickness of the concentric shapes relative to spacing"))
    value_range (1.0, 2.0)
    ui_range (1.0, 2.0)
    ui_meta     ("role", "output-extent")

property_double (x, _("X"), 0.5)
    description (_("X position of the shape center (relative to image width)"))
    value_range (-10.0, 10.0)
    ui_range (0.0, 1.0)
    ui_meta ("unit", "relative-coordinate")
    ui_meta ("axis", "x")

property_double (y, _("Y"), 0.5)
    description (_("Y position of the shape center (relative to image height)"))
    value_range (-10.0, 10.0)
    ui_range (0.0, 1.0)
    ui_meta ("unit", "relative-coordinate")
    ui_meta ("axis", "y")

property_double (skew, _("Skew"), 1.0)
    description (_("Skews the shape by stretching it along the horizontal axis"))
    value_range (1.0, 2.0)
    ui_range (1.0, 2.0)

property_double (rotate, _("Rotate"), 0.0)
    description (_("Rotation angle of the shapes in degrees"))
    value_range (0.0, 360.0)
    ui_range (0.0, 360.0)
    ui_meta ("visible", "shape_type {square,diamond,hexagon,octagon,decagon}")

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     concentric_shapes
#define GEGL_OP_C_SOURCE concentric-shapes.c

#include "gegl-op.h"

// Function prototypes
static void prepare (GeglOperation *operation);
static gboolean process (GeglOperation *operation, void *in_buf, void *out_buf, glong n_pixels, const GeglRectangle *roi, gint level);
static gboolean is_point_in_shape (gfloat x, gfloat y, gfloat radius, GeglConcentricShapeType3501 shape_type, gfloat rotate_angle, gfloat skew);

static gboolean
is_point_in_shape (gfloat x, gfloat y, gfloat radius, GeglConcentricShapeType3501 shape_type, gfloat rotate_angle, gfloat skew)
{
  // Apply skew by stretching the x-coordinate
  gfloat skewed_x = x / skew;

  // Convert rotation angle from degrees to radians
  gfloat angle_rad = rotate_angle * G_PI / 180.0;

  // Rotate the point (skewed_x, y) by -angle_rad to simulate rotating the shape by angle_rad
  gfloat rotated_x = skewed_x * cos(angle_rad) + y * sin(angle_rad);
  gfloat rotated_y = -skewed_x * sin(angle_rad) + y * cos(angle_rad);

  gfloat angle = atan2(rotated_y, rotated_x);
  if (angle < 0) angle += 2.0 * G_PI;
  gfloat dist = sqrt(rotated_x * rotated_x + rotated_y * rotated_y);

  switch (shape_type)
  {
    case GEGL_CONCENTRIC_SHAPE_CIRCLE:
      return dist <= radius;

    case GEGL_CONCENTRIC_SHAPE_SQUARE:
    {
      gfloat abs_x = fabs(rotated_x);
      gfloat abs_y = fabs(rotated_y);
      return MAX(abs_x, abs_y) <= radius;
    }

    case GEGL_CONCENTRIC_SHAPE_DIAMOND:
    {
      gfloat abs_x = fabs(rotated_x);
      gfloat abs_y = fabs(rotated_y);
      return (abs_x + abs_y) <= radius * sqrt(2.0);
    }

    case GEGL_CONCENTRIC_SHAPE_HEXAGON:
    case GEGL_CONCENTRIC_SHAPE_OCTAGON:
    case GEGL_CONCENTRIC_SHAPE_DECAGON:
    {
      gint sides;
      if (shape_type == GEGL_CONCENTRIC_SHAPE_HEXAGON)
        sides = 6;
      else if (shape_type == GEGL_CONCENTRIC_SHAPE_OCTAGON)
        sides = 8;
      else // GEGL_CONCENTRIC_SHAPE_DECAGON
        sides = 10;

      gfloat angle_increment = 2.0 * G_PI / sides;
      gfloat max_dist = 0.0;
      for (gint i = 0; i < sides; i++)
      {
        gfloat axis_angle = i * angle_increment;
        gfloat projection = fabs(rotated_x * cos(axis_angle) + rotated_y * sin(axis_angle));
        if (projection > max_dist)
          max_dist = projection;
      }
      gfloat side_angle = G_PI / sides;
      gfloat scale_factor = 1.0 / cos(side_angle);
      return max_dist <= radius * scale_factor;
    }

    default:
      return FALSE;
  }
}

static void
prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "input", babl_format ("RGBA float"));
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
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

  gfloat c1[4], c2[4], c3[4], c4[4], c5[4], c6[4], c7[4], c8[4], bg[4];
  gegl_color_get_pixel (o->color1, babl_format ("RGBA float"), c1);
  gegl_color_get_pixel (o->color2, babl_format ("RGBA float"), c2);
  gegl_color_get_pixel (o->color3, babl_format ("RGBA float"), c3);
  gegl_color_get_pixel (o->color4, babl_format ("RGBA float"), c4);
  gegl_color_get_pixel (o->color5, babl_format ("RGBA float"), c5);
  gegl_color_get_pixel (o->color6, babl_format ("RGBA float"), c6);
  gegl_color_get_pixel (o->color7, babl_format ("RGBA float"), c7);
  gegl_color_get_pixel (o->color8, babl_format ("RGBA float"), c8);
  gegl_color_get_pixel (o->bg_color, babl_format ("RGBA float"), bg);

  GeglRectangle *canvas = gegl_operation_source_get_bounding_box (operation, "input");
  gfloat canvas_width = canvas ? canvas->width : roi->width;
  gfloat canvas_height = canvas ? canvas->height : roi->height;

  gfloat cx = o->x * canvas_width;
  gfloat cy = o->y * canvas_height;

  gint num_colors;
  switch (o->num_colors)
  {
    case gegl_concentric_num_colors2341_2: num_colors = 2; break;
    case gegl_concentric_num_colors2341_3: num_colors = 3; break;
    case gegl_concentric_num_colors2341_4: num_colors = 4; break;
    case gegl_concentric_num_colors2341_5: num_colors = 5; break;
    case gegl_concentric_num_colors2341_6: num_colors = 6; break;
    case gegl_concentric_num_colors2341_7: num_colors = 7; break;
    case gegl_concentric_num_colors2341_8: num_colors = 8; break;
    default: num_colors = 5;
  }

  for (glong i = 0; i < n_pixels; i++)
  {
    gint x = (i % roi->width) + roi->x;
    gint y = (i / roi->width) + roi->y;

    gfloat dx = x - cx;
    gfloat dy = y - cy;
    gfloat dist = sqrt(dx * dx + dy * dy);

    gfloat *color = c1; // Default to the first color to ensure no background
    gint color_index = 0;

    // Find the innermost shape that contains the point
    for (gint shape_index = 0; ; shape_index++)
    {
      gfloat radius = (shape_index + 1) * o->shape_spacing * o->shape_thickness;
      if (is_point_in_shape(dx, dy, radius, o->shape_type, o->rotate, o->skew))
      {
        color_index = shape_index % num_colors;
        break;
      }
      if (radius > dist + o->shape_spacing * o->shape_thickness)
        break;
    }

    // Assign the color based on the color_index
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
      default: color = c1;
    }

    out_pixel[0] = color[0];
    out_pixel[1] = color[1];
    out_pixel[2] = color[2];
    out_pixel[3] = 1.0;

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
    "name", "ai/lb:concentric-shapes",
    "title", _("Concentric Shapes"),
    "reference-hash", "concentricshapesgegl",
    "description", _("Generates multicolored concentric shapes (circles, squares, diamonds, hexagons, octagons, or decagons) with up to eight customizable colors"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Concentric Shapes..."),
    NULL);
}

#endif
