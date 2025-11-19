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

enum_start(pattern_type251294)
  enum_value(SINE_WAVE_BANDS, "sine_wave_bands", "Sine Wave Bands")
  enum_value(SINE_WAVE_ZIGZAG, "sine_wave_zigzag", "Sine Wave Zigzag")
  enum_value(SINE_WAVE_SHARP, "sine_wave_sharp", "Sine Wave Sharp")
  enum_value(SINE_WAVE_FOLDS, "sine_wave_folds", "Sine Wave Folds")
  enum_value(SINE_WAVE_KINKS, "sine_wave_kinks", "Sine Wave Kinks")
  enum_value(SINE_WAVE_BRAIDS, "sine_wave_braids", "Sine Wave Braids")
  enum_value(SINE_WAVE_CRESTS, "sine_wave_crests", "Sine Wave Crests")
  enum_value(SINE_WAVE_FRINGES, "sine_wave_fringes", "Sine Wave Fringes")
  enum_value(SINE_WAVE_RIPPLES, "sine_wave_ripples", "Sine Wave Ripples")
  enum_value(SINE_WAVE_VORTICES, "sine_wave_vortices", "Sine Wave Vortices")
  enum_value(SINE_WAVE_ISOBUBBLES, "sine_wave_isobubbles", "Sine Wave Isolated Bubbles")
enum_end(pattern_type251294)

property_enum(pattern, _("Pattern"),
              pattern_type251294, pattern_type251294,
              SINE_WAVE_BANDS)
    description(_("Select the pattern to render"))

property_double(tile_size, _("Tile Size"), 500.0)
    description(_("Size of the repeating tile in pixels"))
    value_range(50.0, 4000.0)
    ui_range(50.0, 4000.0)

property_double(line_width, _("Line Width"), 0.7)
    description(_("Adjust the thickness of the pattern lines"))
    value_range(0.01, 3.0)
    ui_range(0.01, 3.0)

property_double(rotation, _("Rotation"), 0.0)
    description(_("Rotate the pattern (degrees)"))
    value_range(0.0, 360.0)
    ui_range(0.0, 360.0)

property_double(offset_x, _("X Offset"), 0.0)
    description(_("Horizontal shift of the pattern (0.0 to 1.0)"))
    value_range(0.0, 1.0)
    ui_range(0.0, 1.0)

property_double(offset_y, _("Y Offset"), 0.0)
    description(_("Vertical shift of the pattern (0.0 to 1.0)"))
    value_range(0.0, 1.0)
    ui_range(0.0, 1.0)

property_color(foreground_color, _("Foreground Color"), "#5BA9EA")
    description(_("Color of the pattern shapes"))

property_color(background_color, _("Background Color"), "#70D0FF")
    description(_("Color of the background"))
    ui_meta("visible", "! transparent_background")

