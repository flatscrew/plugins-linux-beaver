/* 
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
#include <gegl-plugin.h>
#include <glib/gi18n-lib.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_grokgradient_gradient_type5823)
  enum_value (GEGL_GROKGRADIENT_GRADIENT_TYPE_LINEAR_5823,            "linear",             N_("Linear"))
  enum_value (GEGL_GROKGRADIENT_GRADIENT_TYPE_BILINEAR_5823,         "bilinear",           N_("Bilinear"))
  enum_value (GEGL_GROKGRADIENT_GRADIENT_TYPE_RADIAL_5823,           "radial",             N_("Radial"))
  enum_value (GEGL_GROKGRADIENT_GRADIENT_TYPE_SPIRAL_5823,           "spiral",             N_("Spiral"))
  enum_value (GEGL_GROKGRADIENT_GRADIENT_TYPE_SPIRAL_CCW_5823,       "spiral_ccw",         N_("Spiral Counter-Clockwise"))
  enum_value (GEGL_GROKGRADIENT_GRADIENT_TYPE_SQUARE_5823,           "square",             N_("Square"))
enum_end (GeglGrokgradientGradientType5823)



property_enum (gradient_shape, _("Gradient Shape"),
               GeglGrokgradientGradientType5823, gegl_grokgradient_gradient_type5823,
               GEGL_GROKGRADIENT_GRADIENT_TYPE_LINEAR_5823)
    description (_("Shape of the gradient pattern"))

enum_start (gegl_grokgradient_num_colors7194)
  enum_value (GEGL_GROKGRADIENT_NUM_COLORS_2_7194, "two",   N_("2 Colors"))
  enum_value (GEGL_GROKGRADIENT_NUM_COLORS_3_7194, "three", N_("3 Colors"))
  enum_value (GEGL_GROKGRADIENT_NUM_COLORS_4_7194, "four",  N_("4 Colors"))
  enum_value (GEGL_GROKGRADIENT_NUM_COLORS_5_7194, "five",  N_("5 Colors"))
  enum_value (GEGL_GROKGRADIENT_NUM_COLORS_6_7194, "six",   N_("6 Colors"))
  enum_value (GEGL_GROKGRADIENT_NUM_COLORS_7_7194, "seven", N_("7 Colors"))
  enum_value (GEGL_GROKGRADIENT_NUM_COLORS_8_7194, "eight", N_("8 Colors"))
enum_end (GeglGrokgradientNumColors7194)

property_enum (num_colors, _("Number of Colors"),
               GeglGrokgradientNumColors7194, gegl_grokgradient_num_colors7194,
               GEGL_GROKGRADIENT_NUM_COLORS_2_7194)
    description (_("Number of colors in the gradient (2 to 8)"))

property_color (color1, _("Color 1"), "#ff6f61") // Coral
    description (_("First color of the gradient"))

property_color (color2, _("Color 2"), "#00ff48") // Green
    description (_("Second color of the gradient"))
    ui_meta ("visible", "num_colors {two,three,four,five,six,seven,eight}")

property_color (color3, _("Color 3"), "#fff176") // Lemon Yellow
    description (_("Third color of the gradient"))
    ui_meta ("visible", "num_colors {three,four,five,six,seven,eight}")

property_color (color4, _("Color 4"), "#ce93d8") // Lavender
    description (_("Fourth color of the gradient"))
    ui_meta ("visible", "num_colors {four,five,six,seven,eight}")

property_color (color5, _("Color 5"), "#4caf50") // Emerald Green
    description (_("Fifth color of the gradient"))
    ui_meta ("visible", "num_colors {five,six,seven,eight}")

property_color (color6, _("Color 6"), "#3f51b5") // Sapphire Blue
    description (_("Sixth color of the gradient"))
    ui_meta ("visible", "num_colors {six,seven,eight}")

property_color (color7, _("Color 7"), "#ec407a") // Magenta
    description (_("Seventh color of the gradient"))
    ui_meta ("visible", "num_colors {seven,eight}")

property_color (color8, _("Color 8"), "#37474f") // Charcoal
    description (_("Eighth color of the gradient"))
    ui_meta ("visible", "num_colors {eight}")

property_double (angle, _("Gradient Angle"), 0.0)
    description (_("Angle of the gradient in degrees (affects Linear and Bilinear shapes; rotates starting point for Spiral shapes)"))
    value_range (0.0, 360.0)
    ui_range (0.0, 360.0)
    ui_meta ("unit", "degree")
    ui_meta ("direction", "ccw")
    ui_meta ("visible", "!gradient_shape{radial,square}")

property_double (frequency, _("Frequency"), 1.0)
    description (_("Number of gradient cycles across the image"))
    value_range (0.1, 10.0)
    ui_range (0.1, 5.0)
    ui_meta ("visible", "!gradient_shape{spiral,spiral_ccw}")

property_int (frequency_2, _("Spiral Frequency"), 1)
    description (_("Number of spiral gradient cycles across the image (whole numbers for seamless spirals)"))
    value_range (1, 10)
    ui_range (1, 10)
    ui_meta ("visible", "gradient_shape{spiral,spiral_ccw}")

property_double (saturation, _("Saturation"), 1.0)
    description (_("Color intensity (0.0 = grayscale, 1.0 = full color)"))
    value_range (0.0, 1.0)
    ui_range (0.5, 1.0)

property_double (brightness, _("Brightness"), 1.0)
    description (_("Color brightness (0.0 = dark, 1.0 = bright)"))
    value_range (0.0, 1.0)
    ui_range (0.5, 1.0)

property_double (offset_x, _("X Offset"), 0.0)
    description (_("Horizontal offset of the gradient center (as a percentage of image width, -100 to 100)"))
    value_range (-100.0, 100.0)
    ui_range (-100.0, 100.0)
    ui_meta ("unit", "percent")

property_double (offset_y, _("Y Offset"), 0.0)
    description (_("Vertical offset of the gradient center (as a percentage of image height, -100 to 100)"))
    value_range (-100.0, 100.0)
    ui_range (-100.0, 100.0)
    ui_meta ("unit", "percent")

property_double (blend, _("Blend"), 0.0)
    description (_("Blending with the input image"))
    value_range (0.0, 0.1)
    ui_range (0.0, 0.1)
    ui_meta ("unit", "percent")
    ui_meta     ("role", "output-extent")

property_boolean (alpha_lock, _("Lock Alpha Channel"), FALSE)
    description (_("Lock the alpha channel to preserve the input image"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     customgradient
#define GEGL_OP_C_SOURCE customgradient.c

#include "gegl-op.h"

// HSV to RGB conversion (from grokgradient.c)
static void
hsv_to_rgb (gfloat h, gfloat s, gfloat v, gfloat *r, gfloat *g, gfloat *b)
{
  h = fmodf(h, 360.0);
  if (h < 0.0)
    h += 360.0;
  s = CLAMP(s, 0.0, 1.0);
  v = CLAMP(v, 0.0, 1.0);

  gfloat c = v * s;
  gfloat x = c * (1.0 - fabsf(fmodf(h / 60.0, 2.0) - 1.0));
  gfloat m = v - c;

  gfloat r1, g1, b1;
  if (h < 60.0)
    { r1 = c; g1 = x; b1 = 0.0; }
  else if (h < 120.0)
    { r1 = x; g1 = c; b1 = 0.0; }
  else if (h < 180.0)
    { r1 = 0.0; g1 = c; b1 = x; }
  else if (h < 240.0)
    { r1 = 0.0; g1 = x; b1 = c; }
  else if (h < 300.0)
    { r1 = x; g1 = 0.0; b1 = c; }
  else
    { r1 = c; g1 = 0.0; b1 = x; }

  *r = r1 + m;
  *g = g1 + m;
  *b = b1 + m;
}

// RGB to HSV conversion (from grokgradient.c)
static void
rgb_to_hsv (gfloat r, gfloat g, gfloat b, gfloat *h, gfloat *s, gfloat *v)
{
  r = CLAMP(r, 0.0, 1.0);
  g = CLAMP(g, 0.0, 1.0);
  b = CLAMP(b, 0.0, 1.0);

  gfloat max = fmaxf(fmaxf(r, g), b);
  gfloat min = fminf(fminf(r, g), b);
  gfloat delta = max - min;

  *v = max;

  if (max == 0.0 || delta == 0.0) {
    *s = 0.0;
    *h = 0.0;
  } else {
    *s = delta / max;
    if (r == max)
      *h = 60.0 * (g - b) / delta;
    else if (g == max)
      *h = 60.0 * (2.0 + (b - r) / delta);
    else
      *h = 60.0 * (4.0 + (r - g) / delta);
    if (*h < 0.0)
      *h += 360.0;
  }
}

// Color interpolation for user-defined colors with cyclic behavior
static void
interpolate_gradient(gfloat t, gfloat *colors[], gint num_colors, gfloat *r, gfloat *g, gfloat *b)
{
  // Ensure t is in [0,1]
  t = fmodf(t, 1.0);
  if (t < 0.0)
    t += 1.0;

  // Compute total gradient length and segment size
  gfloat segment = 1.0f / num_colors;
  gint index = (gint)(t / segment);
  gfloat t_scaled = (t - index * segment) / segment;

  // Handle cyclic interpolation (wrap back to color1 after last color)
  gfloat *color_a = colors[index % num_colors];
  gfloat *color_b = colors[(index + 1) % num_colors];

  *r = color_a[0] + t_scaled * (color_b[0] - color_a[0]);
  *g = color_a[1] + t_scaled * (color_b[1] - color_a[1]);
  *b = color_a[2] + t_scaled * (color_b[2] - color_a[2]);
}

static void
prepare (GeglOperation *operation)
{
  gegl_operation_set_format(operation, "input", babl_format("RGBA float"));
  gegl_operation_set_format(operation, "output", babl_format("RGBA float"));
}

static gboolean
process (GeglOperation *operation, void *in_buf, void *out_buf, glong n_pixels, const GeglRectangle *roi, gint level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  gfloat *in_pixel = (gfloat *)in_buf;
  gfloat *out_pixel = (gfloat *)out_buf;

  // Get canvas dimensions
  GeglRectangle *canvas = gegl_operation_source_get_bounding_box(operation, "input");
  gfloat width = canvas ? canvas->width : roi->width;
  gfloat height = canvas ? canvas->height : roi->height;

  // Precompute angle in radians
  gfloat angle_rad = o->angle * G_PI / 180.0;
  gfloat cos_a = cosf(angle_rad);
  gfloat sin_a = sinf(angle_rad);

  // Select frequency based on shape
  gfloat freq = (o->gradient_shape == GEGL_GROKGRADIENT_GRADIENT_TYPE_SPIRAL_5823 || 
                 o->gradient_shape == GEGL_GROKGRADIENT_GRADIENT_TYPE_SPIRAL_CCW_5823) ? 
                (gfloat)o->frequency_2 : o->frequency;

  // Convert percentage offsets to normalized offsets
  gfloat offset_x = o->offset_x / 100.0;
  gfloat offset_y = o->offset_y / 100.0;

  // Get colors
  const Babl *format = babl_format("RGBA float");
  gfloat c1[4], c2[4], c3[4], c4[4], c5[4], c6[4], c7[4], c8[4];
  gegl_color_get_pixel(o->color1, format, c1);
  gegl_color_get_pixel(o->color2, format, c2);
  gegl_color_get_pixel(o->color3, format, c3);
  gegl_color_get_pixel(o->color4, format, c4);
  gegl_color_get_pixel(o->color5, format, c5);
  gegl_color_get_pixel(o->color6, format, c6);
  gegl_color_get_pixel(o->color7, format, c7);
  gegl_color_get_pixel(o->color8, format, c8);
  gfloat *colors[8] = {c1, c2, c3, c4, c5, c6, c7, c8};

  // Get number of colors
  gint num_colors;
  switch (o->num_colors)
  {
    case GEGL_GROKGRADIENT_NUM_COLORS_2_7194: num_colors = 2; break;
    case GEGL_GROKGRADIENT_NUM_COLORS_3_7194: num_colors = 3; break;
    case GEGL_GROKGRADIENT_NUM_COLORS_4_7194: num_colors = 4; break;
    case GEGL_GROKGRADIENT_NUM_COLORS_5_7194: num_colors = 5; break;
    case GEGL_GROKGRADIENT_NUM_COLORS_6_7194: num_colors = 6; break;
    case GEGL_GROKGRADIENT_NUM_COLORS_7_7194: num_colors = 7; break;
    case GEGL_GROKGRADIENT_NUM_COLORS_8_7194: num_colors = 8; break;
    default: num_colors = 4; break;
  }

  for (glong i = 0; i < n_pixels; i++)
  {
    // Compute normalized coordinates, centered at (0.5, 0.5) with offset
    gfloat x = ((i % roi->width) + roi->x) / width - 0.5 - offset_x;
    gfloat y = ((i / roi->width) + roi->y) / height - 0.5 - offset_y;

    // Compute t based on gradient type (using grokgradient.c's seamless math)
    gfloat t;
    switch (o->gradient_shape)
    {
      case GEGL_GROKGRADIENT_GRADIENT_TYPE_LINEAR_5823:
        t = (x * cos_a + y * sin_a + 0.5) * freq;
        t = fmodf(t, 1.0);
        if (t < 0.0) t += 1.0;
        break;
      case GEGL_GROKGRADIENT_GRADIENT_TYPE_BILINEAR_5823:
        t = fabsf(x * cos_a + y * sin_a) * freq;
        t = fmodf(t, 1.0);
        if (t < 0.0) t += 1.0;
        break;
      case GEGL_GROKGRADIENT_GRADIENT_TYPE_RADIAL_5823:
        t = sqrtf(x * x + y * y) * freq;
        t = fmodf(t, 1.0);
        if (t < 0.0) t += 1.0;
        break;
      case GEGL_GROKGRADIENT_GRADIENT_TYPE_SPIRAL_5823:
      {
        gfloat r = sqrtf(x * x + y * y);
        gfloat theta = atan2f(y, x) + angle_rad;
        t = (theta / (2.0 * G_PI) + r) * freq;
        t = fmodf(t, 1.0);
        if (t < 0.0) t += 1.0;
        break;
      }
      case GEGL_GROKGRADIENT_GRADIENT_TYPE_SPIRAL_CCW_5823:
      {
        gfloat r = sqrtf(x * x + y * y);
        gfloat theta = -atan2f(y, x) + angle_rad;
        t = (theta / (2.0 * G_PI) + r) * freq;
        t = fmodf(t, 1.0);
        if (t < 0.0) t += 1.0;
        break;
      }
      case GEGL_GROKGRADIENT_GRADIENT_TYPE_SQUARE_5823:
        t = fmaxf(fabsf(x), fabsf(y)) * freq;
        t = fmodf(t, 1.0);
        if (t < 0.0) t += 1.0;
        break;
      default:
        t = 0.0;
        break;
    }

    // Interpolate color
    gfloat grad_r, grad_g, grad_b;
    interpolate_gradient(t, colors, num_colors, &grad_r, &grad_g, &grad_b);

    // Apply saturation and brightness adjustments
    gfloat h, s, v;
    rgb_to_hsv(grad_r, grad_g, grad_b, &h, &s, &v);
    s *= o->saturation;
    v *= o->brightness;
    hsv_to_rgb(h, s, v, &grad_r, &grad_g, &grad_b);

    // Get input pixel color
    gfloat in_r = in_pixel[0];
    gfloat in_g = in_pixel[1];
    gfloat in_b = in_pixel[2];
    gfloat in_a = in_pixel[3];

    // Initialize output RGB and alpha
    gfloat final_r, final_g, final_b, final_a;

    if (o->blend == 0.0) {
      // No blending: use gradient color directly
      final_r = grad_r;
      final_g = grad_g;
      final_b = grad_b;
      final_a = o->alpha_lock ? in_a : 1.0;
    } else {
      // Convert input pixel and gradient to HSV for blending
      gfloat in_h, in_s, in_v;
      rgb_to_hsv(in_r, in_g, in_b, &in_h, &in_s, &in_v);

      gfloat grad_h, grad_s, grad_v;
      rgb_to_hsv(grad_r, grad_g, grad_b, &grad_h, &grad_s, &grad_v);

      // Blend in HSV space
      gfloat final_h = grad_h; // Use gradient's hue
      gfloat final_s = in_s * (1.0 - o->blend) + grad_s * o->blend;
      gfloat final_v = in_v * (1.0 - o->blend) + grad_v * o->blend;

      // Convert blended HSV back to RGB
      hsv_to_rgb(final_h, final_s, final_v, &final_r, &final_g, &final_b);

      // Handle alpha channel
      final_a = in_a;
      if (!o->alpha_lock) {
        final_a = 1.0;
      } else if (in_a < 1.0) {
        final_r = final_r * in_a + in_r * (1.0 - in_a);
        final_g = final_g * in_a + in_g * (1.0 - in_a);
        final_b = final_b * in_a + in_b * (1.0 - in_a);
      }
    }

    // Write output pixel
    out_pixel[0] = final_r;
    out_pixel[1] = final_g;
    out_pixel[2] = final_b;
    out_pixel[3] = final_a;

    // Move to next pixel
    in_pixel += 4;
    out_pixel += 4;
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
    "name", "ai/lb:custom-gradient",
    "title", _("Custom Gradient"),
    "description", _("Generate a custom gradient with 2 to 8 user-defined colors, supporting linear, bilinear, radial, spiral, and square patterns with seamless transitions, customizable frequency, saturation, brightness, and transparency handling"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL/Gradients",
    "gimp:menu-label", _("Custom Gradient..."),
    NULL);
}

#endif
