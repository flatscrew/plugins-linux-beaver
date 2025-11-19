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
 * Grok for making Shapes  
 * 2025, beaver, Glass Shapes

id=0 over aux=[ ref=0 
id=1 dst-out aux=[ ref=1  ]
ai/lb:shape-maker shape=flower
src-atop aux=[ ref=1  gaussian-blur std-dev-x=4 std-dev-y=4  ]

id=2 src-atop aux=[ ref=2  lb:glassovertext azimuth=43   hyperopacity=0.8    ]
id=2 src-atop aux=[ ref=2  lb:glassovertext azimuth=203 hyperopacity=0.8     ]
lb:edgesmooth
dropshadow x=0 y=0 radius=1 grow-radius=1 opacity=1

#over]

*/

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES



enum_start (gegl_shape_type521432)
  enum_value (GEGL_SHAPE_ELLIPSE,   "ellipse",   N_("Ellipse"))
  enum_value (GEGL_SHAPE_RECTANGLE, "rectangle", N_("Rectangle"))
  enum_value (GEGL_SHAPE_TRIANGLE,  "triangle",  N_("Triangle"))
  enum_value (GEGL_SHAPE_DIAMOND,   "diamond",   N_("Diamond"))
  enum_value (GEGL_SHAPE_HEART,     "heart",     N_("Heart"))
  enum_value (GEGL_SHAPE_STAR,      "star",      N_("Star"))
  enum_value (GEGL_SHAPE_FLOWER,    "flower",    N_("Flower"))
  enum_value (GEGL_SHAPE_POLYGON,   "even_polygon", N_("Even Polygons"))
  enum_value (GEGL_SHAPE_ARROW,     "arrow",     N_("Arrow"))
  enum_value (GEGL_SHAPE_CROSS,     "cross",     N_("Cross"))
  enum_value (GEGL_SHAPE_SEMICIRCLE,"semicircle",N_("Semicircle"))
  enum_value (GEGL_SHAPE_TRAPEZOID, "trapezoid", N_("Trapezoid"))
  enum_value (GEGL_SHAPE_UNEVEN_POLYGON, "uneven_polygon", N_("Uneven Polygons"))
  enum_value (GEGL_SHAPE_HEXAGRAM,  "hexagram",  N_("Hexagram"))
  enum_value (GEGL_SHAPE_CRESCENT,  "crescent",  N_("Crescent"))
  enum_value (GEGL_SHAPE_PARALLELOGRAM, "parallelogram", N_("Parallelogram"))
  enum_value (GEGL_SHAPE_OVAL,      "oval",      N_("Oval"))
  enum_value (GEGL_SHAPE_RING,      "ring",      N_("Ring"))
  enum_value (GEGL_SHAPE_CHEVRON,   "chevron",   N_("Chevron"))
  enum_value (GEGL_SHAPE_QUARTER_CIRCLE, "quarter_circle", N_("Quarter Circle"))
  enum_value (GEGL_SHAPE_RIGHT_TRIANGLE, "right_triangle", N_("Right Triangle"))
enum_end (GeglShapeType521432)




property_enum (shape, _("Shape"),
               GeglShapeType521432, gegl_shape_type521432,
               GEGL_SHAPE_RECTANGLE)
    description (_("The shape to render"))

property_int (uneven_polygon_sides, _("Uneven Polygon Sides"), 1)
    description (_("Number of sides: 1 = Pentagon (5), 2 = Heptagon (7), 3 = Nonagon (9)"))
    value_range (1, 3)
    ui_range (1, 3)
    ui_meta ("visible", "shape {uneven_polygon}")



property_double (scale, _("Overall Scale"), 0.3)
    description (_("Overall scale of the shape size relative to the canvas"))
    value_range (0.0, 2.0)
    ui_range (0.1, 0.8)

property_double (width_scale, _("Width Scale"), 1.0)
    description (_("Horizontal scale of the shape to adjust its width"))
    value_range (0.5, 2.0)
    ui_range (0.5, 1.5)

property_double (height_scale, _("Height Scale"), 1.0)
    description (_("Vertical scale of the shape to adjust its height"))
    value_range (0.5, 2.0)
    ui_range (0.5, 1.5)

property_double (rotation, _("Rotation"), 0.0)
    description (_("Rotation of the shape in degrees"))
    value_range (0.0, 360.0)
    ui_range (0.0, 360.0)
    ui_meta ("visible", "!shape {ellipse, ring}")

property_double (pointiness, _("Pointiness"), 0.0)
    description (_("Adjusts the sharpness of the shape's points (for Hearts, Stars, and Flowers)"))
    value_range (-0.5, 0.5)
    ui_range (-0.5, 0.5)
    ui_meta ("visible", "shape {heart, star, flower}")

property_double (lobe_roundness, _("Lobe Roundness"), 0.0)
    description (_("Controls the roundness of the shape's lobes (for Hearts)"))
    value_range (-0.5, 0.5)
    ui_range (-0.5, 0.5)
    ui_meta ("visible", "shape {heart}")

property_int (star_points, _("Points"), 5)
    description (_("Number of points on the Star or Flower"))
    value_range (5, 10)
    ui_range (5, 10)
    ui_meta ("visible", "shape {star, flower}")

property_int (polygon_sides, _("Even Polygon Sides"), 1)
    description (_("Number of sides: 1 = Hexagon (6), 2 = Octagon (8), 3 = Decagon (10), 4 = Duodecagon (12)"))
    value_range (1, 4)
    ui_range (1, 4)
    ui_meta ("visible", "shape {even_polygon}")

property_int (trapezoid_top_width, _("Trapezoid Top Width"), 1)
    description (_("Width of the trapezoid's top side: 1 = narrow, 2 = medium, 3 = wide"))
    value_range (1, 3)
    ui_range (1, 3)
    ui_meta ("visible", "shape {trapezoid}")

property_double (square_to_rectangle, _("Square to Rectangle"), 1.0)
    description (_("Adjusts the length of the rectangle (1.0 = square, higher values = longer rectangle)"))
    value_range (1.0, 6.0)
    ui_range (1.0, 6.0)
    ui_meta ("visible", "shape {rectangle}")

property_double (ring_thickness, _("Ring Thickness"), 0.5)
    description (_("Adjusts the thickness of the ring (0.1 = thickest, 0.9 = thinnest)"))
    value_range (0.0, 1.0)
    ui_range (0.1, 1.0)
    ui_meta ("visible", "shape {ring}")

property_double (blur, _("Blur the glass"), 6.0)
  description   (_("Blur to make the glass less translucent"))
  ui_range      (0.0, 12.0)
  ui_steps      (1.0, 4.0)
  ui_meta       ("unit", "pixel-distance")


property_double (x, _("Shape X"), 0.5)
    description (_("X position of the shape center (relative to image width)"))
    value_range (-1.0, 2.0)
    ui_range (0.0, 1.0)
    ui_meta ("unit", "relative-coordinate")
    ui_meta ("axis", "x")

property_double (y, _("Shape Y"), 0.5)
    description (_("Y position of the shape center (relative to image height)"))
    value_range (-1.0, 2.0)
    ui_range (0.0, 1.0)
    ui_meta ("unit", "relative-coordinate")
    ui_meta ("axis", "y")



property_double (azimuth1, _("Half azimuth 1"), 23.0)
    description (_("Half azimuth part one of two"))
    value_range (0.0, 180.0)
    ui_meta ("unit", "degree")

property_double (azimuth2, _("Half azimuth 2"), 203.0)
    description (_("Half azimuth part two of two"))
    value_range (180.0, 360.0)
    ui_meta ("unit", "degree")

property_double (shadow_x, _("Shadow X"), 20.0)
  description   (_("Horizontal shadow offset"))
  ui_range      (-40.0, 40.0)
  ui_steps      (1, 10)
  ui_meta       ("unit", "pixel-distance")
  ui_meta       ("axis", "x")

property_double (shadow_y, _("Shadow Y"), 20.0)
  description   (_("Vertical shadow offset"))
  ui_range      (-40.0, 40.0)
  ui_steps      (1, 10)
  ui_meta       ("unit", "pixel-distance")
  ui_meta       ("axis", "y")


property_double (shadow_radius, _("Shadow Blur radius"), 10.0)
  value_range   (0.0, G_MAXDOUBLE)
  ui_range      (0.0, 300.0)
  ui_steps      (1, 5)
  ui_gamma      (1.5)
  ui_meta       ("unit", "pixel-distance")

property_double (shadow_opacity, _("Shadow Opacity"), 0.5)
  value_range   (0.0, 1.0)
  ui_steps      (0.01, 0.10)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     glass_shapes
#define GEGL_OP_C_SOURCE glass_shapes.c

#include "gegl-op.h"

/*starred nodes go inside typedef struct */

typedef struct
{
 GeglNode *input;
 GeglNode *shapes;
 GeglNode *color;
 GeglNode *blur;
 GeglNode *over;
 GeglNode *dstout;
 GeglNode *glassoverlay1;
 GeglNode *glassoverlay2;
 GeglNode *edgesmooth;
 GeglNode *srcatop1;
 GeglNode *srcatop2;
 GeglNode *srcatop3;
 GeglNode *idref1;
 GeglNode *idref2;
 GeglNode *idref3;
 GeglNode *idref4;
 GeglNode *dropshadow;

 GeglNode *output;
}State;

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglProperties *o = GEGL_PROPERTIES (operation);
  GeglColor *blackcolor = gegl_color_new ("#000000");
  GeglColor *colorsaves = gegl_color_new ("#ffffff");
  GeglColor *colorsavez = gegl_color_new ("#ffffff");

  State *state = o->user_data = g_malloc0 (sizeof (State));

/*new child node list is here, this is where starred nodes get defined

 state->newchildname = gegl_node_new_child (gegl, "operation", "lb:name", NULL);*/
  state->input    = gegl_node_get_input_proxy (gegl, "input");
  state->output   = gegl_node_get_output_proxy (gegl, "output");
 state->shapes = gegl_node_new_child (gegl, "operation", "ai/lb:shape-maker", NULL);

 state->blur = gegl_node_new_child (gegl, "operation", "gegl:gaussian-blur", "abyss-policy", 0, "std-dev-x", 5.0, "std-dev-y", 5.0, NULL);
 state->over = gegl_node_new_child (gegl, "operation", "gegl:over", NULL);
 state->dstout = gegl_node_new_child (gegl, "operation", "gegl:dst-out", NULL);
 state->glassoverlay1 = gegl_node_new_child (gegl, "operation", "lb:glassovertext", "azimuth", 23.0, "hyperopacity", 1.2, "color", colorsaves,  NULL);
 state->glassoverlay2 = gegl_node_new_child (gegl, "operation", "lb:glassovertext", "azimuth", 203.0, "hyperopacity", 1.2, "color", colorsavez,  NULL);
 state->dropshadow = gegl_node_new_child (gegl, "operation", "gegl:dropshadow", "x", 0.0, "y", 0.0, "grow-radius", 1.0, "opacity", 1.0, "color", blackcolor,  NULL);
 state->edgesmooth = gegl_node_new_child (gegl, "operation", "lb:edgesmooth", NULL);
 state->srcatop1 = gegl_node_new_child (gegl, "operation", "gegl:src-atop", NULL);
 state->srcatop2 = gegl_node_new_child (gegl, "operation", "gegl:src-atop", NULL);
 state->srcatop3 = gegl_node_new_child (gegl, "operation", "gegl:src-atop", NULL);
 state->idref1 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->idref2 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->idref3 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);
 state->idref4 = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);

}

