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
 * 2024 Beaver, Noise in Motion

Recreate graph using this GEGL syntax

id=x
over aux=[ ref=x
color value=#59cbf3
noise-rgb red=1 green=1 blue=1  gaussian=true linear=true seed=223334
gray 
]
crop
motion-blur length=70
unsharp-mask scale=5
opacity value=10
median-blur radius=0 abyss-policy=none clip-extent
multiply aux=[ color value=#e98944]
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

enum_start (motion_select)
  enum_value (MOTION1, "linear",
              N_("Linear Motion Blur"))
  enum_value (MOTION2,      "zoom",
              N_("Zoom Motion Blur"))
  enum_value (MOTION3,      "circular",
              N_("Cicular Motion Blur"))
enum_end (motionselect)

property_enum (motiontype, _("Internal Motion Type"),
    motionselect, motion_select,
    MOTION1)
    description (_("The internal motion to choose that controls the structure of the noise"))

property_color (color, _("Color"), "#e98944")
    description (_("The color to paint over the noise. The default color makes noise"))

property_double (anglecircular, _("Angle"), 15.0)
    description (_("Rotation blur angle of the circular motion blur. A large angle may take some time to render"))
    value_range (10.0, 75.0)
    ui_meta     ("unit", "degree")
ui_meta ("visible", "!motiontype {zoom, linear}" )

property_double (direction, _("Direction"), 0.0)
    description (_("Direction of noise"))
    value_range (-180, 180)
    ui_meta     ("unit", "degree")
    ui_meta     ("direction", "cw")
ui_meta ("visible", "!motiontype {zoom, circular}" )

property_double (center_x, _("Center X"), 0.5)
    value_range (-10.0, 10.0)
    ui_range    (0.0, 1.0)
    ui_meta     ("unit", "relative-coordinate")
    ui_meta     ("axis", "x")
ui_meta ("visible", "!motiontype {linear}" )

property_double (center_y, _("Center Y"), 0.5)
    value_range (-10.0, 10.0)
    ui_range    (0.0, 1.0)
    ui_meta     ("unit", "relative-coordinate")
    ui_meta     ("axis", "y")

property_double (blurzoom, _("Blurring factor"), 0.1)
    value_range (-10, 1.0)
    ui_range    (-0.5, 1.0)
    ui_gamma    (2.0)
ui_meta ("visible", "!motiontype {linear, circular}" )

property_double (length, _("Length"), 70.0)
    description (_("Length of noise in pixels"))
    value_range (20.0, 120.0)
    ui_range    (20.0, 120.0)
    ui_gamma    (1.5)
    ui_meta     ("unit", "pixel-distance")
ui_meta ("visible", "!motiontype {zoom, circular}" )

property_double (sharpen, _("Sharpen"), 4.5)
    description(_("Sharpen the noise "))
    value_range (2.0, 10.0)
    ui_range    (2.0, 10.0)
    ui_gamma    (3.0)

property_seed    (seed, _("Random seed"), rand)
    description  (_("The random seed for the noise function"))

property_double (desaturate, _("Desaturate"), 1.0)
    description(_("Desaturation option that makes a brushed metal effect easier"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)

property_double (edge, _("Add light around dark areas"), 0.0)
    description(_("Make a shiny outline around dark areas using edge detect"))
    value_range (0.0, 0.11)
    ui_range (0.0, 0.10)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     brushed_metal_wood
#define GEGL_OP_C_SOURCE brushed_metal_wood.c

#include "gegl-op.h"

typedef struct
{
  GeglNode *input;
  GeglNode *normal;
  GeglNode *idref;
  GeglNode *hcolor;
  GeglNode *noise;
  GeglNode *gray;
  GeglNode *crop;
  GeglNode *motion1;
  GeglNode *motion2;
  GeglNode *motion3;
  GeglNode *sharpen;
  GeglNode *opacity;
  GeglNode *endfix;
  GeglNode *multiply;
  GeglNode *color;
  GeglNode *desaturate;
  GeglNode *edge;
  GeglNode *normal2;
  GeglNode *idref2;
  GeglNode *opacity2;
  GeglNode *endgraph;
  GeglNode *output;
}State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);

  State *state = o->user_data = g_malloc0 (sizeof (State));

  GeglColor *defaultcolor = gegl_color_new ("#e98944");

    state->input    = gegl_node_get_input_proxy (gegl, "input");
    state->output   = gegl_node_get_output_proxy (gegl, "output");

    state->normal    = gegl_node_new_child (gegl,
                                  "operation", "gegl:over",
                                  NULL);
    state->idref    = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);
