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

//  [[script::bindings::enumeration]]
    enum CellularReturnType {
        CELLULAR_RETURN_TYPE_INDEX0,
        CELLULAR_RETURN_TYPE_INDEX0_ADD1,
        CELLULAR_RETURN_TYPE_INDEX0_SUB1,
        CELLULAR_RETURN_TYPE_INDEX0_MUL1,
        CELLULAR_RETURN_TYPE_INDEX0_DIV1
    };

//  [[script::bindings::enumeration]]
    enum CellularDistanceFunction {
        CELLULAR_DISTANCE_FUNCTION_EUCLIDEAN,
        CELLULAR_DISTANCE_FUNCTION_EUCLIDEAN_SQUARED,
        CELLULAR_DISTANCE_FUNCTION_MANHATTAN,
        CELLULAR_DISTANCE_FUNCTION_HYBRID,
        CELLULAR_DISTANCE_FUNCTION_MAX_AXIS,
        CELLULAR_DISTANCE_FUNCTION_MINKOWSKI
    };

public:
//  [[script::bindings::auto]]
    Generator(Generator::NoiseType type = Generator::NOISE_TYPE_SIMPLEX, int32_t seed = 0);
    ~Generator();

//  [[script::bindings::property]]
    int32_t seed() const;
//  [[script::bindings::property]]
    void setSeed(int32_t seed);

//  [[script::bindings::property]]
    float scale() const;
//  [[script::bindings::property]]
    void setScale(float scale);

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

//  Cellular-only settings; no-ops for Simplex/Perlin generators.
//  [[script::bindings::auto]]
    void setCellularReturnType(Generator::CellularReturnType returnType);
//  [[script::bindings::auto]]
    void setCellularDistanceFunction(Generator::CellularDistanceFunction distanceFunction);
//  [[script::bindings::auto]]
    void setGridJitter(float jitter);

//  [[script::bindings::auto]]
    float noise2D(float x, float y) const;
//  [[script::bindings::auto]]
    float noise3D(float x, float y, float z) const;

//  [[script::bindings::auto]]
    sp<FloatArray> noiseMap2D(const RectI& bounds, sp<Future> future = nullptr) const;

private:
    void ensureFractalGenerator() const;

    struct Stub;

private:
    int32_t _seed;
    float _scale;

    op<Stub> _stub;
};

}
