#pragma once

#include "noise/api.h"

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "graphics/forwarding.h"

namespace ark::plugin::noise {

class ARK_PLUGIN_NOISE_API Generator {
public:
//  [[script::bindings::enumeration]]
    enum NoiseType {
        NOISE_TYPE_CELLULAR,
        NOISE_TYPE_SIMPLEX,
        NOISE_TYPE_PERLIN
    };

public:
//  [[script::bindings::auto]]
    Generator(Generator::NoiseType type = Generator::NOISE_TYPE_SIMPLEX, int32_t seed = 0, float frequency = 1.0f);
    ~Generator();

//  [[script::bindings::property]]
    int32_t seed() const;
//  [[script::bindings::property]]
    void setSeed(int32_t seed);

//  [[script::bindings::property]]
    float frequency() const;
//  [[script::bindings::property]]
    void setFrequency(float frequency);

//  [[script::bindings::property]]
    bool useFractal() const;
//  [[script::bindings::property]]
    void setUseFractal(bool enabled);

//  [[script::bindings::auto]]
    void setFractalOctaves(int32_t octaves);
//  [[script::bindings::auto]]
    void setFractalGain(float gain);
//  [[script::bindings::auto]]
    void setFractalLacunarity(float lacunarity);
//  [[script::bindings::auto]]
    void setFractalWeightedStrength(float weightedStrength);

//  [[script::bindings::auto]]
    float noise2d(float x, float y) const;
//  [[script::bindings::auto]]
    float noise3d(float x, float y, float z) const;

//  [[script::bindings::auto]]
    sp<FloatArray> noiseMap2d(const RectI& bounds, sp<Future> future = nullptr) const;

private:
    void ensureFractalGenerator() const;

    struct Stub;

private:
    int32_t _seed;
    float _frequency;

    op<Stub> _stub;
};

}
