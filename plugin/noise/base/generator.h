#pragma once

#include "noise/api.h"
#include "FastNoise/FastNoise.h"

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {
namespace plugin {
namespace noise {

class ARK_PLUGIN_NOISE_API Generator {
public:
//  [[script::bindings::enumeration]]
    enum NoiseType {
        NOISE_TYPE_SIMPLEX,
        NOISE_TYPE_PERLIN
    };

public:
//  [[script::bindings::auto]]
    Generator(Generator::NoiseType type = Generator::NoiseType::NOISE_TYPE_SIMPLEX, int32_t seed = 0);

//  [[script::bindings::property]]
    int32_t seed() const;
//  [[script::bindings::property]]
    void setSeed(int32_t seed);

//  [[script::bindings::property]]
    float frequency();
//  [[script::bindings::property]]
    void setFrequency(float frequency);

//  [[script::bindings::property]]
    void setFractalOctaves(int32_t octaves);
//  [[script::bindings::property]]
    void setFractalGain(float gain);
//  [[script::bindings::property]]
    void setFractalLacunarity(float lacunarity);
//  [[script::bindings::property]]
    void setFractalWeightedStrength(float weightedStrength);

//  [[script::bindings::auto]]
    float noise2d(float x, float y);
//  [[script::bindings::auto]]
    float noise3d(float x, float y, float z);

//  [[script::bindings::auto]]
    sp<FloatArray> noiseMap2d(uint32_t rows, uint32_t cols);

private:
    void ensureFractalGenerator();

private:
    int32_t _seed;
    float _frequency;

    FastNoise::SmartNode<FastNoise::Generator> _noise_generator;
    FastNoise::SmartNode<FastNoise::Fractal<>> _fractal_generator;
};

}
}
}
