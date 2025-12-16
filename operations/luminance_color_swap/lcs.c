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
 * 2006 Credit to Pippin for major GEGL contributions
 * 2025 Beaver Luminance Color Swap
* 
* Graph here
id=1 src-in aux=[ ref=1 gegl:svg-luminancetoalpha  dst-over aux=[ color value=#fff400  ]  ]
*/

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_color (color, _("Color"), "#fff400")


#else

#define GEGL_OP_META
#define GEGL_OP_NAME     lcs
#define GEGL_OP_C_SOURCE lcs.c

#include "gegl-op.h"

static void
attach (GeglOperation *operation)
{
  GeglNode *gegl   = operation->node;
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");
  GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");


  GeglNode *replacealphalock  = gegl_node_new_child (gegl,
                                          "operation", "gegl:src-in",
                                          NULL);
  GeglNode *behind  = gegl_node_new_child (gegl,
                                          "operation", "gegl:dst-over",
                                          NULL);

  GeglNode *lumaremove  = gegl_node_new_child (gegl,
                                          "operation", "gegl:svg-luminancetoalpha",
                                          NULL);
  GeglNode *color  = gegl_node_new_child (gegl,
                                          "operation", "gegl:color",
                                          NULL);

  gegl_node_link_many (input, replacealphalock, output, NULL);
  gegl_node_connect (replacealphalock, "aux", behind, "output");
  gegl_node_link_many (input, lumaremove, behind, NULL);
  gegl_node_connect (behind, "aux", color, "output");

  gegl_operation_meta_redirect (operation, "color",    color, "value");

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_class->threaded = FALSE;

  gegl_operation_class_set_keys (operation_class,
    "name",           "lb:lcs",
    "title",          _("Swap lighting with color"),
    "reference-hash", "sweetbutshespsychoalittlebitpsycho",
    "description", _("Swap lighting channel with a color fill"),
    "gimp:menu-path", "<Image>/Colors",
    "gimp:menu-label", _("Light Color Swap..."),
                                 NULL);
}

#endif
