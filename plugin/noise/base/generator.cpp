#include "noise/base/generator.h"

#include "core/inf/array.h"

namespace ark {
namespace plugin {
namespace noise {

Generator::Generator(NoiseType type, int32_t seed)
    : _seed(seed), _frequency(200.0f)
{
    if(type == NOISE_TYPE_SIMPLEX)
        _noise_generator = FastNoise::New<FastNoise::Simplex>();
    else if(type == NOISE_TYPE_PERLIN)
        _noise_generator = FastNoise::New<FastNoise::Perlin>();
    CHECK(_noise_generator, "Unknow noise type: %d", type);
}

int32_t Generator::seed() const
{
    return _seed;
}

void Generator::setSeed(int32_t seed)
{
    _seed = seed;
}

float Generator::frequency()
{
    return _frequency;
}

void Generator::setFrequency(float frequency)
{
    _frequency = frequency;
}

void Generator::setFractalOctaves(int32_t octaves)
{
    ensureFractalGenerator();
    _fractal_generator->SetOctaveCount(octaves);
}

void Generator::setFractalGain(float gain)
{
    ensureFractalGenerator();
    _fractal_generator->SetGain(gain);
}

void Generator::setFractalLacunarity(float lacunarity)
{
    ensureFractalGenerator();
    _fractal_generator->SetLacunarity(lacunarity);
}

void Generator::setFractalWeightedStrength(float weightedStrength)
{
    ensureFractalGenerator();
    _fractal_generator->SetWeightedStrength(weightedStrength);
}

float Generator::noise2d(float x, float y)
{
    return _noise_generator->GenSingle2D(x, y, _seed);
}

float Generator::noise3d(float x, float y, float z)
{
    return _noise_generator->GenSingle3D(x, y, z, _seed);
}

sp<FloatArray> Generator::noiseMap2d(uint32_t rows, uint32_t cols)
{
    std::vector<float> output(rows * cols);
    _noise_generator->GenUniformGrid2D(output.data(), 0, 0, cols, rows, _frequency, _seed);
    return sp<FloatArray::Vector>::make(std::move(output));
}

void Generator::ensureFractalGenerator()
{
    if(_fractal_generator)
        return;

    _fractal_generator = FastNoise::New<FastNoise::FractalFBm>();
    _fractal_generator->SetSource(_noise_generator);
}

}
}
}
