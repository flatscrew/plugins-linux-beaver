#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start(enum_list_name2394)
  enum_value(PATTERN_GRID, "grid", "Grid")
  enum_value(PATTERN_DIAMOND_GRID, "diamond_grid", "Diamond Grid")
  enum_value(PATTERN_SPIRAL, "spiral", "Spiral")
  enum_value(PATTERN_EYE_MOSAIC, "eye_mosaic", "Eye Mosaic")
  enum_value(PATTERN_BAROQUE_CURLS, "baroque_curls", "Baroque Curls")
  enum_value(PATTERN_FLORAL_MOTIFS, "floral_motifs", "Floral Motifs")
  enum_value(PATTERN_MARBLE_VEINS, "marble_veins", "Marble Veins")
  enum_value(PATTERN_CIRCLE_GRID, "circle_grid", "Circle Grid")
  enum_value(PATTERN_OVAL_WAVES, "oval_waves", "Oval Waves")
  enum_value(PATTERN_PEBBLE_GRID, "pebble_grid", "Pebble Grid")
  enum_value(PATTERN_CONCENTRIC_CIRCLES, "concentric_circles", "Concentric Circles")
  enum_value(PATTERN_TRIANGLE_SPIRAL, "triangle_spiral", "Triangle Spiral")
  enum_value(PATTERN_ARC_WAVES, "arc_waves", "Arc Waves")
  enum_value(PATTERN_SQUARE_TILES, "square_tiles", "Square Tiles")
  enum_value(PATTERN_BLINDS, "blinds", "Blinds")
  enum_value(PATTERN_STITCH_LINES, "stitch_lines", "Stitch Lines")
  enum_value(PATTERN_CROSS_STITCHES, "cross_stitches", "Cross Stitches")
  enum_value(PATTERN_STITCH_WEAVE, "stitch_weave", "Stitch Weave")
  enum_value(PATTERN_STITCH_MOSAIC, "stitch_mosaic", "Stitch Mosaic")
  enum_value(PATTERN_THREE_COLOR_STRIPES, "three_color_stripes", "Three Color Stripes")
  enum_value(PATTERN_LONG_PEBBLE_GRID, "long_pebble_grid", "Long Pebble Grid")
  enum_value(PATTERN_CUBE_GRID, "cube_grid", "Cube Grid")
  enum_value(PATTERN_RADIAL_LINES, "radial_lines", "Radial Lines")
  enum_value(PATTERN_RADIAL_BURST, "radial_burst", "Radial Burst")
  enum_value(PATTERN_ORGANIC_GRID, "organic_grid", "Organic Grid")
  enum_value(PATTERN_PSYCHEDELIC_SWIRLS, "psychedelic_swirls", "Psychedelic Swirls")
  enum_value(PATTERN_ORNAMENTAL_LATTICE, "ornamental_lattice", "Ornamental Lattice")
  enum_value(PATTERN_CIRCUIT_TRACES, "circuit_traces", "Circuit Traces")
  enum_value(PATTERN_CRYSTAL_FACETS, "crystal_facets", "Crystal Facets")
  enum_value(PATTERN_ORGANIC_SHARDS, "organic_shards", "Organic Shards")
  enum_value(PATTERN_CHAOS_WEBS, "chaos_webs", "Chaos Webs")
  enum_value(PATTERN_NEBULA_CLOUDS, "nebula_clouds", "Nebula Clouds")
  enum_value(PATTERN_OVAL_GRID, "oval_grid", "Oval Grid")
enum_end(enumlistname2394)

property_enum(pattern, _("Pattern Type"),
              enumlistname2394, enum_list_name2394, PATTERN_GRID)
    description(_("Select the pattern type to render"))

property_color(color1, _("Primary Color"), "#32e0f8")
    description(_("First color of the pattern"))

property_color(color2, _("Secondary Color"), "#00ffff")
    description(_("Second color of the pattern"))

property_color(color3, _("Tertiary Color"), "#c0d8ff")
    description(_("Third color of the pattern"))

