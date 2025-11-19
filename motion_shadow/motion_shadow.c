/* This file is an image processing operation for GEGL
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
* 
* Beaver 2024, GEGL Motion Shadow
* 
* id=1 dst-over aux=[ ref=1 gaussian-blur std-dev-x=5 clip-extent=none std-dev-y=40
color-overlay value=red opacity value=3 ] 
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_gaussian_blur_filter2plugin)
   enum_value (GEGL_GAUSSIAN_BLUR_FILTER2_AUTO, "auto", N_("Auto"))
   enum_value (GEGL_GAUSSIAN_BLUR_FILTER2_FIR,  "fir",  N_("FIR"))
   enum_value (GEGL_GAUSSIAN_BLUR_FILTER2_IIR,  "iir",  N_("IIR"))
enum_end (GeglGaussianBlurFilter2plugin)

enum_start (gegl_gaussian_blur_policyplugin)
   enum_value (GEGL_GAUSSIAN_BLUR_ABYSS_NONE,  "none",   N_("None"))
   enum_value (GEGL_GAUSSIAN_BLUR_ABYSS_CLAMP, "clamp",  N_("Clamp"))
   enum_value (GEGL_GAUSSIAN_BLUR_ABYSS_BLACK, "black",  N_("Black"))
   enum_value (GEGL_GAUSSIAN_BLUR_ABYSS_WHITE, "white",  N_("White"))
enum_end (GeglGaussianBlurPolicyplugin)

property_double (std_dev_x, _("Horizontal Shadow"), 5.0)
   description (_("Motion blur for the horizontal shadow"))
   value_range (0.0, 1500.0)
   ui_range    (0.24, 100.0)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")


property_double (std_dev_y, _("Vertical Shadow"), 50.0)
   description (_("Motion blur for the vertical shadow"))
   value_range (0.0, 1500.0)
   ui_range    (0.24, 100.0)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")

property_double (opacity, _("Hyper Opacity"), 2.0)
   description (_("Hyper opacity value"))
   value_range (0.0, 4.0)
   ui_range    (0.0, 4.0)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")

property_color (value, _("Color"), "#ff0000")
    description (_("The color to paint over the input"))


property_enum (filter, _("Filter"),
               GeglGaussianBlurFilter2plugin, gegl_gaussian_blur_filter2plugin,
               GEGL_GAUSSIAN_BLUR_FILTER2_AUTO)
   description (_("How the gaussian kernel is discretized"))
    ui_meta    ("role", "output-extent")

property_enum (abyss_policy, _("Abyss policy"), GeglGaussianBlurPolicyplugin,
               gegl_gaussian_blur_policyplugin, GEGL_GAUSSIAN_BLUR_ABYSS_NONE)
   description (_("How image edges are handled"))
    ui_meta    ("role", "output-extent")

property_boolean (clip_extent, _("Clip to the input extent"), FALSE)
   description (_("Should the output extent be clipped to the input extent"))
    ui_meta    ("role", "output-extent")


#else

#define GEGL_OP_META
#define GEGL_OP_NAME     motion_shadow
#define GEGL_OP_C_SOURCE motion_shadow.c

#include "gegl-op.h"

static void
attach (GeglOperation *operation)
{
  GeglNode *gegl   = operation->node;
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");

  GeglNode *vblur  = gegl_node_new_child (gegl,
                                          "operation", "gegl:gblur-1d",
                                          "orientation", 1,
                                          NULL);

  GeglNode *hblur  = gegl_node_new_child (gegl,
                                          "operation", "gegl:gblur-1d",
                                          "orientation", 0,
                                          NULL);

  GeglNode *behind  = gegl_node_new_child (gegl,
                                          "operation", "gegl:dst-over",
                                          NULL);

  GeglNode *opacity  = gegl_node_new_child (gegl,
                                          "operation", "gegl:opacity",
                                          NULL);

  GeglNode *color  = gegl_node_new_child (gegl,
                                          "operation", "gegl:color-overlay",
                                          NULL);

  GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");

  gegl_node_link_many (input,  behind,  output,  NULL);
  gegl_node_connect (behind, "aux", opacity, "output");
  gegl_node_link_many (input, hblur, vblur, color,  opacity,   NULL);

  gegl_operation_meta_redirect (operation, "std-dev-x",    hblur, "std-dev");
  gegl_operation_meta_redirect (operation, "abyss-policy", hblur, "abyss-policy");
  gegl_operation_meta_redirect (operation, "filter",       hblur, "filter");
  gegl_operation_meta_redirect (operation, "clip-extent",  hblur, "clip-extent");

  gegl_operation_meta_redirect (operation, "std-dev-y",    vblur, "std-dev");
  gegl_operation_meta_redirect (operation, "abyss-policy", vblur, "abyss-policy");
  gegl_operation_meta_redirect (operation, "filter",       vblur, "filter");
  gegl_operation_meta_redirect (operation, "clip-extent",  vblur, "clip-extent");
  gegl_operation_meta_redirect (operation, "opacity",  opacity, "value");
  gegl_operation_meta_redirect (operation, "value",  color, "value");
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_class->threaded = FALSE;

  gegl_operation_class_set_keys (operation_class,
    "name",           "lb:motion-shadow",
    "title",          _("Motion Shadow"),
    "reference-hash", "vaporwavechristmas",
    "description", _("Motion Shadow"),
    "gimp:menu-path", "<Image>/Filters/Text Styling",
                                 NULL);
}

#endif
