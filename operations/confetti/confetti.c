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
 * 2025 Grok with Beaver's help (
*/

#include "config.h"
#include <stdio.h>
#include <glib/gi18n-lib.h>
#include <math.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <string.h>

#ifdef GEGL_PROPERTIES

property_double (tile_size, _("Tile Size"), 50.0)
    description (_("Size of the grid tiles for confetti placement in pixels"))
    value_range (20.0, 100.0)
    ui_range (50.0, 100.0)

property_seed (seed, _("Seed"), rand)
    description (_("Randomizes confetti placement and rotation"))

property_color (color1, _("Confetti Color 1 (Red)"), "#FF0000")
    description (_("First confetti color"))

property_color (color2, _("Confetti Color 2 (Blue)"), "#0000FF")
    description (_("Second confetti color"))

property_color (color3, _("Confetti Color 3 (Yellow)"), "#FFFF00")
    description (_("Third confetti color"))

property_color (color4, _("Confetti Color 4 (Green)"), "#00FF00")
    description (_("Fourth confetti color"))

property_color (color5, _("Confetti Color 5 (Purple)"), "#800080")
    description (_("Fifth confetti color"))

property_color (color6, _("Confetti Color 6 (Pink)"), "#FF69B4")
    description (_("Sixth confetti color"))

property_color (color7, _("Confetti Color 7 (Orange)"), "#FFA500")
    description (_("Seventh confetti color"))

property_color (background_color, _("Background Color"), "white")
    description (_("Color of the background"))

property_double (confetti_length, _("Confetti Length"), 8.0)
    description (_("Length of confetti pieces in pixels"))
    value_range (4.0, 50.0)
    ui_range (4.0, 40.0)

property_double (confetti_width, _("Confetti Width"), 2.0)
    description (_("Width of confetti pieces in pixels"))
    value_range (1.0, 20.0)
    ui_range (1.0, 20.0)

property_double (density, _("Confetti Density"), 5.0)
    description (_("Average number of confetti pieces per tile"))
    value_range (1.0, 10.0)
    ui_range (1.0, 10.0)

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     confetti
#define GEGL_OP_C_SOURCE confetti.c

#include "gegl-op.h"

// Structure for a confetti piece
typedef struct {
  gdouble x, y;         // Center position
  gdouble angle;        // Rotation in radians
  gfloat color[4];      // RGBA color
} ConfettiPiece;

static void prepare(GeglOperation *operation) {
  gegl_operation_set_format(operation, "input", babl_format("RGBA float"));
  gegl_operation_set_format(operation, "output", babl_format("RGBA float"));
}