static void update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;
  if (!state) return;





  gegl_node_link_many ( state->input, state->over, state->output,  NULL);

  gegl_node_connect (state->over, "aux", state->dropshadow, "output");
  gegl_node_link_many ( state->input, state->idref1, state->dstout, state->shapes, state->srcatop1, state->idref2, state->srcatop2, state->idref3,  state->srcatop3,  state->edgesmooth, state->dropshadow,  NULL);
  gegl_node_connect (state->dstout, "aux", state->idref1, "output");
  gegl_node_link_many (  state->idref1, state->blur,  NULL);
  gegl_node_connect (state->srcatop1, "aux", state->blur, "output");
  gegl_node_link_many (  state->idref2, state->glassoverlay1,  NULL);
  gegl_node_connect (state->srcatop2, "aux", state->glassoverlay1, "output");
  gegl_node_connect (state->srcatop3, "aux", state->glassoverlay2, "output");
  gegl_node_link_many ( state->idref3, state->glassoverlay2,  NULL);

/*optional connect from and too is here
  gegl_node_connect (state->blendmode, "aux", state->lastnodeinlist, "output"); */

  gegl_operation_meta_redirect (operation, "shape", state->shapes, "shape");
  gegl_operation_meta_redirect (operation, "uneven_polygon_sides", state->shapes, "uneven_polygon_sides");
  gegl_operation_meta_redirect (operation, "scale", state->shapes, "scale");
  gegl_operation_meta_redirect (operation, "width_scale", state->shapes, "width_scale");
  gegl_operation_meta_redirect (operation, "height_scale", state->shapes, "height_scale");
  gegl_operation_meta_redirect (operation, "rotation", state->shapes, "rotation");
  gegl_operation_meta_redirect (operation, "pointiness", state->shapes, "pointiness");
  gegl_operation_meta_redirect (operation, "lobe_roundness", state->shapes, "lobe_roundness");
  gegl_operation_meta_redirect (operation, "star_points", state->shapes, "star_points");
  gegl_operation_meta_redirect (operation, "polygon_sides", state->shapes, "polygon_sides");
  gegl_operation_meta_redirect (operation, "trapezoid_top_width", state->shapes, "trapezoid_top_width");
  gegl_operation_meta_redirect (operation, "ring_thickness", state->shapes, "ring_thickness");
  gegl_operation_meta_redirect (operation, "square_to_rectangle", state->shapes, "square_to_rectangle");
  gegl_operation_meta_redirect (operation, "x", state->shapes, "x");
  gegl_operation_meta_redirect (operation, "y", state->shapes, "y");
  gegl_operation_meta_redirect (operation, "x", state->shapes, "x");
  gegl_operation_meta_redirect (operation, "blur", state->blur, "std-dev-x");
  gegl_operation_meta_redirect (operation, "blur", state->blur, "std-dev-y");
  gegl_operation_meta_redirect (operation, "azimuth1", state->glassoverlay1,  "azimuth");
  gegl_operation_meta_redirect (operation, "azimuth2", state->glassoverlay2,  "azimuth");
  gegl_operation_meta_redirect (operation, "shadow_x", state->dropshadow,  "x");
  gegl_operation_meta_redirect (operation, "shadow_y", state->dropshadow,  "y");
  gegl_operation_meta_redirect (operation, "shadow_radius", state->dropshadow,  "radius");
  gegl_operation_meta_redirect (operation, "shadow_opacity", state->dropshadow,  "opacity");

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
    "name",        "lb:glass-shapes",
    "title",       _("Glass Shapes"),
    "reference-hash", "summer2025paradise",
    "description", _("Glass Shapes"),
/*<Image>/Colors <Image>/Filters are top level menus in GIMP*/
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Glass Shapes..."),
    NULL);
}

#endif
