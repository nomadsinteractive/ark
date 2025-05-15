#include "core/util/random.h"

#include <utility>

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

namespace {

template<typename T, typename U = typename T::result_type> class Distribution final : public Variable<U> {
public:
    Distribution(T distribution, sp<std::mt19937> generator)
        : _distribution(std::move(distribution)), _generator(std::move(generator)) {
    }

    U val() override {
        return _distribution(*_generator);
    }

    bool update(uint64_t /*timestamp*/) override {
        return true;
    }

private:
    T _distribution;
    sp<std::mt19937> _generator;
};

template<typename T, typename U = typename T::result_type> class DynamicDistribution final : public Variable<U> {
public:
    DynamicDistribution(sp<Variable<U>> a, sp<Variable<U>> b, sp<std::mt19937> generator)
        : _a(std::move(a)), _b(std::move(b)), _generator(std::move(generator)) {
    }

    U val() override {
        T dis(_a->val(), _b->val());
        return dis(*_generator);
    }

    bool update(uint64_t /*timestamp*/) override {
        return true;
    }

private:
    sp<Variable<U>> _a;
    sp<Variable<U>> _b;
    sp<std::mt19937> _generator;
};

template<typename T> class Nonvolatile final : public Variable<T> {
public:
    Nonvolatile(sp<Variable<T>> generator)
        : _generator(std::move(generator)), _timestamp(0), _value(_generator->val())
    {
    }

    bool update(const uint64_t timestamp) override
    {
        if(!timestamp || _timestamp != timestamp)
        {
            _timestamp = timestamp;
            _value = _generator->val();
            return true;
        }
        return false;
    }

    T val() override
    {
        return _value;
    }

private:
    sp<Variable<T>> _generator;
    uint64_t _timestamp;
    T _value;
};

template<typename T> sp<Variable<T>> toNonvolatile(sp<Variable<T>> generator)
{
    return sp<Variable<T>>::template make<Nonvolatile<T>>(std::move(generator));
}

}

Random::Random(const uint32_t seed, const bool nonvolatile)
    : _seed(seed), _nonvolatile(nonvolatile), _generator(sp<std::mt19937>::make(_seed))
{
}

uint32_t Random::seed() const
{
    return _seed;
}

void Random::setSeed(const uint32_t seed)
{
    _seed = seed;
    _generator->seed(seed);
}

uint32_t Random::rand() const
{
    return (*_generator)();
}

float Random::randf() const
{
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    return dis(*_generator);
}

sp<Integer> Random::randint(const int32_t a, const int32_t b) const
{
    sp<Integer> g = sp<Integer>::make<Distribution<std::uniform_int_distribution<int32_t>, int32_t>>(std::uniform_int_distribution<int32_t>(a, b), _generator);
    if(_nonvolatile)
        return toNonvolatile<int32_t>(std::move(g));
    return g;
}

sp<Integer> Random::randint(sp<Integer> a, sp<Integer> b) const
{
    sp<Integer> g = sp<Integer>::make<DynamicDistribution<std::uniform_int_distribution<int32_t>, int32_t>>(std::move(a), std::move(b), _generator);
    if(_nonvolatile)
        return toNonvolatile<int32_t>(std::move(g));
    return g;
}

sp<Numeric> Random::uniform(const float a, const float b)
{
    sp<Numeric> g = sp<Numeric>::make<Distribution<std::uniform_real_distribution<float>, float>>(std::uniform_real_distribution<float>(a, b), _generator);
    if(_nonvolatile)
        return toNonvolatile<float>(std::move(g));
    return g;
}

sp<Numeric> Random::uniform(sp<Numeric> a, sp<Numeric> b)
{
    sp<Numeric> g = sp<Numeric>::make<DynamicDistribution<std::uniform_real_distribution<float>, float>>(std::move(a), std::move(b), _generator);
    if(_nonvolatile)
        return toNonvolatile<float>(std::move(g));
    return g;
}

sp<Numeric> Random::normal(const float a, const float b)
{
    sp<Numeric> g = sp<Numeric>::make<Distribution<std::normal_distribution<float>, float>>(std::normal_distribution<float>(a, b), _generator);
    if(_nonvolatile)
        return toNonvolatile<float>(std::move(g));
    return g;
}

sp<Numeric> Random::normal(sp<Numeric> a, sp<Numeric> b)
{
    sp<Numeric> g = sp<Numeric>::make<DynamicDistribution<std::normal_distribution<float>>>(std::move(a), std::move(b), _generator);
    if(_nonvolatile)
        return toNonvolatile<float>(std::move(g));
    return g;
}

}
