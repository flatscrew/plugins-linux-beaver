/*
 * Pippin (2006) for making GEGL
 * Beaver for vibe coding
 * Grok 3 for making the GEGL plugin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start(fill_mode_type2222)
  enum_value(OUTLINES, "outlines", "Outlines")
  enum_value(FILLED, "filled", "Filled Circles")
enum_end(fill_mode_type2222)

enum_start(variation_type2222)
  enum_value(CLASSIC, "classic", "Classic (Aligned)")
  enum_value(SPARSE_GRID, "sparse_grid", "Sparse Grid (Aligned)")
  enum_value(ROTATED_CENTERS, "rotated_centers", "Rotated Centers (Aligned)")
  enum_value(SCALED_CIRCLES, "scaled_circles", "Scaled Circles (Aligned)")
  enum_value(VESICA_HIGHLIGHT, "vesica_highlight", "Vesica Highlight (Aligned)")
  enum_value(INNER_CIRCLES, "inner_circles", "Inner Circles (Aligned)")
  enum_value(OUTER_RINGS, "outer_rings", "Outer Rings (Aligned)")
  enum_value(SKEWED_GRID, "skewed_grid", "Skewed Grid (Aligned)")
  enum_value(WAVY_CIRCLES, "wavy_circles", "Wavy Circles (Aligned)")
  enum_value(RANDOM_OFFSET, "random_offset", "Random Offset (Aligned)")
  enum_value(CLASSIC_CENTERED, "classic_centered", "Classic")
  enum_value(DENSE_CENTERED, "dense_centered", "Dense")
  enum_value(EXPANDED_CENTERED, "expanded_centered", "Expanded")
  enum_value(SPIRAL_CENTERED, "spiral_centered", "Spiral")
  enum_value(SCALED_CIRCLES_CENTERED, "scaled_circles_centered", "Scaled Circles")
enum_end(variation_type2222)

property_enum(variation, _("Variation"),
              variation_type2222, variation_type2222,
              CLASSIC_CENTERED)
    description("")

property_enum(fill_mode, _("Fill Mode"),
              fill_mode_type2222, fill_mode_type2222,
              OUTLINES)
    description(_("Select the fill style"))
    ui_meta("visible", "variation {classic,sparse_grid,rotated_centers,scaled_circles,vesica_highlight,inner_circles,outer_rings,skewed_grid,wavy_circles,random_offset}")

property_double(tile_size, _("Tile Size"), 500.0)
    description(_("Size of the repeating tile in pixels"))
    value_range(50.0, 1000.0)
    ui_range(50.0, 1000.0)

property_double(circle_radius, _("Radius"), 0.5)
    description(_("Size of each circle"))
    value_range(0.0, 1.0)
    ui_range(0.1, 1.0)

property_double(outline_width, _("Outline Width"), 0.05)
    description(_("Thickness of lines in 'Outlines' mode"))
    value_range(0.00, 0.5)
    ui_range(0.00, 0.25)
    ui_meta("visible", "fill_mode {outlines} variation {classic_centered,dense_centered,expanded_centered,spiral_centered,scaled_circles_centered}")

property_double(rotation, _("Rotation"), 0.0)
    description(_("Rotate the pattern (degrees)"))
    value_range(0.0, 360.0)
    ui_range(0.0, 360.0)

property_double(offset_x, "X", 0.0)
    description((""))
    value_range(-50.0, 50.0)
    ui_range(-50.0, 50.0)

property_double(offset_y, "Y", 0.0)
    description((""))
    value_range(-50.0, 50.0)
    ui_range(-50.0, 50.0)

property_boolean(tileable, _("Tileable"), FALSE)
    description(_("Enable to make the Flower of Life pattern form a grid of repeating patterns"))
    ui_meta("visible", "variation {classic_centered,dense_centered,expanded_centered,spiral_centered,scaled_circles_centered}")

property_color(foreground_color, _("Foreground Color"), "#5BA9EA")
    description(_("Color of the circles or outlines"))

property_color(background_color, _("Background Color"), "#70D0FF")
    description(_("Color of the background"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     fol
#define GEGL_OP_C_SOURCE fol.c

#include "gegl-op.h"

// Simple pseudo-random number generator for Random Offset variation
static gdouble random_value(gint seed)
{
  seed = (seed * 1103515245 + 12345) & 0x7fffffff;
  return (gdouble)(seed % 1000) / 1000.0;
}

static void prepare(GeglOperation *operation)
{
  gegl_operation_set_format(operation, "input", babl_format("RGBA float"));
  gegl_operation_set_format(operation, "output", babl_format("RGBA float"));
}

static GeglRectangle get_bounding_box(GeglOperation *operation)
{
  return gegl_rectangle_infinite_plane();
}

static gboolean
process(GeglOperation        *operation,
        GeglOperationContext *context,
        const gchar          *output_prop,
        const GeglRectangle  *result,
        gint                 level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  GeglBuffer *output = gegl_operation_context_get_target(context, output_prop);
  GeglRectangle rect = *result;

  // Get color values
  gdouble fg_color[4], bg_color[4];
  gegl_color_get_rgba(o->foreground_color, &fg_color[0], &fg_color[1], &fg_color[2], &fg_color[3]);
  gegl_color_get_rgba(o->background_color, &bg_color[0], &bg_color[1], &bg_color[2], &bg_color[3]);

  // Get canvas dimensions
  GeglRectangle *canvas = gegl_operation_source_get_bounding_box(operation, "input");
  gdouble canvas_width = canvas ? canvas->width : result->width;
  gdouble canvas_height = canvas ? canvas->height : result->height;

  // Convert rotation to radians
  gdouble rotation_rad = o->rotation * G_PI / 180.0;
  gdouble cos_rot = cos(rotation_rad);
  gdouble sin_rot = sin(rotation_rad);

  // Circle radius and grid spacing
  gdouble tile_size = o->tile_size;
  gboolean is_original_variation = (o->variation >= CLASSIC && o->variation <= RANDOM_OFFSET);
  gdouble grid_spacing, radius, outline;

  if (is_original_variation)
  {
    // Original variations use grid_spacing = tile_size / 6.0
    grid_spacing = tile_size / 6.0;
    if (o->variation == SPARSE_GRID)
      grid_spacing *= 1.5; // Larger spacing
    radius = grid_spacing * o->circle_radius;
    outline = (o->fill_mode == OUTLINES) ? o->outline_width * grid_spacing : 0.0;
  }
  else
  {
    // Centered variations use grid_spacing = radius
    radius = tile_size * o->circle_radius;
    grid_spacing = radius;
    if (o->variation == DENSE_CENTERED)
      grid_spacing *= 0.75; // Tighter grid
    else if (o->variation == EXPANDED_CENTERED)
      grid_spacing *= 1.25; // Larger spacing
    outline = o->outline_width * radius;
  }

  // Tile dimensions
  gdouble tile_width = tile_size;
  gdouble tile_height = tile_size * sqrt(3.0) / 2.0;
  gdouble num_cells_x = ceil(tile_width / grid_spacing);
  gdouble num_cells_y = ceil(tile_height / (grid_spacing * sqrt(3.0) / 2.0));
  tile_width = num_cells_x * grid_spacing;
  tile_height = num_cells_y * grid_spacing * sqrt(3.0) / 2.0;

  // Center of the pattern (for centered variations)
  gdouble cx = canvas_width / 2.0 + o->offset_x;
  gdouble cy = canvas_height / 2.0 + o->offset_y;

  for (gint y = rect.y; y < rect.y + rect.height; y++)
  {
    for (gint x = rect.x; x < rect.x + rect.width; x++)
    {
      gdouble px = (gdouble)x;
      gdouble py = (gdouble)y;

      gfloat out[4] = {bg_color[0], bg_color[1], bg_color[2], 1.0};
      gboolean in_shape = FALSE;

      if (is_original_variation)
      {
        // Apply rotation around (0, 0) for original variations
        gdouble px_rot = px * cos_rot - py * sin_rot;
        gdouble py_rot = px * sin_rot + py * cos_rot;

        // Apply offsets
        gdouble offset_x = o->offset_x;
        gdouble offset_y = o->offset_y;

        // Map to tile space (always tiled)
        gdouble tx = fmod(px_rot + offset_x + tile_width, tile_width);
        gdouble ty = fmod(py_rot + offset_y + tile_height, tile_height);
        if (tx < 0) tx += tile_width;
        if (ty < 0) ty += tile_height;

        // Hexagonal grid coordinates
        gdouble s = grid_spacing;
        gdouble s_sqrt3 = s * sqrt(3.0);
        gdouble min_dist = tile_width;
        gdouble second_min_dist = tile_width;

        // Check nearby grid points
        gdouble max_radius = radius + outline;
        if (o->variation == INNER_CIRCLES || o->variation == OUTER_RINGS)
          max_radius *= 1.5;
        gint range = ceil(max_radius / s) + 1;

        for (gint i = -range; i <= range; i++)
        {
          for (gint j = -range; j <= range; j++)
          {
            // Base grid points
            gdouble cx = i * s;
            gdouble cy = j * s_sqrt3;
            gdouble cx_offset = (i + 0.5) * s;
            gdouble cy_offset = (j + 0.5) * s_sqrt3;

            // Apply variations to grid points
            switch (o->variation)
            {
              case SKEWED_GRID:
              {
                gdouble skew = 0.3 * s * sin((i + j) * 0.5);
                cx += skew;
                cx_offset += skew;
                break;
              }
              case ROTATED_CENTERS:
              {
                gdouble angle = (i + j) * 0.3;
                gdouble dx = 0.2 * s * cos(angle);
                gdouble dy = 0.2 * s * sin(angle);
                cx += dx; cy += dy;
                cx_offset += dx; cy_offset += dy;
                break;
              }
              case RANDOM_OFFSET:
              {
                gint seed = i * 1000 + j;
                gdouble dx = (random_value(seed) - 0.5) * 0.2 * s;
                gdouble dy = (random_value(seed + 1) - 0.5) * 0.2 * s;
                cx += dx; cy += dy;
                cx_offset += dx; cy_offset += dy;
                break;
              }
              default:
                break;
            }

            // Wrap grid points within tile
            cx = fmod(cx + tile_width, tile_width);
            cy = fmod(cy + tile_height, tile_height);
            cx_offset = fmod(cx_offset + tile_width, tile_width);
            cy_offset = fmod(cy_offset + tile_height, tile_height);
            if (cx < 0) cx += tile_width;
            if (cy < 0) cy += tile_height;
            if (cx_offset < 0) cx_offset += tile_width;
            if (cy_offset < 0) cy_offset += tile_height;

            // Distance to first grid point
            gdouble dx1 = tx - cx;
            gdouble dy1 = ty - cy;
            dx1 -= tile_width * round(dx1 / tile_width);
            dy1 -= tile_height * round(dy1 / tile_height);
            gdouble dist1 = sqrt(dx1 * dx1 + dy1 * dy1);

            // Distance to second grid point
            gdouble dx2 = tx - cx_offset;
            gdouble dy2 = ty - cy_offset;
            dx2 -= tile_width * round(dx2 / tile_width);
            dy2 -= tile_height * round(dy2 / tile_height);
            gdouble dist2 = sqrt(dx2 * dx2 + dy2 * dy2);

            // Update minimum and second-minimum distances
            if (dist1 < min_dist)
            {
              second_min_dist = min_dist;
              min_dist = dist1;
            }
            else if (dist1 < second_min_dist)
            {
              second_min_dist = dist1;
            }

            if (dist2 < min_dist)
            {
              second_min_dist = min_dist;
              min_dist = dist2;
            }
            else if (dist2 < second_min_dist)
            {
              second_min_dist = dist2;
            }

            // Apply variations
            gdouble local_radius = radius;

            switch (o->variation)
            {
              case SCALED_CIRCLES:
              {
                if ((i + j) % 2 == 0)
                  local_radius *= 0.7;
                break;
              }
              case WAVY_CIRCLES:
              {
                local_radius *= (1.0 + 0.3 * sin((i + j) * 0.5));
                break;
              }
              default:
                break;
            }

            // Rendering logic for original variations
            switch (o->variation)
            {
              case VESICA_HIGHLIGHT:
              {
                if (min_dist <= local_radius && second_min_dist <= local_radius)
                {
                  in_shape = TRUE;
                }
                break;
              }
              case INNER_CIRCLES:
              {
                if (min_dist <= local_radius * 0.5) // Inner circle
                {
                  in_shape = TRUE;
                }
                else if (o->fill_mode == OUTLINES && fabs(min_dist - local_radius * 0.5) < outline)
                {
                  in_shape = TRUE;
                }
                else if (min_dist <= local_radius) // Outer circle
                {
                  if (o->fill_mode == OUTLINES)
                  {
                    if (fabs(min_dist - local_radius) < outline)
                    {
                      in_shape = TRUE;
                    }
                  }
                  else
                  {
                    in_shape = TRUE;
                  }
                }
                break;
              }
              case OUTER_RINGS:
              {
                if (min_dist <= local_radius * 1.5) // Outer ring
                {
                  if (o->fill_mode == OUTLINES)
                  {
                    if (fabs(min_dist - local_radius * 1.5) < outline)
                    {
                      in_shape = TRUE;
                    }
                  }
                  else
                  {
                    in_shape = TRUE;
                  }
                }
                else if (min_dist <= local_radius) // Main circle
                {
                  if (o->fill_mode == OUTLINES)
                  {
                    if (fabs(min_dist - local_radius) < outline)
                    {
                      in_shape = TRUE;
                    }
                  }
                  else
                  {
                    in_shape = TRUE;
                  }
                }
                break;
              }
              default:
              {
                if (o->fill_mode == OUTLINES)
                {
                  if (fabs(min_dist - local_radius) < outline)
                  {
                    in_shape = TRUE;
                  }
                }
                else if (min_dist <= local_radius)
                {
                  in_shape = TRUE;
                }
                break;
              }
            }
          }
        }
      }
      else
      {
        // Centered variations
        gdouble px_rot = cos_rot * (px - cx) - sin_rot * (py - cy) + cx;
        gdouble py_rot = sin_rot * (px - cx) + cos_rot * (py - cy) + cy;

        // Check neighboring tiles if tileable
        gint tile_x_min = o->tileable ? -1 : 0;
        gint tile_x_max = o->tileable ? 1 : 0;
        gint tile_y_min = o->tileable ? -1 : 0;
        gint tile_y_max = o->tileable ? 1 : 0;

        for (gint tx = tile_x_min; tx <= tile_x_max && !in_shape; tx++)
        {
          for (gint ty = tile_y_min; ty <= tile_y_max && !in_shape; ty++)
          {
            // Offset center for this tile
            gdouble tile_cx = cx + tx * tile_width;
            gdouble tile_cy = cy + ty * tile_height;

            // Hexagonal grid coordinates
            gdouble s = grid_spacing;
            gdouble s_sqrt3_2 = s * sqrt(3.0) / 2.0;
            gdouble min_dist = tile_width;

            // Check nearby grid points
            gint range = ceil(radius / s) + 1;

            for (gint i = -range; i <= range; i++)
            {
              for (gint j = -range; j <= range; j++)
              {
                // Hexagonal grid: offset every other row
                gdouble grid_x = i * s + (j % 2) * (s / 2.0);
                gdouble grid_y = j * s_sqrt3_2;
                gdouble local_radius = radius;

                // Apply variations
                switch (o->variation)
                {
                  case SPIRAL_CENTERED:
                  {
                    // Compute distance from tile center
                    gdouble dx = grid_x;
                    gdouble dy = grid_y;
                    gdouble r = sqrt(dx * dx + dy * dy);
                    gdouble theta = 0.05 * r; // Spiral tightness
                    grid_x += 0.1 * s * cos(theta);
                    grid_y += 0.1 * s * sin(theta);
                    break;
                  }
                  case SCALED_CIRCLES_CENTERED:
                  {
                    if ((i + j) % 2 == 0)
                      local_radius *= 0.7;
                    break;
                  }
                  default:
                    break;
                }

                // Grid point position relative to tile center
                gdouble cx_grid = tile_cx + grid_x;
                gdouble cy_grid = tile_cy + grid_y;

                // Distance to grid point
                gdouble dx = px_rot - cx_grid;
                gdouble dy = py_rot - cy_grid;

                // Adjust for tiling if enabled
                if (o->tileable)
                {
                  dx -= tile_width * round(dx / tile_width);
                  dy -= tile_height * round(dy / tile_height);
                }

                gdouble dist = sqrt(dx * dx + dy * dy);

                // Update minimum distance
                if (dist < min_dist)
                {
                  min_dist = dist;
                }

                // Check if pixel is on outline (centered variations always use outlines)
                if (fabs(dist - local_radius) < outline)
                {
                  in_shape = TRUE;
                }
              }
            }
          }
        }
      }

      if (in_shape)
      {
        out[0] = fg_color[0];
        out[1] = fg_color[1];
        out[2] = fg_color[2];
        out[3] = 1.0;
      }

      gegl_buffer_set(output, GEGL_RECTANGLE(x, y, 1, 1), 0, babl_format("RGBA float"), out, GEGL_AUTO_ROWSTRIDE);
    }
  }

  return TRUE;
}

static void
gegl_op_class_init(GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS(klass);
  operation_class->prepare = prepare;
  operation_class->get_bounding_box = get_bounding_box;
  operation_class->process = process;

  gegl_operation_class_set_keys(operation_class,
    "name",        "ai/lb:flower-of-life",
    "title",       _("Flower of Life"),
    "reference-hash", "floweroflifegegl2025",
    "description", _("Renders a seamless Flower of Life pattern with adjustable variations, tile size, circle radius, outline width, rotation, offsets, and colors"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL/Geometric Lines",
    "gimp:menu-label", _("Flower of Life..."),
    NULL);
}

#endif
