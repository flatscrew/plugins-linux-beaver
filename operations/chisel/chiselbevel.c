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
 * 2025, beaver, Chisel and Inner Bevel

I was surprised to learn that a chisel and a inner bevel are the same theory, I made this plugin intending to do a inner bevel something I theorized for years
but accidentially discovered that when tweaking its parameters and using the normal and behind blend mode at low opacity it becomes a quality chisel  


dst-over can be  over or src

id=1 dst-over aux=[ ref=1
gaussian-blur std-dev-x=3 std-dev-y=3 abyss-policy=none clip-extent=none
median-blur radius=10 alpha-percentile=100 abyss-policy=none
inner-glow radius=20 grow-radius=10 value=#808080
bevel type=chamfer blendmode=gimpblend
opacity value=1.0
 ]


end of syntax
 */


#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

enum_start (kbevel_type623465xj9)
  enum_value (CHISEL_USE_REPLACE,     "replace",
              N_("Replace (to blend)"))
  enum_value (CHISEL_USE_NORMAL,     "normal",
              N_("Normal"))
  enum_value (CHISEL_USE_BEHIND,  "behind",
              N_("Behind"))
  enum_value (INNERBEVEL_USE_REPLACE,  "replace-alpha-lock",
              N_("Replace (Inner Bevel to blend)"))
  enum_value (INNERBEVEL_USE_NORMAL,  "normal-alpha-lock",
              N_("Normal (Inner Bevel)"))
enum_end (kbeveltype623465xj9)


property_enum (position, _("Chisel Position/Filter Choice"),
    kbeveltype623465xj9, kbevel_type623465xj9,
    CHISEL_USE_NORMAL)
    description (_("The position of the chisel, on replace it is expected to be manually blended. This also includes a bonus Innel Bevel filter that has similar logic to chisel"))


property_enum (type, _("Bevel Type"),
    gbeveltype623465, gbevel_type623465,
    GEGL_BEVEL_CHAMFER)
    description (_("The type of bevel to use"))

enum_start (gbevel_type623465)
  enum_value (GEGL_BEVEL_CHAMFER,  "chamfer",
              N_("Sharp Chamfer"))
  enum_value (GEGL_BEVEL_BUMP,     "bump",
              N_("Bump"))
enum_end (gbeveltype623465)


/*
enum_start (gchamfer_blend_mode2231)
  enum_value (CHAMFER_BLEND_GIMPBLEND, "gimpblend",
              N_("None (for use with GIMPs blending options)"))
  enum_value (CHAMFER_BLEND_HARDLIGHT, "hardlight",
              N_("Hard Light"))
  enum_value (CHAMFER_BLEND_MULTIPLY,  "multiply",
              N_("Multiply"))
  enum_value (CHAMFER_BLEND_COLORDODGE,  "colordodge",
              N_("Color Dodge"))
  enum_value (CHAMFER_BLEND_DARKEN,    "darken",
              N_("Darken"))
  enum_value (CHAMFER_BLEND_LIGHTEN,   "lighten",
              N_("Lighten"))
enum_end (gChamferBlendMode2231)

property_enum (blendmode, _("Blend Mode"),
    gChamferBlendMode2231, gchamfer_blend_mode2231,
    CHAMFER_BLEND_GIMPBLEND)
  description (_("What blending mode the bevel's emboss will be. Light Map is a special blend mode that allows users to extract the filters output as a light map which should be put on a layer above or be used with Gimp's blending options"))
ui_meta ("visible", "!type {easy_chisel}" )
*/

property_double (blur_below_chisel, _("Blur below chisel"), 3.0)
    description (_("A blur at the beginning before the chisel starts. A general rule is if the text is larger make this higher, if the text is smaller make this smaller"))
    value_range (0.0, 4.0)
    ui_range (0.0, 3.0)
  ui_steps      (1.0, 3.0)

property_double (blur_radius, _("Chisel's soft radius"), 8.5)
  value_range   (5.0, 25.0)
  ui_range      (5.0, 10.0)
  ui_steps      (1.0, 5.0)
  ui_meta       ("unit", "pixel-distance")
  description (_("An internal inner glow blur radius that makes that usually makes the chisel go from rough to smooth"))

property_double (grow_radius, _("Chisel's grow radius"), 5.0)
  value_range   (1, 30.0)
  ui_range      (1, 15.0)
  ui_digits     (0)
  ui_steps      (1, 5)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")
  description (_("The distance to expand the grow the chisel bevel internally"))

property_enum (metric, _("Distance map setting"),
               GeglDistanceMetric, gegl_distance_metric, GEGL_DISTANCE_METRIC_EUCLIDEAN)
    description (_("Distance Map is unique to sharp chamfer bevel and in this case a chisel, it has three settings that alter the structure of the chamfer."))
ui_meta ("visible", "!type {bump}" )

property_double (radius, _("Radius"), 3.0)
    description (_("Radius of softening for a chisel bump bevel"))
  value_range (1.0, 5.0)
  ui_range (1.0, 3.5)
  ui_gamma (1.5)
ui_meta ("visible", "!type {chamfer}" )
  ui_steps      (0.01, 0.50)

