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
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 * Credit to Pippin for major GEGL contributions
 * Beaver Vibe coded the plugin with DeepSeek, DeepSeek made the plugin   
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <gegl.h>
#include <gegl-plugin.h>
#include <math.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_recursive_square_style72325)
  enum_value (GEGL_RECURSIVE_SQUARE_STYLE_1, "style1",  N_("Default Square"))
  enum_value (GEGL_RECURSIVE_SQUARE_STYLE_2, "style2",  N_("Double Square"))
  enum_value (GEGL_RECURSIVE_SQUARE_STYLE_3, "style3",  N_("Fractal Square"))
  enum_value (GEGL_RECURSIVE_SQUARE_STYLE_4, "style4",  N_("Wave Square"))
  enum_value (GEGL_RECURSIVE_SQUARE_STYLE_5, "style5",  N_("Square Spiral"))
  enum_value (GEGL_RECURSIVE_SQUARE_STYLE_6, "style6",  N_("Dotted Square"))
  enum_value (GEGL_RECURSIVE_SQUARE_STYLE_7, "style7",  N_("Complex Square Fret"))
enum_end (GeglRecursiveSquareStyle72325)

property_enum (pattern_style, _("Pattern Style"),
               GeglRecursiveSquareStyle72325, gegl_recursive_square_style72325,
               GEGL_RECURSIVE_SQUARE_STYLE_1)
    description (_("Select from 7 different recursive square patterns"))

property_color (pattern_color, _("Pattern Color"), "#00ff65")
    description (_("Color of the square pattern"))

property_color (background_color, _("Background Color"), "#f6ff7f")
    description (_("Background color for the pattern"))

property_double (scale, _("Pattern Scale"), 70.0)
    description (_("Size of each square element"))
    value_range (10.0, 400.0)

property_double (line_width, _("Line Width"), 4.0)
    description (_("Width of the pattern lines"))
    value_range (1.0, 40.0)

property_int (complexity, _("Complexity Level"), 2)
    description (_("Number of nested levels in fractal patterns"))
    value_range (1, 5)
    ui_meta ("visible", "pattern_style {style3, style7}")  // Only show for Fractal Square and Complex Square Fret

property_double (wave_amplitude, _("Wave Amplitude"), 0.3)
    description (_("Amplitude of wave patterns"))
    value_range (0.1, 0.8)
    ui_meta ("visible", "pattern_style {style4}")  // Only show for Wave Square

property_double (spiral_thickness, _("Spiral Thickness"), 0.2)
    description (_("Thickness of the spiral pattern lines"))
    value_range (0.05, 0.5)
    ui_meta ("visible", "pattern_style {style5}")  // Only show for Square Spiral

property_double (rotation, _("Rotation"), 0.0)
    description (_("Rotation angle in degrees"))
    value_range (0.0, 360.0)
    ui_meta ("unit", "degree")
    ui_meta ("direction", "ccw")


property_double (offset_x, _("Offset X"), 0.0)
    description (_("Horizontal offset position"))
    value_range (-500.0, 500.0)

property_double (offset_y, _("Offset Y"), 0.0)
    description (_("Vertical offset position"))
    value_range (-500.0, 500.0)

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     recursive_square
#define GEGL_OP_C_SOURCE recursivesquares.c

#include "gegl-op.h"

static void
prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "input", babl_format ("RGBA float"));
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
}

