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
 * Fish Scales GEGL plugin 
* 
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions
 * 2025 Grok solved a major bug with fishscales.c, with Beaver's
 * 
 * Test this plugin without installing by pasting this syntax into GIMP's GEGL graph filter
 * 
ai/lb:fish-scales-core 

dropshadow x=0 y=0 radius=0 grow-radius=5 color=#ffffff opacity=1

dst-over aux=[ color value=#00caff ]
 
end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_int(scale_type, _("Scale Type"), 7)
    description(_("Type of scale pattern to generate"))
    value_range(0, 7)
    ui_meta("enum-names", "Default|Rounded|Pointy|Flat|Irregular|Large|Tight|Asymmetrical")

property_color(scale_color, _("Scale Color"), "#4682b4")
    description(_("Main scale color"))

property_boolean(enable_gradient, _("Enable Gradient"), TRUE)
    description(_("Enable highlight/shadow gradient effects"))

property_boolean(enable_background, _("Enable Background"), TRUE)
    description(_("Enable background color"))

property_color(background_color, _("Background Color"), "#00567b")
    description(_("Background color (only used when enabled)"))
  ui_meta     ("sensitive", " enable_background")

property_double(scale_resize, _("Scale Size"), 100.0)
    description(_("Size of individual scales"))
    value_range(5.0, 250.0)
    ui_range(10.0, 250.0)

property_double(overlap, _("Vertical Overlap"), 0.3)
    description(_("How much scales overlap vertically"))
    value_range(0.1, 0.5)
    ui_range(0.1, 0.5)

property_double(horizontal_spacing, _("Horizontal Spacing"), 1.0)
    description(_("Horizontal scale spacing (lower = closer) Don't make this too low or it will be incoherrent. Due to a known bug I have to allow the user to reach values that ruin the effect."))
    value_range(0.0, 1.3)
    ui_range(0.0, 1.3)

property_double(rotation, _("Rotation"), 0.0)
    description(_("Pattern rotation in degrees"))
    value_range(0.0, 360.0) 
    ui_meta("unit", "degree")
    ui_meta("direction", "ccw")

property_boolean(enable_outline, _("Enable Outline"), FALSE)
    description(_("Enable outline"))

property_int(outline_grow_radius, _("Grow radius"), 10)
    value_range(1, 30)
    ui_range(1, 30)
    ui_steps(1, 5)
    ui_meta("unit", "pixel-distance")
    description(_("Outline Grow radius"))
  ui_meta     ("sensitive", " enable_outline")

property_color(outline_color, _("Outline color"), "#ffffff")
    description(_("The outline's color (defaults to 'white')"))
  ui_meta     ("sensitive", " enable_outline")
#else

#define GEGL_OP_META
#define GEGL_OP_NAME     fishscales
#define GEGL_OP_C_SOURCE fishscales.c

#include "gegl-op.h"

typedef struct
{
  GeglNode *input;
  GeglNode *fishscales;
  GeglNode *proxy;
  GeglNode *outline;
  GeglNode *behind;
  GeglNode *crop;
  GeglNode *nothing;
  GeglNode *idref;
  GeglNode *color;
  GeglNode *output;
  gint counter; /* To vary dummy property */
} State;

static void attach(GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES(operation);

  State *state = o->user_data = g_malloc0(sizeof(State));
  state->counter = 0;

  state->input = gegl_node_get_input_proxy(gegl, "input");
  state->output = gegl_node_get_output_proxy(gegl, "output");

  state->fishscales = gegl_node_new_child(gegl,
                                         "operation", "ai/lb:fish-scales-core",
                                         NULL);

  state->proxy = gegl_node_new_child(gegl,
                                     "operation", "gegl:nop",
                                     NULL);

  state->outline = gegl_node_new_child(gegl,
                                       "operation", "gegl:dropshadow",
                                       "x", 0.0, "y", 0.0, "opacity", 1.0, "radius", 0.0,
                                       NULL);

  state->behind = gegl_node_new_child(gegl,
                                      "operation", "gegl:dst-over",
                                      NULL);

  state->nothing = gegl_node_new_child(gegl,
                                      "operation", "gegl:nop",
                                      NULL);

  state->idref = gegl_node_new_child(gegl,
                                     "operation", "gegl:nop",
                                     NULL);

  state->crop = gegl_node_new_child(gegl,
                                    "operation", "gegl:crop",
                                    NULL);

  state->color = gegl_node_new_child(gegl,
                                     "operation", "gegl:color",
                                     NULL);

  gegl_operation_meta_redirect(operation, "background_color", state->color, "value");
  gegl_operation_meta_redirect(operation, "scale_type", state->fishscales, "scale_type");
  gegl_operation_meta_redirect(operation, "scale_color", state->fishscales, "scale_color");
  gegl_operation_meta_redirect(operation, "scale_resize", state->fishscales, "scale_size");
  gegl_operation_meta_redirect(operation, "enable_gradient", state->fishscales, "enable_gradient");
  gegl_operation_meta_redirect(operation, "overlap", state->fishscales, "overlap");
  gegl_operation_meta_redirect(operation, "horizontal_spacing", state->fishscales, "horizontal_spacing");
  gegl_operation_meta_redirect(operation, "rotation", state->fishscales, "rotation");
  gegl_operation_meta_redirect(operation, "outline_color", state->outline, "color");
  gegl_operation_meta_redirect(operation, "outline_grow_radius", state->outline, "grow_radius");
}

static void update_graph(GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  State *state = o->user_data;
  if (!state) return;

  /* Reset connections to ensure fresh graph */
  gegl_node_disconnect(state->behind, "aux");
  gegl_node_disconnect(state->crop, "aux");

  /* Increment counter for dummy property */
  state->counter++;

  /* Create a new dropshadow node if outline is enabled */
  if (o->enable_outline)
  {
    if (state->outline)
    {
      gegl_node_disconnect(state->outline, "input");
      state->outline = NULL;
    }
    state->outline = gegl_node_new_child(operation->node,
                                        "operation", "gegl:dropshadow",
                                        "x", 0.0, "y", 0.0, "opacity", 1.0, "radius", 0.0,
                                        NULL);
    gegl_operation_meta_redirect(operation, "outline_color", state->outline, "color");
    gegl_operation_meta_redirect(operation, "outline_grow_radius", state->outline, "grow_radius");
  }

  /* Select outline node */
  GeglNode *outline = o->enable_outline ? state->outline : state->nothing;

  /* Force reprocessing with a dummy property tweak */
  gdouble dummy_x = 0.0 + (state->counter % 2 ? 0.0001 : -0.0001);


  if (o->enable_background)
  {
    gegl_node_link_many(state->input, state->fishscales, state->proxy, outline, state->behind, state->idref, state->crop, state->output, NULL);
    gegl_node_connect(state->behind, "aux", state->color, "output");
    gegl_node_connect(state->crop, "aux", state->idref, "output");
  }
  else
  {
    gegl_node_link_many(state->input, state->fishscales, state->proxy, outline, state->output, NULL);
  }
}

static void gegl_op_class_init(GeglOpClass *klass)
{
  GeglOperationClass *operation_class;
  GeglOperationMetaClass *operation_meta_class = GEGL_OPERATION_META_CLASS(klass);
  operation_class = GEGL_OPERATION_CLASS(klass);

  operation_class->attach = attach;
  operation_meta_class->update = update_graph;

  gegl_operation_class_set_keys(operation_class,
    "name",        "ai/lb:fish-scales",
    "title",       _("Fish Scales"),
    "reference-hash", "fishscales2025",
    "description", _("Renders a grid of fish scale patterns with customizable outline and background"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Fish Scales..."),
    NULL);
}

#endif
