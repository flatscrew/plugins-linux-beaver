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
 * 2024, beaver, Saber


id=1

cubism tile-size=13 seed=33
lb:neon-border stroke=3 stroke2=2 opacityglow=0
over aux=[ ref=1 lb:neon-border stroke=1 stroke2=1 opacityglow=0 colorneon=#a1ff00  ]
opacity value=1.3
hue-chroma hue=0


]

end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

/*Properties go here*/



property_boolean (policy, _("Merge with image"), TRUE)
  description    (_("Should Saber merge with the image content or be alone?"))


property_double (electricity, _("Electricity effect size"), 13.0)
    description (_("Size of the electricity effect caused by an internal cubism"))
    value_range (4.0, 20.0)
    ui_meta     ("unit", "pixel-distance")

property_seed (seed, _("Random seed"), rand)

property_double (outline, _("Outline grow radius"), 2.0)
  value_range   (1, 5.0)
  ui_range      (1, 5.0)
  ui_digits     (0)
  ui_steps      (1, 5)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")
  description (_("The size of the outline"))


property_double (electrical1, _("Electrical current 1"), 3.0)
  value_range   (1, 10.0)
  ui_range      (1, 10.0)
  ui_digits     (0)
  ui_steps      (1, 5)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")
  description (_("The size of the eletrical current"))

property_double (electrical2, _("Electrical current 2"), 2.0)
  value_range   (1, 8.0)
  ui_range      (1, 8.0)
  ui_digits     (0)
  ui_steps      (1, 5)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")
  description (_("The size of the electrical current"))

property_double (hue, _("Color rotation"), 0.0)
  value_range   (-180.0, 180.0)
  ui_range      (-180.0, 180.0)
  ui_digits     (0)
  ui_steps      (1, 5)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")
  description (_("Hue rotation"))

property_boolean (offcanvasclip, _("Off canvas clipping"), TRUE)
  description    (_("A trade off between two technical settings that actually modifies Saber. Default TRUE is best for text styles to prevent the glowing border outlines from clipping, but the consequence is canvas bordering content will always clip with a glowing line.  When disabled FALSE Saber doesn't clip at the end of the canvas so opaque pixels touching the canvas canvas don't have a glowing line, but text styles will clip if the outline borders are large, thus making the text style bad. In example, when TRUE said glowing line clip will present itself on a transparent covered image of a human that cuts off at the ankles, because the ankles are the canvas edge. Said image of person's ankles will clip if TRUE, and not clip of FALSE. This exist because there is no perfect GEGL graph only trade offs. Put simply, consider making FALSE for human legs or any content touching the canvas, for text styles on canvas keep TRUE"))

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     saber
#define GEGL_OP_C_SOURCE saber.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *neonborder;
 GeglNode *neonborder2;
 GeglNode *altneonborder;
 GeglNode *altneonborder2;
 GeglNode *opacity;
 GeglNode *hue;
 GeglNode *cubism;
 GeglNode *spread;
 GeglNode *replace;
 GeglNode *normal;
 GeglNode *oilify;
 GeglNode *over;
 GeglNode *output;
}State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);
/*
  GeglColor *colornodename = gegl_color_new ("#00ff36");
  GeglColor *white = gegl_color_new ("#ffffff");
*/
  State *state = o->user_data = g_malloc0 (sizeof (State));

