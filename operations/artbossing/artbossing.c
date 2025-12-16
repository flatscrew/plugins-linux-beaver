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
 * 2006 Credit to Pippin for major GEGL contributions
 * 2025 Beaver Art Bossing
* 
* Graph here
id=0
gaussian-blur std-dev-x=15 std-dev-y=60
svg:darken aux=[ ref=0 ]
dst-in aux=[ ref=0 ]
id=1 hard-light aux=[ ref=1 
emboss depth=2 elevation=30 mean-curvature-blur iterations=9
gamma value=2
opacity value=0.5 ]
saturation scale=1.2
shadows-highlights highlights=90.0
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_double (bump, _("Bump"), 0.0)
   description (_("Make the effect more bumpish"))
   value_range (0.0, 3.0)
   ui_range    (0.0, 3.0)
   ui_meta     ("unit", "pixel-distance")
  ui_steps      (0.1, 1.0)


property_double (std_dev_x, _("Dark blended Blur X"), 20.0)
   description (_("Standard deviation for the horizontal axis"))
   value_range (0.0, 60.0)
   ui_range    (0.24, 60.0)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")
   ui_meta     ("axis", "x")
  ui_steps      (0.1, 1.0)

property_double (std_dev_y, _("Dark blended Blur Y"), 20.0)
   description (_("Standard deviation for the vertical axis"))
   value_range (0.0, 60.0)
   ui_range    (0.24, 60.0)
   ui_gamma    (3.0)
   ui_meta     ("unit", "pixel-distance")
   ui_meta     ("axis", "y")
  ui_steps      (0.1, 1.0)

property_double (saturation, _("Saturation"), 1.2)
    description(_("Scale, strength of effect"))
    value_range (0.0, 1.5)
    ui_range (0.0, 1.5)
  ui_steps      (0.1, 0.5)

property_double (hardlight, _("Hardlight Composer's Opacity"), 0.5)
    description(_("The hardlight blend which contains emboss and gamma inside it, has an opacity slider and this is its control"))
    value_range (0.5, 1.0)
    ui_range (0.5, 1.0)

property_double (gamma, _("Gamma"), 2.0)
    description(_("Gamma"))
    value_range (1.0, 2.0)
    ui_range (1.0, 2.0)
  ui_steps      (0.1, 0.5)

property_double (azimuth, _("Azimuth"), 75)
    description(_("Artistic emboss azimuth"))
    value_range (0, 180)
  ui_steps      (0.5, 2.0)

property_int (depth, _("Depth"), 3)
    description(_("Artistic emboss depth"))
    value_range (1, 25)
    ui_range (1, 25)

property_double (elevation, _("Elevation"), 30)
    description(_("Artistic emboss elevation"))
    value_range (25, 45)
    ui_range (25, 45)
  ui_steps      (0.1, 1.0)

property_int (smooth, _("Smooth"), 9)
    description(_("Smooth the artistic emboss"))
    value_range (2, 9)
    ui_range (2, 9)


#else

#define GEGL_OP_META
#define GEGL_OP_NAME     artbossing
#define GEGL_OP_C_SOURCE artbossing.c

#include "gegl-op.h"

static void
attach (GeglOperation *operation)
{
  GeglNode *gegl   = operation->node;
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");
  GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");

  GeglNode *emboss  = gegl_node_new_child (gegl,
                                          "operation", "gegl:emboss",
                                          NULL);
  GeglNode *blur  = gegl_node_new_child (gegl,
                                          "operation", "gegl:gaussian-blur", "std-dev-x", 15.0, "std-dev-y", 60.0, 
                                          NULL);
  GeglNode *darken  = gegl_node_new_child (gegl,
                                          "operation", "gegl:darken",
                                          NULL);
  GeglNode *hardlight  = gegl_node_new_child (gegl,
                                          "operation", "gegl:hard-light",
                                          NULL);
  GeglNode *saturation  = gegl_node_new_child (gegl,
                                          "operation", "gegl:saturation", "scale", 1.2, 
                                          NULL);
  GeglNode *shadowshighlights  = gegl_node_new_child (gegl,
                                          "operation", "gegl:shadows-highlights", "highlights", 90.0,
                                          NULL);

  GeglNode *gamma  = gegl_node_new_child (gegl,
                                          "operation", "gegl:gamma", "value", 1.5, 
                                          NULL);

  GeglNode *opacity  = gegl_node_new_child (gegl,
                                          "operation", "gegl:opacity", "value", 0.5, 
                                          NULL);

  GeglNode *conform2contentabove  = gegl_node_new_child (gegl,
                                          "operation", "gegl:dst-in", 
                                          NULL);

  GeglNode *idref  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop", 
                                          NULL);

  GeglNode *smooth  = gegl_node_new_child (gegl,
                                          "operation", "gegl:mean-curvature-blur", "iterations", 9, 
                                          NULL);

  GeglNode *gaus  = gegl_node_new_child (gegl,
                                          "operation", "gegl:gaussian-blur", 
                                          NULL);

  GeglNode *input2  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop", 
                                          NULL);


  gegl_node_link_many (input, gaus, input2, blur, darken, conform2contentabove, idref, hardlight, saturation, shadowshighlights, output, NULL);
  gegl_node_connect (darken, "aux", input2, "output");
  gegl_node_connect (conform2contentabove, "aux", input2, "output");
  gegl_node_connect (hardlight, "aux", opacity, "output");
  gegl_node_link_many (idref, emboss, smooth, gamma, opacity, NULL);

  gegl_operation_meta_redirect (operation, "std-dev-x",    blur, "std-dev-x");
  gegl_operation_meta_redirect (operation, "std-dev-y",    blur, "std-dev-y");
  gegl_operation_meta_redirect (operation, "smooth",    smooth, "iterations");
  gegl_operation_meta_redirect (operation, "saturation",    saturation, "scale");
  gegl_operation_meta_redirect (operation, "depth",    emboss, "depth");
  gegl_operation_meta_redirect (operation, "elevation",    emboss, "elevation");
  gegl_operation_meta_redirect (operation, "azimuth",    emboss, "azimuth");
  gegl_operation_meta_redirect (operation, "hardlight",    opacity, "value");
  gegl_operation_meta_redirect (operation, "gamma",    gamma, "value");
  gegl_operation_meta_redirect (operation, "bump",    gaus, "std-dev-x");
  gegl_operation_meta_redirect (operation, "bump",    gaus, "std-dev-y");
}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_class->threaded = FALSE;

  gegl_operation_class_set_keys (operation_class,
    "name",           "lb:art-bossing",
    "title",          _("Art Bossed"),
    "reference-hash", "3faflledwithcuterainbows",
    "description", _("An artistic emboss"),
    "gimp:menu-path", "<Image>/Filters/Artistic",
    "gimp:menu-label", _("Art Bossing..."),
                                 NULL);
}

#endif
