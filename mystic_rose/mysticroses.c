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
 * 2025 Beaver modifying mostly Grok's work.
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <math.h>
#include <gegl.h>
#include <gegl-plugin.h>

#ifdef GEGL_PROPERTIES

enum_start(gegl_mystic_rose_variation)
  enum_value(GEGL_MYSTIC_ROSE_CLASSIC, "classic", N_("Classic Mystic Rose"))
  enum_value(GEGL_MYSTIC_ROSE_WAVE, "wave", N_("Wavy Rose"))
  enum_value(GEGL_MYSTIC_ROSE_CLUSTER, "cluster", N_("Clustered Rose"))
  enum_value(GEGL_MYSTIC_ROSE_GRID, "grid", N_("Grid Rose"))
  enum_value(GEGL_MYSTIC_ROSE_OFFSET, "offset", N_("Offset Rings"))
  enum_value(GEGL_MYSTIC_ROSE_DENSE, "dense", N_("Dense Core"))
  enum_value(GEGL_MYSTIC_ROSE_RANDOM, "random", N_("Random Links"))
  enum_value(GEGL_MYSTIC_ROSE_KNOT, "knot", N_("Knotted Rose"))
  enum_value(GEGL_MYSTIC_ROSE_RIPPLE, "ripple", N_("Ripple Rose"))
  enum_value(GEGL_MYSTIC_ROSE_SWIRL, "swirl", N_("Swirl Rose"))
  enum_value(GEGL_MYSTIC_ROSE_CROSS, "cross", N_("Cross Links"))
enum_end(GeglMysticRoseVariation)

property_enum(variation, _("Variation"),
              GeglMysticRoseVariation, gegl_mystic_rose_variation,
              GEGL_MYSTIC_ROSE_CLASSIC)
    description(_("Select a variation of the Mystic Rose pattern"))

property_int(points, _("Number of Points"), 12)
    description(_("Number of vertices in the Mystic Rose"))
    value_range(3, 50)
    ui_range(3, 30)

property_double(line_thickness, _("Line Thickness"), 5.0)
    description(_("Thickness of the Mystic Rose lines"))
    value_range(0.5, 10.0)
    ui_range(0.5, 10.0)

property_color(rose_color, _("Rose Color"), "#ffffff")
    description(_("Color of the Mystic Rose lines"))

property_boolean(bg_enabled, _("Enable Background Color"), TRUE)
    description(_("Enable or disable the background color"))

property_color(bg_color, _("Background Color"), "#0094ff")
    description(_("Color of the background (only visible if enabled)"))
    ui_meta("visible", "bg_enabled")

property_double(rotation, _("Rotation"), 0.0)
    description(_("Rotation angle of the Mystic Rose in degrees"))
    value_range(0.0, 360.0)
    ui_range(0.0, 360.0)
    ui_meta("unit", "degree")
    ui_meta("direction", "ccw")

property_double(scale, _("Scale"), 1.0)
    description(_("Scale of the Mystic Rose (1.0 is full size, 0.5 is half size, etc.)"))
    value_range(0.1, 3.0)
    ui_range(0.1, 3.0)

property_boolean(tileable, _("Tileable"), FALSE)
    description(_("Enable to make the Mystic Rose pattern form a grid of repeating patterns"))

property_double(x_offset, _("X Offset"), 0.0)
    description(_("Horizontal offset of the Mystic Rose pattern (in pixels)"))
    value_range(-1000.0, 1000.0)
    ui_range(-500.0, 500.0)
    ui_meta("unit", "pixel")

property_double(y_offset, _("Y Offset"), 0.0)
    description(_("Vertical offset of the Mystic Rose pattern (in pixels)"))
    value_range(-1000.0, 1000.0)
    ui_range(-500.0, 500.0)
    ui_meta("unit", "pixel")

property_double(inner_radius, _("Inner Radius"), 100.0)
    description(_("Radius of the inner circle as a percentage of the outer radius"))
    value_range(0.0, 100.0)
    ui_range(0.0, 100.0)

property_double(tuning, _("Tuning"), 0.0)
    description(_("Adjusts the step distance for connecting vertices (0 to use variation-based connections)"))
    value_range(0.0, 100.0)
    ui_range(0.0, 100.0)

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     mystic_roses
#define GEGL_OP_C_SOURCE mysticroses.c

#include "gegl-op.h"

/* Custom struct for vertex coordinates */
typedef struct {
  gfloat x;
  gfloat y;
} Point;

