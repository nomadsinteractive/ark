#include "core/util/random.h"

#include <utility>

#include "core/inf/variable.h"

namespace ark {

namespace {

template<typename T, typename U = typename T::result_type> class Distribution : public Variable<U> {
public:
    Distribution(T distribution, std::mt19937 generator)
        : _distribution(std::move(distribution)), _generator(std::move(generator)) {
    }

    virtual U val() override {
        return _distribution(_generator);
    }

private:
    T _distribution;
    std::mt19937 _generator;
};

template<typename T, typename U = typename T::result_type> class DynamicDistribution : public Variable<U> {
public:
    DynamicDistribution(sp<Variable<U>> a, sp<Variable<U>> b, std::mt19937 generator)
        : _a(std::move(a)), _b(std::move(b)), _generator(std::move(generator)) {
    }

    virtual U val() override {
        T dis(_a->val(), _b->val());
        return dis(_generator);
    }

private:
    sp<Variable<U>> _a;
    sp<Variable<U>> _b;
    std::mt19937 _generator;
};

}

Random::Random(uint32_t seed)
    : _seed(seed), _generator(_seed)
{
}

uint32_t Random::seed() const
{
    return _seed;
}

void Random::setSeed(uint32_t seed)
{
    _seed = seed;
    _generator.seed(seed);
}

float Random::randf()
{
    std::uniform_real_distribution<float> dis(1.0, 2.0);
    return dis(_generator);
}

sp<Numeric> Random::randv()
{
    return uniform(0, 1.0f);
}

sp<Integer> Random::rand(int32_t a, int32_t b)
{
    return sp<Distribution<std::uniform_int_distribution<int32_t>, int32_t>>::make(std::uniform_int_distribution<int32_t>(a, b), _generator);
}

sp<Integer> Random::rand(const sp<Integer>& a, const sp<Integer>& b)
{
    return sp<DynamicDistribution<std::uniform_int_distribution<int32_t>, int32_t>>::make(a, b, _generator);
}

sp<Numeric> Random::uniform(float a, float b)
{
    return sp<Distribution<std::uniform_real_distribution<float>, float>>::make(std::uniform_real_distribution<float>(a, b), _generator);
}

sp<Numeric> Random::uniform(const sp<Numeric>& a, const sp<Numeric>& b)
{
    return sp<DynamicDistribution<std::uniform_real_distribution<float>, float>>::make(a, b, _generator);
}

sp<Numeric> Random::normal(float a, float b)
{
    return sp<Distribution<std::normal_distribution<float>, float>>::make(std::normal_distribution<float>(a, b), _generator);
}

sp<Numeric> Random::normal(const sp<Numeric>& a, const sp<Numeric>& b)
{
    return sp<DynamicDistribution<std::normal_distribution<float>>>::make(a, b, _generator);
}

}
