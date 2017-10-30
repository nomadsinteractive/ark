// texturejade.cpp
//
// This program uses the libnoise library to generate texture maps consisting
// of African jade.
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

namespace ark {
namespace plugin {
namespace noise {

namespace {

void CreateTextureColor(CommandLineOptions& options) {
    // Create a dark-stained wood palette (oak?)
    options._gradient_points.push_back(GradientPoint(-1.000, utils::Color ( 24, 146, 102, 255)));
    options._gradient_points.push_back(GradientPoint(0.000, utils::Color ( 78, 154, 115, 255)));
    options._gradient_points.push_back(GradientPoint(0.250, utils::Color (128, 204, 165, 255)));
    options._gradient_points.push_back(GradientPoint(0.375, utils::Color ( 78, 154, 115, 255)));
    options._gradient_points.push_back(GradientPoint(1.000, utils::Color ( 29, 135, 102, 255)));
}

}

int jade(CommandLineOptions& options)
{
  // Primary jade texture.  The ridges from the ridged-multifractal module
  // produces the veins.
  module::RidgedMulti primaryJade;
  primaryJade.SetSeed (options._seed);
  primaryJade.SetFrequency (options._frequency);
  primaryJade.SetLacunarity (options._lacunarity);
  primaryJade.SetOctaveCount (options._octave_count);
  primaryJade.SetNoiseQuality (QUALITY_STD);

  // Base of the secondary jade texture.  The base texture uses concentric
  // cylinders aligned on the z axis, which will eventually be perturbed.
  module::Cylinders baseSecondaryJade;
  baseSecondaryJade.SetFrequency (2.0);

  // Rotate the base secondary jade texture so that the cylinders are not
  // aligned with any axis.  This produces more variation in the secondary
  // jade texture since the texture is parallel to the y-axis.
  module::RotatePoint rotatedBaseSecondaryJade;
  rotatedBaseSecondaryJade.SetSourceModule (0, baseSecondaryJade);
  rotatedBaseSecondaryJade.SetAngles (90.0, 25.0, 5.0);

  // Slightly perturb the secondary jade texture for more realism.
  module::Turbulence perturbedBaseSecondaryJade;
  perturbedBaseSecondaryJade.SetSourceModule (0, rotatedBaseSecondaryJade);
  perturbedBaseSecondaryJade.SetSeed (options._seed + 1);
  perturbedBaseSecondaryJade.SetFrequency (4.0);
  perturbedBaseSecondaryJade.SetPower (1.0 / 4.0);
  perturbedBaseSecondaryJade.SetRoughness (4);

  // Scale the secondary jade texture so it contributes a small part to the
  // final jade texture.
  module::ScaleBias secondaryJade;
  secondaryJade.SetSourceModule (0, perturbedBaseSecondaryJade);
  secondaryJade.SetScale (0.25);
  secondaryJade.SetBias (0.0);

  // Add the two jade textures together.  These two textures were produced
  // using different combinations of coherent noise, so the final texture will
  // have a lot of variation.
  module::Add combinedJade;
  combinedJade.SetSourceModule (0, primaryJade);
  combinedJade.SetSourceModule (1, secondaryJade);

  // Finally, perturb the combined jade textures to produce the final jade
  // texture.  A low roughness produces nice veins.
  module::Turbulence finalJade;
  finalJade.SetSourceModule (0, combinedJade);
  finalJade.SetSeed (options._seed + 2);
  finalJade.SetFrequency (4.0);
  finalJade.SetPower (1.0 / 16.0);
  finalJade.SetRoughness (2);

  CreateTextureColor(options);
  CreateTexture(finalJade, options);
  return 0;
}

}
}
}
