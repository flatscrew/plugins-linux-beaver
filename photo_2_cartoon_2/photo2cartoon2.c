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
 * 2021 cli345 on GimpChat (Discovering part of the GEGL GRAPH
 * 2024, Beaver modifying the graph heavily and making the Graph into a GEGL filter
 * 
* 
* 
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES


property_int (median, _("Median radius"), 7)
   description (_("Standard deviation for the horizontal axis"))
   value_range (2, 10)
   ui_range    (2, 10)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")

property_double (radius1, _("Difference of Gaussian 1"), 1.2)
  ui_range (0.0, 2.0)
  ui_gamma (1.5)
   value_range  (0.500, 2.00)

property_double (radius2, _("Difference of Gaussian 2"), 0.0)
  ui_gamma (1.5)
   ui_range  (0.0, 0.62)
   value_range  (0.0, 1.00)

property_double (light, _("Light control"), 1.0)
  ui_gamma (1.5)
   ui_range  (1.0, 2.0)
   value_range  (1.0, 2.0)

property_double (saturation, _("Saturation control"), 1.4)
  ui_gamma (1.5)
   ui_range  (1.0, 2.0)
   value_range  (1.0, 2.0)



#else

#define GEGL_OP_META
#define GEGL_OP_NAME     photo2cartoon2
#define GEGL_OP_C_SOURCE photo2cartoon2.c

#include "gegl-op.h"

static void
attach (GeglOperation *operation)
{
  GeglNode *gegl   = operation->node;
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");

GeglNode*median = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "radius", 4,  NULL);
GeglNode*dt = gegl_node_new_child (gegl, "operation", "gegl:domain-transform",  NULL);
GeglNode*multiply = gegl_node_new_child (gegl, "operation", "gegl:multiply",  NULL);
GeglNode*inhigh = gegl_node_new_child (gegl, "operation", "gegl:multiply",  NULL);
GeglNode*saturation = gegl_node_new_child (gegl, "operation", "gegl:saturation", "scale", 1.5, "colorspace", 1,  NULL);
GeglNode*pencil = gegl_node_new_child (gegl, "operation", "lb:pencil", "dg1", 1.4, "dg2", 0.0, "gaus", 0.0,  NULL);
GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");

gegl_operation_meta_redirect (operation, "median", median, "radius");
gegl_operation_meta_redirect (operation, "saturation", saturation, "scale");
gegl_operation_meta_redirect (operation, "radius1", pencil, "dg1");
gegl_operation_meta_redirect (operation, "radius2", pencil, "dg2");
gegl_operation_meta_redirect (operation, "light", inhigh, "value");
gegl_operation_meta_redirect (operation, "saturation", saturation, "scale");

gegl_node_link_many (input, median, dt, multiply, saturation, inhigh, output, NULL);
gegl_node_connect (multiply, "aux", pencil, "output");
gegl_node_link_many (input, pencil, NULL);
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_class->threaded = FALSE;

  gegl_operation_class_set_keys (operation_class,
    "name",           "lb:photo-2-cartoon-2",
    "title",          _("Photo to Cartoon 2"),
    "reference-hash", "ptcartoon2",
    "description", _("Photo to Cartoon 2"),   NULL);
                               
}

#endif
