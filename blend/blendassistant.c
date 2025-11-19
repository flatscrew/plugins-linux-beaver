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
 * 2024, beaver, blend assistant

id=1 src aux=[ ref=1 
gaussian-blur std-dev-x=130 std-dev-y=130 abyss-policy=none clip-extent=false
 opacity value=1
]

src-atop aux=[ ref=1 opacity value=1 ]



]

end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

/*Properties go here*/

property_double (std_dev_x, _("Blur Size X"), 100.0)
   description (_("Blur Vertical Scale"))
   value_range (0.0, 1000.0)
   ui_range    (0.24, 300.0)
   ui_gamma    (2.0)
   ui_meta     ("unit", "pixel-distance")
   ui_meta     ("axis", "x")

property_double (std_dev_y, _("Blur Size Y"), 100.0)
   description (_("Blur Horizontal Scale"))
   value_range (0.0, 1000.0)
   ui_range    (0.24, 300.0)
   ui_gamma    (2.0)
   ui_meta     ("unit", "pixel-distance")
   ui_meta     ("axis", "y")

property_double (opacity, _("Opacity"), 1.0)
    description (_("Opacity control of the content on top of the blur"))
    value_range (0.0, 1.0)
    ui_range    (0.0, 1.0)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     blendassistant
#define GEGL_OP_C_SOURCE blendassistant.c




#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *gaussian;
 GeglNode *opacity;
 GeglNode *alphalock;
 GeglNode *replace;
 GeglNode *blur;
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
  state->replace = gegl_node_new_child (gegl, "operation", "gegl:src", NULL);
  state->alphalock = gegl_node_new_child (gegl, "operation", "gegl:src-atop", NULL);
  state->opacity = gegl_node_new_child (gegl, "operation", "gegl:opacity", NULL);
  state->blur = gegl_node_new_child (gegl, "operation", "gegl:gaussian-blur", "abyss-policy", 0, "clip-extent", 0, "std-dev-x", 100.0, "std-dev-y", 100.0, NULL);

 gegl_operation_meta_redirect (operation, "std_dev_y", state->blur,  "std-dev-y");
 gegl_operation_meta_redirect (operation, "std_dev_x", state->blur,  "std-dev-x");
 gegl_operation_meta_redirect (operation, "opacity", state->opacity,  "value");
}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;

gegl_node_link_many (state->input, state->replace,  state->alphalock,  state->output,  NULL);
gegl_node_link_many (state->input, state->blur,    NULL);
gegl_node_connect (state->replace, "aux", state->blur, "output");
gegl_node_connect (state->alphalock, "aux", state->opacity, "output");
gegl_node_link_many (state->input, state->opacity,  NULL);


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
    "name",        "lb:blend",
    "title",       _("Blend assistant"),
    "reference-hash", "steelserieskeyboard",
    "description", _("Helps blend images by alpha locking image content on a blur"),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Artistic",
    "gimp:menu-label", _("Blend assistant..."),
    NULL);
}

#endif
