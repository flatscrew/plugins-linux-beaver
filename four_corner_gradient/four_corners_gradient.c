/*
 * Pippin (2006) for making GEGL
 * Beaver for vibe coding
 * Grok 3 for making the GEGL plugin
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
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

/* Enum for interpolation types */
enum_start(gegl_interpolation_type2351)
  enum_value(GEGL_INTERPOLATION_LINEAR, "linear", N_("Linear"))
  enum_value(GEGL_INTERPOLATION_CUBIC,  "cubic",  N_("Cubic"))
enum_end(GeglInterpolationType2351)

property_enum(interpolation_type, _("Interpolation Type"),
              GeglInterpolationType2351, gegl_interpolation_type2351,
              GEGL_INTERPOLATION_CUBIC)
    description(_("Select the interpolation method for the gradient (Linear for standard blending, Cubic for smoother transitions)"))

property_color(top_left_color, _("Top-Left Color"), "#ff0000")  /* Red */
    description(_("Color for the top-left corner of the gradient"))

property_color(top_right_color, _("Top-Right Color"), "#00ff00")  /* Green */
    description(_("Color for the top-right corner of the gradient"))

property_color(bottom_left_color, _("Bottom-Left Color"), "#0000ff")  /* Blue */
    description(_("Color for the bottom-left corner of the gradient"))

property_color(bottom_right_color, _("Bottom-Right Color"), "#ffff00")  /* Yellow */
    description(_("Color for the bottom-right corner of the gradient"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     four_corners_gradient
#define GEGL_OP_C_SOURCE four_corners_gradient.c

#include <gegl-op.h>

static void
prepare(GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space(operation, "input");
  gegl_operation_set_format(operation, "input", babl_format_with_space("RGBA float", space));
  gegl_operation_set_format(operation, "output", babl_format_with_space("RGBA float", space));
}

/* Helper function for cubic interpolation */
static gfloat cubic_interpolate(gfloat p0, gfloat p1, gfloat p2, gfloat p3, gfloat t)
{
  /* Catmull-Rom spline for smooth cubic interpolation */
  gfloat t2 = t * t;
  gfloat t3 = t2 * t;
  return 0.5f * (
    (2.0f * p1) +
    (-p0 + p2) * t +
    (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
    (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3
  );
}

static void
render_gradient(GeglOperation *operation, gfloat *out, gint width, gint height, const GeglRectangle *roi, gint level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  GeglRectangle *canvas = gegl_operation_source_get_bounding_box(operation, "input");
  gfloat canvas_width = canvas ? canvas->width : roi->width;
  gfloat canvas_height = canvas ? canvas->height : roi->height;

  /* Get the colors for each corner */
  gfloat tl_rgb[4], tr_rgb[4], bl_rgb[4], br_rgb[4];
  gegl_color_get_pixel(o->top_left_color, babl_format("RGBA float"), tl_rgb);
  gegl_color_get_pixel(o->top_right_color, babl_format("RGBA float"), tr_rgb);
  gegl_color_get_pixel(o->bottom_left_color, babl_format("RGBA float"), bl_rgb);
  gegl_color_get_pixel(o->bottom_right_color, babl_format("RGBA float"), br_rgb);

  /* Render the gradient for each pixel */
  for (gint y = 0; y < height; y++)
  {
    for (gint x = 0; x < width; x++)
    {
      /* Normalize the pixel coordinates to [0, 1] */
      gfloat u = (gfloat)(x + roi->x) / canvas_width;  /* Horizontal position (0 to 1) */
      gfloat v = (gfloat)(y + roi->y) / canvas_height; /* Vertical position (0 to 1) */
      u = CLAMP(u, 0.0f, 1.0f);
      v = CLAMP(v, 0.0f, 1.0f);

      glong idx = (y * width + x) * 4;
      if (o->interpolation_type == GEGL_INTERPOLATION_LINEAR)
      {
        /* Linear (Bilinear) Interpolation */
        /* Interpolate horizontally along the top edge (top-left to top-right) */
        gfloat top_rgb[4];
        for (gint i = 0; i < 4; i++)
        {
          top_rgb[i] = tl_rgb[i] + u * (tr_rgb[i] - tl_rgb[i]);
        }

        /* Interpolate horizontally along the bottom edge (bottom-left to bottom-right) */
        gfloat bottom_rgb[4];
        for (gint i = 0; i < 4; i++)
        {
          bottom_rgb[i] = bl_rgb[i] + u * (br_rgb[i] - bl_rgb[i]);
        }

        /* Interpolate vertically between the top and bottom edges */
        for (gint i = 0; i < 4; i++)
        {
          out[idx + i] = top_rgb[i] + v * (bottom_rgb[i] - top_rgb[i]);
        }
      }
      else /* GEGL_INTERPOLATION_CUBIC */
      {
        /* Cubic (Bicubic) Interpolation */
        /* Since we only have four points, we'll extend the control points by mirroring:
         * For horizontal: assume points before TL/BL and after TR/BR mirror the colors.
         * For vertical: assume points above TL/TR and below BL/BR mirror the colors.
         */
        gfloat result_rgb[4];
        for (gint i = 0; i < 4; i++)
        {
          /* Horizontal cubic interpolation for top edge */
          gfloat top_p0 = tr_rgb[i];  /* Mirror TR as point before TL */
          gfloat top_p1 = tl_rgb[i];
          gfloat top_p2 = tr_rgb[i];
          gfloat top_p3 = tl_rgb[i];  /* Mirror TL as point after TR */
          gfloat top_value = cubic_interpolate(top_p0, top_p1, top_p2, top_p3, u);

          /* Horizontal cubic interpolation for bottom edge */
          gfloat bottom_p0 = br_rgb[i];  /* Mirror BR as point before BL */
          gfloat bottom_p1 = bl_rgb[i];
          gfloat bottom_p2 = br_rgb[i];
          gfloat bottom_p3 = bl_rgb[i];  /* Mirror BL as point after BR */
          gfloat bottom_value = cubic_interpolate(bottom_p0, bottom_p1, bottom_p2, bottom_p3, u);

          /* Vertical cubic interpolation between top and bottom */
          gfloat vert_p0 = bottom_value;  /* Mirror bottom as point above top */
          gfloat vert_p1 = top_value;
          gfloat vert_p2 = bottom_value;
          gfloat vert_p3 = top_value;  /* Mirror top as point below bottom */
          result_rgb[i] = cubic_interpolate(vert_p0, vert_p1, vert_p2, vert_p3, v);
        }

        /* Clamp the result to valid color ranges and assign to output */
        for (gint i = 0; i < 4; i++)
        {
          out[idx + i] = CLAMP(result_rgb[i], 0.0f, 1.0f);
        }
      }
    }
  }
}

static gboolean
process(GeglOperation *operation,
        void          *in_buf,
        void          *out_buf,
        glong          n_pixels,
        const GeglRectangle *roi,
        gint           level)
{
  gfloat *out = (gfloat *) out_buf;
  render_gradient(operation, out, roi->width, roi->height, roi, level);
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
    "name",        "ai/lb:four-corners-gradient",
    "title",       _("Four Corners Gradient"),
    "reference-hash", "fourcornersgradient1",
    "description", _("Renders a gradient with customizable colors at each of the four corners of the canvas"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL/Gradients",
    "gimp:menu-label", _("Four Corners Gradient..."),
    NULL); 
}

#endif
