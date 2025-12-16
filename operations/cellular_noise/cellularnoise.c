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
 * 2025 Beaver modifying mostly Grok's work. Deep Seek helped a little too
 */

#include "config.h"
#include <glib/gi18n-lib.h>
#include <math.h>
#include <gegl.h>
#include <gegl-plugin.h>

#ifdef GEGL_PROPERTIES

enum_start (gegl_noise_type2529)
  enum_value (GEGL_NOISE_TYPE_CELLULAR,    "cellular",    N_("Cellular"))
  enum_value (GEGL_NOISE_TYPE_WORLEY,      "worley",      N_("Worley"))
  enum_value (GEGL_NOISE_TYPE_VORONOI,     "voronoi",     N_("Voronoi"))
  enum_value (GEGL_NOISE_TYPE_CELLULAR_CRACKED, "cellular_cracked", N_("Cellular Cracked"))
  enum_value (GEGL_NOISE_TYPE_CELLULAR_MOSAIC, "cellular_mosaic", N_("Cellular Mosaic"))
  enum_value (GEGL_NOISE_TYPE_RUSTY_PATINA, "rusty_patina", N_("Rusty Patina"))
  enum_value (GEGL_NOISE_TYPE_RIPPLED_DOTS, "rippled_dots", N_("Rippled Dots"))
  enum_value (GEGL_NOISE_TYPE_CELLULAR_SHARDS, "cellular_shards", N_("Cellular Shards"))
  enum_value (GEGL_NOISE_TYPE_CRYSTAL_LATTICE, "crystal_lattice", N_("Crystal Lattice"))
  enum_value (GEGL_NOISE_TYPE_ORGANIC_BLOBS, "organic_blobs", N_("Organic Blobs"))
  enum_value (GEGL_NOISE_TYPE_FRACTURED_TILES, "fractured_tiles", N_("Fractured Tiles"))
  enum_value (GEGL_NOISE_TYPE_BUBBLE_FOAM, "bubble_foam", N_("Bubble Foam"))
  enum_value (GEGL_NOISE_TYPE_VORONOI_RIPPLES, "voronoi_ripples", N_("Voronoi Ripples"))
  enum_value (GEGL_NOISE_TYPE_SQUARE_CELLS, "square_cells", N_("Square Cells"))
  enum_value (GEGL_NOISE_TYPE_JELLY_BEAN, "jelly_bean", N_("Jelly Bean Cells"))
  enum_value (GEGL_NOISE_TYPE_HEXAGONAL, "hexagonal", N_("Hexagonal Cells"))
  enum_value (GEGL_NOISE_TYPE_ROUNDED_SQUARES, "rounded_squares", N_("Rounded Square Cells"))
enum_end (GeglNoiseType2529)

enum_start (gegl_cell_shape231)
  enum_value (GEGL_CELL_SHAPE_CIRCLE,  "circle",  N_("Circle"))
  enum_value (GEGL_CELL_SHAPE_SQUARE,  "square",  N_("Square"))
  enum_value (GEGL_CELL_SHAPE_DIAMOND, "diamond", N_("Diamond"))
  enum_value (GEGL_CELL_SHAPE_OVAL,    "oval",    N_("Oval"))
  enum_value (GEGL_CELL_SHAPE_STAR,    "star",    N_("Star"))
/*  enum_value (GEGL_CELL_SHAPE_FLOWER,  "flower",  N_("Flower")) */ 
enum_end (GeglCellShape231)

property_enum (noise_type, _("Noise Type"),
               GeglNoiseType2529, gegl_noise_type2529,
               GEGL_NOISE_TYPE_CELLULAR)
    description (_("Type of noise pattern to generate"))

property_enum (cell_shape, _("Cell Shape"),
               GeglCellShape231, gegl_cell_shape231,
               GEGL_CELL_SHAPE_CIRCLE)
    description (_("Shape of the cells for cellular noise patterns"))
    ui_meta ("visible", "noise_type {cellular,voronoi,cellular_shards,square_cells,jelly_bean,hexagonal,rounded_squares}")

property_double (scale, _("Scale"), 180.0)
    description (_("Scale of the noise pattern (higher values create smaller, finer noise)"))
    value_range (15.0, 600.0)
    ui_range (85.0, 200.0)

