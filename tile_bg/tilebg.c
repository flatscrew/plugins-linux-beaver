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
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions
 * 2024, beaver, Tile Background

graph sytax here

gegl:load path=/home/contrast/Pictures/cards/11h.png
tile
lens-distortion zoom=-29
rotate degrees=-20 

end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

/*Properties go here*/

property_file_path (image, _("Image Upload"), "")

property_double (rotate, _("Rotate"), 0.0)
    description (_("Rotate (degrees)"))
    value_range (-180, 180)
    ui_meta ("unit", "degree")
    ui_meta ("direction", "ccw")

property_double (zoom, _("Zoom"), 0.0)
    description (_("Zoom in and out of image tiles"))
    value_range (-99, 100)
    ui_meta ("unit", "degree")



#else

#define GEGL_OP_META
#define GEGL_OP_NAME     tilebg
#define GEGL_OP_C_SOURCE tilebg.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *load;
 GeglNode *zoom;
 GeglNode *tile;
 GeglNode *rotate; 
 GeglNode *output;
}State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);

  State *state = o->user_data = g_malloc0 (sizeof (State));

/*new child node list is here, this is where starred nodes get defined

 state->newchildname = gegl_node_new_child (gegl, "operation", "lb:name", NULL);*/
 state->input    = gegl_node_get_input_proxy (gegl, "input");
 state->output   = gegl_node_get_output_proxy (gegl, "output");
 state->load = gegl_node_new_child (gegl, "operation", "gegl:load", NULL);
 state->tile = gegl_node_new_child (gegl, "operation", "gegl:tile", NULL);
 state->zoom = gegl_node_new_child (gegl, "operation", "gegl:lens-distortion", NULL);
 state->rotate = gegl_node_new_child (gegl, "operation", "gegl:rotate", NULL);
}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;

  gegl_node_link_many (state->load, state->tile, state->zoom, state->rotate,  state->output,  NULL);

 gegl_operation_meta_redirect (operation, "image", state->load,  "path");
 gegl_operation_meta_redirect (operation, "zoom", state->zoom,  "zoom");
 gegl_operation_meta_redirect (operation, "rotate", state->rotate,  "degrees");

/*optional connect from and too is here
  gegl_node_connect (state->blendmode, "aux", state->lastnodeinlist, "output"); */

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;
GeglOperationMetaClass *operation_meta_class = GEGL_OPERATION_META_CLASS (klass);
  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_meta_class->update = update_graph;

  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:tilebg",
    "title",       _("Tiled Background Image"),
    "reference-hash", "332thisisgettingmonotonous343",
    "description", _("Make a background tile, rotate and resize it"),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Render/Fun",
    "gimp:menu-label", _("Tiled Background Image..."),
    NULL);
}

#endif