static void
prepare(GeglOperation *operation)
{
  gegl_operation_set_format(operation, "input", babl_format("RGBA float"));
  gegl_operation_set_format(operation, "output", babl_format("RGBA float"));
}

static gboolean
process(GeglOperation *operation,
        void *in_buf,
        void *out_buf,
        glong n_pixels,
        const GeglRectangle *roi,
        gint level)
{
  GeglProperties *o = GEGL_PROPERTIES(operation);
  gfloat *out_pixel = (gfloat *)out_buf;

  /* Get colors */
  gfloat rose[4], bg[4];
  gegl_color_get_pixel(o->rose_color, babl_format("RGBA float"), rose);
  gegl_color_get_pixel(o->bg_color, babl_format("RGBA float"), bg);

  /* Get canvas dimensions */
  GeglRectangle *canvas = gegl_operation_source_get_bounding_box(operation, "input");
  gfloat canvas_width = canvas ? canvas->width : roi->width;
  gfloat canvas_height = canvas ? canvas->height : roi->height;

  /* Center and radius for a single tile */
  gfloat cx = canvas_width / 2.0 + o->x_offset;
  gfloat cy = canvas_height / 2.0 + o->y_offset;
  gfloat tile_size = MIN(canvas_width, canvas_height) / (o->tileable ? 2.0 : 1.0);
  gfloat base_radius = tile_size / 2.0 - o->line_thickness;
  gfloat radius = base_radius * o->scale; /* Outer radius */
  gfloat inner_rad = radius * o->inner_radius / 100.0; /* Inner radius as percentage of outer */

  /* Convert rotation to radians */
  gfloat rotation_rad = o->rotation * G_PI / 180.0;

  /* Compute vertex positions and connections based on variation */
  Point *vertices = g_new(Point, o->points);
  gboolean *connect = g_new(gboolean, o->points * o->points); /* Connection matrix */
  for (gint i = 0; i < o->points * o->points; i++) connect[i] = FALSE;

  /* Base vertex placement (modified by variation and inner radius) */
  for (gint i = 0; i < o->points; i++)
    {
      gfloat angle = 2.0 * G_PI * i / o->points + rotation_rad;
      gfloat r = (i % 2 == 0) ? radius : inner_rad; /* Alternate between outer and inner radius */

      switch (o->variation)
        {
        case GEGL_MYSTIC_ROSE_WAVE: /* Wavy radius */
          r *= (1.0 + 0.2 * sin(6.0 * angle));
          break;
        case GEGL_MYSTIC_ROSE_CLUSTER: /* Clustered vertices */
          angle += 0.1 * sin(3.0 * angle);
          break;
        case GEGL_MYSTIC_ROSE_OFFSET: /* Two rings (now additive with inner_radius) */
          r *= (i % 2 == 0) ? 1.0 : 0.8;
          break;
        case GEGL_MYSTIC_ROSE_DENSE: /* Smaller inner radius (now additive) */
          r *= (1.0 - 0.5 * (i % 2));
          break;
        case GEGL_MYSTIC_ROSE_SWIRL: /* Swirled angles */
          angle += 0.5 * i / o->points;
          break;
        case GEGL_MYSTIC_ROSE_RIPPLE: /* Rippled radius */
          r *= (1.0 + 0.15 * cos(8.0 * angle));
          break;
        default: /* Classic, Grid, Random, Knotted, Cross use base radius */
          break;
        }

      vertices[i].x = cx + r * cos(angle);
      vertices[i].y = cy + r * sin(angle);
    }

  /* Define connections based on tuning or variation */
  if (o->tuning > 0.0)
    {
      /* Map tuning (0.0 to 100.0) to step (1 to points/2) */
      gint step = (gint)(o->tuning * (o->points / 2 - 1) / 100.0) + 1;
      for (gint i = 0; i < o->points; i++)
        {
          gint j = (i + step) % o->points;
          connect[i * o->points + j] = TRUE;
          connect[j * o->points + i] = TRUE; /* Symmetric */
        }
    }
  else
    {
      /* Use variation-based connections */
      for (gint j = 0; j < o->points; j++)
        {
          for (gint k = j + 1; k < o->points; k++)
            {
              gboolean should_connect = FALSE;
              switch (o->variation)
                {
                case GEGL_MYSTIC_ROSE_CLASSIC:
                case GEGL_MYSTIC_ROSE_WAVE:
                case GEGL_MYSTIC_ROSE_CLUSTER:
                case GEGL_MYSTIC_ROSE_OFFSET:
                case GEGL_MYSTIC_ROSE_DENSE:
                case GEGL_MYSTIC_ROSE_RIPPLE:
                case GEGL_MYSTIC_ROSE_SWIRL:
                  should_connect = TRUE;
                  break;
                case GEGL_MYSTIC_ROSE_GRID:
                  should_connect = ((k - j) % 2 == 0);
                  break;
                case GEGL_MYSTIC_ROSE_RANDOM:
                  should_connect = (g_random_double() < 0.3);
                  break;
                case GEGL_MYSTIC_ROSE_KNOT:
                  should_connect = ((k - j) % 3 == 0);
                  break;
                case GEGL_MYSTIC_ROSE_CROSS:
                  should_connect = (abs(k - j) % (o->points / 4) == 0);
                  break;
                }

              connect[j * o->points + k] = should_connect;
              connect[k * o->points + j] = should_connect;
            }
        }
    }

  /* Half thickness for distance comparison */
  gfloat half_thickness = o->line_thickness / 2.0;

  for (glong i = 0; i < n_pixels; i++)
    {
      /* Compute global coordinates */
      gfloat x = (i % roi->width) + roi->x;
      gfloat y = (i / roi->width) + roi->y;

      gboolean on_line = FALSE;

      /* Check neighboring tiles for tiling */
      gint tile_x_min = o->tileable ? -1 : 0;
      gint tile_x_max = o->tileable ? 1 : 0;
      gint tile_y_min = o->tileable ? -1 : 0;
      gint tile_y_max = o->tileable ? 1 : 0;

      for (gint tx = tile_x_min; tx <= tile_x_max && !on_line; tx++)
        {
          for (gint ty = tile_y_min; ty <= tile_y_max && !on_line; ty++)
            {
              /* Offset center for this tile */
              gfloat tile_cx = cx + tx * tile_size;
              gfloat tile_cy = cy + ty * tile_size;

              /* Check distance to each line segment in this tile */
              for (gint j = 0; j < o->points && !on_line; j++)
                {
                  for (gint k = j + 1; k < o->points && !on_line; k++)
                    {
                      if (!connect[j * o->points + k]) continue;

                      gfloat x1 = vertices[j].x + tx * tile_size;
                      gfloat y1 = vertices[j].y + ty * tile_size;
                      gfloat x2 = vertices[k].x + tx * tile_size;
                      gfloat y2 = vertices[k].y + ty * tile_size;

                      gfloat dx = x2 - x1;
                      gfloat dy = y2 - y1;
                      gfloat length_sq = dx * dx + dy * dy;

                      if (length_sq < 1e-6) continue; /* Skip degenerate lines */

                      /* Project point (x, y) onto line segment */
                      gfloat t = ((x - x1) * dx + (y - y1) * dy) / length_sq;
                      t = CLAMP(t, 0.0, 1.0);

                      /* Closest point on segment */
                      gfloat closest_x = x1 + t * dx;
                      gfloat closest_y = y1 + t * dy;

                      /* Distance from pixel to closest point */
                      gfloat dist_x = x - closest_x;
                      gfloat dist_y = y - closest_y;
                      gfloat dist = sqrt(dist_x * dist_x + dist_y * dist_y);

                      if (dist <= half_thickness)
                        {
                          on_line = TRUE;
                        }
                    }
                }
            }
        }

      if (on_line)
        {
          out_pixel[0] = rose[0];
          out_pixel[1] = rose[1];
          out_pixel[2] = rose[2];
          out_pixel[3] = rose[3];
        }
      else
        {
          out_pixel[0] = o->bg_enabled ? bg[0] : 0.0;
          out_pixel[1] = o->bg_enabled ? bg[1] : 0.0;
          out_pixel[2] = o->bg_enabled ? bg[2] : 0.0;
          out_pixel[3] = o->bg_enabled ? bg[3] : 0.0;
        }

      out_pixel += 4;
    }

  g_free(vertices);
  g_free(connect);
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
    "name",        "ai/lb:mystic-rose",
    "title",       _("Mystic Rose"),
    "reference-hash", "mysticrosegegl005", /* Updated for inner_radius and tuning */
    "description", _("Renders a Mystic Rose pattern or creative variations, optionally tiled in a grid with adjustable position and connection tuning"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL/Geometric Lines",
    "gimp:menu-label", _("Mystic Rose..."),
    NULL);
}

#endif
