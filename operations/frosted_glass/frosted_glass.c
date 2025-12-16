/* This file is an image processing operation for GEGL
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
 * 2025 beaver, Frosted Glass 
based on a GEGL Graph I wrote back in 2021.

Recreate plugin using this GEGL syntax

gaussian-blur std-dev-x=0 std-dev-y=0
noise-spread amount-y=50 amount-x=50 seed=0
oilify mask-radius=2 exponent=1 intensities=150
noise-reduction iterations=1
unsharp-mask  std-dev=3
id=1 plus aux=[ ref=1  image-gradient  opacity value=0.3  ]

 */



#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES


property_int (spread, _("Spread"), 50)
   description (_("Spread the frosted glass"))
   value_range (30, 200)
   ui_range    (30, 150)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")

property_int (smooth, _("Smooth"), 2)
   description (_("Smooth the frosted glass"))
   value_range (1, 5)
   ui_range    (1, 5)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")

property_double (frost, _("Frost"), 0.15)
   description (_("Intensity of the frost effect on the glass"))
   value_range (0.0, 1.0)
   ui_range    (0.0, 0.6)
   ui_gamma    (1.0)
   ui_meta     ("unit", "pixel-distance")

property_double (blur, _("Blur behind frost"), 0)
   description (_("Blur the original image behind the frost"))
   value_range (0, 12)
   ui_range    (0, 12)
   ui_gamma    (1.0)
   ui_meta     ("unit", "pixel-distance")


property_seed (seed, _("Random seed"), rand)
   description (_("Noise seed of the frosted glass"))


#else

#define GEGL_OP_META
#define GEGL_OP_NAME     frosted_glass
#define GEGL_OP_C_SOURCE frosted_glass.c

#include "gegl-op.h"

static void
attach (GeglOperation *operation)
{
  GeglNode *gegl   = operation->node;
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");

  GeglNode *oilify  = gegl_node_new_child (gegl,
                                          "operation", "gegl:oilify", "mask-radius", 2, "exponent", 1,  "intensities", 115, 
                                          NULL);

  GeglNode *smooth  = gegl_node_new_child (gegl,
                                          "operation", "gegl:noise-reduction", "iterations", 2,
                                          NULL);

  GeglNode *edge  = gegl_node_new_child (gegl,
                                          "operation", "gegl:image-gradient",
                                          NULL);

  GeglNode *gaus  = gegl_node_new_child (gegl,
                                          "operation", "gegl:gaussian-blur", "std-dev-x", 0, "std-dev-y", 0,
                                          NULL);

  GeglNode *noise  = gegl_node_new_child (gegl,
                                          "operation", "gegl:noise-hsv", "hue-distance", 0.0, "saturation-distance", 0.0, "value-distance", 0.12,
                                          NULL);

  GeglNode *plus  = gegl_node_new_child (gegl,
                                          "operation", "gegl:plus",
                                          NULL);

  GeglNode *idref  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop",
                                          NULL);

  GeglNode *avoidtransparent  = gegl_node_new_child (gegl,
                                          "operation", "gegl:src-atop",
                                          NULL);

  GeglNode *opacity  = gegl_node_new_child (gegl,
                                          "operation", "gegl:opacity", "value", 0.3, 
                                          NULL);

  GeglNode *spread  = gegl_node_new_child (gegl,
                                          "operation", "gegl:noise-spread", "amount-y", 50, "amount-x", 50, 
                                          NULL);

  GeglNode *sharp  = gegl_node_new_child (gegl,
                                          "operation", "gegl:unsharp-mask", "std-dev", 3.0, 
                                          NULL);

  GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");

gegl_node_link_many (input, avoidtransparent, output,  NULL);
gegl_node_link_many (input, gaus, noise, spread, oilify, smooth, sharp, idref, plus,  NULL);
gegl_node_connect (plus, "aux", opacity, "output");
gegl_node_link_many (idref, edge, opacity,  NULL);
gegl_node_connect (avoidtransparent, "aux", plus, "output");

  gegl_operation_meta_redirect (operation, "frost",   opacity, "value");
  gegl_operation_meta_redirect (operation, "smooth",  smooth, "iterations");
  gegl_operation_meta_redirect (operation, "spread",  spread, "amount-x");
  gegl_operation_meta_redirect (operation, "spread",  spread, "amount-y");
  gegl_operation_meta_redirect (operation, "seed",  spread, "seed");
  gegl_operation_meta_redirect (operation, "blur",  gaus, "std-dev-x");
  gegl_operation_meta_redirect (operation, "blur",  gaus, "std-dev-y");
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_class->threaded = FALSE;

  gegl_operation_class_set_keys (operation_class,
    "name",           "lb:frosted-glass",
    "title",          _("Frosted Glass"),
    "reference-hash", "futurefunkradiochillbeaver",
    "description", _("Make your image appear as if it were behind frosted glass"),
    "gimp:menu-path", "<Image>/Filters/Blur",
    "gimp:menu-label", _("Frosted Glass..."),
                                 NULL);
}

#endif