// Function to draw different recursive square patterns
static gboolean
is_recursive_square_segment(gdouble x, gdouble y, gdouble unit_size, gdouble line_width, 
                    gint complexity, gdouble wave_amplitude, gdouble spiral_thickness,
                    gdouble rotation, gdouble offset_x, gdouble offset_y,
                    GeglRecursiveSquareStyle72325 style)
{
    // Apply offsets
    gdouble transformed_x = x - offset_x;
    gdouble transformed_y = y - offset_y;
    
    // Apply rotation if needed
    if (rotation != 0.0) {
        gdouble angle_rad = rotation * G_PI / 180.0;
        gdouble cos_angle = cos(angle_rad);
        gdouble sin_angle = sin(angle_rad);
        
        // Rotate around origin
        gdouble orig_x = transformed_x;
        gdouble orig_y = transformed_y;
        transformed_x = orig_x * cos_angle - orig_y * sin_angle;
        transformed_y = orig_x * sin_angle + orig_y * cos_angle;
    }
    
    // Normalize coordinates within a unit cell
    gdouble nx = fmod(transformed_x, unit_size) / unit_size;
    gdouble ny = fmod(transformed_y, unit_size) / unit_size;
    gdouble lw = line_width / unit_size;
    
    gboolean in_pattern = FALSE;
    
    switch (style)
    {
        case GEGL_RECURSIVE_SQUARE_STYLE_1: // Classic Square
            // Classic square pattern
            in_pattern = (nx < lw) || (nx > 1.0 - lw) || (ny < lw) || (ny > 1.0 - lw) ||
                        (nx > 0.25 && nx < 0.75 && ny > 0.25 && ny < 0.75 && 
                         ((nx < 0.25 + lw) || (nx > 0.75 - lw) || 
                          (ny < 0.25 + lw) || (ny > 0.75 - lw)));
            break;
            
        case GEGL_RECURSIVE_SQUARE_STYLE_2: // Double Square
            // Nested square pattern with two levels
            in_pattern = (nx < lw) || (nx > 1.0 - lw) || (ny < lw) || (ny > 1.0 - lw) ||
                        (nx > 0.166 && nx < 0.833 && ny > 0.166 && ny < 0.833 && 
                         ((nx < 0.166 + lw) || (nx > 0.833 - lw) || 
                          (ny < 0.166 + lw) || (ny > 0.833 - lw))) ||
                        (nx > 0.333 && nx < 0.666 && ny > 0.333 && ny < 0.666 && 
                         ((nx < 0.333 + lw) || (nx > 0.666 - lw) || 
                          (ny < 0.333 + lw) || (ny > 0.666 - lw)));
            break;
            
        case GEGL_RECURSIVE_SQUARE_STYLE_3: // Fractal Square
            // Fractal pattern where each square contains a smaller square 
            {
                gdouble fractal_x = nx;
                gdouble fractal_y = ny;
                gdouble current_scale = 1.0;
                
                for (int level = 0; level < complexity; level++) {
                    gdouble cell_size = 1.0 / pow(2, level);
                    gint cell_x = (int)(fractal_x / cell_size);
                    gint cell_y = (int)(fractal_y / cell_size);
                    gdouble local_x = fmod(fractal_x, cell_size) / cell_size;
                    gdouble local_y = fmod(fractal_y, cell_size) / cell_size;
                    
                    // Check if in the square pattern at this level
                    if (local_x < lw * current_scale || local_x > 1.0 - lw * current_scale ||
                        local_y < lw * current_scale || local_y > 1.0 - lw * current_scale ||
                        (local_x > 0.25 && local_x < 0.75 && local_y > 0.25 && local_y < 0.75 &&
                         (local_x < 0.25 + lw * current_scale || local_x > 0.75 - lw * current_scale ||
                          local_y < 0.25 + lw * current_scale || local_y > 0.75 - lw * current_scale))) {
                        in_pattern = TRUE;
                        break;
                    }
                    
                    current_scale *= 0.5;
                }
            }
            break;
            
        case GEGL_RECURSIVE_SQUARE_STYLE_4: // Wave Square
            // Wave-like square pattern
            {
                gdouble wave1 = 0.5 + wave_amplitude * sin(nx * G_PI * 4.0);
                gdouble wave2 = 0.5 + wave_amplitude * sin(ny * G_PI * 4.0);
                
                in_pattern = (fabs(ny - wave1) < lw) || (fabs(nx - wave2) < lw) ||
                            (nx < lw) || (nx > 1.0 - lw) || (ny < lw) || (ny > 1.0 - lw);
                
                // Add connecting elements to create continuous pattern
                for (int i = 1; i <= 4; i++) {
                    gdouble phase = i * G_PI / 2.0;
                    gdouble connector_x = 0.5 + 0.25 * cos(phase);
                    gdouble connector_y = 0.5 + 0.25 * sin(phase);
                    if (sqrt(pow(nx - connector_x, 2) + pow(ny - connector_y, 2)) < lw * 2.0) {
                        in_pattern = TRUE;
                    }
                }
            }
            break;
            
        case GEGL_RECURSIVE_SQUARE_STYLE_5: // Square Spiral
            // Square spiral pattern with adjustable thickness
            {
                gdouble dist_to_center = fmax(fabs(nx - 0.5), fabs(ny - 0.5));
                gdouble spiral_turns = 2.0 * 2.0; // Use fixed complexity for this pattern
                gdouble spiral_param = fmod(dist_to_center * spiral_turns, 1.0);
                
                // Use the spiral_thickness parameter instead of fixed 0.2 value
                in_pattern = (spiral_param < spiral_thickness) || (spiral_param > 1.0 - spiral_thickness) ||
                            (fabs(nx - 0.5) < lw && dist_to_center > 0.2) ||
                            (fabs(ny - 0.5) < lw && dist_to_center > 0.2) ||
                            (nx < lw) || (nx > 1.0 - lw) || (ny < lw) || (ny > 1.0 - lw);
                
                // Add pattern breaks
                gdouble angle = atan2(ny - 0.5, nx - 0.5);
                gdouble break_pos = fmod(angle / (2.0 * G_PI) + 0.25, 1.0);
                if (break_pos < 0.1 && dist_to_center > 0.3 && dist_to_center < 0.7) {
                    in_pattern = !in_pattern;
                }
            }
            break;
            
        case GEGL_RECURSIVE_SQUARE_STYLE_6: // Infinity Square
            // Continuous square knot pattern
            {
                gdouble knot_x = fmod(nx * 2.0, 1.0);
                gdouble knot_y = fmod(ny * 2.0, 1.0);
                
                in_pattern = (fabs(knot_x - 0.5) < lw && knot_y > 0.25 && knot_y < 0.75) ||
                            (fabs(knot_y - 0.5) < lw && knot_x > 0.25 && knot_x < 0.75) ||
                            (fabs(knot_x - knot_y) < lw && knot_x > 0.25 && knot_x < 0.75 && knot_y > 0.25 && knot_y < 0.75) ||
                            (fabs(knot_x + knot_y - 1.0) < lw && knot_x > 0.25 && knot_x < 0.75 && knot_y > 0.25 && knot_y < 0.75);
                
                // Add connecting elements to ensure continuity
                for (int i = 0; i < 4; i++) {
                    gdouble phase = i * G_PI / 2.0;
                    gdouble conn_x = 0.5 + 0.35 * cos(phase);
                    gdouble conn_y = 0.5 + 0.35 * sin(phase);
                    if (sqrt(pow(nx - conn_x, 2) + pow(ny - conn_y, 2)) < lw * 1.5) {
                        in_pattern = TRUE;
                    }
                }
            }
            break;
            
        case GEGL_RECURSIVE_SQUARE_STYLE_7: // Complex Square Fret
            // Complex square fret pattern with multiple levels 
            {
                gdouble levels = complexity;
                for (int i = 0; i < levels; i++) {
                    gdouble margin = 0.1 * i;
                    gdouble inner_margin = 0.1 * (i + 1);
                    
                    if ((nx > margin && nx < 1.0 - margin && ny > margin && ny < 1.0 - margin) &&
                        !(nx > inner_margin && nx < 1.0 - inner_margin && ny > inner_margin && ny < 1.0 - inner_margin)) {
                        
                        // Check if on the fret pattern at this level
                        if ((fabs(nx - margin) < lw || fabs(nx - (1.0 - margin)) < lw ||
                             fabs(ny - margin) < lw || fabs(ny - (1.0 - margin)) < lw) ||
                            (nx > margin + 0.2 && nx < 1.0 - margin - 0.2 && 
                             (fabs(ny - (margin + 0.2)) < lw || fabs(ny - (1.0 - margin - 0.2)) < lw)) ||
                            (ny > margin + 0.2 && ny < 1.0 - margin - 0.2 && 
                             (fabs(nx - (margin + 0.2)) < lw || fabs(nx - (1.0 - margin - 0.2)) < lw))) {
                            in_pattern = TRUE;
                            break;
                        }
                    }
                }
            }
            break;
            
        default:
            in_pattern = (nx < lw) || (nx > 1.0 - lw) || (ny < lw) || (ny > 1.0 - lw);
            break;
    }
    
    return in_pattern;
}

