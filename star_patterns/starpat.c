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
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions for making GEGL
 * Star Patters by Grok - Vibe coded by Beaver
 */
#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start(gegl_star_pattern2521)
  enum_value(STAR_PATTERN_BASIC_GRID, "basic_grid", N_("Basic Grid"))
  enum_value(STAR_PATTERN_OFFSET_GRID, "offset_grid", N_("Offset Grid"))
  enum_value(STAR_PATTERN_HEXAGONAL_GRID, "hexagonal_grid", N_("Hexagonal Grid"))
  enum_value(STAR_PATTERN_DIAGONAL_GRID, "diagonal_grid", N_("Diagonal Grid"))
  enum_value(STAR_PATTERN_RANDOM_SCATTER, "random_scatter", N_("Random Scatter"))
  enum_value(STAR_PATTERN_WAVE_GRID, "wave_grid", N_("Wave Grid"))
  enum_value(STAR_PATTERN_RANDOM_BIG_SMALL, "random_big_small", N_("Random Big-Small"))
  enum_value(STAR_PATTERN_RANDOM_BIG_MEDIUM_SMALL, "random_big_medium_small", N_("Random Big-Medium-Small"))
  enum_value(STAR_PATTERN_STAGGERED_SIZES, "staggered_sizes", N_("Staggered Sizes"))
enum_end(GeglStarPattern2521)

property_enum(pattern, _("Star Pattern"),
              GeglStarPattern2521, gegl_star_pattern2521,
              STAR_PATTERN_HEXAGONAL_GRID)
    description(_("Select the star pattern to use"))

property_int(num_points, _("Number of Points"), 5)
    description(_("Number of points on the star (3 to 12)"))
    value_range(3, 12)
    ui_range(3, 12)

property_int(thickness, _("Thickness"), 2)
    description(_("Thickness of the star points: 1 (Thin), 2 (Medium), 3 (Thick)"))
    value_range(1, 3)
    ui_range(1, 3)

property_double(size, _("Star Size"), 50.0)
    description(_("Size of the stars in pixels"))
    value_range(10.0, 200.0)
    ui_range(10.0, 150.0)

property_double(rotation, _("Rotation"), 0.0)
    value_range(-180.0, 180.0)
    description(_("Rotation angle of the stars in degrees"))

property_double(pan_x, _("X Pan"), 0.0)
    value_range(-100.0, 100.0)
    description(_("Horizontal offset of the tiled pattern"))

property_double(pan_y, _("Y Pan"), 0.0)
    value_range(-100.0, 100.0)
    description(_("Vertical offset of the tiled pattern"))

property_double(zoom, _("Zoom"), 1.0)
    value_range(0.1, 5.0)
    description(_("Zoom level of the stars and tiling grid"))

property_color(star_color, _("Star Color"), "#ffd559")
    description(_("Color of the stars (default is yellow)"))

property_color(bg_color, _("Background Color"), "#ac62ff")
    description(_("Color of the background (default is purple)"))
 ui_meta     ("sensitive", "! transparent_bg")