#define HIDDENCOLOR \
" color value=#59cbf3  "\

    state->hcolor    = gegl_node_new_child (gegl,
                                  "operation", "gegl:gegl", "string", HIDDENCOLOR,
                                  NULL);
     state->noise   = gegl_node_new_child (gegl,
                                  "operation", "gegl:noise-rgb", "red", 1.0, "green", 1.0, "blue", 1.0, "gaussian", TRUE, "linear", TRUE, 
                                  NULL);
     state->gray    = gegl_node_new_child (gegl,
                                  "operation", "gegl:gray",
                                  NULL);
     state->crop    = gegl_node_new_child (gegl,
                                  "operation", "gegl:crop",
                                  NULL);
     state->motion1    = gegl_node_new_child (gegl,
                                  "operation", "gegl:motion-blur-linear",
                                  NULL);
     state->motion2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:motion-blur-zoom",
                                  NULL);
     state->motion3    = gegl_node_new_child (gegl,
                                  "operation", "gegl:motion-blur-circular",
                                  NULL);
     state->sharpen    = gegl_node_new_child (gegl,
                                  "operation", "gegl:unsharp-mask",
                                  NULL);
     state->opacity    = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity", "value", 10.0,
                                  NULL);
     state->endfix    = gegl_node_new_child (gegl,
                                  "operation", "gegl:median-blur", "radius", 0, "abyss-policy", 0,
                                  NULL);
     state->multiply    = gegl_node_new_child (gegl,
                                  "operation", "gegl:multiply",
                                  NULL);
     state->color    = gegl_node_new_child (gegl,
                                  "operation", "gegl:color", "value", defaultcolor,
                                  NULL);
     state->desaturate    = gegl_node_new_child (gegl,
                                  "operation", "gegl:saturation", 
                                  NULL);
     state->edge    = gegl_node_new_child (gegl,
                                  "operation", "gegl:edge", "amount", 2.0,
                                  NULL);
    state->normal2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:over",
                                  NULL);
    state->idref2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);
    state->opacity2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity",
                                  NULL);
#define END \
" opacity value=10 median-blur radius=0 abyss-policy=none  "\

    state->endgraph    = gegl_node_new_child (gegl,
                                  "operation", "gegl:gegl", "string", END,
                                  NULL);
gegl_operation_meta_redirect (operation, "seed", state->noise, "seed"); 
gegl_operation_meta_redirect (operation, "sharpen", state->sharpen, "scale"); 
gegl_operation_meta_redirect (operation, "length", state->motion1, "length");
gegl_operation_meta_redirect (operation, "direction", state->motion1, "angle");  
gegl_operation_meta_redirect (operation, "color", state->color, "value");  
gegl_operation_meta_redirect (operation, "desaturate", state->desaturate, "scale");  
gegl_operation_meta_redirect (operation, "edge", state->opacity2, "value");  
gegl_operation_meta_redirect (operation, "center_x", state->motion2, "center_x");  
gegl_operation_meta_redirect (operation, "center_y", state->motion2, "center_y");  
gegl_operation_meta_redirect (operation, "center_x", state->motion3, "center_x");  
gegl_operation_meta_redirect (operation, "center_y", state->motion3, "center_y");  
gegl_operation_meta_redirect (operation, "anglecircular", state->motion3, "angle");  
gegl_operation_meta_redirect (operation, "blurzoom", state->motion2, "factor");  

}

static void update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;    

 /*Pick between linear motion blur, zoom motion blur and circular motion blur (1 2 and 3)*/
  GeglNode *motion = state->motion1; 
  switch (o->motiontype) {
    case MOTION1: motion = state->motion1; break;
    case MOTION2: motion = state->motion2; break;
    case MOTION3: motion = state->motion3; break;
default: motion = state->motion1;

}
 /*main graph here. normal and multiply are composers (aka blend modes)*/
  gegl_node_link_many (state->input, state->hcolor, state->idref, state->normal, state->crop, motion, state->sharpen, state->opacity, state->endfix, state->multiply, state->desaturate, state->idref2, state->normal2, state->endgraph, state->output, NULL);
 /*the normal blend mode has everything from idref to gray inside it*/
  gegl_node_connect (state->normal, "aux", state->gray, "output");
  gegl_node_link_many (state->idref,  state->noise, state->gray,  NULL);
 /*the multiply blend mode has a color node inside of it. obviously to blend the color*/
  gegl_node_connect (state->multiply, "aux", state->color, "output");
 /*content inside normal 2*/
  gegl_node_link_many (state->idref2, state->edge, state->opacity2,  NULL);
 /*connecting normal2 to opacity*/
  gegl_node_connect (state->normal2, "aux", state->opacity2, "output");
  gegl_node_connect (state->crop, "aux", state->input, "output");
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
    "name",        "lb:motion-noise",
    "title",       _("Noise in Motion"),
    "reference-hash", "dancingthroughthenightonthegroove",
    "description", _("RGB Noise optimally manipulated by motion blur filters to create effects like brushed metal and wood."),
    "gimp:menu-path", "<Image>/Filters/Noise",
    "gimp:menu-label", _("Noise in Motion..."),
    NULL);
}

#endif