static gboolean process(GeglOperation *operation, void *in_buf, void *out_buf, glong n_pixels, const GeglRectangle *roi, gint level) {
  GeglProperties *o = GEGL_PROPERTIES(operation);
  gfloat *out_pixel = (gfloat *)out_buf;

  // Background color
  gfloat bg[4];
  gegl_color_get_pixel(o->background_color, babl_format("RGBA float"), bg);

  // Collect confetti colors
  GPtrArray *colors = g_ptr_array_new();
  gfloat rgba[4];
  gegl_color_get_pixel(o->color1, babl_format("RGBA float"), rgba);
  g_ptr_array_add(colors, g_memdup2(rgba, 4 * sizeof(gfloat)));
  gegl_color_get_pixel(o->color2, babl_format("RGBA float"), rgba);
  g_ptr_array_add(colors, g_memdup2(rgba, 4 * sizeof(gfloat)));
  gegl_color_get_pixel(o->color3, babl_format("RGBA float"), rgba);
  g_ptr_array_add(colors, g_memdup2(rgba, 4 * sizeof(gfloat)));
  gegl_color_get_pixel(o->color4, babl_format("RGBA float"), rgba);
  g_ptr_array_add(colors, g_memdup2(rgba, 4 * sizeof(gfloat)));
  gegl_color_get_pixel(o->color5, babl_format("RGBA float"), rgba);
  g_ptr_array_add(colors, g_memdup2(rgba, 4 * sizeof(gfloat)));
  gegl_color_get_pixel(o->color6, babl_format("RGBA float"), rgba);
  g_ptr_array_add(colors, g_memdup2(rgba, 4 * sizeof(gfloat)));
  gegl_color_get_pixel(o->color7, babl_format("RGBA float"), rgba);
  g_ptr_array_add(colors, g_memdup2(rgba, 4 * sizeof(gfloat)));

  GeglRectangle *canvas = gegl_operation_source_get_bounding_box(operation, "input");
  gfloat canvas_width = canvas ? canvas->width : roi->width;
  GRand *rand = g_rand_new_with_seed(o->seed);
  gfloat tile_size = o->tile_size;
  gfloat half_length = o->confetti_length / 2.0;
  gfloat half_width = o->confetti_width / 2.0;
  gfloat density = o->density;

  for (glong i = 0; i < n_pixels; i++) {
    gfloat x = (i % roi->width) + roi->x;
    gfloat y = (i / roi->width) + roi->y;

    // Default to background
    out_pixel[0] = bg[0];
    out_pixel[1] = bg[1];
    out_pixel[2] = bg[2];
    out_pixel[3] = bg[3];

    // Check neighboring tiles
    gint tile_x = (gint)floorf(x / tile_size);
    gint tile_y = (gint)floorf(y / tile_size);
    for (gint dx = -1; dx <= 1; dx++) {
      for (gint dy = -1; dy <= 1; dy++) {
        gint tx = tile_x + dx;
        gint ty = tile_y + dy;
        guint tile_seed = (tx + ty * (gint)(canvas_width / tile_size)) ^ o->seed;
        g_rand_set_seed(rand, tile_seed);

        gint num_pieces = g_rand_int_range(rand, (gint)density / 2, (gint)density + 1);
        num_pieces = MIN(num_pieces, 15); // Cap for performance

        for (gint j = 0; j < num_pieces; j++) {
          ConfettiPiece piece;
          piece.x = g_rand_double_range(rand, 0.0, tile_size) + tx * tile_size;
          piece.y = g_rand_double_range(rand, 0.0, tile_size) + ty * tile_size;
          piece.angle = g_rand_double_range(rand, 0.0, 2.0 * G_PI);
          gfloat *color = (gfloat *)g_ptr_array_index(colors, g_rand_int_range(rand, 0, colors->len));
          memcpy(piece.color, color, 4 * sizeof(gfloat));

          // Transform pixel to confetti-local coordinates
          gfloat dx = x - piece.x;
          gfloat dy = y - piece.y;
          gfloat cos_a = cosf(-piece.angle);
          gfloat sin_a = sinf(-piece.angle);
          gfloat local_x = dx * cos_a - dy * sin_a;
          gfloat local_y = dx * sin_a + dy * cos_a;

          // Check if pixel is inside rectangle
          if (fabsf(local_x) <= half_length && fabsf(local_y) <= half_width) {
            out_pixel[0] = piece.color[0];
            out_pixel[1] = piece.color[1];
            out_pixel[2] = piece.color[2];
            out_pixel[3] = 1.0;
            goto next_pixel; // Skip other pieces
          }
        }
      }
    }
next_pixel:
    out_pixel += 4;
  }

  // Cleanup
  g_ptr_array_foreach(colors, (GFunc)g_free, NULL);
  g_ptr_array_free(colors, TRUE);
  g_rand_free(rand);
  return TRUE;
}

static void gegl_op_class_init(GeglOpClass *klass) {
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS(klass);
  GeglOperationPointFilterClass *point_filter_class = GEGL_OPERATION_POINT_FILTER_CLASS(klass);

  operation_class->prepare = prepare;
  point_filter_class->process = process;

  gegl_operation_class_set_keys(operation_class,
    "name", "ai/lb:confetti",
    "title", _("Confetti Scatter"),
    "reference-hash", "confettiscatter",
    "description", _("Generates a festive confetti pattern with randomized placement, customizable colors, size, and density"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Confetti..."),
    NULL);
}

#endif
