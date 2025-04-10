#include "noise/base/generator.h"

#include "core/ark.h"
#include "core/base/future.h"
#include "core/inf/array.h"
#include "core/inf/runnable.h"
#include "core/util/float_array_type.h"

#include "graphics/base/rect.h"

#include "app/base/application_context.h"

namespace ark::plugin::noise {

namespace {

class RunnableGenUniformGrid2D final : public Runnable {
public:
    RunnableGenUniformGrid2D(FastNoise::SmartNode<FastNoise::Generator> generator, sp<Future> future, sp<FloatArray> floatArray, const RectI& bounds, const int32_t seed, const float frequency)
        : _generator(std::move(generator)), _future(std::move(future)), _float_array(std::move(floatArray)), _bounds(bounds), _seed(seed), _frequency(frequency)
    {
    }

    void run() override
    {
        _generator->GenUniformGrid2D(_float_array->buf(), _bounds.left(), _bounds.top(), _bounds.width(), _bounds.height(), _frequency, _seed);
        Ark::instance().applicationContext()->executorMain()->execute(_future);
    }

private:
    FastNoise::SmartNode<FastNoise::Generator> _generator;
    sp<Future> _future;
    sp<FloatArray> _float_array;
    RectI _bounds;
    int32_t _seed;
    float _frequency;
};

}

Generator::Generator(const NoiseType type, const int32_t seed, const float frequence)
    : _seed(seed), _frequency(frequence)
{
    if(type == NOISE_TYPE_CELLULAR)
        _source_generator = FastNoise::New<FastNoise::CellularDistance>();
    else if(type == NOISE_TYPE_SIMPLEX)
        _source_generator = FastNoise::New<FastNoise::Simplex>();
    else if(type == NOISE_TYPE_PERLIN)
        _source_generator = FastNoise::New<FastNoise::Perlin>();
    CHECK(_source_generator, "Unknow noise type: %d", type);
    _generator = _source_generator;
}

int32_t Generator::seed() const
{
    return _seed;
}

void Generator::setSeed(const int32_t seed)
{
    _seed = seed;
}

float Generator::frequency() const
{
    return _frequency;
}

void Generator::setFrequency(const float frequency)
{
    _frequency = frequency;
}

void Generator::setFractalOctaves(const int32_t octaves)
{
    ensureFractalGenerator();
    _fractal_generator->SetOctaveCount(octaves);
}

void Generator::setFractalGain(const float gain)
{
    ensureFractalGenerator();
    _fractal_generator->SetGain(gain);
}

void Generator::setFractalLacunarity(const float lacunarity)
{
    ensureFractalGenerator();
    _fractal_generator->SetLacunarity(lacunarity);
}

void Generator::setFractalWeightedStrength(const float weightedStrength)
{
    ensureFractalGenerator();
    _fractal_generator->SetWeightedStrength(weightedStrength);
}

float Generator::noise2d(const float x, const float y) const
{
    return _generator->GenSingle2D(x, y, _seed);
}

float Generator::noise3d(const float x, const float y, const float z) const
{
    return _generator->GenSingle3D(x, y, z, _seed);
}

sp<FloatArray> Generator::noiseMap2d(const RectI& bounds, sp<Future> future) const
{
    ASSERT(bounds.width() > 0 && bounds.height() > 0);
    sp<FloatArray> floatArray = FloatArrayType::create(bounds.width() * bounds.height());
    if(future)
        Ark::instance().applicationContext()->executorThreadPool()->execute(sp<Runnable>::make<RunnableGenUniformGrid2D>(_generator, std::move(future), floatArray, bounds, _seed, _frequency));
    else
        _generator->GenUniformGrid2D(floatArray->buf(), bounds.left(), bounds.top(), bounds.width(), bounds.height(), _frequency, _seed);
    return floatArray;
}

void Generator::ensureFractalGenerator()
{
    if(_fractal_generator)
        return;

    _fractal_generator = FastNoise::New<FastNoise::FractalFBm>();
    _fractal_generator->SetSource(_source_generator);
    _generator = _fractal_generator;
}

}
