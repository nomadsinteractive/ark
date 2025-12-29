#pragma once

#include <random>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Random {
public:
//  [[script::bindings::auto]]
    Random(uint32_t seed = std::random_device()(), bool nonvolatile = true);

//  [[script::bindings::property]]
    uint32_t seed() const;
//  [[script::bindings::property]]
    void setSeed(uint32_t seed);

//  [[script::bindings::auto]]
    uint32_t rand() const;

//  [[script::bindings::auto]]
    int32_t randint(int32_t a, int32_t b) const;
//  [[script::bindings::auto]]
    int64_t randint(int64_t a, int64_t b) const;
//  [[script::bindings::auto]]
    float uniform(float a, float b) const;

//  [[script::bindings::auto]]
    sp<Integer> randInteger(int32_t a, int32_t b) const;
//  [[script::bindings::auto]]
    sp<Integer> randInteger(sp<Integer> a, sp<Integer> b) const;

//  [[script::bindings::auto]]
    sp<Numeric> randNumeric(float a, float b);
//  [[script::bindings::auto]]
    sp<Numeric> randNumeric(sp<Numeric> a, sp<Numeric> b);

//  [[script::bindings::auto]]
    sp<Numeric> normal(float mean, float sigma);
//  [[script::bindings::auto]]
    sp<Numeric> normal(sp<Numeric> mean, sp<Numeric> sigma);

//  [[script::bindings::auto]]
    sp<Numeric> choice(Vector<float> choices);
//  [[script::bindings::auto]]
    sp<Integer> choice(Vector<int32_t> choices);

private:
    uint32_t _seed;
    bool _nonvolatile;
    sp<std::mt19937> _generator;
};

}
