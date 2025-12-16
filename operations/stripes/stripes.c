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
 * 2024, beaver, template plugin

optionally put graph here

]

end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

/*Properties go here*/

property_double (x_scale, _("Resize blinds X"), 177.0)
  description (_("Resize Blinds X axis"))
  value_range (6, G_MAXDOUBLE)
  ui_range    (6.0, 550.0)
  ui_meta     ("unit", "pixel-distance")
  ui_meta     ("axis", "x")

property_double (y_scale, _("Resize blinds Y"), 177.0)
  description (_("Resize Blinds Y axis"))
  value_range (6, G_MAXDOUBLE)
  ui_range    (6.0, 550.0)
  ui_meta     ("unit", "pixel-distance")
  ui_meta     ("axis", "y")


property_double (time_x, _("Time X"), 0.0)
  description (_("As if time passed and the blinds moved horizontal"))
  value_range (-G_MAXDOUBLE, G_MAXDOUBLE)
  ui_range    (-512.0, 512.0)
  ui_meta     ("unit", "pixel-coordinate")
  ui_meta     ("axis", "x")

property_double (time_y, _("Time Y"), 0.0)
  description (_("As if time passed and the blinds moved vertical"))
  value_range (-G_MAXDOUBLE, G_MAXDOUBLE)
  ui_range    (-512.0, 512.0)
  ui_meta     ("unit", "pixel-coordinate")
  ui_meta     ("axis", "y")


property_double (rotation, _("Rotation"), 137.0)
  description(_("Blind rotation angle"))
  value_range (0.0, 360.0)
  ui_meta     ("unit", "degree")
  ui_meta     ("direction", "ccw")

property_color (color1, _("Color"), "#ffffff")
    description (_("The color to paint over the blinds"))


property_color (color2, _("Color 2"), "#ff0200")
    description (_("The color to paint over the blinds"))

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     stripes
#define GEGL_OP_C_SOURCE stripes.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *ls;
 GeglNode *gm;
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

 state->ls = gegl_node_new_child (gegl, "operation", "gegl:linear-sinusoid", "x-amplitude", 7.0, "y-amplitude", 7.0,  "x-phase", 741.0, "y-phase", 741.0, "angle", 0.0,    NULL);

 state->gm = gegl_node_new_child (gegl, "operation", "port:gradient-map", NULL);

/*meta redirect property to new child orders go here

 gegl_operation_meta_redirect (operation, "propertyname", state->newchildname,  "originalpropertyname");
*/

 gegl_operation_meta_redirect (operation, "time-x", state->ls,  "x-offset");
 gegl_operation_meta_redirect (operation, "time-y", state->ls,  "y-offset");
 gegl_operation_meta_redirect (operation, "rotation", state->ls,  "rotation");
 gegl_operation_meta_redirect (operation, "x-scale", state->ls,  "x-period");
 gegl_operation_meta_redirect (operation, "y-scale", state->ls,  "y-period");
 gegl_operation_meta_redirect (operation, "color1", state->gm,  "color1");
 gegl_operation_meta_redirect (operation, "color2", state->gm,  "color2");
}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;

  gegl_node_link_many (state->ls, state->gm,  state->output,  NULL);

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
    "name",        "lb:stripes",
    "title",       _("Stripes"),
    "reference-hash", "neartheend",
    "description", _("Two colored blind stripes "),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Render/Fun",
    "gimp:menu-label", _("Stripes..."),
    NULL);
}

#endif
