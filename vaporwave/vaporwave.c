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
 * Copyright 2025 Beaver
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>
#include <stdlib.h>

#ifdef GEGL_PROPERTIES

property_boolean(background_gradient, _("Background Gradient"), TRUE)
    description(_("Use a linear gradient for the background"))

property_boolean(scanlines, _("Scanlines"), TRUE)
    description(_("Add retro scanline effect"))

property_boolean(noise, _("Noise"), TRUE)
    description(_("Add subtle noise for texture"))

property_seed(seed, _("Random Seed"), rand)
    description(_("Seed for noise randomization"))
ui_meta("visible", "noise")

property_boolean(mountains, _("Mountains Silhouette"), TRUE)
    description(_("Enable silhouette color fill of mountains"))

property_color(mountains_color, _("Mountains Color"), "#ffffff")
    description(_("Color of the mountains silhouette"))
    ui_meta("visible", "mountains")

property_double(mountains_phase, _("Mountains Wave Position"), 0.0)
    description(_("Adjust the horizontal position of the mountain wave pattern"))
    value_range(0.0, 6.28)
    ui_range(0.0, 6.28)
    ui_meta("visible", "mountains")

property_color(background_color, _("Background Color"), "#5309ff")
    description(_("Base color for the sky gradient"))

property_color(background_gradient_color, _("Background Gradient Color"), "#ff1309")
    description(_("End color for the background linear gradient"))
    ui_meta("visible", "background_gradient")

property_double(gradient_offset_y, _("Gradient Y Offset"), 0.0)
    description(_("Vertical offset of the gradient start point (relative to canvas height)"))
    value_range(-1.0, 1.0)
    ui_range(-1.0, 1.0)
    ui_meta("visible", "background_gradient")

property_color(sun_color, _("Sun Color"), "#ffffff")
    description(_("Color of the sun and its glow"))

property_boolean(sun_lines, _("Sun Lines"), TRUE)
    description(_("Add transparent horizontal lines across the sun"))

property_double(sun_glow_size, _("Sun Glow Size"), 2.5)
    description(_("Adjust the size of the sun's glow (relative to sun size)"))
    value_range(1.0, 3.0)
    ui_range(1.0, 3.0)

property_color(grid_color, _("Grid Color"), "rgb(0.0, 1.0, 1.0)")
    description(_("Color of the grid lines"))

property_double(grid_density, _("Grid Density"), 0.5)
    description(_("Density of the grid lines (0.5 = sparse, 2.0 = dense)"))
    value_range(0.5, 2.0)
    ui_range(0.5, 2.0)

property_int(grid_thickness, _("Grid Thickness"), 3)
    description(_("Thickness of the grid lines (1 = thinnest, 5 = thickest)"))
    value_range(1, 5)
    ui_range(1, 5)

property_double(sun_size, _("Sun Size"), 0.15)
    description(_("Size of the sun relative to image width"))
    value_range(0.05, 0.5)
    ui_range(0.05, 0.5)

property_double(sun_height, _("Sun Height"), 0.3)
    description(_("Height of the sun above the horizon (relative to image height)"))
    value_range(0.0, 0.5)
    ui_range(0.0, 0.5)

property_int(horizon_height, _("Horizon Height"), 40)
    description(_("Height of the horizon line (as a percentage of image height)"))
    value_range(20, 80)
    ui_range(20, 80)


#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     vaporwave
#define GEGL_OP_C_SOURCE vaporwave.c

