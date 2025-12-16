 /*
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
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/.
 *
 * Credit to Øyvind Kolås (pippin) for major GEGL contributions
 * 2025 Beaver modifying mostly Grok's work. Deep Seek helped a little too
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_frame_mask_style13123)
  enum_value (GEGL_FRAME_MASK_STYLE_1, "style1",  N_("Fractal Spiral"))
  enum_value (GEGL_FRAME_MASK_STYLE_2, "style2",  N_("Radial Mosaic"))
  enum_value (GEGL_FRAME_MASK_STYLE_3, "style3",  N_("Wavy Petals"))
  enum_value (GEGL_FRAME_MASK_STYLE_4, "style4",  N_("Star Cluster"))
  enum_value (GEGL_FRAME_MASK_STYLE_5, "style5",  N_("Geometric Lattice"))
  enum_value (GEGL_FRAME_MASK_STYLE_6, "style6",  N_("Organic Bloom"))
  enum_value (GEGL_FRAME_MASK_STYLE_7, "style7",  N_("Cracked Earth"))
  enum_value (GEGL_FRAME_MASK_STYLE_8, "style8",  N_("Pixelated Swirl"))
  enum_value (GEGL_FRAME_MASK_STYLE_9, "style9",  N_("Celestial Orbit"))
  enum_value (GEGL_FRAME_MASK_STYLE_10, "style10", N_("Crystal Shard"))
  enum_value (GEGL_FRAME_MASK_STYLE_11, "style11", N_("Feathered Edge"))
  enum_value (GEGL_FRAME_MASK_STYLE_12, "style12", N_("Marble Veins"))
  enum_value (GEGL_FRAME_MASK_STYLE_13, "style13", N_("Kaleidoscope Burst"))
  enum_value (GEGL_FRAME_MASK_STYLE_14, "style14", N_("Frosted Glass"))
  enum_value (GEGL_FRAME_MASK_STYLE_15, "style15", N_("Rustic Weave"))
  enum_value (GEGL_FRAME_MASK_STYLE_16, "style16", N_("Neon Glow"))
  enum_value (GEGL_FRAME_MASK_STYLE_17, "style17", N_("Sand Dune"))
  enum_value (GEGL_FRAME_MASK_STYLE_18, "style18", N_("Ink Splash"))
  enum_value (GEGL_FRAME_MASK_STYLE_19, "style19", N_("Cosmic Dust"))
  enum_value (GEGL_FRAME_MASK_STYLE_20, "style20", N_("Glacial Rift"))
  enum_value (GEGL_FRAME_MASK_STYLE_21, "style21", N_("Tropical Vine"))
  enum_value (GEGL_FRAME_MASK_STYLE_22, "style22", N_("Aurora Wave"))
  enum_value (GEGL_FRAME_MASK_STYLE_23, "style23", N_("Stained Glass"))
  enum_value (GEGL_FRAME_MASK_STYLE_24, "style24", N_("Desert Mirage"))
enum_end (GeglFrameMaskStyle13123)

property_enum (mask_style, _("Frame Mask Style"),
               GeglFrameMaskStyle13123, gegl_frame_mask_style13123,
               GEGL_FRAME_MASK_STYLE_1)
    description (_("Select one of the 24 predefined frame mask styles inspired by FastStone"))

property_color (mask_color, _("Mask Color"), "black")
    description (_("Color of the frame mask"))

property_int (border_width, _("Border Width"), 15)
    description (_("Width of the frame border in pixels"))
    value_range (1, 200)
    ui_range (1, 200)

property_double (pattern_density, _("Pattern Density"), 10.0)
    description (_("Density of the pattern (higher = more detailed)"))
    value_range (1.0, 50.0)
    ui_range (1.0, 50.0)
    ui_meta ("visible", "mask_style {style1, style2, style3, style4, style5, style6, style7, style8, style9, style10, style11, style12, style13, style14, style15, style16, style17, style18, style19, style20, style21, style22, style23, style24}")

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     border2
#define GEGL_OP_C_SOURCE border2.c

#include "gegl-op.h"

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
  gfloat *in_pixel = (gfloat *) in_buf;
  gfloat *out_pixel = (gfloat *) out_buf;

  if (!in_buf || !out_buf || !roi || n_pixels <= 0)
    return FALSE;

  gfloat mask_color[4];
  gegl_color_get_pixel (o->mask_color, babl_format ("RGBA float"), mask_color);

  GeglRectangle *canvas = gegl_operation_source_get_bounding_box (operation, "input");
  if (!canvas || canvas->width <= 0 || canvas->height <= 0)
    return FALSE;

  gint canvas_width = canvas->width;
  gint canvas_height = canvas->height;
  gfloat bw = (gfloat) o->border_width;
  gfloat center_x = canvas_width / 2.0;
  gfloat center_y = canvas_height / 2.0;

  for (glong i = 0; i < n_pixels; i++)
  {
    gint x = (i % roi->width) + roi->x;
    gint y = (i / roi->width) + roi->y;

    if (x < 0 || x >= canvas_width || y < 0 || y >= canvas_height)
    {
      out_pixel[0] = in_pixel[0];
      out_pixel[1] = in_pixel[1];
      out_pixel[2] = in_pixel[2];
      out_pixel[3] = in_pixel[3];
      in_pixel += 4;
      out_pixel += 4;
      continue;
    }

    gfloat dist_left = (gfloat) x;
    gfloat dist_right = (gfloat) (canvas_width - 1 - x);
    gfloat dist_top = (gfloat) y;
    gfloat dist_bottom = (gfloat) (canvas_height - 1 - y);
    gfloat min_dist = fmin(fmin(dist_left, dist_right), fmin(dist_top, dist_bottom));

    gfloat position;
    if (min_dist == dist_left || min_dist == dist_right)
      position = (gfloat) y / canvas_height;
    else
      position = (gfloat) x / canvas_width;

    gfloat dx = x - center_x;
    gfloat dy = y - center_y;
    gfloat dist_center = sqrt(dx * dx + dy * dy);
    gfloat norm_dist = dist_center / (MIN(canvas_width, canvas_height) / 2.0);
    gfloat angle = atan2(dy, dx);

    gboolean is_border = FALSE;
    gfloat alpha = 1.0;

    switch (o->mask_style)
    {
      case GEGL_FRAME_MASK_STYLE_1: // Fractal Spiral
        {
          gfloat spiral = sin(o->pattern_density * (angle + dist_center * 0.05));
          gfloat fractal = spiral + 0.3 * sin(2.0 * o->pattern_density * angle);
          gfloat threshold = bw * (0.5 + 0.5 * fractal);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_2: // Radial Mosaic
        {
          gfloat tile_size = 10.0 / o->pattern_density;
          gint tile_x = (gint) (dist_center * cos(angle) / tile_size);
          gint tile_y = (gint) (dist_center * sin(angle) / tile_size);
          gfloat noise = sin(tile_x * 123.456 + tile_y * 789.012);
          gfloat threshold = bw * (0.5 + 0.5 * noise);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_3: // Wavy Petals
        {
          gfloat petal = sin(o->pattern_density * position * 2.0 * G_PI) * cos(o->pattern_density * position * G_PI);
          gfloat threshold = bw * (0.5 + 0.5 * petal);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_4: // Star Cluster
        {
          gfloat star = sin(o->pattern_density * angle * 5.0);
          gfloat threshold = bw * (0.5 + 0.5 * star);
          is_border = (min_dist < threshold) && (fmod(dist_center, 10.0) < 5.0);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_5: // Geometric Lattice
        {
          gfloat lattice = sin(o->pattern_density * x / canvas_width) * sin(o->pattern_density * y / canvas_height);
          gfloat threshold = bw * (0.5 + 0.5 * lattice);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_6: // Organic Bloom
        {
          gfloat bloom = 0.5 * (1.0 + sin(o->pattern_density * position * 2.0 * G_PI)) * cos(o->pattern_density * position * G_PI);
          gfloat threshold = bw * (0.5 + 0.5 * bloom);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_7: // Cracked Earth
        {
          gfloat crack = sin(o->pattern_density * position * 2.0 * G_PI) * cos(o->pattern_density * position * 4.0 * G_PI);
          gfloat threshold = bw * (0.5 + 0.5 * crack);
          is_border = (min_dist < threshold) && (fmod(dist_center, 15.0) < 7.0);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_8: // Pixelated Swirl
        {
          gfloat swirl = sin(o->pattern_density * angle + dist_center * 0.1);
          gfloat pixel_size = 5.0 / o->pattern_density;
          gint pixel_x = (gint) (x / pixel_size);
          gint pixel_y = (gint) (y / pixel_size);
          gfloat noise = (sin(pixel_x * 456.789 + pixel_y * 123.456) > 0.0) ? 1.0 : 0.0;
          gfloat threshold = bw * (0.5 + 0.5 * swirl * noise);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_9: // Celestial Orbit
        {
          gfloat orbit = sin(o->pattern_density * angle * 3.0) * cos(dist_center * 0.05);
          gfloat threshold = bw * (0.5 + 0.5 * orbit);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_10: // Crystal Shard
        {
          gfloat shard = sin(o->pattern_density * angle * 4.0) * cos(o->pattern_density * position * G_PI);
          gfloat threshold = bw * (0.5 + 0.5 * shard);
          is_border = (min_dist < threshold) && (fmod(dist_center, 12.0) < 6.0);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_11: // Feathered Edge
        {
          gfloat feather = 0.5 * (1.0 + cos(o->pattern_density * position * 2.0 * G_PI));
          gfloat threshold = bw * (0.6 + 0.4 * feather);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_12: // Marble Veins
        {
          gfloat vein = sin(o->pattern_density * position * 2.0 * G_PI) + 0.3 * sin(3.0 * o->pattern_density * position * 2.0 * G_PI);
          gfloat threshold = bw * (0.5 + 0.5 * vein);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_13: // Kaleidoscope Burst
        {
          gfloat burst = sin(o->pattern_density * angle * 6.0) * cos(dist_center * 0.1);
          gfloat threshold = bw * (0.5 + 0.5 * burst);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_14: // Frosted Glass
        {
          gfloat frost = sin(o->pattern_density * position * 2.0 * G_PI) * sin(o->pattern_density * position * 4.0 * G_PI);
          gfloat threshold = bw * (0.5 + 0.5 * frost);
          is_border = (min_dist < threshold) && (fmod(dist_center, 8.0) < 4.0);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_15: // Rustic Weave
        {
          gfloat weave = sin(o->pattern_density * x / canvas_width) * cos(o->pattern_density * y / canvas_height);
          gfloat threshold = bw * (0.5 + 0.5 * weave);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_16: // Neon Glow
        {
          gfloat glow = sin(o->pattern_density * angle * 2.0) * cos(dist_center * 0.05);
          gfloat threshold = bw * (0.5 + 0.5 * glow);
          is_border = (min_dist < threshold) && (fmod(dist_center, 10.0) < 5.0);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_17: // Sand Dune
        {
          gfloat dune = 0.5 * (1.0 + sin(o->pattern_density * position * 2.0 * G_PI)) * cos(o->pattern_density * position * 3.0 * G_PI);
          gfloat threshold = bw * (0.5 + 0.5 * dune);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_18: // Ink Splash
        {
          gfloat splash = sin(o->pattern_density * position * 2.0 * G_PI) * cos(o->pattern_density * angle * 3.0);
          gfloat threshold = bw * (0.5 + 0.5 * splash);
          is_border = (min_dist < threshold) && (fmod(dist_center, 15.0) < 7.0);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_19: // Cosmic Dust
        {
          gfloat dust = sin(o->pattern_density * angle * 5.0) + 0.2 * sin(2.0 * o->pattern_density * dist_center * 0.1);
          gfloat threshold = bw * (0.5 + 0.5 * dust);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_20: // Glacial Rift
        {
          gfloat rift = sin(o->pattern_density * position * 2.0 * G_PI) * cos(o->pattern_density * angle * 2.0);
          gfloat threshold = bw * (0.5 + 0.5 * rift);
          is_border = (min_dist < threshold) && (fmod(dist_center, 10.0) < 5.0);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_21: // Tropical Vine
        {
          gfloat vine = 0.5 * (1.0 + sin(o->pattern_density * position * 2.0 * G_PI)) * sin(o->pattern_density * angle * G_PI);
          gfloat threshold = bw * (0.5 + 0.5 * vine);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_22: // Aurora Wave
        {
          gfloat aurora = sin(o->pattern_density * angle * 3.0) * cos(dist_center * 0.1);
          gfloat threshold = bw * (0.5 + 0.5 * aurora);
          is_border = (min_dist < threshold);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_23: // Stained Glass
        {
          gfloat glass = sin(o->pattern_density * position * 2.0 * G_PI) * cos(o->pattern_density * angle * 4.0);
          gfloat threshold = bw * (0.5 + 0.5 * glass);
          is_border = (min_dist < threshold) && (fmod(dist_center, 12.0) < 6.0);
        }
        break;

      case GEGL_FRAME_MASK_STYLE_24: // Desert Mirage
        {
          gfloat mirage = 0.5 * (1.0 + sin(o->pattern_density * position * 2.0 * G_PI)) * cos(o->pattern_density * dist_center * 0.05);
          gfloat threshold = bw * (0.5 + 0.5 * mirage);
          is_border = (min_dist < threshold);
        }
        break;

      default:
        is_border = (min_dist < bw);
        break;
    }

    if (is_border)
    {
      out_pixel[0] = mask_color[0];
      out_pixel[1] = mask_color[1];
      out_pixel[2] = mask_color[2];
      out_pixel[3] = mask_color[3] * alpha;
    }
    else
    {
      out_pixel[0] = in_pixel[0];
      out_pixel[1] = in_pixel[1];
      out_pixel[2] = in_pixel[2];
      out_pixel[3] = in_pixel[3];
    }

    in_pixel += 4;
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

  gegl_operation_class_set_keys(operation_class,
    "name",        "ai/lb:frame2",
    "title",       _("Frame 2"),
    "reference-hash", "framemaskgeglfair3",
    "description", _("Applies a customizable frame mask with adjustable width and color."),
    NULL);
}

#endif
