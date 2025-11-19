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
 * 2025, beaver, Edge Bevel

Recreate this plugin with GEGL syntax alone

id=0
color-overlay value=#b43d00
id=1
src-atop aux=[ ref=1 emboss depth=100 elevation=1 azimuth=3 
difference-of-gaussians radius2=1
opacity value=0.24
]
emboss depth=100 elevation=2 azimuth=30 
difference-of-gaussians radius2=20
invert-gamma 

shadows-highlights  highlights=65 whitepoint=30
id=2 over aux=[ ref=2 edge-sobel  horizontal=true vertical=true opacity value=0.5 ]
multiply aux=[  ref=1  ]
gray 
hue-chroma lightness=1
multiply aux=[  ref=0 ]
median-blur radius=0 abyss-policy=none


end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

/*Properties go here*/


property_boolean (edgebevelcolorpolicy , _("Enable color priority mode"), FALSE)
  description    (_("This (when enabled) disables the bevel's ability to get the layers color information. But in return the color overlay will be enabled. Due to the logic of this filter very dark colors or black will not work with it."))

property_color (color, _("Color"), "#ffffff")
    description (_("The color to overlay (defaults to 'white')"))
    ui_meta     ("role", "color-primary")
  ui_meta     ("sensitive", " edgebevelcolorpolicy")


property_double (radius, _("Radius of differenced gaussian"), 94)
    description (_("Intensity of edge effect"))
    value_range (30, 100)
    ui_meta ("unit", "degree")

property_double (azimuth, _("Azimuth"), 0.0)
    description (_("Light angle (degrees)"))
    value_range (0, 360)
    ui_meta ("unit", "degree")
    ui_meta ("direction", "ccw")
  ui_steps      (0.5, 0.50)

property_double (elevation, _("Elevation"), 45.0)
    description (_("Elevation angle (degrees)"))
    value_range (0, 45)
    ui_meta ("unit", "degree")

property_int (depth, _("Depth"), 100)
    description (_("Depth of the bevel"))
    value_range (1, 100)
    ui_meta ("unit", "degree")

property_double (intensity, _("Edge intensity"), 1)
    description (_("Intensity of edge effect"))
    value_range (0, 1)
    ui_meta ("unit", "degree")

property_double (shine, _("Shine light "), 40)
    description (_("Increase light of shine effect"))
    value_range (35, 65)
    ui_meta ("unit", "degree")

property_int (smooth, _("Smooth"), 0)
    description (_("Smooth rough areas of the bevel"))
    value_range (0, 20)
    ui_meta ("unit", "degree")


#else

#define GEGL_OP_META
#define GEGL_OP_NAME     edgebevel
#define GEGL_OP_C_SOURCE edgebevel.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *smooth;
 GeglNode *crop;
 GeglNode *color;
 GeglNode *color2;
 GeglNode *idref1;
 GeglNode *idref2;
 GeglNode *idref3;
 GeglNode *srcatop;
 GeglNode *emboss1;
 GeglNode *emboss2;
 GeglNode *dog1;
 GeglNode *dog2;
 GeglNode *microopacity;
 GeglNode *opacity;
 GeglNode *invert;
 GeglNode *sh; 
 GeglNode *normal; 
 GeglNode *edge;
 GeglNode *multiply1;
 GeglNode *gray;
 GeglNode *lightfix;     
 GeglNode *multiply2;
 GeglNode *opacity90;      
 GeglNode *fix;   

 GeglNode *output;
}State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);
  GeglColor *colorhere = gegl_color_new ("#b43d00");
  GeglColor *colorhere2 = gegl_color_new ("#ffffff");
  State *state = o->user_data = g_malloc0 (sizeof (State));

