/* GEGL operation implementing Krita's Freeze blend mode
 *
 * This file is an image processing operation for GEGL
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
 * Credit to Pippin for making GEGL
 * Beaver vibe coded the plugin that Grok made  
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES
property_boolean (srgb, _("sRGB"), FALSE)
    description (_("Use sRGB gamma instead of linear"))

property_double (opacity, _("Opacity"), 1.0)
    description (_("Blend opacity (0.0 to 1.0)"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)
    ui_gamma (1.0)
    ui_steps (1, 0)
#else

#define GEGL_OP_POINT_COMPOSER
#define GEGL_OP_NAME     freezinggoat
#define GEGL_OP_C_SOURCE freezeblend.c

#include "gegl-op.h"

static void
prepare (GeglOperation *operation)
{
  const Babl *format = gegl_operation_get_source_format (operation, "input");
  if (!format)
    format = gegl_operation_get_source_format (operation, "aux");

  if (GEGL_PROPERTIES (operation)->srgb)
    format = gegl_babl_variant (format, GEGL_BABL_VARIANT_PERCEPTUAL_PREMULTIPLIED);
  else
    format = gegl_babl_variant (format, GEGL_BABL_VARIANT_LINEAR_PREMULTIPLIED);

  gegl_operation_set_format (operation, "input",  format);
  gegl_operation_set_format (operation, "aux",    format);
  gegl_operation_set_format (operation, "output", format);
}

static gboolean
operation_process (GeglOperation        *operation,
                  GeglOperationContext *context,
                  const gchar          *output_prop,
                  const GeglRectangle  *result,
                  gint                 level)
{
  GeglOperationClass  *operation_class;
  gpointer input, aux;
  operation_class = GEGL_OPERATION_CLASS (gegl_op_parent_class);

  /* get the raw values this does not increase the reference count */
  input = gegl_operation_context_get_object (context, "input");
  aux = gegl_operation_context_get_object (context, "aux");

  /* pass the input/aux buffers directly through if they are alone */
  {
    const GeglRectangle *in_extent = NULL;
    const GeglRectangle *aux_extent = NULL;

    if (input)
      in_extent = gegl_buffer_get_abyss (input);

    if (!input || (aux && !gegl_rectangle_intersect (NULL, in_extent, result)))
      {
        gegl_operation_context_take_object (context, "output",
                                           g_object_ref (aux));
        return TRUE;
      }
    if (aux)
      aux_extent = gegl_buffer_get_abyss (aux);

    if (!aux || (input && !gegl_rectangle_intersect (NULL, aux_extent, result)))
      {
        gegl_operation_context_take_object (context, "output",
                                           g_object_ref (input));
        return TRUE;
      }
  }
  /* chain up, which will create the needed buffers for our actual
   * process function
   */
  return operation_class->process (operation, context, output_prop, result, level);
}

static gboolean
process (GeglOperation       *op,
         void                *in_buf,
         void                *aux_buf,
         void                *out_buf,
         glong               n_pixels,
         const GeglRectangle *roi,
         gint                level)
{
  const Babl *format = gegl_operation_get_format (op, "output");
  gint    components = babl_format_get_n_components (format);
  gfloat * GEGL_ALIGNED in = in_buf;
  gfloat * GEGL_ALIGNED aux = aux_buf;
  gfloat * GEGL_ALIGNED out = out_buf;
  glong   i;

  if (aux == NULL)
    return TRUE;

  /* Access the property */
  gdouble opacity = GEGL_PROPERTIES (op)->opacity;

  for (i = 0; i < n_pixels; i++)
    {
      gfloat a_r = aux[0];  /* Source (aux) red */
      gfloat a_g = aux[1];  /* Source green */
      gfloat a_b = aux[2];  /* Source blue */
      gfloat b_r = in[0];   /* Destination (input) red */
      gfloat b_g = in[1];   /* Destination green */
      gfloat b_b = in[2];   /* Destination blue */
      gfloat b_a = in[3];   /* Destination alpha */

      /* Freeze blend mode: b^2 / (1 - a), clamped to [0, 1] */
      gfloat denom_r = 1.0f - a_r;
      gfloat denom_g = 1.0f - a_g;
      gfloat denom_b = 1.0f - a_b;

      gfloat processed_r = (denom_r > 0.0f) ? CLAMP((b_r * b_r) / denom_r, 0.0f, 1.0f) : b_r;
      gfloat processed_g = (denom_g > 0.0f) ? CLAMP((b_g * b_g) / denom_g, 0.0f, 1.0f) : b_g;
      gfloat processed_b = (denom_b > 0.0f) ? CLAMP((b_b * b_b) / denom_b, 0.0f, 1.0f) : b_b;

      /* Apply opacity */
      out[0] = b_r + opacity * (processed_r - b_r);
      out[1] = b_g + opacity * (processed_g - b_g);
      out[2] = b_b + opacity * (processed_b - b_b);

      out[3] = b_a;  /* Preserve destination alpha */

      in  += components;
      aux += components;
      out += components;
    }

  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass              *operation_class;
  GeglOperationPointComposerClass *point_composer_class;

  operation_class      = GEGL_OPERATION_CLASS (klass);
  point_composer_class = GEGL_OPERATION_POINT_COMPOSER_CLASS (klass);

  point_composer_class->process = process;
  operation_class->process      = operation_process;
  operation_class->prepare      = prepare;

  gegl_operation_class_set_keys (operation_class,
    "name",        "ai/lb:freeze",
    "title",       _("Freeze Blend Mode"),
    "reference-hash", "freezeblendsmooth",
    "categories", "hidden",
    "description", _("Freeze Blend Mode"),
    NULL);
}

#endif