property_double(scale, _("Scale"), 1.0)
    description(_("Zoom level of the pattern (smaller values zoom in, larger values zoom out)"))
    value_range(1.0, 10.0)

property_double(size_1, _("Shape Size 1"), 1.0)
    description(_("Thickness of the primary color region"))
    value_range(1.0, 7.0)

property_double(size_2, _("Shape Size 2"), 1.0)
    description(_("Thickness of the secondary color outline"))
    value_range(1.0, 7.0)

property_double(line_size_3, _("Line Size 3"), 1.0)
    description(_("Thickness of the tertiary color region"))
    value_range(0.1, 5.0)
  ui_meta     ("role", "output-extent")

property_double(rotation, _("Rotation"), 0.0)
    description(_("Rotation angle of the pattern in degrees"))
    value_range(0.0, 360.0)

property_double(x_offset, _("X Offset"), 0.0)
    description(_("Horizontal movement of the pattern in pixels"))
    value_range(-1000.0, 1000.0)

property_double(y_offset, _("Y Offset"), 0.0)
    description(_("Vertical movement of the pattern in pixels"))
    value_range(-1000.0, 1000.0)

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     tricolorpattern
#define GEGL_OP_C_SOURCE tricolorpattern.c

#include <gegl-op.h>

static void
prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "input", babl_format ("RGBA float"));
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
}