#include "gegl-op.h"

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
  gfloat *output = (gfloat *)out_buf;
  gfloat bg_rgb[4], bg_grad_rgb[4], sun_rgb[4], grid_rgb[4], mountains_rgb[4];
  gint x, y;

  /* Get colors */
  gegl_color_get_pixel(o->background_color, babl_format("RGBA float"), bg_rgb);
  gegl_color_get_pixel(o->background_gradient_color, babl_format("RGBA float"), bg_grad_rgb);
  gegl_color_get_pixel(o->sun_color, babl_format("RGBA float"), sun_rgb);
  gegl_color_get_pixel(o->grid_color, babl_format("RGBA float"), grid_rgb);
  gegl_color_get_pixel(o->mountains_color, babl_format("RGBA float"), mountains_rgb);
  srand(o->seed);

  /* Get full canvas dimensions */
  GeglRectangle *canvas = gegl_operation_source_get_bounding_box(operation, "input");
  gfloat canvas_width = canvas ? canvas->width : roi->width;
  gfloat canvas_height = canvas ? canvas->height : roi->height;

  /* Parameters */
  gfloat grid_density = o->grid_density;
  gfloat sun_size = o->sun_size * canvas_width;
  gfloat horizon_y = canvas_height * (1.0f - o->horizon_height / 100.0f);
  gfloat sun_center_y = horizon_y - o->sun_height * canvas_height;
  gfloat sun_center_x = canvas_width / 2.0f;

  /* Gradient offsets */
  gfloat offset_y = o->gradient_offset_y * canvas_height;

  /* Vanishing point for grid */
  gfloat vp_x = canvas_width / 2.0f;
  gfloat vp_y = horizon_y - canvas_height * 0.2f;

  /* Grid spacing (canvas-wide) */
  gfloat grid_spacing_x = canvas_width / (20.0f * grid_density);
  gfloat grid_spacing_y = (canvas_height - horizon_y) / (20.0f * grid_density);

  /* Grid thickness scaling: map 1-5 to 0.01f-0.05f */
  gfloat grid_line_thickness = 0.01f + (o->grid_thickness - 1) * 0.01f;

  for (y = 0; y < roi->height; y++) {
    for (x = 0; x < roi->width; x++) {
      gint idx = (y * roi->width + x) * 4;
      gfloat px = x + roi->x + 0.5f;
      gfloat py = y + roi->y + 0.5f;
      gfloat r, g, b, a;

      /* Initialize with sky or ground color */
      if (o->background_gradient) {
        /* 2D linear gradient, now strictly vertical */
        gfloat dy = py - offset_y;
        gfloat t = dy / canvas_height;
        t = CLAMP(t, 0.0f, 1.0f);
        r = bg_rgb[0] * (1.0f - t) + bg_grad_rgb[0] * t;
        g = bg_rgb[1] * (1.0f - t) + bg_grad_rgb[1] * t;
        b = bg_rgb[2] * (1.0f - t) + bg_grad_rgb[2] * t;
        a = bg_rgb[3] * (1.0f - t) + bg_grad_rgb[3] * t;
      } else {
        if (py <= horizon_y) {
          /* Vertical gradient: bright at horizon, darker at top */
          gfloat t = 1.0f - py / horizon_y;
          r = bg_rgb[0] * (0.5f + 0.5f * t);
          g = bg_rgb[1] * (0.5f + 0.5f * t);
          b = bg_rgb[2] * (0.5f + 0.5f * t);
          a = bg_rgb[3];
        } else {
          /* Ground: darker version of background */
          r = bg_rgb[0] * 0.4f;
          g = bg_rgb[1] * 0.4f;
          b = bg_rgb[2] * 0.4f;
          a = bg_rgb[3];
        }
      }

      /* Sun and glow (behind mountains and grid) */
      gfloat dx = px - sun_center_x;
      gfloat dy = py - sun_center_y;
      gfloat dist = sqrt(dx * dx + dy * dy);
      if (dist < sun_size * o->sun_glow_size) {
        gfloat sun_r = sun_rgb[0];
        gfloat sun_g = sun_rgb[1];
        gfloat sun_b = sun_rgb[2];
        gfloat sun_a = sun_rgb[3];

        if (dist < sun_size) {
          /* Solid sun */
          sun_a = sun_rgb[3];
        } else {
          /* Quadratic falloff for glow */
          gfloat glow = 1.0f - dist / (sun_size * o->sun_glow_size);
          glow = glow * glow;
          sun_a = glow * sun_rgb[3];
        }

        /* Add transparent horizontal lines to the sun if enabled */
        if (o->sun_lines) {
          gfloat line_spacing = sun_size * 2.0f / 8.0f; /* 8 lines across the sun's diameter */
          gfloat line_pos = fmod(py - (sun_center_y - sun_size), line_spacing);
          gfloat line_thickness = line_spacing * 0.2f; /* Lines are 20% of the spacing */
          if (line_pos < line_thickness) {
            sun_a = 0.0f; /* Make the line transparent */
          }
        }

        /* Blend the sun with the background */
        r = r * (1.0f - sun_a) + sun_r * sun_a;
        g = g * (1.0f - sun_a) + sun_g * sun_a;
        b = b * (1.0f - sun_a) + sun_b * sun_a;
        a = a * (1.0f - sun_a) + sun_a;
      }

      /* Mountains silhouette (two layers for depth, sine-based curves) */
      if (o->mountains) {
        /* First layer (farther mountains) */
        gfloat mountain_height1 = horizon_y - (canvas_height * 0.15f) * (0.5f + 0.5f * sin(px * 0.01f + o->mountains_phase) + 0.3f * sin(px * 0.05f + o->mountains_phase));
        if (py > mountain_height1 && py <= horizon_y) {
          r = mountains_rgb[0] * 0.8f; /* Slightly darker for depth */
          g = mountains_rgb[1] * 0.8f;
          b = mountains_rgb[2] * 0.8f;
          a = mountains_rgb[3];
        }

        /* Second layer (closer mountains) */
        gfloat mountain_height2 = horizon_y - (canvas_height * 0.1f) * (0.5f + 0.5f * sin(px * 0.015f + o->mountains_phase) + 0.2f * sin(px * 0.07f + o->mountains_phase));
        if (py > mountain_height2 && py <= horizon_y) {
          r = mountains_rgb[0];
          g = mountains_rgb[1];
          b = mountains_rgb[2];
          a = mountains_rgb[3];
        }
      }

      /* Grid (below horizon) */
      if (py > horizon_y) {
        gfloat t = (py - vp_y) / (canvas_height - vp_y);
        t = CLAMP(t, 0.01f, 1.0f);
        gfloat grid_x = (px - vp_x) / t + vp_x;
        gfloat grid_y = py;
        gfloat line_x = fabs(grid_x / grid_spacing_x - floor(grid_x / grid_spacing_x + 0.5f));
        gfloat line_y = fabs(grid_y / grid_spacing_y - floor(grid_y / grid_spacing_y + 0.5f));
        if (line_x < grid_line_thickness || line_y < grid_line_thickness) {
          r = grid_rgb[0];
          g = grid_rgb[1];
          b = grid_rgb[2];
          a = grid_rgb[3];
        }
      }

      /* Scanlines */
      if (o->scanlines && ((y + roi->y) % 8) == 0) {
        r *= 0.7f; /* 30% darkening */
        g *= 0.7f;
        b *= 0.7f;
        b = CLAMP(b + 0.1f, 0.0f, 1.0f); /* Stronger blue tint */
      }

      /* Noise */
      if (o->noise) {
        gfloat n = (rand() / (gfloat)RAND_MAX - 0.5f) * 0.06f; /* Increased noise strength */
        r = CLAMP(r + n, 0.0f, 1.0f);
        g = CLAMP(g + n, 0.0f, 1.0f);
        b = CLAMP(b + n, 0.0f, 1.0f);
      }

      output[idx + 0] = r;
      output[idx + 1] = g;
      output[idx + 2] = b;
      output[idx + 3] = a;
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
    "name",        "ai/lb:vaporwave",
    "title",       _("Vaporwave"),
    "description", _("Vaporwave wallpaper"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Vaporwave..."),
    NULL);
}

#endif
