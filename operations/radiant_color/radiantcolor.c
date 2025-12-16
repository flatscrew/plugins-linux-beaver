/* GEGL operation implementing Radiant Color blend mode
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
 *
 * Credit to Pippin for making GEGL
 * Beaver vibe coded the plugin that Grok made
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

property_boolean (srgb, _("sRGB"), FALSE)
    description (_("Use sRGB gamma instead of linear"))

property_double (intensity_factor, _("Intensity Factor"), 1.0)
    description (_("Adjusts the strength of the Radiant Color glow (0.0 to 2.0)"))
    value_range (0.0, 2.0)
    ui_range (0.0, 2.0)
    ui_gamma (1.0)
    ui_steps (1, 0)

property_double (opacity, _("Opacity"), 1.0)
    description (_("Blend opacity (0.0 to 1.0)"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)
    ui_gamma (1.0)
    ui_steps (1, 0)

#else

#define GEGL_OP_POINT_COMPOSER
#define GEGL_OP_NAME     radiant_color_goat
#define GEGL_OP_C_SOURCE radiantcolor.c

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

  /* Get the raw values; this does not increase the reference count */
  input = gegl_operation_context_get_object (context, "input");
  aux = gegl_operation_context_get_object (context, "aux");

  /* Pass the input/aux buffers directly through if they are alone */
  {
    const GeglRectangle *in_extent = NULL;
    const GeglRectangle *aux_extent = NULL;

    if (input)
      in_extent = gegl_buffer_get_abyss (input);

    if (!input ||
        (aux && !gegl_rectangle_intersect (NULL, in_extent, result)))
      {
         gegl_operation_context_take_object (context, "output",
                                             g_object_ref (aux));
         return TRUE;
      }
    if (aux)
      aux_extent = gegl_buffer_get_abyss (aux);

    if (!aux ||
        (input && !gegl_rectangle_intersect (NULL, aux_extent, result)))
      {
        gegl_operation_context_take_object (context, "output",
                                            g_object_ref (input));
        return TRUE;
      }
  }
  /* Chain up to create the needed buffers for our actual process function */
  return operation_class->process (operation, context, output_prop, result, level);
}

static gboolean
process (GeglOperation       *op,
         void                *in_buf,
         void                *aux_buf,
         void                *out_buf,
         glong                n_pixels,
         const GeglRectangle *roi,
         gint                 level)
{
  const Babl *format = gegl_operation_get_format (op, "output");
  gint    components = babl_format_get_n_components (format);
  gfloat * GEGL_ALIGNED in = in_buf;
  gfloat * GEGL_ALIGNED aux = aux_buf;
  gfloat * GEGL_ALIGNED out = out_buf;
  glong    i;

  if (aux == NULL)
    return TRUE;

  /* Access the properties */
  gdouble intensity_factor = GEGL_PROPERTIES (op)->intensity_factor;
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

      /* Calculate luminance of grayscale-inverted input (assuming input is grayscale) */
      gfloat luminance = (b_r + b_g + b_b) / 3.0f; /* Simple average for grayscale */

      /* Radiant Color: Amplify aux color based on luminance */
      gfloat blend_factor = fminf(luminance * intensity_factor, 1.0f);

      /* Blend aux color stronger in bright areas */
      gfloat processed_r = b_r + blend_factor * (a_r - b_r);
      gfloat processed_g = b_g + blend_factor * (a_g - b_g);
      gfloat processed_b = b_b + blend_factor * (a_b - b_b);

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
    "name",        "ai/lb:radiant-color",
    "title",       _("Radiant Color Blend Mode"),
    "reference-hash", "radiantcolorblend",
    "description", _("Applies a Radiant Color blend mode, enhancing the aux color in bright areas of the grayscale-inverted input for a vibrant glow, adjustable via Intensity Factor and Opacity, with optional sRGB gamma"),
    NULL);
}

#endif