/*new child node list is here, this is where starred nodes get defined

 state->newchildname = gegl_node_new_child (gegl, "operation", "lb:name", NULL);*/
 state->input    = gegl_node_get_input_proxy (gegl, "input");
 state->fix = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "radius", 0, "abyss-policy", 0,  NULL);
 state->color = gegl_node_new_child (gegl, "operation", "gegl:color-overlay", "value", colorhere, NULL);
 state->color2 = gegl_node_new_child (gegl, "operation", "gegl:color", "value", colorhere2, NULL);
 state->idref1 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->idref2 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->idref3 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->smooth = gegl_node_new_child (gegl, "operation", "gegl:mean-curvature-blur", NULL);
 state->multiply1 = gegl_node_new_child (gegl, "operation", "gegl:multiply", NULL);
 state->multiply2 = gegl_node_new_child (gegl, "operation", "gegl:multiply", NULL);
 state->gray = gegl_node_new_child (gegl, "operation", "gegl:gray", NULL);
 state->crop = gegl_node_new_child (gegl, "operation", "gegl:crop", NULL);
 state->lightfix = gegl_node_new_child (gegl, "operation", "gegl:hue-chroma", "lightness", 1.0, NULL);
 state->microopacity = gegl_node_new_child (gegl, "operation", "gegl:opacity", "value", 0.2, NULL);
 state->opacity = gegl_node_new_child (gegl, "operation", "gegl:opacity", "value", 0.5, NULL);
 state->normal = gegl_node_new_child (gegl, "operation", "gegl:over", NULL);
 state->sh = gegl_node_new_child (gegl, "operation", "gegl:shadows-highlights", "highlights", 65.0,  NULL);
 state->edge = gegl_node_new_child (gegl, "operation", "gegl:edge-sobel", "horizontal", TRUE, "vertical", FALSE,  NULL);
 state->invert = gegl_node_new_child (gegl, "operation", "gegl:invert-gamma", NULL);
 state->srcatop = gegl_node_new_child (gegl, "operation", "gegl:src-atop", NULL);
 state->opacity90 = gegl_node_new_child (gegl, "operation", "gegl:opacity", "value", 0.10, NULL);
 state->emboss1 = gegl_node_new_child (gegl, "operation", "gegl:emboss", "depth", 100, "elevation", 1.0, "azimuth", 3.0,  NULL);
 state->emboss2 = gegl_node_new_child (gegl, "operation", "gegl:emboss", "depth", 100, "elevation", 2.0, "azimuth", 30.0,  NULL);
 state->dog1 = gegl_node_new_child (gegl, "operation", "gegl:difference-of-gaussians", "radius2", 1.0,  NULL);
 state->dog2 = gegl_node_new_child (gegl, "operation", "gegl:difference-of-gaussians", "radius2", 20.0,  NULL);
 state->output   = gegl_node_get_output_proxy (gegl, "output");

/*meta redirect property to new child orders go here

 gegl_operation_meta_redirect (operation, "propertyname", state->newchildname,  "originalpropertyname");
*/

 gegl_operation_meta_redirect (operation, "radius", state->dog1,  "radius2");
 gegl_operation_meta_redirect (operation, "intensity", state->microopacity,  "value");
 gegl_operation_meta_redirect (operation, "azimuth", state->emboss2,  "azimuth");
 gegl_operation_meta_redirect (operation, "elevation", state->emboss1,  "elevation");
 gegl_operation_meta_redirect (operation, "depth", state->emboss2,  "depth");
 gegl_operation_meta_redirect (operation, "shine", state->sh,  "highlights");
 gegl_operation_meta_redirect (operation, "color", state->color2,  "value");
 gegl_operation_meta_redirect (operation, "smooth", state->smooth,  "iterations");
}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;

  if (o->edgebevelcolorpolicy)
  {
  gegl_node_link_many (state->input, state->color, state->idref1, state->srcatop, state->emboss2, state->dog2, state->invert, state->sh, state->idref2, state->normal, state->multiply1, state->gray, state->lightfix, state->idref3, state->multiply2,  state->crop, state->smooth, state->fix, state->output,  NULL);
  gegl_node_connect (state->srcatop, "aux", state->microopacity, "output"); 
  gegl_node_link_many (state->idref1, state->emboss1, state->dog1, state->opacity90, state->microopacity,   NULL);
  gegl_node_connect (state->normal, "aux", state->opacity, "output"); 
  gegl_node_link_many (state->idref2, state->edge, state->opacity,   NULL);
  gegl_node_connect (state->multiply1, "aux", state->idref1, "output"); 
  gegl_node_connect (state->multiply2, "aux", state->color2, "output"); 
  gegl_node_connect (state->crop, "aux", state->idref3, "output"); 
  }

else

  {
  gegl_node_link_many (state->input, state->color, state->idref1, state->srcatop, state->emboss2, state->dog2, state->invert, state->sh, state->idref2, state->normal, state->multiply1, state->gray, state->lightfix, state->multiply2, state->smooth, state->fix, state->output,  NULL);
  gegl_node_connect (state->srcatop, "aux", state->microopacity, "output"); 
  gegl_node_link_many (state->idref1, state->emboss1, state->dog1, state->opacity90, state->microopacity,   NULL);
  gegl_node_connect (state->normal, "aux", state->opacity, "output"); 
  gegl_node_link_many (state->idref2, state->edge, state->opacity,   NULL);
  gegl_node_connect (state->multiply1, "aux", state->idref1, "output"); 
  gegl_node_connect (state->multiply2, "aux", state->input, "output"); 
  }
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
    "name",        "lb:edge-bevel",
    "title",       _("Edge Bevel"),
    "reference-hash", "geglprideworldwide",
    "description", _("This filter presents difference of gaussian's edge detect with other things to make a unique bevel effect. "),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Text Styling",
    "gimp:menu-label", _("Edge Bevel..."),
    NULL);
}

#endif