property_double (azimuth, _("Light Angle"), 360.0)
    description (_("Direction of a light source illuminating and shading the chisel bevel."))
    value_range (0, 360)
  ui_steps      (0.01, 0.50)
    ui_meta ("unit", "degree")
    ui_meta ("direction", "ccw")

property_int (outward, _("Expand chisel outline outward"), 5)
    description (_("Expand the entire chisel bevel as if it were an outline going beyond its default constraints"))
    value_range (0, 20)
    ui_range (0, 20)

property_double (light, _("Light adjustment"), 0.0)
    description (_("Lightness adjustment"))
    value_range (-20.0, 20.0)
    ui_range (-20.0, 20.0)
  ui_steps      (0.01, 1.0)

property_double (opacity, _("Opacity"), 0.88)
    description (_("Opacity of the chisel bevel"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     lb_chiselbevel
#define GEGL_OP_C_SOURCE chiselbevel.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *bevel;
 GeglNode *innerglow;
 GeglNode *gaussian;
 GeglNode *grow;
 GeglNode *grow2;
 GeglNode *normal;
 GeglNode *multiply;
 GeglNode *lightbevel;
 GeglNode *light;
 GeglNode *opacity;
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
 state->bevel = gegl_node_new_child (gegl, "operation", "gegl:bevel", "blendmode", 0, "elevation", 180.0, "depth", 100, NULL);
  GeglColor *gray = gegl_color_new ("#808080");
 state->innerglow = gegl_node_new_child (gegl, "operation", "gegl:inner-glow", "value", gray,  NULL);
 state->gaussian = gegl_node_new_child (gegl, "operation", "gegl:gaussian-blur", "std-dev-x", 1.0, "std-dev-y", 1.0, "abyss-policy", 0, "clip-extent", 0,  NULL);
 state->grow = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "alpha-percentile", 100.0, "percentile", 100.0, "abyss-policy", 0,   NULL);
 state->normal = gegl_node_new_child (gegl, "operation", "gegl:over",  NULL);
 state->opacity = gegl_node_new_child (gegl, "operation", "gegl:opacity",  NULL);
 state->light = gegl_node_new_child (gegl, "operation", "gegl:hue-chroma",  NULL);
/* state->multiply = gegl_node_new_child (gegl, "operation", "gegl:multiply",  NULL);
   state->lightbevel = gegl_node_new_child (gegl, "operation", "gegl:add", "value", 0.1, NULL);
 state->grow2 = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "radius", 100, "alpha-percentile", 100.0, "percentile", 50.0, "abyss-policy", 0,   NULL); */

  gegl_operation_meta_redirect (operation, "blur_below_chisel", state->gaussian, "std-dev-y");
  gegl_operation_meta_redirect (operation, "blur_below_chisel", state->gaussian, "std-dev-x");
  gegl_operation_meta_redirect (operation, "blur-radius", state->innerglow, "radius");
  gegl_operation_meta_redirect (operation, "grow-radius", state->innerglow, "grow-radius");
  gegl_operation_meta_redirect (operation, "radius", state->bevel, "radius");
  gegl_operation_meta_redirect (operation, "azimuth", state->bevel, "azimuth");
  gegl_operation_meta_redirect (operation, "metric", state->bevel, "metric");
  gegl_operation_meta_redirect (operation, "type", state->bevel, "type"); 
/*  gegl_operation_meta_redirect (operation, "blendmode", state->bevel, "blendmode"); */
  gegl_operation_meta_redirect (operation, "outward", state->grow, "radius");
  gegl_operation_meta_redirect (operation, "opacity", state->opacity, "value");
  gegl_operation_meta_redirect (operation, "light", state->light, "lightness");
}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;

  const char *placechisel = "gegl:over";
  switch (o->position) {
    case CHISEL_USE_REPLACE:  placechisel = "gegl:src"; break;
    case CHISEL_USE_NORMAL:  placechisel = "gegl:over"; break;
    case CHISEL_USE_BEHIND:  placechisel = "gegl:dst-over"; break;
    case INNERBEVEL_USE_REPLACE:  placechisel = "gegl:src-in"; break;
    case INNERBEVEL_USE_NORMAL:  placechisel = "gegl:src-atop"; break;
  }

  gegl_node_set (state->normal, "operation", placechisel, NULL);


  gegl_node_link_many (state->input, state->normal, state->output,  NULL);
  gegl_node_link_many (state->input, state->grow, state->gaussian,  state->innerglow, state->bevel, state->light, state->opacity, NULL);
  gegl_node_connect (state->normal, "aux", state->opacity, "output");


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
    "name",        "lb:chisel",
    "title",       _("Chisel"),
    "reference-hash", "luminautgrassroomsambient",
    "description", _("The term chisel refers to a bevel that extends outside the shape. The goal of a chisel is to blend the shape with outside content. On replace it is expected to be manually blended. Filter also comes with a bonus Inner Bevel ability due to it being logically very similar (but visually distinct)"),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/Text Styling/",
    "gimp:menu-label", _("Chisel..."),
    NULL);
}

#endif
