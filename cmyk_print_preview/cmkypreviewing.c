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
 */
#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>

#ifdef GEGL_PROPERTIES

enum_start (cmyk34222)
  enum_value (CMYK_ORIGINAL, "original", N_("Original"))
  enum_value (CMYK_VIBRANT_REDUCTION, "vibrant_reduction", N_("Vibrant Reduction"))
  enum_value (CMYK_GAMUT_COMPRESSION, "gamut_compression", N_("Gamut Compression"))
  enum_value (CMYK_MATTE_PAPER, "matte_paper", N_("Matte Paper"))
  enum_value (CMYK_GLOSSY_PAPER, "glossy_paper", N_("Glossy Paper"))
enum_end (cmyk3422)

property_enum (algorithm, _("CMYK Algorithm"),
               cmyk3422, cmyk34222,
               CMYK_GAMUT_COMPRESSION)
    description (_("Select the RGB to CMYK conversion algorithm"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     cmkypreviewing
#define GEGL_OP_C_SOURCE cmkypreviewing.c

#include <gegl-op.h>
#include <math.h>

static void
prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "input", babl_format ("RGBA float"));
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
}

static gboolean
process (GeglOperation *operation,
         void *in_buf,
         void *out_buf,
         glong n_pixels,
         const GeglRectangle *roi,
         gint level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  gfloat *in = in_buf;
  gfloat *out = out_buf;

  /* Debug output to confirm processing */
  g_print ("Processing %ld pixels in smooth.c with algorithm %d\n", n_pixels, o->algorithm);

  for (gint i = 0; i < n_pixels; i++)
    {
      /* Read RGB and alpha values */
      gfloat r = in[0];
      gfloat g = in[1];
      gfloat b = in[2];
      gfloat a = in[3];

      gfloat c, m, y, k;

      /* Base RGB to CMY conversion */
      c = 1.0 - r;
      m = 1.0 - g;
      y = 1.0 - b;

      switch (o->algorithm)
        {
        case CMYK_ORIGINAL:
          /* Original: Slight amplification for visibility */
          k = MIN (MIN (c, m), y);
          c = (c - k) / (1.0 - k + 0.0001);
          m = (m - k) / (1.0 - k + 0.0001);
          y = (y - k) / (1.0 - k + 0.0001);
          c = CLAMP (c * 1.1, 0.0, 1.0);
          m = CLAMP (m * 1.1, 0.0, 1.0);
          y = CLAMP (y * 1.1, 0.0, 1.0);
          k = CLAMP (k * 0.9, 0.0, 1.0);
          break;

        case CMYK_VIBRANT_REDUCTION:
          /* Vibrant Reduction: Reduce vibrancy to mimic CMYK gamut */
          k = MIN (MIN (c, m), y);
          c = (c - k) / (1.0 - k + 0.0001) * 0.8;
          m = (m - k) / (1.0 - k + 0.0001) * 0.8;
          y = (y - k) / (1.0 - k + 0.0001) * 0.8;
          c = CLAMP (c, 0.0, 1.0);
          m = CLAMP (m, 0.0, 1.0);
          y = CLAMP (y, 0.0, 1.0);
          break;

        case CMYK_GAMUT_COMPRESSION:
          /* Gamut Compression: Perceptual adjustment to fit CMYK gamut */
          k = MIN (MIN (c, m), y);
          c = (c - k) / (1.0 - k + 0.0001);
          m = (m - k) / (1.0 - k + 0.0001);
          y = (y - k) / (1.0 - k + 0.0001);
          gfloat saturation = (c + m + y) / 3.0;
          c = c * (1.0 - saturation * 0.3);
          m = m * (1.0 - saturation * 0.3);
          y = y * (1.0 - saturation * 0.3);
          c = CLAMP (c, 0.0, 1.0);
          m = CLAMP (m, 0.0, 1.0);
          y = CLAMP (y, 0.0, 1.0);
          break;

        case CMYK_MATTE_PAPER:
          /* Matte Paper: Simulate ink absorption, duller colors, lower contrast */
          k = MIN (MIN (c, m), y);
          c = (c - k) / (1.0 - k + 0.0001) * 0.75; /* Stronger vibrancy reduction */
          m = (m - k) / (1.0 - k + 0.0001) * 0.75;
          y = (y - k) / (1.0 - k + 0.0001) * 0.75;
          k = k * 1.1; /* Slightly increase black to simulate ink spread */
          k = CLAMP (k, 0.0, 1.0);
          c = CLAMP (c, 0.0, 1.0);
          m = CLAMP (m, 0.0, 1.0);
          y = CLAMP (y, 0.0, 1.0);
          break;

        case CMYK_GLOSSY_PAPER:
          /* Glossy Paper: Slightly higher contrast, still duller than RGB */
          k = MIN (MIN (c, m), y);
          c = (c - k) / (1.0 - k + 0.0001) * 0.85; /* Moderate vibrancy reduction */
          m = (m - k) / (1.0 - k + 0.0001) * 0.85;
          y = (y - k) / (1.0 - k + 0.0001) * 0.85;
          k = k * 0.95; /* Slightly reduce black for higher contrast */
          k = CLAMP (k, 0.0, 1.0);
          c = CLAMP (c, 0.0, 1.0);
          m = CLAMP (m, 0.0, 1.0);
          y = CLAMP (y, 0.0, 1.0);
          break;

        default:
          k = MIN (MIN (c, m), y);
          c = (c - k) / (1.0 - k + 0.0001);
          m = (m - k) / (1.0 - k + 0.0001);
          y = (y - k) / (1.0 - k + 0.0001);
          break;
        }

      /* Convert back to RGB for preview */
      out[0] = CLAMP (1.0 - (c * (1.0 - k) + k), 0.0, 1.0);
      out[1] = CLAMP (1.0 - (m * (1.0 - k) + k), 0.0, 1.0);
      out[2] = CLAMP (1.0 - (y * (1.0 - k) + k), 0.0, 1.0);
      out[3] = a; /* Preserve alpha */

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
    "name",        "ai/lb:cmyk",
    "title",       _("CMYK Print Preview"),
    "reference-hash", "cmykpreviewgegl3",
    "description", _("Converts an RGB image to a CMYK preview to show what an image might look like when printed. Recommended to use as an 'adjustment layer' by applying on a passthrough layer group with an empty layer. "),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("CMYK Preview (print preview)..."),
    NULL);
}

#endif
