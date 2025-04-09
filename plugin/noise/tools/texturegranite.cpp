// texturegranite.cpp
//
// This program uses the libnoise library to generate texture maps consisting
// of granite.
//
// Copyright (C) 2004 Jason Bevins
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// (COPYING.txt) for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc., 59
// Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// The developer's email is jlbezigvins@gmzigail.com (for great email, take
// off every 'zig'.)
//

#include <noise/noise.h>

#include "common.h"

using namespace noise;
using namespace ark::plugin::noise;

namespace ark::plugin::noise {

namespace {

void CreateTextureColor (CommandLineOptions& options) {
    options._gradient_points.push_back(GradientPoint(-1.0000, utils::Color (  0,   0,   0, 255)));
    options._gradient_points.push_back(GradientPoint(-0.9375, utils::Color (  0,   0,   0, 255)));
    options._gradient_points.push_back(GradientPoint(-0.8750, utils::Color (216, 216, 242, 255)));
    options._gradient_points.push_back(GradientPoint(0.0000, utils::Color (191, 191, 191, 255)));
    options._gradient_points.push_back(GradientPoint(0.5000, utils::Color (210, 116, 125, 255)));
    options._gradient_points.push_back(GradientPoint(0.7500, utils::Color (210, 113,  98, 255)));
    options._gradient_points.push_back(GradientPoint(1.0000, utils::Color (255, 176, 192, 255)));

}

}

int granite (CommandLineOptions& options)
{
  // Primary granite texture.  This generates the "roughness" of the texture
  // when lit by a light source.
  module::Billow primaryGranite;
  primaryGranite.SetSeed (options._seed);
  primaryGranite.SetFrequency (options._frequency);
  primaryGranite.SetPersistence (options._persistence);
  primaryGranite.SetLacunarity (options._lacunarity);
  primaryGranite.SetOctaveCount (options._octave_count);
  primaryGranite.SetNoiseQuality (QUALITY_STD);

  // Use Voronoi polygons to produce the small grains for the granite texture.
  module::Voronoi baseGrains;
  baseGrains.SetSeed (1);
  baseGrains.SetFrequency (16.0);
  baseGrains.EnableDistance (true);

  // Scale the small grain values so that they may be added to the base
  // granite texture.  Voronoi polygons normally generate pits, so apply a
  // negative scaling factor to produce bumps instead.
  module::ScaleBias scaledGrains;
  scaledGrains.SetSourceModule (0, baseGrains);
  scaledGrains.SetScale (-0.5);
  scaledGrains.SetBias (0.0);

  // Combine the primary granite texture with the small grain texture.
  module::Add combinedGranite;
  combinedGranite.SetSourceModule (0, primaryGranite);
  combinedGranite.SetSourceModule (1, scaledGrains);

  // Finally, perturb the granite texture to add realism.
  module::Turbulence finalGranite;
  finalGranite.SetSourceModule (0, combinedGranite);
  finalGranite.SetSeed (2);
  finalGranite.SetFrequency (4.0);
  finalGranite.SetPower (1.0 / 8.0);
  finalGranite.SetRoughness (6);

  CreateTextureColor(options);
  CreateTexture(finalGranite, options);
  return 0;
}

}
