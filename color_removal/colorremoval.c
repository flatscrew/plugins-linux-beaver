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
 * Grok for writing the plugin and Beaver for directing Grok 
* */
#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>

#ifdef GEGL_PROPERTIES

property_color(background_color, _("Background Color"), "0.0, 1.0, 0.0")
    description(_("Color of the background to remove (default is green)"))

property_double(tolerance, _("Tolerance"), 0.2)
    description(_("Tolerance for background color matching (0.0 to 1.0)"))
    value_range(0.0, 1.0)

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     colorremoval
#define GEGL_OP_C_SOURCE colorremoval.c

#include <gegl-op.h>

static void
prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "input", babl_format ("RGBA float"));
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
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
  gfloat *in = in_buf;
  gfloat *out = out_buf;

  gdouble bg_color[4];
  gegl_color_get_rgba (o->background_color, &bg_color[0], &bg_color[1], &bg_color[2], &bg_color[3]);

  gfloat tolerance = o->tolerance;

  for (gint i = 0; i < n_pixels; i++)
    {
      gfloat r = in[0];
      gfloat g = in[1];
      gfloat b = in[2];
      gfloat a = in[3];

      // Compute Euclidean distance in RGB space
      gfloat dr = r - (gfloat)bg_color[0];
      gfloat dg = g - (gfloat)bg_color[1];
      gfloat db = b - (gfloat)bg_color[2];
      gfloat distance = sqrt(dr * dr + dg * dg + db * db);

      // Normalize distance (max distance in RGB space is sqrt(3) ≈ 1.732)
      gfloat normalized_distance = distance / 1.732;

      // If the pixel's color is within tolerance, make it transparent
      if (normalized_distance <= tolerance)
        {
          out[0] = r;
          out[1] = g;
          out[2] = b;
          out[3] = 0.0; // Set alpha to 0 (transparent)
        }
      else
        {
          // Keep the pixel unchanged (including its alpha)
          out[0] = r;
          out[1] = g;
          out[2] = b;
          out[3] = a;
        }

      in += 4;
      out += 4;
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
    "name",        "ai/lb:color-removal",
    "title",       _("Color Removal"),
    "reference-hash", "bgremovalgeglgrok2",
    "description", _("Removes a selected color using late 90s-style color thresholding"),
    NULL);
}

#endif