property_double (roughness, _("Roughness and Distortion"), 3.5)
    description (_("Roughness of the noise (higher values create more distorted patterns)"))
    value_range (0.0, 12.0)
    ui_range (0.0, 8.0)
    ui_meta ("visible", "noise_type {cellular_cracked,cellular_mosaic,rusty_patina,rippled_dots,cellular_shards,organic_blobs,fractured_tiles,voronoi_ripples}")

property_int (octaves, _("Octaves"), 1)
    description (_("Number of octaves for fractal summation (adds finer details)"))
    value_range (1, 3)
    ui_range (1, 3)
    ui_meta ("visible", "noise_type {rusty_patina}")

property_double (cell_jitter, _("Cell Jitter"), 0.0)
    description (_("Randomness of cell point positions (higher values create more irregular cells)"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)
    ui_meta ("visible", "noise_type {cellular,worley,voronoi,cellular_shards,crystal_lattice,bubble_foam,voronoi_ripples,square_cells,jelly_bean,hexagonal,rounded_squares}")

property_double (edge_sharpness, _("Edge Sharpness"), 0.0)
    description (_("Sharpness of cell edges (positive values sharpen, negative values soften)"))
    value_range (-1.0, 1.0)
    ui_range (-1.0, 1.0)
    ui_meta ("visible", "noise_type {cellular,worley,voronoi,cellular_shards,crystal_lattice,square_cells,jelly_bean,hexagonal,rounded_squares}")

property_double (cluster_factor, _("Cluster Factor"), 0.5)
    description (_("How tightly cells cluster together (higher values create tighter clusters)"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)
    ui_meta ("visible", "noise_type {}")

property_double (softness, _("Softness"), 0.5)
    description (_("Smoothness of cell edges (higher values create softer edges)"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)
    ui_meta ("visible", "noise_type {organic_blobs,jelly_bean}")

property_double (fracture_intensity, _("Fracture Intensity"), 0.5)
    description (_("Intensity of fractures within cells (higher values create more fractures)"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)
    ui_meta ("visible", "noise_type {fractured_tiles}")

property_double (ripple_strength, _("Ripple Strength"), 0.5)
    description (_("Strength of ripples radiating from cell centers (higher values create stronger ripples)"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)
    ui_meta ("visible", "noise_type {voronoi_ripples}")

property_double (detail_scale, _("Detail Scale"), 1.0)
    description (_("Scale of fine details in the noise pattern (smaller for finer details, larger for coarser)"))
    value_range (0.5, 4.0)
    ui_range (0.5, 4.0)
    ui_meta ("visible", "noise_type {rusty_patina,fractured_tiles,voronoi_ripples}")

property_double (cell_distortion, _("Cell Distortion"), 0.0)
    description (_("Distorts the shape of cells (higher values create more warped cells)"))
    value_range (0.0, 1.0)
    ui_range (0.0, 1.0)
    ui_meta ("visible", "noise_type {cellular,voronoi,cellular_shards,bubble_foam,organic_blobs,square_cells,jelly_bean,hexagonal,rounded_squares}")

property_double (cell_stretch, _("Cell Stretch"), 1.0)
    description (_("Stretches cells horizontally (lower values compress, higher values stretch)"))
    value_range (0.5, 2.0)
    ui_range (0.5, 2.0)
    ui_meta ("visible", "noise_type {cellular,voronoi,cellular_shards,bubble_foam,organic_blobs,jelly_bean}")

property_seed (seed, _("Seed"), rand)
    description (_("Random seed for the noise pattern"))

#else

#define GEGL_OP_POINT_FILTER
#define GEGL_OP_NAME     cellularnoise
#define GEGL_OP_C_SOURCE cellularnoise.c

#include "gegl-op.h"

// Simple hash function for randomization
static gfloat
hash (gfloat x, gfloat y, gint seed)
{
  gint hash = (gint)(x * 73856093 + y * 19349663 + seed * 83492791) & 0x7fffffff;
  return (hash % 10000) / 10000.0;
}

// Value noise implementation (needed for Rusty Patina)
static gfloat
value_noise (gfloat x, gfloat y, gint seed)
{
  gint xi = (gint) x;
  gint yi = (gint) y;
  gfloat xf = x - xi;
  gfloat yf = y - yi;

  gfloat v00 = hash (xi, yi, seed);
  gfloat v10 = hash (xi + 1, yi, seed);
  gfloat v01 = hash (xi, yi + 1, seed);
  gfloat v11 = hash (xi + 1, yi + 1, seed);

  gfloat u = xf * xf * (3.0 - 2.0 * xf);
  gfloat v = yf * yf * (3.0 - 2.0 * yf);

  gfloat nx0 = v00 + u * (v10 - v00);
  gfloat nx1 = v01 + u * (v11 - v01);

  return nx0 + v * (nx1 - nx0);
}

// Perlin noise implementation (needed for Rippled Dots, Voronoi Ripples, and cell distortion)
static gfloat
perlin_noise (gfloat x, gfloat y, gint seed)
{
  gint xi = (gint) x;
  gint yi = (gint) y;
  gfloat xf = x - xi;
  gfloat yf = y - yi;

  gfloat g00 = hash (xi, yi, seed) * 2.0 * G_PI;
  gfloat g10 = hash (xi + 1, yi, seed) * 2.0 * G_PI;
  gfloat g01 = hash (xi, yi + 1, seed) * 2.0 * G_PI;
  gfloat g11 = hash (xi + 1, yi + 1, seed) * 2.0 * G_PI;

  gfloat d00 = cos (g00) * xf + sin (g00) * yf;
  gfloat d10 = cos (g10) * (xf - 1.0) + sin (g10) * yf;
  gfloat d01 = cos (g01) * xf + sin (g01) * (yf - 1.0);
  gfloat d11 = cos (g11) * (xf - 1.0) + sin (g11) * (yf - 1.0);

  gfloat u = xf * xf * (3.0 - 2.0 * xf);
  gfloat v = yf * yf * (3.0 - 2.0 * yf);

  gfloat nx0 = d00 + u * (d10 - d00);
  gfloat nx1 = d01 + u * (d11 - d01);
  return nx0 + v * (nx1 - nx0);
}

// Cellular/Worley noise (distance to nearest point) with new sliders and cell shape
static gfloat
cellular_noise (gfloat x, gfloat y, gint seed, gfloat jitter, gfloat sharpness, gfloat scale, GeglCellShape231 shape)
{
  gint xi = (gint) x;
  gint yi = (gint) y;
  gfloat min_dist = 1000.0;

  // Dynamically adjust the sampling range based on scale
  gint range = (scale < 50.0) ? 3 : 2; // Use a larger range for scale < 50

  for (gint i = -range; i <= range; i++)
    for (gint j = -range; j <= range; j++)
    {
      gfloat px = xi + i + hash (xi + i, yi + j, seed);
      gfloat py = yi + j + hash (xi + i + 1, yi + j, seed);

      gfloat jitter_x = (hash (xi + i, yi + j + 1, seed) - 0.5) * jitter;
      gfloat jitter_y = (hash (xi + i + 1, yi + j + 1, seed) - 0.5) * jitter;
      px += jitter_x;
      py += jitter_y;

      gfloat dx = x - px;
      gfloat dy = y - py;
      gfloat dist;

      switch (shape)
      {
        case GEGL_CELL_SHAPE_CIRCLE:
          dist = sqrt (dx * dx + dy * dy); // Euclidean distance
          break;

        case GEGL_CELL_SHAPE_SQUARE:
          dist = fabs (dx) + fabs (dy); // Manhattan distance
          break;

        case GEGL_CELL_SHAPE_DIAMOND:
          dist = fmax (fabs (dx), fabs (dy)); // Chebyshev distance
          break;

        case GEGL_CELL_SHAPE_OVAL:
          dist = sqrt (dx * dx * 0.5 + dy * dy * 2.0); // Scaled Euclidean for elongation
          break;

        case GEGL_CELL_SHAPE_STAR:
          {
            gfloat angle = atan2 (dy, dx);
            gfloat radius = sqrt (dx * dx + dy * dy);
            dist = radius * (1.0 + 0.3 * sin (5.0 * angle)); // 5-pointed star
          }
          break;

/*        case GEGL_CELL_SHAPE_FLOWER: */
          {
            gfloat angle = atan2 (dy, dx);
            gfloat radius = sqrt (dx * dx + dy * dy);
            dist = radius * (1.0 + 0.2 * sin (6.0 * angle)); // 6-petal flower
          }
          break;

        default:
          dist = sqrt (dx * dx + dy * dy); // Fallback to Euclidean
      }

      if (dist < min_dist)
        min_dist = dist;
    }

  if (sharpness != 0.0)
  {
    min_dist = CLAMP (min_dist, 0.0, 1.0);
    if (sharpness > 0.0)
    {
      gfloat exponent = 1.0 + sharpness * 2.0;
      min_dist = pow (min_dist, exponent);
    }
    else if (sharpness < 0.0)
    {
      gfloat exponent = 1.0 + sharpness;
      min_dist = pow (min_dist, exponent);
    }
  }

  return min_dist;
}

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
  gfloat *out_pixel = (gfloat *) out_buf;

  for (glong i = 0; i < n_pixels; i++)
  {
    // Compute local coordinates within the roi
    gint local_x = i % roi->width;
    gint local_y = i / roi->width;

    // Compute absolute coordinates for noise calculation (including roi offset)
    gfloat nx = (local_x + roi->x) / o->scale;
    gfloat ny = (local_y + roi->y) / o->scale;

    // Apply cell distortion and stretching for applicable noise types
    gfloat distorted_nx = nx;
    gfloat distorted_ny = ny;
    if (o->noise_type == GEGL_NOISE_TYPE_CELLULAR ||
        o->noise_type == GEGL_NOISE_TYPE_VORONOI ||
        o->noise_type == GEGL_NOISE_TYPE_CELLULAR_SHARDS ||
        o->noise_type == GEGL_NOISE_TYPE_BUBBLE_FOAM ||
        o->noise_type == GEGL_NOISE_TYPE_ORGANIC_BLOBS ||
        o->noise_type == GEGL_NOISE_TYPE_SQUARE_CELLS ||
        o->noise_type == GEGL_NOISE_TYPE_JELLY_BEAN ||
        o->noise_type == GEGL_NOISE_TYPE_HEXAGONAL ||
        o->noise_type == GEGL_NOISE_TYPE_ROUNDED_SQUARES)
    {
      // Apply distortion using Perlin noise
      gfloat distortion_x = perlin_noise(nx * 0.5, ny * 0.5, o->seed + 100) * o->cell_distortion;
      gfloat distortion_y = perlin_noise(nx * 0.5 + 1000.0, ny * 0.5, o->seed + 200) * o->cell_distortion;
      distorted_nx += distortion_x;
      distorted_ny += distortion_y;

      // Apply stretching (scale nx by cell_stretch)
      distorted_nx *= o->cell_stretch;
    }

    gfloat noise = 0.0;

    switch (o->noise_type)
    {
      case GEGL_NOISE_TYPE_CELLULAR:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, o->cell_jitter, o->edge_sharpness, o->scale, o->cell_shape);
        noise = 1.0 - noise;
        break;

      case GEGL_NOISE_TYPE_WORLEY:
        noise = cellular_noise (nx, ny, o->seed, o->cell_jitter, o->edge_sharpness, o->scale, GEGL_CELL_SHAPE_CIRCLE) * 
                cellular_noise (nx * 2.0, ny * 2.0, o->seed, o->cell_jitter, o->edge_sharpness, o->scale, GEGL_CELL_SHAPE_CIRCLE);
        break;

      case GEGL_NOISE_TYPE_VORONOI:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, o->cell_jitter, o->edge_sharpness, o->scale, o->cell_shape);
        noise = noise * noise;
        break;

      case GEGL_NOISE_TYPE_CELLULAR_CRACKED:
        noise = cellular_noise (nx, ny, o->seed, 0.0, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE);
        noise = 1.0 - noise;
        noise = noise * noise * o->roughness;
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_CELLULAR_MOSAIC:
        noise = cellular_noise (nx, ny, o->seed, 0.0, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE);
        gfloat detail = cellular_noise (nx * 4.0, ny * 4.0, o->seed + 1, 0.0, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE) * o->roughness;
        noise = noise + detail * 0.3;
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_RUSTY_PATINA:
        {
          gfloat amplitude = 1.0;
          gfloat frequency = 1.0;
          gfloat total_amplitude = 0.0;
          noise = 0.0;

          for (gint j = 0; j < o->octaves; j++)
          {
            noise += amplitude * value_noise (nx * frequency * o->detail_scale, ny * frequency * o->detail_scale, o->seed + j);
            total_amplitude += amplitude;
            amplitude *= o->roughness;
            frequency *= 2.0;
          }

          noise /= total_amplitude;
          noise = (noise + 1.0) * 0.5;
          noise = noise + cellular_noise (nx * 2.0, ny * 2.0, o->seed + 1, 0.0, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE) * 0.3;
          noise = CLAMP (noise, 0.0, 1.0);
        }
        break;

      case GEGL_NOISE_TYPE_RIPPLED_DOTS:
        {
          noise = cellular_noise (nx * 2.0, ny * 2.0, o->seed, 0.0, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE);
          noise = 1.0 - noise; // Invert for dots
          gfloat ripple = perlin_noise (nx, ny, o->seed + 1);
          noise += sin (ripple * o->roughness * G_PI) * 0.3; // Add rippling effect
          noise = (noise + 1.0) * 0.5;
          noise = CLAMP (noise, 0.0, 1.0);
        }
        break;

      case GEGL_NOISE_TYPE_CELLULAR_SHARDS:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, o->cell_jitter, o->edge_sharpness, o->scale, o->cell_shape);
        noise = pow (noise, 1.0 + o->roughness * 0.5); // Sharpen edges
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_CRYSTAL_LATTICE:
        noise = cellular_noise (nx, ny, o->seed, o->cell_jitter, o->edge_sharpness, o->scale, GEGL_CELL_SHAPE_CIRCLE);
        noise = sin (noise * 10.0); // Create sharp, crystalline transitions
        noise = (noise + 1.0) * 0.5;
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_ORGANIC_BLOBS:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, 1.0, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE); // High jitter for irregularity
        noise = 1.0 - noise;
        noise = pow (noise, 2.0 - o->softness); // Apply softness
        noise = noise + perlin_noise (nx * 0.5, ny * 0.5, o->seed + 1) * o->roughness * 0.2;
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_FRACTURED_TILES:
        noise = cellular_noise (nx, ny, o->seed, 0.0, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE);
        gfloat fractures = perlin_noise (nx * 2.0 * o->detail_scale, ny * 2.0 * o->detail_scale, o->seed + 1);
        noise = noise + fractures * o->fracture_intensity * o->roughness * 0.5;
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_BUBBLE_FOAM:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, o->cell_jitter, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE);
        noise = 1.0 - noise;
        gfloat secondary = cellular_noise (distorted_nx * 1.5, distorted_ny * 1.5, o->seed + 1, o->cell_jitter, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE);
        noise = noise * (1.0 - secondary * 0.5); // Overlap bubbles
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_VORONOI_RIPPLES:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, o->cell_jitter, 0.0, o->scale, GEGL_CELL_SHAPE_CIRCLE);
        gfloat ripple = perlin_noise (nx * 2.0 * o->detail_scale, ny * 2.0 * o->detail_scale, o->seed + 1);
        noise = noise + sin (noise * 10.0 + ripple) * o->ripple_strength * o->roughness * 0.2;
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_SQUARE_CELLS:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, o->cell_jitter * 0.2, o->edge_sharpness, o->scale, o->cell_shape);
        noise = 1.0 - noise;
        noise = pow (noise, 1.0 + o->edge_sharpness); // Sharpen for square-like edges
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_JELLY_BEAN:
        noise = cellular_noise (distorted_nx * o->cell_stretch * 2.0, distorted_ny, o->seed, o->cell_jitter, -o->softness, o->scale, o->cell_shape);
        noise = 1.0 - noise;
        noise = pow (noise, 2.0 - o->softness); // Soft, elongated shapes
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_HEXAGONAL:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, o->cell_jitter * 0.5, o->edge_sharpness * 0.7, o->scale, o->cell_shape);
        noise = 1.0 - noise;
        noise = pow (noise, 1.5); // Slightly sharpen for hexagonal appearance
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      case GEGL_NOISE_TYPE_ROUNDED_SQUARES:
        noise = cellular_noise (distorted_nx, distorted_ny, o->seed, o->cell_jitter * 0.3, -o->edge_sharpness, o->scale, o->cell_shape);
        noise = 1.0 - noise;
        noise = pow (noise, 2.0 - o->edge_sharpness); // Soften edges for rounded squares
        noise = CLAMP (noise, 0.0, 1.0);
        break;

      default:
        noise = 0.0;
    }

    // Normalize noise to 0-1 range if needed
    noise = CLAMP (noise, 0.0, 1.0);

    // Apply noise as grayscale, replacing the pixel (even transparent ones)
    out_pixel[0] = noise;
    out_pixel[1] = noise;
    out_pixel[2] = noise;
    out_pixel[3] = 1.0; // Full opacity

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

  gegl_operation_class_set_keys (operation_class,
    "name",        "ai/lb:cellular-noise",
    "title",       _("Cellular Noise"),
    "reference-hash", "aiplugin_cellnoise",
    "description", _("Generates various cellular noise patterns with adjustable parameters"),
    "gimp:menu-path", "<Image>/Filters/AI GEGL/",
    "gimp:menu-label", _("Celluar Noise..."),
    NULL);
}

#endif