static gboolean
process (GeglOperation *operation,
         void *in_buf,
         void *out_buf,
         glong samples,
         const GeglRectangle *roi,
         gint level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  gfloat *out = out_buf;
  gdouble r1, g1, b1, r2, g2, b2, r3, g3, b3;
  gdouble rad = o->rotation * G_PI / 180.0;
  gdouble scale = o->scale;
  gdouble outline_scale_1 = o->size_1;
  gdouble outline_scale_2 = o->size_2;
  gdouble outline_scale_3 = o->line_size_3;

  gegl_color_get_rgba (o->color1, &r1, &g1, &b1, NULL);
  gegl_color_get_rgba (o->color2, &r2, &g2, &b2, NULL);
  gegl_color_get_rgba (o->color3, &r3, &g3, &b3, NULL);

  for (gint y = roi->y; y < roi->y + roi->height; y++)
    {
      for (gint x = roi->x; x < roi->x + roi->width; x++)
        {
          gdouble nx = x * cos (rad) - y * sin (rad) - o->x_offset;
          gdouble ny = x * sin (rad) + y * cos (rad) - o->y_offset;
          gdouble val = 0.0;
          gint index = ((y - roi->y) * roi->width + (x - roi->x)) * 4;

          switch (o->pattern)
            {
            case PATTERN_GRID:
              val = sin (nx / (20.0 * scale)) * cos (ny / (20.0 * scale));
              break;
            case PATTERN_DIAMOND_GRID:
              val = sin (nx / (10.0 * scale)) + sin (ny / (10.0 * scale));
              break;
            case PATTERN_SPIRAL:
              {
                gdouble r = sqrt (nx * nx + ny * ny) / (20.0 * scale);
                gdouble theta = atan2 (ny, nx);
                val = sin (r + theta);
              }
              break;
            case PATTERN_EYE_MOSAIC:
              {
                gdouble px = nx / (30.0 * scale);
                gdouble py = ny / (30.0 * scale);
                val = sin (px * G_PI) + cos (py * G_PI * sqrt (3.0));
              }
              break;
            case PATTERN_BAROQUE_CURLS:
              val = sin (nx / (20.0 * scale) + sin (ny / (20.0 * scale))) * cos (ny / (20.0 * scale));
              break;
            case PATTERN_FLORAL_MOTIFS:
              {
                gdouble r = sqrt (nx * nx + ny * ny) / (20.0 * scale);
                gdouble theta = atan2 (ny, nx);
                val = sin (5.0 * theta) * cos (r);
              }
              break;
            case PATTERN_MARBLE_VEINS:
              val = sin (nx / (20.0 * scale) + 2.0 * sin (ny / (20.0 * scale)));
              break;
            case PATTERN_CIRCLE_GRID:
              {
                gdouble px = fmod (nx / (20.0 * scale), 1.0);
                gdouble py = fmod (ny / (20.0 * scale), 1.0);
                val = cos (G_PI * sqrt ((px - 0.5) * (px - 0.5) + (py - 0.5) * (py - 0.5)));
              }
              break;
            case PATTERN_OVAL_WAVES:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = ny / (20.0 * scale) + sin (nx / (20.0 * scale));
                val = cos (G_PI * sqrt (px * px + 0.5 * py * py));
              }
              break;
            case PATTERN_PEBBLE_GRID:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = ny / (20.0 * scale) * sqrt (3.0) / 2.0;
                val = sin (px * G_PI) * cos (py * G_PI);
              }
              break;
            case PATTERN_CONCENTRIC_CIRCLES:
              {
                gdouble r = sqrt (nx * nx + ny * ny) / (20.0 * scale);
                val = sin (r * G_PI);
              }
              break;
            case PATTERN_TRIANGLE_SPIRAL:
              {
                gdouble r = sqrt (nx * nx + ny * ny) / (20.0 * scale);
                gdouble theta = atan2 (ny, nx);
                val = sin (r + 3.0 * theta);
              }
              break;
            case PATTERN_ARC_WAVES:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = ny / (20.0 * scale);
                val = sin (px + cos (py * G_PI));
              }
              break;
            case PATTERN_SQUARE_TILES:
              {
                gdouble px = fmod (nx / (15.0 * scale), 1.0);
                gdouble py = fmod (ny / (15.0 * scale), 1.0);
                val = cos (G_PI * (fabs(px - 0.5) + fabs(py - 0.5)));
              }
              break;
            case PATTERN_BLINDS:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = nx / (20.0 * scale);
                val = sin (px + sin (py * 2.0));
              }
              break;
            case PATTERN_STITCH_LINES:
              {
                gdouble px = fmod (nx / (15.0 * scale), 1.0);
                gdouble py = ny / (15.0 * scale);
                val = px < 0.3 ? cos (G_PI * py) : 0.0;
              }
              break;
            case PATTERN_CROSS_STITCHES:
              {
                gdouble px = fmod (nx / (15.0 * scale), 1.0);
                gdouble py = fmod (ny / (15.0 * scale), 1.0);
                val = (fabs(px - py) < 0.2 || fabs(px + py - 1.0) < 0.2) ? cos (G_PI * px) : 0.0;
              }
              break;
            case PATTERN_STITCH_WEAVE:
              {
                gdouble px = nx / (15.0 * scale);
                gdouble py = ny / (15.0 * scale);
                val = (fabs(px - floor(px) - 0.5) < 0.2 || fabs(py - floor(py) - 0.5) < 0.2) ? cos (G_PI * px) : 0.0;
              }
              break;
            case PATTERN_STITCH_MOSAIC:
              {
                gdouble px = fmod (nx / (15.0 * scale), 1.0);
                gdouble py = fmod (ny / (15.0 * scale), 1.0);
                val = (px < 0.3 || py < 0.3) ? cos (G_PI * (px + py)) : 0.0;
              }
              break;
            case PATTERN_THREE_COLOR_STRIPES:
              {
                gdouble px = nx / (15.0 * scale);
                gdouble py = ny / (15.0 * scale);
                val = cos (G_PI * fabs(fmod(px - py, 2.0) - 1.0));
              }
              break;
            case PATTERN_LONG_PEBBLE_GRID:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = ny / (20.0 * scale) * sqrt (3.0);
                val = cos (G_PI * (px + py)) + cos (G_PI * (px - py));
              }
              break;
            case PATTERN_CUBE_GRID:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = ny / (20.0 * scale);
                val = cos (G_PI * (fabs(px - py) + fabs(px + py)));
              }
              break;
            case PATTERN_RADIAL_LINES:
              {
                gdouble theta = atan2 (ny, nx);
                val = cos (10.0 * theta);
              }
              break;
            case PATTERN_RADIAL_BURST:
              {
                gdouble theta = atan2 (ny, nx);
                gdouble r = sqrt (nx * nx + ny * ny) / (20.0 * scale);
                val = cos (12.0 * theta + floor(r));
              }
              break;
            case PATTERN_ORGANIC_GRID:
              {
                gdouble px = nx / (20.0 * scale) + 0.2 * cos (ny / (20.0 * scale));
                gdouble py = ny / (20.0 * scale) + 0.2 * cos (nx / (20.0 * scale));
                val = cos (G_PI * (fabs(px - floor(px)) + fabs(py - floor(py))));
              }
              break;
            case PATTERN_PSYCHEDELIC_SWIRLS:
              {
                gdouble r = sqrt (nx * nx + ny * ny) / (20.0 * scale);
                gdouble theta = atan2 (ny, nx);
                val = cos (G_PI * (r + 2.0 * theta));
              }
              break;
            case PATTERN_ORNAMENTAL_LATTICE:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = ny / (20.0 * scale);
                val = cos (G_PI * (cos(px) + cos(py)));
              }
              break;
            case PATTERN_CIRCUIT_TRACES:
              {
                gdouble px = nx / (15.0 * scale);
                gdouble py = ny / (15.0 * scale);
                val = cos (G_PI * (fabs(px - floor(px)) + fabs(py - floor(py + 0.5))));
              }
              break;
            case PATTERN_CRYSTAL_FACETS:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = ny / (20.0 * scale);
                val = cos (G_PI * (fabs(px - py) * fabs(px + py)));
              }
              break;
            case PATTERN_ORGANIC_SHARDS:
              {
                gdouble px = nx / (25.0 * scale);
                gdouble py = ny / (25.0 * scale);
                val = cos (G_PI * (px + 0.5 * cos(py * 1.5)) + cos (py + 0.5 * cos(px * 1.5)));
              }
              break;
            case PATTERN_CHAOS_WEBS:
              {
                gdouble px = nx / (20.0 * scale);
                gdouble py = ny / (20.0 * scale);
                val = cos (G_PI * (cos(px + 0.3 * cos(py)) + cos(py + 0.3 * cos(px))));
              }
              break;
            case PATTERN_NEBULA_CLOUDS:
              {
                gdouble px = nx / (25.0 * scale);
                gdouble py = ny / (25.0 * scale);
                val = cos (G_PI * (px * cos(py * 0.5) + py * cos(px * 0.5)));
              }
              break;
            case PATTERN_OVAL_GRID:
              {
                gdouble px = fmod (nx / (20.0 * scale), 1.0);
                gdouble py = fmod (ny / (20.0 * scale), 1.0);
                val = cos (G_PI * sqrt((px - 0.5) * (px - 0.5) + (py - 0.5) * (py - 0.5) + 0.2 * cos(2.0 * G_PI * px)));
              }
              break;
            default:
              val = 0.0;
            }

          gdouble threshold_1 = 0.33 / outline_scale_1;
          gdouble threshold_2 = -0.33 / outline_scale_2;
          gdouble threshold_3 = -0.33 / outline_scale_3;

          if (val > threshold_1)
            {
              out[index] = r1;
              out[index + 1] = g1;
              out[index + 2] = b1;
              out[index + 3] = 1.0;
            }
          else if (val > threshold_2)
            {
              out[index] = r2;
              out[index + 1] = g2;
              out[index + 2] = b2;
              out[index + 3] = 1.0;
            }
          else if (val <= threshold_3)
            {
              out[index] = r3;
              out[index + 1] = g3;
              out[index + 2] = b3;
              out[index + 3] = 1.0;
            }
          else
            {
              out[index] = r3;
              out[index + 1] = g3;
              out[index + 2] = b3;
              out[index + 3] = 1.0;
            }
        }
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
      "name",        "ai/lb:tri-pattern-collection",
      "title",       _("TriColor Pattern Collection"),
      "reference-hash", "fairygeglsdeluxepatternsgegl1234",
      "description", _("Creates a variety of three color patterns with adjustable zoom, with region thickness, rotation, and position"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("TriColor Pattern Collection..."),
      NULL);
}

#endif
