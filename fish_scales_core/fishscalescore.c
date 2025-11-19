/* 
 *
 * Pippin (2006) for making GEGL 
 * Beaver for vibe coding 
 * DeepSeek for making most of the GEGL plugin
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

property_int(scale_type, _("Scale Type"), 7)
    description(_("Type of scale pattern to generate"))
    value_range(0, 7)
    ui_meta("enum-names", "Default|Rounded|Pointy|Flat|Irregular|Large|Tight|Asymmetrical")

property_color(scale_color, _("Scale Color"), "#4682b4")
    description(_("Main scale color"))

property_boolean(enable_gradient, _("Enable Gradient"), TRUE)
    description(_("Enable highlight/shadow gradient effects"))

property_boolean(enable_background, _("Enable Background"), FALSE)
    description(_("Enable background color"))

property_color(background_color, _("Background Color"), "#00567b")
    description(_("Background color (only used when enabled)"))

property_double(scale_size, _("Scale Size"), 100.0)
    description(_("Size of individual scales"))
    value_range(5.0, 250.0)
    ui_range(10.0, 250.0)

property_double(overlap, _("Vertical Overlap"), 0.3)
    description(_("How much scales overlap vertically"))
    value_range(0.1, 0.5)
    ui_range(0.1, 0.5)

property_double(horizontal_spacing, _("Horizontal Spacing"), 1.0)
    description(_("Horizontal scale spacing (lower = closer) Don't make this too low or it will be incoherrent. Due to a known bug I have to allow the user to reach values that ruin the effect."))
    value_range(0.0, 1.3)
    ui_range(0.0, 1.3)

property_double(rotation, _("Rotation"), 0.0)
    description(_("Pattern rotation in degrees"))
    value_range(0.0, 360.0)
    ui_meta("unit", "degree")
    ui_meta("direction", "ccw")

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     fishscalescore
#define GEGL_OP_C_SOURCE fishscalescore.c

#include "gegl-op.h"

static void prepare (GeglOperation *operation)
{
  gegl_operation_set_format (operation, "output", babl_format ("RGBA float"));
}

static GeglRectangle get_bounding_box (GeglOperation *operation)
{
  return gegl_rectangle_infinite_plane ();
}

static gdouble get_pattern_modifier(gint pattern, gdouble x, gdouble y)
{
  switch (pattern) {
    case 1: // Rounded
      return 1.0 + 0.2 * (1.0 - fabs(y));
    case 2: // Pointy
      return 1.0 + 0.6 * (1.0 - fabs(y));
    case 3: // Flat
      return 1.0;
    case 4: // Irregular
      return 1.0 + 0.3 * (1.0 - fabs(y)) * (0.8 + 0.2 * sin(x * 2.0));
    case 5: // Large
      return 1.0 + 0.5 * (1.0 - fabs(y));
    case 6: // Tight
      return 1.0 + 0.3 * (1.0 - fabs(y));
    case 7: // Asymmetrical
      return 1.0 + 0.4 * (1.0 - fabs(y)) * (y > 0 ? 0.8 : 1.2);
    default: // Default
      return 1.0 + 0.4 * (1.0 - fabs(y));
  }
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
  gdouble scale_color[4], background[4] = {0.0, 0.0, 0.0, 0.0};
  gdouble size = o->scale_size;
  gdouble overlap_amount = o->overlap;
  gdouble h_spacing = o->horizontal_spacing;
  const gdouble smoothness = 0.03;
  GeglRectangle rect = *result;
  gint x, y;

  // Convert rotation to radians
  gdouble rotation_rad = o->rotation * G_PI / 180.0;
  gdouble cos_rot = cos(rotation_rad);
  gdouble sin_rot = sin(rotation_rad);

  gegl_color_get_rgba (o->scale_color, &scale_color[0], &scale_color[1], &scale_color[2], &scale_color[3]);
  
  if (o->enable_background) {
    gegl_color_get_rgba (o->background_color, &background[0], &background[1], &background[2], &background[3]);
  }

  // Hexagonal grid parameters
  gdouble hex_width = size * 2.0 * h_spacing;
  gdouble hex_height = size * sqrt(3.0);

  for (y = rect.y; y < rect.y + rect.height; y++)
  {
    for (x = rect.x; x < rect.x + rect.width; x++)
    {
      gdouble px = (gdouble)x;
      gdouble py = (gdouble)y;
      
      // Apply rotation
      gdouble rot_x = px * cos_rot - py * sin_rot;
      gdouble rot_y = px * sin_rot + py * cos_rot;
      
      // Calculate grid position using rotated coordinates
      gint row = floor(rot_y / (hex_height * (1.0 - overlap_amount)));
      gdouble y_in_row = fmod(rot_y, hex_height * (1.0 - overlap_amount));
      
      // Offset every other row
      gdouble x_offset = (row % 2) * size * h_spacing;
      gdouble x_in_col = fmod(rot_x - x_offset, hex_width);
      
      // Convert to normalized hex coordinates
      gdouble hex_x = (x_in_col / (size * h_spacing)) - 1.0;
      gdouble hex_y = (y_in_row / (hex_height * (1.0 - overlap_amount))) * 2.0 - 1.0;
      
      // Fish scale shape with pattern modifier
      gdouble dist = sqrt(hex_x*hex_x + hex_y*hex_y);
      gdouble curved_dist = dist * get_pattern_modifier(o->scale_type, hex_x, hex_y);
      
      // Calculate coverage with fixed smoothness
      gdouble coverage = 1.0;
      gdouble edge = 1.0 - curved_dist;
      if (edge < smoothness) {
        coverage = edge / smoothness;
      }
      
      if (coverage > 0.0 && hex_y < 0.6 + smoothness/2.0)
      {
        gfloat out[4];
        
        if (o->enable_gradient)
        {
          // Calculate lighting for gradient effect
          gdouble highlight_amount = pow(1.0 - fmax(0.0, hex_y), 2.0) * 0.8;
          gdouble shadow_amount = pow(fmax(0.0, hex_y), 2.0) * 0.5;
          
          // Calculate highlight and shadow colors
          gdouble highlight[3] = {
            MIN(scale_color[0] * 1.3, 1.0),
            MIN(scale_color[1] * 1.3, 1.0),
            MIN(scale_color[2] * 1.3, 1.0)
          };
          gdouble shadow[3] = {
            scale_color[0] * 0.7,
            scale_color[1] * 0.7,
            scale_color[2] * 0.7
          };
          
          // Apply gradient effect
          out[0] = scale_color[0] * (1.0 - highlight_amount - shadow_amount) 
                 + highlight[0] * highlight_amount
                 + shadow[0] * shadow_amount;
          out[1] = scale_color[1] * (1.0 - highlight_amount - shadow_amount) 
                 + highlight[1] * highlight_amount
                 + shadow[1] * shadow_amount;
          out[2] = scale_color[2] * (1.0 - highlight_amount - shadow_amount) 
                 + highlight[2] * highlight_amount
                 + shadow[2] * shadow_amount;
          
          // Edge darkening
          gdouble edge_darken = pow(1.0 - curved_dist, 2.0) * 0.3 * coverage;
          out[0] = out[0] * (1.0 - edge_darken) + shadow[0] * edge_darken;
          out[1] = out[1] * (1.0 - edge_darken) + shadow[1] * edge_darken;
          out[2] = out[2] * (1.0 - edge_darken) + shadow[2] * edge_darken;
        }
        else
        {
          // Solid color without gradient
          out[0] = scale_color[0];
          out[1] = scale_color[1];
          out[2] = scale_color[2];
        }
        
        // Final color blending
        if (o->enable_background) {
          out[0] = out[0] * coverage + background[0] * (1.0 - coverage);
          out[1] = out[1] * coverage + background[1] * (1.0 - coverage);
          out[2] = out[2] * coverage + background[2] * (1.0 - coverage);
          out[3] = 1.0;
        } else {
          out[3] = coverage;
        }
        
        gegl_buffer_set (output, GEGL_RECTANGLE (x, y, 1, 1), 0, babl_format ("RGBA float"), out, GEGL_AUTO_ROWSTRIDE);
      }
      else
      {
        // Background or transparent
        gfloat out[4] = {background[0], background[1], background[2], o->enable_background ? 1.0 : 0.0};
        gegl_buffer_set (output, GEGL_RECTANGLE (x, y, 1, 1), 0, babl_format ("RGBA float"), out, GEGL_AUTO_ROWSTRIDE);
      }
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
    "name",        "ai/lb:fish-scales-core",
    "title",       _("Fish Scales Core"),
    "reference-hash", "fishscales-patterns",
   "categories",  "hidden", 
    "description", _("Generates fish scales with different pattern types and optional gradient effects"),
    NULL);
}

#endif
