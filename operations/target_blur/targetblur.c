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
 * 2025 Beaver Opaque Pixel Blur

id=1 src-in aux=[ ref=1 gaussian-blur std-dev-x=6 std-dev-y=5   ]
opacity value=3
median-blur radius=0 abyss-policy=none
* 

 */


#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES


enum_start (gegl_median_blur_neighborhoodtargetblur)
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_SQUARE,  "square",  N_("Square"))
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_CIRCLE,  "circle",  N_("Circle"))
  enum_value (GEGL_MEDIAN_BLUR_NEIGHBORHOOD_DIAMOND, "diamond", N_("Diamond"))
enum_end (GeglMedianBlurNeighborhoodtargetblur)



property_enum (type, _("Type of Blur"),
    opaqueblurmeme, opaqueblur_meme,
    gaussian)
    description (_("Choose a blur"))


enum_start (opaqueblur_meme)
  enum_value (median,      "median",
              N_("Median"))
  enum_value (box,      "box",
              N_("Box"))
  enum_value (gaussian,      "gaussian",
              N_("Gaussian"))
  enum_value (pixel,      "pixel",
              N_("Pixel"))
enum_end (opaqueblurmeme)

property_enum (neighborhood, _("Internal base Median shape"),
               GeglMedianBlurNeighborhoodtargetblur, gegl_median_blur_neighborhoodtargetblur,
               GEGL_MEDIAN_BLUR_NEIGHBORHOOD_CIRCLE)
  description (_("Internal shape that the median uses"))
ui_meta ("visible", "type {median}" )

property_double (x, _("Size X"), 6.0)
    value_range (0.0, 100.0)
    ui_range    (0.0, 100.0)
    ui_meta     ("axis", "x")
ui_meta ("visible", "type {gaussian}" )

property_double (y, _("Size Y"), 6.0)
    value_range (0.0, 100.0)
    ui_range    (0.0, 100.0)
    ui_meta     ("axis", "y")
ui_meta ("visible", "type {gaussian}" )

property_int  (radius, _("Radius"), 7)
  value_range (0, 100)
  ui_range    (0, 100)
  ui_meta     ("unit", "pixel-distance")
  description (_("Neighborhood radius, a negative value will calculate with inverted percentiles"))
ui_meta ("visible", "type {box, median}" )

property_double  (percentile, _("Percentile"), 50)
  value_range (0, 100)
  description (_("Neighborhood color percentile"))
ui_meta ("visible", "type {median}" )

property_int    (pixel_x, _("Block width"), 16)
    description (_("Width of blocks in pixels"))
    value_range (1, 40)
    ui_gamma    (1.5)
    ui_meta     ("unit", "pixel-distance")
    ui_meta     ("axis", "x")
ui_meta ("visible", "type {pixel}" )

property_int    (pixel_y, _("Block height"), 16)
    description (_("Height of blocks in pixels"))
    value_range (1, 40)
    ui_gamma    (1.5)
    ui_meta     ("unit", "pixel-distance")
    ui_meta     ("axis", "y")
ui_meta ("visible", "type {pixel}" )


#else

#define GEGL_OP_META
#define GEGL_OP_NAME     targetblur
#define GEGL_OP_C_SOURCE targetblur.c


#include "gegl-op.h"

typedef struct
{
GeglNode *input;
GeglNode *output;
GeglNode *opacity;
GeglNode *repair;
GeglNode *gaus;
GeglNode *median;
GeglNode *box;
GeglNode *pixel;
GeglNode *alphalockreplace;


} State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);

  State *state = o->user_data = g_malloc0 (sizeof (State));

state->input    = gegl_node_get_input_proxy (gegl, "input");
state->output   = gegl_node_get_output_proxy (gegl, "output");

  state->alphalockreplace   = gegl_node_new_child (gegl,
                                  "operation", "gegl:src-in",  NULL);

  state->opacity   = gegl_node_new_child (gegl,
                                  "operation", "gegl:opacity", "value", 3.5,  NULL);

   state->gaus      = gegl_node_new_child (gegl, "operation", "gegl:gaussian-blur",     NULL);


   state->median      = gegl_node_new_child (gegl, "operation", "gegl:median-blur", "alpha-percentile", 100.0,    NULL);


   state->box      = gegl_node_new_child (gegl, "operation", "gegl:box-blur",     NULL);

   state->pixel      = gegl_node_new_child (gegl, "operation", "gegl:pixelize",     NULL);



   state->repair     = gegl_node_new_child (gegl, "operation", "gegl:median-blur",
                                         "radius",       0,
                                         "abyss-policy",    0,
                                         NULL);




}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;
 GeglNode* blur;



  switch (o->type) {
    case median: blur = state->median; break;
    case box: blur = state->box; break;
    case gaussian: blur = state->gaus; break;
    case pixel: blur = state->pixel; break;
default: blur = state->gaus;
}

   gegl_node_link_many ( state->input, state->alphalockreplace, state->opacity, state->repair, state->output,   NULL);
   gegl_node_link_many (state->input, blur, NULL);
   gegl_node_connect (state->alphalockreplace, "aux", blur, "output");


  gegl_operation_meta_redirect (operation, "x", state->gaus, "std-dev-x"); 
  gegl_operation_meta_redirect (operation, "y", state->gaus, "std-dev-y"); 
  gegl_operation_meta_redirect (operation, "pixel_x", state->pixel, "size-x"); 
  gegl_operation_meta_redirect (operation, "pixel_y", state->pixel, "size-y"); 
  gegl_operation_meta_redirect (operation, "radius", state->median, "radius"); 
  gegl_operation_meta_redirect (operation, "neighborhood", state->median, "neighborhood"); 
  gegl_operation_meta_redirect (operation, "radius", state->box, "radius"); 
  gegl_operation_meta_redirect (operation, "percentile", state->median, "percentile"); 


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
    "name",           "lb:target-blur",
    "title",          _("Target Blur"),
    "reference-hash", "lovelynightloversnightfeelingsgotmedancin",
    "description",    _("A blur that only applies on opaque pixels"),
    "gimp:menu-path", "<Image>/Filters/Blur/",
    "gimp:menu-label", _("Target blur..."),               
                NULL);
}

#endif
