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
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions
 * 2024, Beaver GEGL Mini Shape Imprints 
* 
* graph below
* 
 id=1
lighten aux=[  ref=1 median-blur radius=5 percentile=100 abyss-policy=none neighborhood=circle
id=2 multiply aux=[ ref=2 emboss depth=100 elevation=90  ]
]
 * 
* 
* 
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_int (radius, _("Radius"), 4)
    description (_("Size of the mini shapes"))
  value_range (0, 50)
  ui_gamma (1.5)
   ui_range  (0, 30)

property_enum (shape, _("Shape type"),
               GeglMedianBlurNeighborhoodmsi, gegl_median_blur_neighborhoodmsi,
               GEGL_MEDIAN_BLUR_NEIGHBORHOOD_CIRCLE)
  description (_("Neighborhood type"))

enum_start (gegl_median_blur_neighborhoodmsi)
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_SQUARE,  "square",  N_("Square"))
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_CIRCLE,  "circle",  N_("Circle"))
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_DIAMOND, "diamond", N_("Diamond"))
enum_end (GeglMedianBlurNeighborhoodmsi)


property_int (depth, _("Depth"), 25)
    description (_("Depth of the mini shapes"))
    value_range (1, 100)


#else

#define GEGL_OP_META
#define GEGL_OP_NAME     msi
#define GEGL_OP_C_SOURCE msi.c

#include "gegl-op.h"

static void
attach (GeglOperation *operation)
{
  GeglNode *gegl   = operation->node;
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");

GeglNode*lighten = gegl_node_new_child (gegl, "operation", "gegl:lighten",  NULL);
GeglNode*median = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "radius", 5, "percentile", 100.0, "abyss-policy", 0, "neighborhood", 0,  NULL);
GeglNode*multiply = gegl_node_new_child (gegl, "operation", "gegl:multiply",  NULL);
GeglNode*idref = gegl_node_new_child (gegl, "operation", "gegl:nop",  NULL);
GeglNode*emboss = gegl_node_new_child (gegl, "operation", "gegl:emboss", "depth", 100, "elevation", 90.0,  NULL);
GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");


gegl_operation_meta_redirect (operation, "depth", emboss, "depth");

gegl_operation_meta_redirect (operation, "radius", median, "radius");
gegl_operation_meta_redirect (operation, "shape", median, "neighborhood");


gegl_node_link_many (input, lighten, output, NULL);
gegl_node_link_many (input, median, idref, multiply, NULL);
gegl_node_connect (lighten, "aux", multiply, "output");
gegl_node_connect (multiply, "aux", emboss, "output");
gegl_node_link_many (idref, emboss, NULL);


}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_class->threaded = FALSE;

  gegl_operation_class_set_keys (operation_class,
    "name",           "lb:msi",
    "title",          _("Mini Shape Imprints"),
    "reference-hash", "reincarnationpastlives",
    "description", _("Mini Shape Inprints"),   NULL);
                               
}

#endif
