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
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions (for making GEGL)
 * Grok 2025 for making Shapes and Beaver vibe coded it 
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <math.h>
#include <gegl.h>
#include <gegl-plugin.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_shape_type3421)
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
enum_end (GeglShapeType3421)

property_enum (shape, _("Shape"),
               GeglShapeType3421, gegl_shape_type3421,
               GEGL_SHAPE_HEART)
    description (_("The shape to render"))

property_int (uneven_polygon_sides, _("Uneven Polygon Sides"), 1)
    description (_("Number of sides: 1 = Pentagon (5), 2 = Heptagon (7), 3 = Nonagon (9)"))
    value_range (1, 3)
    ui_range (1, 3)
    ui_meta ("visible", "shape {uneven_polygon}")

property_color (shape_color, _("Shape Color"), "#ff0000")
    description (_("Color of the shape (default is red)"))

property_double (scale, _("Overall Scale"), 1.0)
    description (_("Overall scale of the shape size relative to the canvas"))
    value_range (0.1, 5.0)
    ui_range (0.5, 2.0)

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

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     the_shapes_core
#define GEGL_OP_C_SOURCE shapedesigncore.c

#include "gegl-op.h"

// Helper function to check if a point (px, py) is on the left side of a line from (x1, y1) to (x2, y2)
static gfloat point_side(gfloat px, gfloat py, gfloat x1, gfloat y1, gfloat x2, gfloat y2)
{
  return (x2 - x1) * (py - y1) - (y2 - y1) * (px - x1);
}

static void
prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "input", babl_format ("RGBA float"));
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
}

