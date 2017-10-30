// texturewood.cpp
//
// This program uses the libnoise library to generate texture maps consisting
// of stained oak-like wood.
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
    options._gradient_points.push_back(GradientPoint(-1.00, utils::Color(189, 94, 4, 255)));
    options._gradient_points.push_back(GradientPoint(0.50, utils::Color(144, 48, 6, 255)));
    options._gradient_points.push_back(GradientPoint(1.00, utils::Color(60, 10, 8, 255)));
}

}

int wood(CommandLineOptions& options) {
    // Base wood texture.  The base texture uses concentric cylinders aligned
    // on the z axis, like a log.
    module::Cylinders baseWood;
    baseWood.SetFrequency(16.0);

    // Perlin noise to use for the wood grain.
    module::Perlin woodGrainNoise;
    woodGrainNoise.SetSeed(options._seed);
    woodGrainNoise.SetFrequency(options._frequency);
    woodGrainNoise.SetPersistence(options._persistence);
    woodGrainNoise.SetLacunarity(options._lacunarity);
    woodGrainNoise.SetOctaveCount(options._octave_count);
    woodGrainNoise.SetNoiseQuality(QUALITY_STD);

    // Stretch the Perlin noise in the same direction as the center of the
    // log.  This produces a nice wood-grain texture.
    module::ScalePoint scaledBaseWoodGrain;
    scaledBaseWoodGrain.SetSourceModule(0, woodGrainNoise);
    scaledBaseWoodGrain.SetYScale(0.25);

    // Scale the wood-grain values so that they may be added to the base wood
    // texture.
    module::ScaleBias woodGrain;
    woodGrain.SetSourceModule(0, scaledBaseWoodGrain);
    woodGrain.SetScale(0.25);
    woodGrain.SetBias(0.125);

    // Add the wood grain texture to the base wood texture.
    module::Add combinedWood;
    combinedWood.SetSourceModule(0, baseWood);
    combinedWood.SetSourceModule(1, woodGrain);

    // Slightly perturb the wood texture for more realism.
    module::Turbulence perturbedWood;
    perturbedWood.SetSourceModule(0, combinedWood);
    perturbedWood.SetSeed(options._seed + 1);
    perturbedWood.SetFrequency(4.0);
    perturbedWood.SetPower(1.0 / 256.0);
    perturbedWood.SetRoughness(4);

    // Cut the wood texture a small distance from the center of the "log".
    module::TranslatePoint translatedWood;
    translatedWood.SetSourceModule(0, perturbedWood);
    translatedWood.SetZTranslation(1.48);

    // Cut the wood texture on an angle to produce a more interesting wood
    // texture.
    module::RotatePoint rotatedWood;
    rotatedWood.SetSourceModule(0, translatedWood);
    rotatedWood.SetAngles(84.0, 0.0, 0.0);

    // Finally, perturb the wood texture to produce the final texture.
    module::Turbulence finalWood;
    finalWood.SetSourceModule(0, rotatedWood);
    finalWood.SetSeed(options._seed + 2);
    finalWood.SetFrequency(2.0);
    finalWood.SetPower(1.0 / 64.0);
    finalWood.SetRoughness(4);

    CreateTextureColor(options);
    CreateTexture(finalWood, options);
    return 0;
}

}
}
}
