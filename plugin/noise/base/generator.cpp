#include "noise/base/generator.h"

#include "FastNoise/FastNoise.h"

#include "core/ark.h"
#include "core/base/future.h"
#include "core/inf/array.h"
#include "core/inf/executor.h"
#include "core/inf/runnable.h"
#include "core/util/float_array_type.h"

#include "graphics/base/rect.h"

#include "app/base/application_context.h"

namespace ark::plugin::noise {

namespace {

void normalize(FloatArray& floatArray)
{
    const size_t length = floatArray.length();
    float* buf = floatArray.buf();
    for(size_t i = 0; i < length; ++i)
        buf[i] = (buf[i] + 1.0f) * 0.5f;
}

class RunnableGenUniformGrid2D final : public Runnable {
public:
    RunnableGenUniformGrid2D(FastNoise::SmartNode<FastNoise::Generator> generator, sp<Future> future, sp<FloatArray> floatArray, const RectI& bounds, const int32_t seed, const float frequency)
        : _generator(std::move(generator)), _future(std::move(future)), _float_array(std::move(floatArray)), _bounds(bounds), _seed(seed), _frequency(frequency)
    {
    }

    void run() override
    {
        _generator->GenUniformGrid2D(_float_array->buf(), _bounds.left(), _bounds.top(), _bounds.width(), _bounds.height(), _frequency, _seed);
        normalize(_float_array);
        Ark::instance().applicationContext()->coreExecutor()->execute(_future);
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

struct Generator::Stub {
    FastNoise::SmartNode<FastNoise::Generator> _generator;
    FastNoise::SmartNode<FastNoise::Generator> _source_generator;
    FastNoise::SmartNode<FastNoise::Fractal<>> _fractal_generator;
};

Generator::Generator(const NoiseType type, const int32_t seed, const float frequency)
    : _seed(seed), _frequency(frequency), _stub(new Stub())
{
    if(type == NOISE_TYPE_CELLULAR)
        _stub->_source_generator = FastNoise::New<FastNoise::CellularDistance>();
    else if(type == NOISE_TYPE_SIMPLEX)
        _stub->_source_generator = FastNoise::New<FastNoise::Simplex>();
    else if(type == NOISE_TYPE_PERLIN)
        _stub->_source_generator = FastNoise::New<FastNoise::Perlin>();
    CHECK(_stub->_source_generator, "Unknow noise type: %d", type);
    _stub->_generator = _stub->_source_generator;
}

Generator::~Generator()
{
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

bool Generator::useFractal() const
{
    return static_cast<bool>(_stub->_fractal_generator);
}

void Generator::setUseFractal(const bool enabled)
{
    if(!enabled && useFractal())
        _stub->_fractal_generator = nullptr;
    else if(enabled && !useFractal())
        setFractalOctaves(4);
}

void Generator::setFractalOctaves(const int32_t octaves)
{
    ensureFractalGenerator();
    _stub->_fractal_generator->SetOctaveCount(octaves);
}

void Generator::setFractalGain(const float gain)
{
    ensureFractalGenerator();
    _stub->_fractal_generator->SetGain(gain);
}

void Generator::setFractalLacunarity(const float lacunarity)
{
    ensureFractalGenerator();
    _stub->_fractal_generator->SetLacunarity(lacunarity);
}

void Generator::setFractalWeightedStrength(const float weightedStrength)
{
    ensureFractalGenerator();
    _stub->_fractal_generator->SetWeightedStrength(weightedStrength);
}

float Generator::noise2d(const float x, const float y) const
{
    return _stub->_generator->GenSingle2D(x, y, _seed);
}

float Generator::noise3d(const float x, const float y, const float z) const
{
    return _stub->_generator->GenSingle3D(x, y, z, _seed);
}

sp<FloatArray> Generator::noiseMap2d(const RectI& bounds, sp<Future> future) const
{
    ASSERT(bounds.width() > 0 && bounds.height() > 0);
    sp<FloatArray> floatArray = FloatArrayType::create(bounds.width() * bounds.height());
    if(future)
        Ark::instance().applicationContext()->threadPoolExecutor()->execute(sp<Runnable>::make<RunnableGenUniformGrid2D>(_stub->_generator, std::move(future), floatArray, bounds, _seed, _frequency));
    else
    {
        _stub->_generator->GenUniformGrid2D(floatArray->buf(), bounds.left(), bounds.top(), bounds.width(), bounds.height(), _frequency, _seed);
        normalize(floatArray);
    }
    return floatArray;
}

void Generator::ensureFractalGenerator() const
{
    if(_stub->_fractal_generator)
        return;

    _stub->_fractal_generator = FastNoise::New<FastNoise::FractalFBm>();
    _stub->_fractal_generator->SetSource(_stub->_source_generator);
    _stub->_generator = _stub->_fractal_generator;
}

}
