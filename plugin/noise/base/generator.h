#ifndef ARK_PLUGIN_NOISE_BASE_GENERATOR_H_
#define ARK_PLUGIN_NOISE_BASE_GENERATOR_H_

#include "noise/api.h"
#include "FastNoise.h"

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {
namespace plugin {
namespace noise {

class ARK_PLUGIN_NOISE_API Generator {
public:
//  [[script::bindings::enumeration]]
    enum NoiseType {
        NOISE_TYPE_PERLIN = FastNoise::Perlin,
        NOISE_TYPE_PERLIN_FRACTAL = FastNoise::PerlinFractal,
        NOISE_TYPE_SIMPLEX = FastNoise::Simplex,
        NOISE_TYPE_SIMPLEX_FRACTAL = FastNoise::SimplexFractal
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
    array<array<float>> noiseMap2d(uint32_t rows, uint32_t cols, float x1, float x2, float y1, float y2);

private:
    FastNoise _noise_impl;
};

}
}
}

#endif
