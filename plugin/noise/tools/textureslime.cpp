// textureslime.cpp
//
// This program uses the libnoise library to generate texture maps consisting
// of shiny green slime.
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

namespace ark {
namespace plugin {
namespace noise {

namespace {

void CreateTextureColor (CommandLineOptions& options) {
    options._gradient_points.push_back(GradientPoint(-1.0000, utils::Color (160,  64,  42, 255)));
    options._gradient_points.push_back(GradientPoint(0.0000, utils::Color ( 64, 192,  64, 255)));
    options._gradient_points.push_back(GradientPoint(1.0000, utils::Color (128, 255, 128, 255)));

}

}

int slime (CommandLineOptions& options)
{
  // Large slime bubble texture.
  module::Billow largeSlime;
  largeSlime.SetSeed (options._seed);
  largeSlime.SetFrequency (options._frequency);
  largeSlime.SetLacunarity (options._lacunarity);
  largeSlime.SetOctaveCount (options._octave_count);
  largeSlime.SetNoiseQuality (QUALITY_BEST);

  // Base of the small slime bubble texture.  This texture will eventually
  // appear inside cracks in the large slime bubble texture.
  module::Billow smallSlimeBase;
  smallSlimeBase.SetSeed (options._seed + 1);
  smallSlimeBase.SetFrequency (24.0);
  smallSlimeBase.SetLacunarity (2.14453125);
  smallSlimeBase.SetOctaveCount (1);
  smallSlimeBase.SetNoiseQuality (QUALITY_BEST);

  // Scale and lower the small slime bubble values.
  module::ScaleBias smallSlime;
  smallSlime.SetSourceModule (0, smallSlimeBase);
  smallSlime.SetScale (0.5);
  smallSlime.SetBias (-0.5);

  // Create a map that specifies where the large and small slime bubble
  // textures will appear in the final texture map.
  module::RidgedMulti slimeMap;
  slimeMap.SetSeed (options._seed);
  slimeMap.SetFrequency (2.0);
  slimeMap.SetLacunarity (2.20703125);
  slimeMap.SetOctaveCount (3);
  slimeMap.SetNoiseQuality (QUALITY_STD);

  // Choose between the large or small slime bubble textures depending on the
  // corresponding value from the slime map.  Choose the small slime bubble
  // texture if the slime map value is within a narrow range of values,
  // otherwise choose the large slime bubble texture.  The edge falloff is
  // non-zero so that there is a smooth transition between the two textures.
  module::Select slimeChooser;
  slimeChooser.SetSourceModule (0, largeSlime);
  slimeChooser.SetSourceModule (1, smallSlime);
  slimeChooser.SetControlModule (slimeMap);
  slimeChooser.SetBounds (-0.375, 0.375);
  slimeChooser.SetEdgeFalloff (0.5);

  // Finally, perturb the slime texture to add realism.
  module::Turbulence finalSlime;
  finalSlime.SetSourceModule (0, slimeChooser);
  finalSlime.SetSeed (options._seed + 2);
  finalSlime.SetFrequency (8.0);
  finalSlime.SetPower (1.0 / 32.0);
  finalSlime.SetRoughness (2);

  CreateTextureColor(options);
    CreateTexture(finalSlime, options);

  return 0;
}

}
}
}
