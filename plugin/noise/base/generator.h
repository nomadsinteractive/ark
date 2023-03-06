#pragma once

#include <vector>

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
        NOISE_TYPE_PERLIN,
        NOISE_TYPE_PERLIN_FRACTAL,
        NOISE_TYPE_SIMPLEX,
        NOISE_TYPE_SIMPLEX_FRACTAL
    };

public:
//  [[script::bindings::auto]]
    Generator(uint32_t seed, Generator::NoiseType type);

//  [[script::bindings::property]]
    int32_t seed() const;
//  [[script::bindings::property]]
    void setSeed(int32_t seed);

//  [[script::bindings::property]]
    void setFrequency(float frequency);

//  [[script::bindings::property]]
    void setFractalOctaves(int32_t octaves);
//  [[script::bindings::property]]
    void setFractalGain(float gain);
//  [[script::bindings::property]]
    void setFractalLacunarity(float lacunarity);

//  [[script::bindings::auto]]
    float noise2d(float x, float y);
//  [[script::bindings::auto]]
    float noise3d(float x, float y, float z);

//  [[script::bindings::auto]]
    std::vector<array<float>> noiseMap2d(uint32_t rows, uint32_t cols, float x1, float x2, float y1, float y2);

private:
    FastNoise::SmartNode<FastNoise::Generator> _noise_generator;
    int32_t _seed;
    float _frequency;
};

}
}
}