/*new child node list is here, this is where starred nodes get defined

 state->newchildname = gegl_node_new_child (gegl, "operation", "lb:name", NULL);*/
  state->input    = gegl_node_get_input_proxy (gegl, "input");
  state->output   = gegl_node_get_output_proxy (gegl, "output");


 state->neonborder = gegl_node_new_child (gegl, "operation", "lb:neon-border", "opacityglow", 0.0,   "type", 1,  NULL);
 state->neonborder2 = gegl_node_new_child (gegl, "operation", "lb:neon-border", "opacityglow", 0.0,  "stroke", 2.0, "stroke2", 2.0, "type", 1,  "blurstroke", 1.0, "blurstroke2", 2.0, NULL);
 state->altneonborder = gegl_node_new_child (gegl, "operation", "lb:neon-border", "opacityglow", 0.0, "clipbugpolicy", FALSE,  "type", 0, "offcanvasclip", FALSE,  NULL);
 state->altneonborder2 = gegl_node_new_child (gegl, "operation", "lb:neon-border", "opacityglow", 0.0,  "stroke", 2.0, "stroke2", 2.0, "clipbugpolicy", FALSE,  "blurstroke", 1.0, "blurstroke2", 2.0, "type", 0, "offcanvasclip", FALSE, NULL);
 state->opacity = gegl_node_new_child (gegl, "operation", "gegl:opacity", "value", 1.3,  NULL);
 state->hue = gegl_node_new_child (gegl, "operation", "gegl:hue-chroma",  NULL);
 state->over = gegl_node_new_child (gegl, "operation", "gegl:over",  NULL);
 state->cubism = gegl_node_new_child (gegl, "operation", "gegl:cubism", NULL);
 state->spread = gegl_node_new_child (gegl, "operation", "gegl:noise-spread", NULL);
 state->replace = gegl_node_new_child (gegl, "operation", "gegl:src",   NULL);
 state->normal = gegl_node_new_child (gegl, "operation", "gegl:over",    NULL);
 state->oilify = gegl_node_new_child (gegl, "operation", "gegl:oilify", "mask-radius", 4,    NULL);
/*meta redirect property to new child orders go here

 gegl_operation_meta_redirect (operation, "propertyname", state->newchildname,  "originalpropertyname");
*/

 gegl_operation_meta_redirect (operation, "electrical1", state->altneonborder,  "stroke");
 gegl_operation_meta_redirect (operation, "electrical2", state->altneonborder,  "stroke2");
 gegl_operation_meta_redirect (operation, "electrical1", state->neonborder,  "stroke");
 gegl_operation_meta_redirect (operation, "electrical2", state->neonborder,  "stroke2");
 gegl_operation_meta_redirect (operation, "electricity", state->cubism,  "tile-size");
 gegl_operation_meta_redirect (operation, "seed", state->cubism,  "seed");
 gegl_operation_meta_redirect (operation, "hue", state->hue,  "hue");
 gegl_operation_meta_redirect (operation, "outline", state->neonborder2,  "stroke");
 gegl_operation_meta_redirect (operation, "outline", state->neonborder2,  "stroke2");
 gegl_operation_meta_redirect (operation, "outline", state->altneonborder2,  "stroke");
 gegl_operation_meta_redirect (operation, "outline", state->altneonborder2,  "stroke2");
}



static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;
  GeglNode *saberpolicy;
  GeglNode *neonborder;
  GeglNode *neonborder2;

   if (!o->policy) saberpolicy = state->replace;
   if (o->policy) saberpolicy = state->normal;
   if (!o->offcanvasclip) neonborder = state->altneonborder;
   if (!o->offcanvasclip) neonborder2 = state->altneonborder2;
   if (o->offcanvasclip) neonborder = state->neonborder;
   if (o->offcanvasclip) neonborder2 = state->neonborder2;

  gegl_node_link_many (state->input, saberpolicy, state->output,  NULL);
  gegl_node_link_many (state->input, state->cubism, state->oilify, neonborder, state->over, state->opacity, state->hue,  NULL);
  gegl_node_link_many (state->input, state->spread, neonborder2,  NULL);
  gegl_node_connect (state->over, "aux", neonborder2, "output");
  gegl_node_connect (saberpolicy, "aux", state->hue, "output");

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
    "name",        "lb:saber",
    "title",       _("Saber"),
    "reference-hash", "christmas2024punchout",
    "description", _("An electrical neon border effect combined "),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Text Styling",
    "gimp:menu-label", _("Saber..."),
    NULL);
}

#endif
