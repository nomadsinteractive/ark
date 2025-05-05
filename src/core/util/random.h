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
    float randf() const;

//  [[script::bindings::auto]]
    sp<Integer> randint(int32_t a, int32_t b) const;
//  [[script::bindings::auto]]
    sp<Integer> randint(const sp<Integer>& a, const sp<Integer>& b) const;

//  [[script::bindings::auto]]
    sp<Numeric> uniform(float a, float b);
//  [[script::bindings::auto]]
    sp<Numeric> uniform(const sp<Numeric>& a, const sp<Numeric>& b);

//  [[script::bindings::auto]]
    sp<Numeric> normal(float a, float b);
//  [[script::bindings::auto]]
    sp<Numeric> normal(const sp<Numeric>& a, const sp<Numeric>& b);

private:
    uint32_t _seed;
    bool _nonvolatile;
    sp<std::mt19937> _generator;
};

}
