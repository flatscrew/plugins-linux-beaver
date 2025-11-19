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
*  2017 Thomas Manni (for making segment kmeans in GEGL workshop his filter was labeled port) 
 * 2024, beaver, Recursive Diamonds/Circles

Recreate this filter using this GEGL Graph 

src id=1  aux=[ ref=1
cell-noise
scale=0.020
shape=1
rank=1
iterations=1
seed=3322

]
crop

segment-kmeans n-clusters=12 max-iterations=38 


lb:glowstick
color=#00b4ff
blendmode=lchcolor
noisereduction=1
sat=11.163000106811523
lightness=-1.3350000381469727
strength=31.378999710083008
softness=52.240001678466797
radius=41.790000915527344
brightness=0.0
glow-radius=73.279998779296875

unsharp-mask scale=2
]

end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

/*Properties go here*/

enum_start (shape_fun_select)
  enum_value (GEGL_DIAMOND, "diamond",
              N_("Diamond"))
  enum_value (GEGL_CIRCLE,      "circle",
              N_("Circle"))
enum_end (shapefunselect)

property_enum (shape, _("Shape"),
    shapefunselect, shape_fun_select,
    GEGL_DIAMOND)
    description (_("Shape to select"))

property_color (color, _("Color of the background"), "#00b4ff")
    description (_("Color of the background"))


property_double (size, _("Size (reverse controls)"), 0.020)
    description (_("Size of the recursive shapes, lower numbers are larger"))
    value_range (0.005, 0.060)


property_int (amount, _("Internal clusters/amount of recursive shapes"), 12)
    description (_("The amount of recursive shapes"))
    value_range (3, 20)
    ui_meta ("unit", "degree")

property_double (sharpen,_("Sharpen"), 2.0)
    description (_("Sharpen the recursive shapes"))
    value_range (0.0, 5.0)
    ui_meta ("unit", "degree")

property_seed (seed, _("Random placement seed"), rand)
    description (_("Seed of the shapes, to move them around randomly"))

property_double (brightness,_("Soft Glow brightness"), 0.0)
    description (_("Increase the brightness and soft glow"))
    value_range (0.0, 0.250)
    ui_meta ("unit", "degree")

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     diamondscircles
#define GEGL_OP_C_SOURCE diamondscircles.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *replace;
 GeglNode *crop;
 GeglNode *sharpen;
 GeglNode *diamond;
 GeglNode *circle;
 GeglNode *kmeans;
 GeglNode *glowstick;
 GeglNode *output;
}State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);
  GeglColor *glowcolor = gegl_color_new ("#00b4ff");

  State *state = o->user_data = g_malloc0 (sizeof (State));

/*new child node list is here, this is where starred nodes get defined

 state->newchildname = gegl_node_new_child (gegl, "operation", "lb:name", NULL);*/

state->input    = gegl_node_get_input_proxy (gegl, "input");

state->glowstick = gegl_node_new_child (gegl, "operation", "lb:glowstick", "color", glowcolor, "noisereduction", 1, "sat", 11.1, "lightness", -1.3, 
                                                 "glow-radius", 73.0, "strength", 31.0,  "softness", 52.4, "radius", 41.7,  NULL);

state->crop = gegl_node_new_child (gegl, "operation", "gegl:crop", NULL);

/*shape property determines rather the filter is a diamond or a circle.  diamond is 1.0, circle is 2.0*/
state->diamond = gegl_node_new_child (gegl, "operation", "gegl:cell-noise", "scale", 0.020, "shape", 1.0, "rank", 1,  "iterations", 1,  NULL);

state->circle = gegl_node_new_child (gegl, "operation", "gegl:cell-noise", "scale", 0.020, "shape", 2.0, "rank", 1,  "iterations", 1,  NULL);

state->replace = gegl_node_new_child (gegl, "operation", "gegl:src", NULL);

state->sharpen = gegl_node_new_child (gegl, "operation", "gegl:unsharp-mask", "scale", 2.0, NULL);

state->kmeans = gegl_node_new_child (gegl, "operation", "port:segment-kmeans",  "n-clusters", 12, "max-iterations", 38,  NULL);

state->output   = gegl_node_get_output_proxy (gegl, "output");

/*meta redirect property to new child orders go here
 * 
 gegl_operation_meta_redirect (operation, "propertyname", state->newchildname,  "originalpropertyname");
*/

/*diamond and circle are different cell noises with the same parameters*/
 gegl_operation_meta_redirect (operation, "size", state->diamond,  "scale");
 gegl_operation_meta_redirect (operation, "size", state->circle,  "scale");
 gegl_operation_meta_redirect (operation, "seed", state->circle,  "seed");
 gegl_operation_meta_redirect (operation, "seed", state->diamond,  "seed");

 gegl_operation_meta_redirect (operation, "amount", state->kmeans,  "n-clusters");
 gegl_operation_meta_redirect (operation, "sharpen", state->sharpen,  "scale");
 gegl_operation_meta_redirect (operation, "color", state->glowstick,  "color");
 gegl_operation_meta_redirect (operation, "brightness", state->glowstick,  "brightness");

}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;

  GeglNode *cellnoise = state->diamond; /* the default */
  switch (o->shape) {
    case GEGL_DIAMOND: cellnoise = state->diamond; break;
    case GEGL_CIRCLE: cellnoise = state->circle; break;
   }

  gegl_node_link_many (state->input, state->replace, state->crop, state->kmeans,  state->glowstick, state->sharpen, state->output,  NULL);
  gegl_node_connect (state->replace, "aux", cellnoise, "output"); 
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
    "name",        "lb:recursive-dc",
    "title",       _("Recursive diamonds/circles"),
    "reference-hash", "hardworkingredhatengineer",
    "description", _("Render a background of recursive diamonds or circles"),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Render/Fun/",
    "gimp:menu-label", _("Recursive Diamonds/Circles background..."),
    NULL);
}

#endif