static gboolean
process (GeglOperation       *operation,
         void                *in_buf,
         void                *out_buf,
         glong                n_pixels,
         const GeglRectangle *roi,
         gint                 level)
{
  GeglProperties *o = GEGL_PROPERTIES (operation);
  gfloat *in_pixel = (gfloat *) in_buf;
  gfloat *out_pixel = (gfloat *) out_buf;

  if (!in_buf || !out_buf || !roi || n_pixels <= 0)
    return FALSE;

  gfloat pattern_color[4];
  gfloat background_color[4];
  
  gegl_color_get_pixel (o->pattern_color, babl_format ("RGBA float"), pattern_color);
  gegl_color_get_pixel (o->background_color, babl_format ("RGBA float"), background_color);

  GeglRectangle *canvas = gegl_operation_source_get_bounding_box (operation, "input");
  if (!canvas || canvas->width <= 0 || canvas->height <= 0)
    return FALSE;

  gdouble unit_size = o->scale;
  gdouble line_width = o->line_width;
  gint complexity = o->complexity;
  gdouble wave_amplitude = o->wave_amplitude;
  gdouble spiral_thickness = o->spiral_thickness;
  gdouble rotation = o->rotation;
  gdouble offset_x = o->offset_x;
  gdouble offset_y = o->offset_y;
  GeglRecursiveSquareStyle72325 style = o->pattern_style;

  for (glong i = 0; i < n_pixels; i++)
  {
    gint x = (i % roi->width) + roi->x;
    gint y = (i / roi->width) + roi->y;

    if (x < 0 || x >= canvas->width || y < 0 || y >= canvas->height)
    {
      out_pixel[0] = in_pixel[0];
      out_pixel[1] = in_pixel[1];
      out_pixel[2] = in_pixel[2];
      out_pixel[3] = in_pixel[3];
      in_pixel += 4;
      out_pixel += 4;
      continue;
    }

    // Check if this pixel is part of the square pattern
    gboolean in_pattern = is_recursive_square_segment(x, y, unit_size, line_width, 
                                              complexity, wave_amplitude, spiral_thickness,
                                              rotation, offset_x, offset_y, style);

    if (in_pattern)
    {
      out_pixel[0] = pattern_color[0];
      out_pixel[1] = pattern_color[1];
      out_pixel[2] = pattern_color[2];
      out_pixel[3] = pattern_color[3];
    }
    else
    {
      out_pixel[0] = background_color[0];
      out_pixel[1] = background_color[1];
      out_pixel[2] = background_color[2];
      out_pixel[3] = background_color[3];
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
    "name",        "ai/lb:recursive-square-patterns",
    "title",       _("Recursive Square Patterns"),
    "reference-hash", "recursivesquare123",
    "description", _("Generates 7 different recursive square patterns including fractal designs and geometric variations with rotation and offset controls"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL",
    "gimp:menu-label", _("Recursive Square Patterns..."),
    NULL);
}

#endif
