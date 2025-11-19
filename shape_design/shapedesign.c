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
 * Copyright 2025 LinuxBeaver
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_shape_style9274)
  enum_value (GEGL_SHAPE_STYLE_FILL,            "fill",            N_("Fill"))
  enum_value (GEGL_SHAPE_STYLE_OUTLINE_AND_FILL,"outline_and_fill",N_("Outline and Fill"))
  enum_value (GEGL_SHAPE_STYLE_OUTLINE_ONLY,    "outline_only",    N_("Outline Only"))
enum_end (GeglShapeStyle9274)

property_enum (style, _("Style"),
               GeglShapeStyle9274, gegl_shape_style9274,
               GEGL_SHAPE_STYLE_FILL)
  description (_("Select the rendering style for the shape"))

enum_start (gegl_shape_type5432)
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
enum_end (GeglShapeType5432)

property_double (outline_grow_radius, _("Grow Radius"), 20.0)
    description (_("Radius of the Stroke (Outline) in pixels"))
    value_range (0.0, 60.0)
    ui_range (0.0, 60.0)
    ui_meta ("visible", "style {outline_and_fill, outline_only}")

property_color (outline_color, _("Color of Outline"), "#ffffff")
    description (_("Color of the Stroke (Outline) defaults to white"))
    ui_meta ("visible", "style {outline_and_fill, outline_only}")

property_enum (shape, _("Shape"),
               GeglShapeType5432, gegl_shape_type5432,
               GEGL_SHAPE_RECTANGLE)
    description (_("The shape to render"))

property_int (uneven_polygon_sides, _("Uneven Polygon Sides"), 1)
    description (_("Number of sides: 1 = Pentagon (5), 2 = Heptagon (7), 3 = Nonagon (9)"))
    value_range (1, 3)
    ui_range (1, 3)
    ui_meta ("visible", "shape {uneven_polygon}")

property_color (shape_color, _("Shape Color"), "#ff0000")
    description (_("Color of the shape (default is red)"))
    ui_meta ("visible", "style {fill, outline_and_fill}")

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

property_double (x, _("X"), 0.5)
    description (_("X position of the shape center (relative to image width)"))
    value_range (-1.0, 2.0)
    ui_range (0.0, 1.0)
    ui_meta ("unit", "relative-coordinate")
    ui_meta ("axis", "x")

property_double (y, _("Y"), 0.5)
    description (_("Y position of the shape center (relative to image height)"))
    value_range (-1.0, 2.0)
    ui_range (0.0, 1.0)
    ui_meta ("unit", "relative-coordinate")
    ui_meta ("axis", "y")

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     shapedesign
#define GEGL_OP_C_SOURCE shapedesign.c

#include "gegl-op.h"

typedef struct
{
  GeglNode *input;
  GeglNode *output;
  GeglNode *shapes_core;
  GeglNode *ssg;
  GeglNode *over;
  GeglNode *src;
  GeglNode *nop_id;
} State;

static void
attach (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  GeglNode *gegl = operation->node;

  // Initialize the state
  State *state = g_malloc0 (sizeof (State));
  o->user_data = state;

  // Create nodes
  state->input      = gegl_node_get_input_proxy (gegl, "input");
  state->output     = gegl_node_get_output_proxy (gegl, "output");
  state->shapes_core= gegl_node_new_child (gegl, "operation", "ai/lb:shapes-core", NULL);
  state->ssg        = gegl_node_new_child (gegl, "operation", "lb:ssg", NULL);
  state->over       = gegl_node_new_child (gegl, "operation", "gegl:over", NULL);
  state->src        = gegl_node_new_child (gegl, "operation", "gegl:src", NULL);
  state->nop_id     = gegl_node_new_child (gegl, "operation", "gegl:nop", NULL);

  // Redirect properties to the shapes_core node
  gegl_operation_meta_redirect (operation, "shape", state->shapes_core, "shape");
  gegl_operation_meta_redirect (operation, "uneven_polygon_sides", state->shapes_core, "uneven_polygon_sides");
  gegl_operation_meta_redirect (operation, "shape_color", state->shapes_core, "shape_color");
  gegl_operation_meta_redirect (operation, "scale", state->shapes_core, "scale");
  gegl_operation_meta_redirect (operation, "width_scale", state->shapes_core, "width_scale");
  gegl_operation_meta_redirect (operation, "height_scale", state->shapes_core, "height_scale");
  gegl_operation_meta_redirect (operation, "rotation", state->shapes_core, "rotation");
  gegl_operation_meta_redirect (operation, "pointiness", state->shapes_core, "pointiness");
  gegl_operation_meta_redirect (operation, "lobe_roundness", state->shapes_core, "lobe_roundness");
  gegl_operation_meta_redirect (operation, "star_points", state->shapes_core, "star_points");
  gegl_operation_meta_redirect (operation, "polygon_sides", state->shapes_core, "polygon_sides");
  gegl_operation_meta_redirect (operation, "trapezoid_top_width", state->shapes_core, "trapezoid_top_width");
  gegl_operation_meta_redirect (operation, "ring_thickness", state->shapes_core, "ring_thickness");
  gegl_operation_meta_redirect (operation, "square_to_rectangle", state->shapes_core, "square_to_rectangle");
  gegl_operation_meta_redirect (operation, "x", state->shapes_core, "x");
  gegl_operation_meta_redirect (operation, "y", state->shapes_core, "y");

  // Redirect properties to the ssg node
  gegl_operation_meta_redirect (operation, "outline_grow_radius", state->ssg, "stroke");
  gegl_operation_meta_redirect (operation, "outline_color", state->ssg, "colorssg");
}

static void
update_graph (GeglOperation *operation)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  State *state = o->user_data;

  if (!state) return;

  switch (o->style)
  {
    case GEGL_SHAPE_STYLE_FILL:
      // Graph: input -> shapes-core -> output
      gegl_node_link_many (state->input, state->shapes_core, state->output, NULL);
      break;

    case GEGL_SHAPE_STYLE_OUTLINE_AND_FILL:
      // Graph: input -> shapes-core -> nop_id -> over -> output
      // Branch: nop_id -> ssg -> over (aux)
      gegl_node_link_many (state->input, state->shapes_core, state->nop_id, state->over, state->output, NULL);
      gegl_node_link_many (state->nop_id, state->ssg, NULL);
      gegl_node_connect (state->over, "aux", state->ssg, "output");
      break;

    case GEGL_SHAPE_STYLE_OUTLINE_ONLY:
      // Graph: input -> shapes-core -> nop_id -> src -> output
      // Branch: nop_id -> ssg -> src (aux)
      gegl_node_link_many (state->input, state->shapes_core, state->nop_id, state->src, state->output, NULL);
      gegl_node_link_many (state->nop_id, state->ssg, NULL);
      gegl_node_connect (state->src, "aux", state->ssg, "output");
      break;
  }
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationMetaClass *operation_meta_class = GEGL_OPERATION_META_CLASS (klass);

  operation_class->attach = attach;
  operation_meta_class->update = update_graph;

  gegl_operation_class_set_keys (operation_class,
    "name",        "ai/lb:shape-maker",
    "title",       _("Shape Maker"),
    "reference-hash", "shapedesign123grokmasterpiece",
    "description", _("A meta operation to render shapes with different styles: Fill, Outline and Fill, or Outline Only"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Shape Maker..."),
    NULL);
}

#endif