static gboolean
process (GeglOperation       *operation,
         void               *in_buf,
         void               *out_buf,
         glong               n_pixels,
         const GeglRectangle *roi,
         gint                level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  gfloat *out_pixel = (gfloat *) out_buf;

  gfloat shape_color[4];
  gegl_color_get_pixel (o->shape_color, babl_format ("RGBA float"), shape_color);

  // Get full canvas dimensions
  GeglRectangle *canvas = gegl_operation_source_get_bounding_box (operation, "input");
  if (!canvas) {
    // Fallback to ROI if canvas bounding box is unavailable
    canvas = (GeglRectangle *)roi;
  }
  gfloat canvas_width = canvas->width;
  gfloat canvas_height = canvas->height;

  // Center in pixel coordinates, directly using o->x and o->y (0.0 to 1.0)
  gfloat cx = o->x * canvas_width;
  gfloat cy = o->y * canvas_height;

  // Adjust for ROI offset when normalizing pixel coordinates
  gfloat roi_offset_x = roi->x;
  gfloat roi_offset_y = roi->y;

  gfloat scale = o->scale / 2.0; // Halve the scale denominator to keep shape twice as large
  gfloat width_scale = o->width_scale;
  gfloat height_scale = o->height_scale;
  gfloat rotation = o->rotation * G_PI / 180.0; // Convert degrees to radians
  gfloat pointiness = o->pointiness;
  gfloat lobe_roundness = o->lobe_roundness;
  gfloat square_to_rectangle = o->square_to_rectangle;
  gfloat ring_thickness = o->ring_thickness;
  gfloat theta, r, t;
  gint star_points = o->star_points;
  gint polygon_sides = o->polygon_sides;
  gint trapezoid_top_width = o->trapezoid_top_width;
  gint uneven_polygon_sides = o->uneven_polygon_sides;

  for (glong i = 0; i < n_pixels; i++)
  {
    // Compute global coordinates, adjusted for ROI offset
    gint x = (i % roi->width) + roi_offset_x;
    gint y = (i / roi->width) + roi_offset_y;

    // Normalize coordinates relative to center and scale
    gfloat nx = (x - cx) / (canvas_width * scale * width_scale);
    gfloat ny = (y - cy) / (canvas_height * scale * height_scale);

    // Apply rotation
    gfloat nx_rot = nx * cos(rotation) + ny * sin(rotation);
    gfloat ny_rot = -nx * sin(rotation) + ny * cos(rotation);
    nx = nx_rot;
    ny = ny_rot;

    // Shape equations
    gfloat shape_value = 0.0;
    switch (o->shape)
    {
      case GEGL_SHAPE_ELLIPSE:
      {
        // Ellipse: x^2 + y^2 <= 1
        shape_value = nx * nx + ny * ny - 1.0;
        break;
      }
      case GEGL_SHAPE_RECTANGLE:
      {
        // Rectangle: Adjust x-axis by square_to_rectangle to make it longer
        gfloat nx_adjusted = nx / square_to_rectangle; // Stretch x-axis
        shape_value = fmax(fabs(nx_adjusted), fabs(ny)) - 1.0;
        break;
      }
      case GEGL_SHAPE_TRIANGLE:
      {
        // Triangle: Simple linear boundaries for an equilateral triangle
        gfloat slope = sqrt(3.0); // Slope for equilateral triangle
        shape_value = fmax(ny - 1.0, fmax(-slope * nx - ny - 1.0, slope * nx - ny - 1.0));
        break;
      }
      case GEGL_SHAPE_DIAMOND:
      {
        // Diamond: |x| + |y| <= 1
        shape_value = fabs(nx) + fabs(ny) - 1.0;
        break;
      }
      case GEGL_SHAPE_HEART:
      {
        // Heart: (x^2 + y^2 - 1)^3 - x^2 * y^3 <= 0
        // Flip vertically to keep right-side up
        ny = -ny;
        gfloat nx2 = nx * nx;
        gfloat ny2 = ny * ny;
        t = nx2 + ny2 - 1.0 + lobe_roundness * (ny2); // Roundness affects lobes
        shape_value = t * t * t - nx2 * (ny + pointiness) * (ny + pointiness) * (ny + pointiness); // Pointiness affects bottom
        break;
      }
      case GEGL_SHAPE_STAR:
      {
        // Star: Polygon-based star using line segments (pentagram style)
        gint total_points = star_points * 2; // Outer and inner points
        gfloat outer_radius = 1.0;
        gfloat inner_radius = 0.4 + pointiness; // Adjusted by pointiness
        gfloat vertices[20][2]; // Up to 10 points * 2 (outer + inner)
        for (gint j = 0; j < star_points; j++)
        {
          gfloat angle_outer = j * 2.0 * G_PI / star_points - G_PI / 2.0;
          gfloat angle_inner = (j + 0.5) * 2.0 * G_PI / star_points - G_PI / 2.0;
          vertices[2 * j][0] = outer_radius * cos(angle_outer);
          vertices[2 * j][1] = outer_radius * sin(angle_outer);
          vertices[2 * j + 1][0] = inner_radius * cos(angle_inner);
          vertices[2 * j + 1][1] = inner_radius * sin(angle_inner);
        }
        // Point-in-polygon test using ray casting
        gint intersections = 0;
        for (gint j = 0; j < total_points; j++)
        {
          gfloat x1 = vertices[j][0];
          gfloat y1 = vertices[j][1];
          gfloat x2 = vertices[(j + 1) % total_points][0];
          gfloat y2 = vertices[(j + 1) % total_points][1];
          if (((y1 > ny) != (y2 > ny)) && (nx < (x2 - x1) * (ny - y1) / (y2 - y1) + x1))
            intersections++;
        }
        shape_value = (intersections % 2 == 0) ? 1.0 : -1.0;
        break;
      }
      case GEGL_SHAPE_FLOWER:
      {
        // Flower: Simplified distance-to-line approach
        theta = atan2(ny, nx);
        r = sqrt(nx * nx + ny * ny);
        gfloat angle_step = G_PI / star_points;
        gfloat sector = fmod(theta + G_PI / 2.0, 2.0 * angle_step);
        sector = fabs(sector - angle_step);
        gfloat outer_radius = 1.0;
        gfloat inner_radius = 0.4 + pointiness;
        t = sector / angle_step;
        gfloat flower_r = (1.0 - t) * outer_radius + t * inner_radius;
        shape_value = r - flower_r;
        break;
      }
      case GEGL_SHAPE_POLYGON:
      {
        // Even Polygons: Hexagon (6 sides) to Duodecagon (12 sides)
        gint sides;
        switch (polygon_sides)
        {
          case 1: sides = 6; break;  // Hexagon
          case 2: sides = 8; break;  // Octagon
          case 3: sides = 10; break; // Decagon
          case 4: sides = 12; break; // Duodecagon
          default: sides = 6; break;
        }
        // Generate vertices for a regular polygon
        gfloat vertices[12][2]; // Up to 12 sides
        gfloat radius = 1.0;
        for (gint j = 0; j < sides; j++)
        {
          gfloat angle = j * 2.0 * G_PI / sides - G_PI / 2.0;
          vertices[j][0] = radius * cos(angle);
          vertices[j][1] = radius * sin(angle);
        }
        // Point-in-polygon test using ray casting
        gint intersections = 0;
        for (gint j = 0; j < sides; j++)
        {
          gfloat x1 = vertices[j][0];
          gfloat y1 = vertices[j][1];
          gfloat x2 = vertices[(j + 1) % sides][0];
          gfloat y2 = vertices[(j + 1) % sides][1];
          if (((y1 > ny) != (y2 > ny)) && (nx < (x2 - x1) * (ny - y1) / (y2 - y1) + x1))
            intersections++;
        }
        shape_value = (intersections % 2 == 0) ? 1.0 : -1.0;
        break;
      }
      case GEGL_SHAPE_ARROW:
      {
        // Classic Arrow: Rectangle shaft with a triangular head
        // Shaft: From x = -1 to 0, y = -0.3 to 0.3
        // Head: Triangle from x = 0 to 1, y = -0.5 to 0.5
        gfloat shaft = (nx >= -1.0 && nx <= 0.0 && ny >= -0.3 && ny <= 0.3) ? -1.0 : 1.0;
        gfloat head = (nx >= 0.0 && nx <= 1.0 && ny >= -0.5 * (1.0 - nx) && ny <= 0.5 * (1.0 - nx)) ? -1.0 : 1.0;
        shape_value = fmin(shaft, head);
        break;
      }
      case GEGL_SHAPE_CROSS:
      {
        // Cross: Two rectangles intersecting at the center
        // Vertical: x = -0.3 to 0.3, y = -1 to 1
        // Horizontal: x = -1 to 1, y = -0.3 to 0.3
        gfloat vertical = (nx >= -0.3 && nx <= 0.3 && ny >= -1.0 && ny <= 1.0) ? -1.0 : 1.0;
        gfloat horizontal = (nx >= -1.0 && nx <= 1.0 && ny >= -0.3 && ny <= 0.3) ? -1.0 : 1.0;
        shape_value = fmin(vertical, horizontal);
        break;
      }
      case GEGL_SHAPE_SEMICIRCLE:
      {
        // Semicircle: Top half of an ellipse (y >= 0)
        shape_value = (nx * nx + ny * ny <= 1.0 && ny >= 0.0) ? -1.0 : 1.0;
        break;
      }
      case GEGL_SHAPE_TRAPEZOID:
      {
        // Trapezoid: Four sides with parallel top and bottom
        gfloat top_width;
        switch (trapezoid_top_width)
        {
          case 1: top_width = 0.4; break;  // Narrow top
          case 2: top_width = 0.7; break;  // Medium top
          case 3: top_width = 1.0; break;  // Wide top (rectangle)
          default: top_width = 0.4; break;
        }
        gfloat vertices[4][2] = {
          {-1.0, -1.0},        // Bottom left
          {1.0, -1.0},         // Bottom right
          {top_width, 1.0},    // Top right
          {-top_width, 1.0}    // Top left
        };
        // Point-in-polygon test using ray casting
        gint intersections = 0;
        for (gint j = 0; j < 4; j++)
        {
          gfloat x1 = vertices[j][0];
          gfloat y1 = vertices[j][1];
          gfloat x2 = vertices[(j + 1) % 4][0];
          gfloat y2 = vertices[(j + 1) % 4][1];
          if (((y1 > ny) != (y2 > ny)) && (nx < (x2 - x1) * (ny - y1) / (y2 - y1) + x1))
            intersections++;
        }
        shape_value = (intersections % 2 == 0) ? 1.0 : -1.0;
        break;
      }
      case GEGL_SHAPE_UNEVEN_POLYGON:
      {
        // Uneven Polygons: Pentagon (5 sides), Heptagon (7 sides), Nonagon (9 sides)
        gint sides;
        switch (uneven_polygon_sides)
        {
          case 1: sides = 5; break;  // Pentagon
          case 2: sides = 7; break;  // Heptagon
          case 3: sides = 9; break;  // Nonagon
          default: sides = 5; break;
        }
        // Generate vertices for a regular polygon
        gfloat vertices[9][2]; // Up to 9 sides
        gfloat radius = 1.0;
        for (gint j = 0; j < sides; j++)
        {
          gfloat angle = j * 2.0 * G_PI / sides - G_PI / 2.0;
          vertices[j][0] = radius * cos(angle);
          vertices[j][1] = radius * sin(angle);
        }
        // Point-in-polygon test using ray casting
        gint intersections = 0;
        for (gint j = 0; j < sides; j++)
        {
          gfloat x1 = vertices[j][0];
          gfloat y1 = vertices[j][1];
          gfloat x2 = vertices[(j + 1) % sides][0];
          gfloat y2 = vertices[(j + 1) % sides][1];
          if (((y1 > ny) != (y2 > ny)) && (nx < (x2 - x1) * (ny - y1) / (y2 - y1) + x1))
            intersections++;
        }
        shape_value = (intersections % 2 == 0) ? 1.0 : -1.0;
        break;
      }
      case GEGL_SHAPE_HEXAGRAM:
      {
        // Hexagram: Two overlapping equilateral triangles (Star of David)
        gfloat slope = sqrt(3.0);
        // First triangle (pointing up)
        gfloat tri1 = fmax(ny - 1.0, fmax(-slope * nx - ny - 1.0, slope * nx - ny - 1.0));
        // Second triangle (pointing down, rotated 60 degrees)
        gfloat tri2 = fmax(-ny - 1.0, fmax(-slope * nx + ny - 1.0, slope * nx + ny - 1.0));
        shape_value = fmin(tri1, tri2);
        break;
      }
      case GEGL_SHAPE_CRESCENT:
      {
        // Crescent: Subtract a smaller, offset ellipse from a larger one
        gfloat outer = nx * nx + ny * ny - 1.0; // Outer ellipse
        gfloat inner = (nx - 0.5) * (nx - 0.5) + ny * ny - 0.64; // Inner ellipse, offset by 0.5, radius 0.8
        shape_value = (outer <= 0.0 && inner >= 0.0) ? -1.0 : 1.0;
        break;
      }
      case GEGL_SHAPE_PARALLELOGRAM:
      {
        // Parallelogram: Four sides with opposite sides equal and parallel
        gfloat vertices[4][2] = {
          {-1.0, -1.0},        // Bottom left
          {0.5, -1.0},         // Bottom right
          {1.0, 1.0},          // Top right
          {-0.5, 1.0}          // Top left
        };
        // Point-in-polygon test using ray casting
        gint intersections = 0;
        for (gint j = 0; j < 4; j++)
        {
          gfloat x1 = vertices[j][0];
          gfloat y1 = vertices[j][1];
          gfloat x2 = vertices[(j + 1) % 4][0];
          gfloat y2 = vertices[(j + 1) % 4][1];
          if (((y1 > ny) != (y2 > ny)) && (nx < (x2 - x1) * (ny - y1) / (y2 - y1) + x1))
            intersections++;
        }
        shape_value = (intersections % 2 == 0) ? 1.0 : -1.0;
        break;
      }
      case GEGL_SHAPE_OVAL:
      {
        // Oval: An elongated ellipse with fixed aspect ratio (2:1 width to height)
        shape_value = (nx * nx / 4.0) + (ny * ny) - 1.0;
        break;
      }
      case GEGL_SHAPE_RING:
      {
        // Ring: A ring shape (outer ellipse minus inner ellipse) with adjustable thickness
        gfloat outer = nx * nx + ny * ny - 1.0; // Outer radius 1.0
        gfloat inner = nx * nx + ny * ny - (ring_thickness * ring_thickness); // Inner radius based on thickness
        shape_value = (outer <= 0.0 && inner >= 0.0) ? -1.0 : 1.0;
        break;
      }
      case GEGL_SHAPE_CHEVRON:
      {
        // Chevron: V-shaped stripe
        // Left arm: From x = -1 to 0, y = -x - 1 to -x + 1
        // Right arm: From x = 0 to 1, y = x - 1 to x + 1
        gfloat left_arm = (nx >= -1.0 && nx <= 0.0 && ny >= -nx - 1.0 && ny <= -nx + 1.0) ? -1.0 : 1.0;
        gfloat right_arm = (nx >= 0.0 && nx <= 1.0 && ny >= nx - 1.0 && ny <= nx + 1.0) ? -1.0 : 1.0;
        shape_value = fmin(left_arm, right_arm);
        break;
      }
      case GEGL_SHAPE_QUARTER_CIRCLE:
      {
        // Quarter Circle: First quadrant of an ellipse (x >= 0, y >= 0)
        theta = atan2(ny, nx);
        shape_value = (nx * nx + ny * ny <= 1.0 && nx >= 0.0 && ny >= 0.0) ? -1.0 : 1.0;
        break;
      }
      case GEGL_SHAPE_RIGHT_TRIANGLE:
      {
        // Right Triangle: Vertices at (-1,-1), (1,-1), (-1,1)
        gfloat vertices[3][2] = {
          {-1.0, -1.0},  // Bottom-left (right angle)
          {1.0, -1.0},   // Bottom-right
          {-1.0, 1.0}    // Top-left
        };
        // Point-in-polygon test using ray casting
        gint intersections = 0;
        for (gint j = 0; j < 3; j++)
        {
          gfloat x1 = vertices[j][0];
          gfloat y1 = vertices[j][1];
          gfloat x2 = vertices[(j + 1) % 3][0];
          gfloat y2 = vertices[(j + 1) % 3][1];
          if (((y1 > ny) != (y2 > ny)) && (nx < (x2 - x1) * (ny - y1) / (y2 - y1) + x1))
            intersections++;
        }
        shape_value = (intersections % 2 == 0) ? 1.0 : -1.0;
        break;
      }
      default:
        shape_value = 1.0; // Fallback, should not happen
        break;
    }

    // Render the shape as a filled shape
    if (shape_value <= 0.0)
    {
      out_pixel[0] = shape_color[0];
      out_pixel[1] = shape_color[1];
      out_pixel[2] = shape_color[2];
      out_pixel[3] = shape_color[3];
    }
    else
    {
      out_pixel[0] = 0.0;
      out_pixel[1] = 0.0;
      out_pixel[2] = 0.0;
      out_pixel[3] = 0.0;
    }

    out_pixel += 4;
  }

  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);
  GeglOperationPointFilterClass *point_filter_class = GEGL_OPERATION_POINT_FILTER_CLASS (klass);

  operation_class->prepare = prepare;
  point_filter_class->process = process;

  gegl_operation_class_set_keys (operation_class,
    "name",        "ai/lb:shapes-core",
    "title",       _("Shapes Core"),
    "reference-hash", "shapesgeglpcalugin12",
    "description", _("Renders various shapes with customizable color, scale, width, height, rotation, and shape-specific adjustments"),
    "categories", "hidden",
    NULL);
}

#endif
