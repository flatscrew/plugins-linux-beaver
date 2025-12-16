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
 * 2025 Beaver RGB Glitch
* 
* Graph here


id=beforestart gegl:dst aux=[ ref=beforestart
id=start
src aux=[  ref=start multiply aux=[ color value=#ff0900  ] ]
id=red
src aux=[  ref=start  multiply aux=[ color value=#00ff00  ]  ]
 id=green
src aux=[  ref=start  multiply aux=[  color value=#0000ff   ]   ]
id=blue
]

color-overlay value=black


screen  aux=[ ref=red  translate x=6 ]
screen aux=[ ref=green  ]
screen aux=[ ref=blue translate y=-6  ]

id=glitchfinal

dst-in aux=[ ref=beforestart ]
dst-in aux=[   ref=beforestart   ]
crop aux=[ ref=beforestart ] 

id=a over aux=[ ref=a

inner-glow value=white radius=20 opacity=2 grow-radius=1
multiply aux=[ ref=beforestart ]
]

*/

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

property_double (x_red, _("X Red"), 9.0)
    description(_("Move the glitch horizontally"))
    value_range (-30.0, 30.0)

property_double (x_green, _("X Green"), 0.0)
    description(_("Move the glitch horizontally"))
    value_range (-30.0, 30.0)

property_double (x_blue, _("X Blue"), -9.0)
    description(_("Move the glitch horizontally"))
    value_range (-30.0, 30.0)

property_double (y_red, _("Y Red"), 0.0)
    description(_("Move the glitch vertically"))
    value_range (-30.0, 30.0)

property_double (y_green, _("Y Green"), 0.0)
    description(_("Move the glitch vertically"))
    value_range (-30.0, 30.0)

property_double (y_blue, _("Y Blue"), 0.0)
    description(_("Move the glitch vertically"))
    value_range (-30.0, 30.0)

property_double (border, _("Unaffected Border"), 30.0)
    description(_("Feather radius of border unaffected by glitching"))
    value_range (0.0, 40.0)

#else

#define GEGL_OP_META
#define GEGL_OP_NAME     rgbglitch
#define GEGL_OP_C_SOURCE rgbglitch.c

#include "gegl-op.h"

static void
attach (GeglOperation *operation)
{
  GeglNode *gegl   = operation->node;
  GeglNode *output = gegl_node_get_output_proxy (gegl, "output");
  GeglNode *input  = gegl_node_get_input_proxy (gegl, "input");

  GeglNode *multiply  = gegl_node_new_child (gegl,
                                          "operation", "gegl:multiply",
                                          NULL);


  GeglNode *idref1  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop",
                                          NULL);

  GeglNode *idref2  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop",
                                          NULL);

  GeglNode *idref3  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop",
                                          NULL);

  GeglNode *idrefred  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop",
                                          NULL);

  GeglNode *idrefgreen  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop",
                                          NULL);

  GeglNode *idrefblue  = gegl_node_new_child (gegl,
                                          "operation", "gegl:nop",
                                          NULL);
 GeglColor *whitecolor = gegl_color_new ("#ffffff");
  GeglNode *ig  = gegl_node_new_child (gegl,
                                          "operation", "gegl:inner-glow", "value", whitecolor, "opacity", 2.0,   
                                          NULL);

  GeglNode *screen1  = gegl_node_new_child (gegl,
                                          "operation", "gegl:screen",
                                          NULL);

  GeglNode *screen2  = gegl_node_new_child (gegl,
                                          "operation", "gegl:screen",
                                          NULL);

  GeglNode *screen3  = gegl_node_new_child (gegl,
                                          "operation", "gegl:screen",
                                          NULL);

  GeglNode *multiply1  = gegl_node_new_child (gegl,
                                          "operation", "gegl:multiply",
                                          NULL);

  GeglNode *multiply2  = gegl_node_new_child (gegl,
                                          "operation", "gegl:multiply",
                                          NULL);

  GeglNode *multiply3  = gegl_node_new_child (gegl,
                                          "operation", "gegl:multiply",
                                          NULL);

  GeglNode *conformtoabove  = gegl_node_new_child (gegl,
                                          "operation", "gegl:dst-in",
                                          NULL);

  GeglNode *invisible  = gegl_node_new_child (gegl,
                                          "operation", "gegl:dst",
                                          NULL);
  GeglNode *crop  = gegl_node_new_child (gegl,
                                          "operation", "gegl:crop",
                                          NULL);

 GeglColor *blackcolor = gegl_color_new ("#000000");
  GeglNode *black  = gegl_node_new_child (gegl,
                                          "operation", "gegl:color-overlay", "value", blackcolor,
                                          NULL);
  GeglNode *translate1  = gegl_node_new_child (gegl,
                                          "operation", "gegl:translate",
                                          NULL);
  GeglNode *translate2  = gegl_node_new_child (gegl,
                                          "operation", "gegl:translate",
                                          NULL);

  GeglNode *translate3  = gegl_node_new_child (gegl,
                                          "operation", "gegl:translate",
                                          NULL);

  GeglNode *normal  = gegl_node_new_child (gegl,
                                          "operation", "gegl:over",
                                          NULL);

  GeglNode *replace1  = gegl_node_new_child (gegl,
                                          "operation", "gegl:src",
                                          NULL);
  GeglNode *replace2  = gegl_node_new_child (gegl,
                                          "operation", "gegl:src",
                                          NULL);
  GeglNode *replace3  = gegl_node_new_child (gegl,
                                          "operation", "gegl:src",
                                          NULL);

 GeglColor *redcolor = gegl_color_new ("#ff0900");
 GeglColor *greencolor = gegl_color_new ("#00ff00");
 GeglColor *bluecolor = gegl_color_new ("#0000ff");

  GeglNode *red  = gegl_node_new_child (gegl,
                                          "operation", "gegl:color", "value", redcolor,
                                          NULL);

  GeglNode *green  = gegl_node_new_child (gegl,
                                          "operation", "gegl:color", "value", greencolor,
                                          NULL);

  GeglNode *blue  = gegl_node_new_child (gegl,
                                          "operation", "gegl:color", "value", bluecolor,
                                          NULL);



/*Note to people studying the code, this graph is extremely complex, far more complicated then most gegl graphs of mine*/
  gegl_node_link_many (input, invisible, black, screen1, screen2, screen3, idref2, conformtoabove, crop, idref3, normal, output, NULL);
  gegl_node_connect (invisible, "aux", idrefblue, "output");
  gegl_node_link_many (input, idref1, replace1, idrefred, replace2, idrefgreen, replace3, idrefblue,  NULL);
  gegl_node_link_many (idref1, multiply1, NULL);
  gegl_node_link_many (idref1, multiply2, NULL);
  gegl_node_link_many (idref1, multiply3, NULL);
  gegl_node_connect (replace1, "aux", multiply1, "output");
  gegl_node_connect (replace2, "aux", multiply2, "output");
  gegl_node_connect (replace3, "aux", multiply3, "output");
  gegl_node_connect (multiply1, "aux", red, "output");
  gegl_node_connect (multiply2, "aux", green, "output");
  gegl_node_connect (multiply3, "aux", blue, "output");
  gegl_node_connect (screen1, "aux", translate1, "output");
  gegl_node_connect (screen2, "aux", translate2, "output");
  gegl_node_connect (screen3, "aux", translate3, "output");
  gegl_node_link_many (idrefred, translate1,  NULL);
  gegl_node_link_many (idrefgreen, translate2,  NULL);
  gegl_node_link_many (idrefblue, translate3,  NULL);
  gegl_node_connect (conformtoabove, "aux", input, "output");
  gegl_node_connect (crop, "aux", input, "output");
  gegl_node_connect (normal, "aux", multiply, "output");
  gegl_node_link_many (idref3, ig, multiply, NULL);
  gegl_node_connect (multiply, "aux", input, "output");

  gegl_operation_meta_redirect (operation, "x_red",    translate1, "x");
  gegl_operation_meta_redirect (operation, "y_red",    translate1, "y");
  gegl_operation_meta_redirect (operation, "x_green",  translate2, "x");
  gegl_operation_meta_redirect (operation, "y_green",  translate2, "y");
  gegl_operation_meta_redirect (operation, "x_blue",   translate3, "x");
  gegl_operation_meta_redirect (operation, "y_blue",   translate3, "y");
  gegl_operation_meta_redirect (operation, "border",   ig, "radius");

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;
  operation_class->threaded = FALSE;

  gegl_operation_class_set_keys (operation_class,
    "name",           "lb:rgb-glitch",
    "title",          _("RGB Glitch"),
    "reference-hash", "imamessimaloserimahaterimmauser",
    "description", _("RGB Glitch the image"),
    "gimp:menu-path", "<Image>/Filters/Artistic",
    "gimp:menu-label", _("RGB Glitch..."),
                                 NULL);
}

#endif