property_boolean(transparent_bg, _("Transparent Background"), FALSE)
    description(_("Enable to make the background transparent"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     starpat
#define GEGL_OP_C_SOURCE starpat.c

#include "gegl-op.h"

// Simple hash function for pseudo-random numbers
static guint32
hash_position(gint x, gint y)
{
  return (x * 131 + y * 151) ^ (x + y);
}

// Pseudo-random float between 0 and 1 based on position
static gfloat
random_float(gint x, gint y)
{
  return (hash_position(x, y) % 1000) / 1000.0f;
}

// Function to check if a point (px, py) is inside a star centered at (cx, cy)
static gboolean
point_in_star(gdouble px, gdouble py, gdouble cx, gdouble cy, gdouble size,
              gint num_points, gdouble ratio, gdouble rotation)
{
  // Translate the point relative to the star center
  px -= cx;
  py -= cy;

  // Apply rotation
  gdouble rot_rad = rotation * G_PI / 180.0;
  gdouble temp_x = px * cos(rot_rad) + py * sin(rot_rad);
  gdouble temp_y = -px * sin(rot_rad) + py * cos(rot_rad);
  px = temp_x;
  py = temp_y;

  // Define the star as a polygon with alternating outer and inner vertices
  gdouble outer_r = size;
  gdouble inner_r = size * ratio;
  gint num_vertices = 2 * num_points;  // Alternating outer and inner points
  gdouble vertices[24][2];  // Array to store vertex coordinates (max 12 points = 24 vertices)

  // Compute the vertex positions
  for (gint i = 0; i < num_vertices; i++)
  {
    gdouble angle = (i * 2 * G_PI / num_vertices) - G_PI / 2;  // Start from top (subtract pi/2)
    gdouble r = (i % 2 == 0) ? outer_r : inner_r;
    vertices[i][0] = r * cos(angle);  // x-coordinate
    vertices[i][1] = r * sin(angle);  // y-coordinate
  }

  // Use ray-casting algorithm to determine if the point is inside the polygon
  gint crossings = 0;
  for (gint i = 0; i < num_vertices; i++)
  {
    gint j = (i + 1) % num_vertices;  // Next vertex (wrap around)
    gdouble x1 = vertices[i][0];
    gdouble y1 = vertices[i][1];
    gdouble x2 = vertices[j][0];
    gdouble y2 = vertices[j][1];

    // Check if the ray (from (px, py) to the right) intersects the edge
    if (((y1 > py) != (y2 > py)) &&
        (px < (x2 - x1) * (py - y1) / (y2 - y1) + x1))
    {
      crossings++;
    }
  }

  // Point is inside if the number of crossings is odd
  return (crossings % 2 == 1);
}

static void
prepare(GeglOperation *operation)
{
  const Babl *space = gegl_operation_get_source_space(operation, "input");
  gegl_operation_set_format(operation, "input", babl_format_with_space("RGBA float", space));
  gegl_operation_set_format(operation, "output", babl_format_with_space("RGBA float", space));
}

static gboolean
process(GeglOperation *operation, void *in_buf, void *out_buf, glong n_pixels, const GeglRectangle *roi, gint level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  gfloat star_rgb[3], bg_rgb[3];

  gegl_color_get_pixel(o->star_color, babl_format("RGB float"), star_rgb);
  gegl_color_get_pixel(o->bg_color, babl_format("RGB float"), bg_rgb);

  gfloat base_size = o->size * o->zoom;  // Base size adjusted by zoom
  gfloat spacing = base_size * 3.0;      // Base spacing between stars

  // Map thickness (1 to 3) to inner/outer radius ratio
  gdouble ratio;
  switch (o->thickness)
  {
    case 1: ratio = 0.3; break;  // Thin
    case 2: ratio = 0.5; break;  // Medium
    case 3: ratio = 0.7; break;  // Thick
    default: ratio = 0.5; break;
  }

  // Cast out_buf to a float array for direct pixel manipulation
  gfloat *out_data = (gfloat *) out_buf;

  for (gint y = roi->y; y < roi->y + roi->height; y++)
  {
    for (gint x = roi->x; x < roi->x + roi->width; x++)
    {
      // Calculate the index into out_buf
      gint idx = ((y - roi->y) * roi->width + (x - roi->x)) * 4;
      gfloat min_distance = G_MAXFLOAT;
      gfloat current_size = base_size;
      gboolean is_inside = FALSE;

      // Apply panning
      gfloat px = x - o->pan_x;
      gfloat py = y - o->pan_y;

      switch (o->pattern)
      {
        case STAR_PATTERN_BASIC_GRID:
        {
          gfloat grid_x = floor(px / spacing) * spacing + spacing / 2;
          gfloat grid_y = floor(py / spacing) * spacing + spacing / 2;
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat center_x = grid_x + c * spacing + o->pan_x;
              gfloat center_y = grid_y + r * spacing + o->pan_y;
              gfloat dx = x - center_x;
              gfloat dy = y - center_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, center_x, center_y, base_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = base_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case STAR_PATTERN_OFFSET_GRID:
        {
          gfloat row_height = spacing;
          gfloat row = floor(py / row_height);
          gfloat col = floor(px / spacing);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + spacing / 2 + o->pan_y;
              gfloat grid_x = (col + c) * spacing + spacing / 2 + o->pan_x;
              if ((gint)(row + r) % 2 == 1) grid_x += spacing / 2;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, grid_x, grid_y, base_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = base_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case STAR_PATTERN_HEXAGONAL_GRID:
        {
          gfloat row_height = spacing * sqrt(3.0f) / 2.0f;
          gfloat row = floor(py / row_height);
          gfloat col = floor(px / spacing);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + spacing / 2 + o->pan_y;
              gfloat grid_x = (col + c) * spacing + spacing / 2 + o->pan_x;
              if ((gint)(row + r) % 2 == 1) grid_x += spacing / 2;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, grid_x, grid_y, base_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = base_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case STAR_PATTERN_DIAGONAL_GRID:
        {
          gfloat diag_spacing = spacing * sqrt(2.0f);
          gfloat diag_x = (px + py) / diag_spacing;
          gfloat diag_y = (px - py) / diag_spacing;
          gfloat col = floor(diag_x);
          gfloat row = floor(diag_y);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_diag_x = (col + c) * diag_spacing;
              gfloat grid_diag_y = (row + r) * diag_spacing;
              gfloat grid_x = (grid_diag_x + grid_diag_y) / 2 + o->pan_x;
              gfloat grid_y = (grid_diag_x - grid_diag_y) / 2 + o->pan_y;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, grid_x, grid_y, base_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = base_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case STAR_PATTERN_RANDOM_SCATTER:
        {
          gfloat grid_x_base = floor(px / spacing) * spacing;
          gfloat grid_y_base = floor(py / spacing) * spacing;
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat base_x = grid_x_base + c * spacing;
              gfloat base_y = grid_y_base + r * spacing;
              gfloat offset_x = (random_float(base_x, base_y) - 0.5f) * spacing;
              gfloat offset_y = (random_float(base_y, base_x) - 0.5f) * spacing;
              gfloat grid_x = base_x + spacing / 2 + offset_x + o->pan_x;
              gfloat grid_y = base_y + spacing / 2 + offset_y + o->pan_y;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, grid_x, grid_y, base_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = base_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case STAR_PATTERN_WAVE_GRID:
        {
          gfloat grid_x = floor(px / spacing) * spacing + spacing / 2;
          gfloat grid_y = floor(py / spacing) * spacing + spacing / 2;
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat center_x = grid_x + c * spacing + sin((grid_y + r * spacing) * 0.05f) * spacing / 2 + o->pan_x;
              gfloat center_y = grid_y + r * spacing + sin((grid_x + c * spacing) * 0.05f) * spacing / 2 + o->pan_y;
              gfloat dx = x - center_x;
              gfloat dy = y - center_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, center_x, center_y, base_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = base_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case STAR_PATTERN_RANDOM_BIG_SMALL:
        {
          gfloat grid_x_base = floor(px / spacing) * spacing;
          gfloat grid_y_base = floor(py / spacing) * spacing;
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat base_x = grid_x_base + c * spacing;
              gfloat base_y = grid_y_base + r * spacing;
              gfloat offset_x = (random_float(base_x, base_y) - 0.5f) * spacing;
              gfloat offset_y = (random_float(base_y, base_x) - 0.5f) * spacing;
              gfloat grid_x = base_x + spacing / 2 + offset_x + o->pan_x;
              gfloat grid_y = base_y + spacing / 2 + offset_y + o->pan_y;
              gfloat star_size = random_float(base_x + base_y, base_x) < 0.5f ? base_size : base_size * 0.5f;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, grid_x, grid_y, star_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = star_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case STAR_PATTERN_RANDOM_BIG_MEDIUM_SMALL:
        {
          gfloat grid_x_base = floor(px / spacing) * spacing;
          gfloat grid_y_base = floor(py / spacing) * spacing;
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat base_x = grid_x_base + c * spacing;
              gfloat base_y = grid_y_base + r * spacing;
              gfloat offset_x = (random_float(base_x, base_y) - 0.5f) * spacing;
              gfloat offset_y = (random_float(base_y, base_x) - 0.5f) * spacing;
              gfloat grid_x = base_x + spacing / 2 + offset_x + o->pan_x;
              gfloat grid_y = base_y + spacing / 2 + offset_y + o->pan_y;
              gfloat rand = random_float(base_x + base_y, base_x);
              gfloat star_size;
              if (rand < 0.33f) star_size = base_size;
              else if (rand < 0.66f) star_size = base_size * 0.75f;
              else star_size = base_size * 0.5f;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, grid_x, grid_y, star_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = star_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
        case STAR_PATTERN_STAGGERED_SIZES:
        {
          gfloat row_height = spacing;
          gfloat row = floor(py / row_height);
          gfloat col = floor(px / spacing);
          for (gint r = -1; r <= 1; r++)
          {
            for (gint c = -1; c <= 1; c++)
            {
              gfloat grid_y = (row + r) * row_height + spacing / 2 + o->pan_y;
              gfloat grid_x = (col + c) * spacing + spacing / 2 + o->pan_x;
              gfloat star_size = (gint)(col + c) % 2 == 0 ? base_size : base_size * 0.5f;
              if ((gint)(row + r) % 2 == 1) star_size = base_size * 0.75f;
              gfloat dx = x - grid_x;
              gfloat dy = y - grid_y;
              gfloat distance = sqrt(dx * dx + dy * dy);
              if (distance < min_distance && point_in_star(x, y, grid_x, grid_y, star_size, o->num_points, ratio, o->rotation))
              {
                min_distance = distance;
                current_size = star_size;
                is_inside = TRUE;
              }
            }
          }
          break;
        }
      }

      if (is_inside)
      {
        out_data[idx + 0] = star_rgb[0];
        out_data[idx + 1] = star_rgb[1];
        out_data[idx + 2] = star_rgb[2];
        out_data[idx + 3] = 1.0f;
      }
      else
      {
        if (o->transparent_bg)
        {
          out_data[idx + 0] = 0.0f;
          out_data[idx + 1] = 0.0f;
          out_data[idx + 2] = 0.0f;
          out_data[idx + 3] = 0.0f;  // Fully transparent background
        }
        else
        {
          out_data[idx + 0] = bg_rgb[0];
          out_data[idx + 1] = bg_rgb[1];
          out_data[idx + 2] = bg_rgb[2];
          out_data[idx + 3] = 1.0f;  // Opaque background color
        }
      }
    }
  }

  return TRUE;
}

static void
gegl_op_class_init(GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS(klass);
  GeglOperationPointFilterClass *point_filter_class = GEGL_OPERATION_POINT_FILTER_CLASS(klass);

  operation_class->prepare = prepare;
  point_filter_class->process = process;

  gegl_operation_class_set_keys(operation_class,
      "name",        "ai/lb:star-patterns",
      "title",       _("Star Patterns"),
      "reference-hash", "starpatterngegl2025",
      "description", _("Generates a tiled background of stars with customizable patterns, rotation, panning, and zoom"),
      "gimp:menu-path", "<Image>/Filters/AI GEGL",
      "gimp:menu-label", _("Star Patterns..."),
      NULL);
}

#endif