property_boolean(transparent_background, _("Transparent Background"), FALSE)
    description(_("Make the background transparent instead of using the background color"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     sinewavespatterns
#define GEGL_OP_C_SOURCE sinewaves.c

#include "gegl-op.h"

static void prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
}

static GeglRectangle get_bounding_box (GeglOperation *operation)
{
  return gegl_rectangle_infinite_plane ();
}

static gboolean
process (GeglOperation        *operation,
         GeglOperationContext *context,
         const gchar          *output_prop,
         const GeglRectangle  *result,
         gint                 level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  GeglBuffer *output = gegl_operation_context_get_target (context, output_prop);
  gdouble fg_color[4], bg_color[4];
  gdouble tile_size = o->tile_size;
  GeglRectangle rect = *result;
  gint x, y;

  // Get color values
  gegl_color_get_rgba (o->foreground_color, &fg_color[0], &fg_color[1], &fg_color[2], &fg_color[3]);
  gegl_color_get_rgba (o->background_color, &bg_color[0], &bg_color[1], &bg_color[2], &bg_color[3]);

  // Convert rotation to radians
  gdouble rotation_rad = o->rotation * G_PI / 180.0;
  gdouble cos_rot = cos(rotation_rad);
  gdouble sin_rot = sin(rotation_rad);

  for (y = rect.y; y < rect.y + rect.height; y++)
  {
    for (x = rect.x; x < rect.x + rect.width; x++)
    {
      gdouble px = (gdouble)x;
      gdouble py = (gdouble)y;

      // Apply rotation to the original pixel coordinates around (0, 0)
      gdouble px_rot = px * cos_rot - py * sin_rot;
      gdouble py_rot = px * sin_rot + py * cos_rot;

      // Apply x and y offsets
      gdouble offset_x = o->offset_x * tile_size;
      gdouble offset_y = o->offset_y * tile_size;

      // Map rotated and offset coordinates to tile space
      gdouble tx = fmod(px_rot + offset_x, tile_size);
      gdouble ty = fmod(py_rot + offset_y, tile_size);
      if (tx < 0) tx += tile_size;
      if (ty < 0) ty += tile_size;

      gfloat out[4];
      gboolean in_shape = FALSE;

      switch (o->pattern)
      {
        case SINE_WAVE_BANDS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble wave = sin((tx / cell_size) * 1.5 * G_PI) * (cell_size / 3.0);
          gdouble band = fmod(ty + wave, cell_size / 2.0);
          gdouble base_line_width = cell_size * 0.15 * o->line_width;
          if (band < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_ZIGZAG:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble wave = sin((tx / cell_size) * 2.0 * G_PI) * (cell_size / 4.0);
          gdouble zigzag = fabs(fmod(ty + wave, cell_size / 2.0) - cell_size / 4.0);
          gdouble base_line_width = cell_size * 0.1 * o->line_width;
          if (fabs(zigzag - cell_size / 8.0) < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_SHARP:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble wave = sin((tx / cell_size) * 4.0 * G_PI) * (cell_size / 4.0);
          gdouble zigzag = fabs(fmod(ty + wave, cell_size / 2.0) - cell_size / 4.0);
          gdouble base_line_width = cell_size * 0.1 * o->line_width;
          if (fabs(zigzag - cell_size / 8.0) < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_FOLDS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble wave = sin((tx / cell_size) * 1.5 * G_PI + (ty / cell_size) * G_PI) * (cell_size / 3.0);
          gdouble band = fmod(ty + wave, cell_size / 2.0);
          gdouble base_line_width = cell_size * 0.15 * o->line_width;
          if (band < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_KINKS:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble wave_x = sin((tx / cell_size) * 2.0 * G_PI) * (cell_size / 4.0);
          gdouble wave_y = sin((ty / cell_size) * 2.0 * G_PI) * (cell_size / 4.0);
          gdouble zigzag = fabs(fmod(ty + wave_x, cell_size / 2.0) - cell_size / 4.0);
          gdouble base_line_width = cell_size * 0.1 * o->line_width;
          if (fabs(zigzag - (cell_size / 8.0 + wave_y / 4.0)) < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_BRAIDS:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble wave1 = sin((tx / cell_size) * 1.5 * G_PI) * (cell_size / 3.0);
          gdouble wave2 = sin((tx / cell_size) * 1.5 * G_PI + G_PI / 2.0) * (cell_size / 3.0);
          gdouble band1 = fmod(ty + wave1, cell_size / 2.0);
          gdouble band2 = fmod(ty + wave2, cell_size / 2.0);
          gdouble base_line_width = cell_size * 0.15 * o->line_width;
          if (band1 < base_line_width || band2 < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_CRESTS:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble amplitude = (cell_size / 4.0) * (1.0 + 0.5 * sin((ty / cell_size) * G_PI));
          gdouble wave = sin((tx / cell_size) * 2.0 * G_PI) * amplitude;
          gdouble zigzag = fabs(fmod(ty + wave, cell_size / 2.0) - cell_size / 4.0);
          gdouble base_line_width = cell_size * 0.1 * o->line_width;
          if (fabs(zigzag - cell_size / 8.0) < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_FRINGES:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble wave = sin((tx / cell_size) * 4.0 * G_PI) * sin((ty / cell_size) * 4.0 * G_PI) * (cell_size / 4.0);
          gdouble zigzag = fabs(fmod(ty + wave, cell_size / 2.0) - cell_size / 4.0);
          gdouble base_line_width = cell_size * 0.1 * o->line_width;
          if (fabs(zigzag - cell_size / 8.0) < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_RIPPLES:
        {
          gdouble cell_size = tile_size / 4.0;
          gdouble phase_x = (tx / tile_size) * 2.0 * G_PI;
          gdouble phase_y = (ty / tile_size) * 2.0 * G_PI;
          gdouble wave = sin(phase_x + sin(phase_y)) * (cell_size / 3.0);
          gdouble band = fmod(ty + wave, cell_size / 2.0);
          gdouble base_line_width = cell_size * 0.15 * o->line_width;
          if (band < base_line_width)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_VORTICES:
        {
          gdouble phase_x = (tx / tile_size) * 2.0 * G_PI;
          gdouble phase_y = (ty / tile_size) * 2.0 * G_PI;
          gdouble vortex = sin(phase_x + cos(phase_y)) * cos(phase_y + sin(phase_x));
          gdouble threshold = 0.2 * o->line_width;
          if (fabs(vortex) < threshold && vortex > -threshold)
          {
            in_shape = TRUE;
          }
          break;
        }
        case SINE_WAVE_ISOBUBBLES:
        {
          gdouble cell_size = tile_size / 5.0;
          gdouble px = (tx / tile_size) * 2.0 * G_PI;
          gdouble py = (ty / tile_size) * 2.0 * G_PI;
          gdouble weave = sin(px * 2.0 + cos(py)) + sin(py * 2.0 + cos(px));
          gdouble base_line_width = cell_size * 0.15 * o->line_width;
          if (fabs(weave - 0.5) < base_line_width / cell_size)
          {
            in_shape = TRUE;
          }
          break;
        }
      }

      if (in_shape)
      {
        out[0] = fg_color[0];
        out[1] = fg_color[1];
        out[2] = fg_color[2];
        out[3] = 1.0;
      }
      else
      {
        if (o->transparent_background)
        {
          out[0] = 0.0;
          out[1] = 0.0;
          out[2] = 0.0;
          out[3] = 0.0;
        }
        else
        {
          out[0] = bg_color[0];
          out[1] = bg_color[1];
          out[2] = bg_color[2];
          out[3] = 1.0;
        }
      }

      gegl_buffer_set (output, GEGL_RECTANGLE (x, y, 1, 1), 0, babl_format ("RGBA float"), out, GEGL_AUTO_ROWSTRIDE);
    }
  }

  return TRUE;
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class = GEGL_OPERATION_CLASS (klass);
  operation_class->prepare = prepare;
  operation_class->get_bounding_box = get_bounding_box;
  operation_class->process = process;

  gegl_operation_class_set_keys (operation_class,
    "name",        "ai/lb:sine-waves",
    "title",       _("Sine Wave Patterns"),
    "reference-hash", "sinewavesgeglvortex2025",
    "description", _("Renders seamless sine wave-based patterns, with adjustable tile size, line width, rotation, x/y offsets, and colors"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL/",
    "gimp:menu-label", _("Sine Wave Patterns..."),
    NULL);
}

#endif
