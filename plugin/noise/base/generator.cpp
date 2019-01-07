#include "noise/base/generator.h"

#include "core/inf/array.h"

namespace ark {
namespace plugin {
namespace noise {

Generator::Generator(uint32_t seed, NoiseType type)
    : _noise_impl(seed)
{
    _noise_impl.SetNoiseType(static_cast<FastNoise::NoiseType>(type));
}

int32_t Generator::seed() const
{
    return _noise_impl.GetSeed();
}

void Generator::setSeed(int32_t seed)
{
    _noise_impl.SetSeed(seed);
}

void Generator::setFrequency(float frequency)
{
    _noise_impl.SetFrequency(frequency);
}

void Generator::setFractalOctaves(int32_t octaves)
{
    _noise_impl.SetFractalOctaves(octaves);
}

void Generator::setFractalGain(float gain)
{
    _noise_impl.SetFractalGain(gain);
}

void Generator::setFractalLacunarity(float lacunarity)
{
    _noise_impl.SetFractalLacunarity(lacunarity);
}

float Generator::noise2d(float x, float y)
{
    return _noise_impl.GetNoise(x, y);
}

float Generator::noise3d(float x, float y, float z)
{
    return _noise_impl.GetNoise(x, y, z);
}

array<array<float>> Generator::noiseMap2d(uint32_t rows, uint32_t cols, float x1, float x2, float y1, float y2)
{
    float dx = (x2 - x1) / cols;
    float dy = (y2 - y1) / rows;
    const array<array<float>> s = sp<Array<array<float>>::Allocated>::make(rows);
    for(uint32_t i = 0; i < rows; ++i)
    {
        const array<float> t = sp<FloatArray::Allocated>::make(cols);
        float* buf = t->buf();
        float y = y1 + dy * i;
        for(uint32_t j = 0; j < cols; ++j)
            buf[j] = _noise_impl.GetNoise(x1 + dx * j, y);
        s->buf()[i] = t;
    }
    return s;
}

}
}
}
