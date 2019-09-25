#ifndef ARK_CORE_UTIL_RANDOM_H_
#define ARK_CORE_UTIL_RANDOM_H_

#include <random>

#include "core/base/api.h"

namespace ark {

class ARK_API Random {
public:
//  [[script::bindings::auto]]
    Random(uint32_t seed = std::random_device()());

//  [[script::bindings::property]]
    uint32_t seed() const;
//  [[script::bindings::property]]
    void setSeed(uint32_t seed);

//  [[script::bindings::auto]]
    float randf();
//  [[script::bindings::auto]]
    sp<Numeric> randv();

//  [[script::bindings::auto]]
    sp<Integer> rand(int32_t a, int32_t b);
//  [[script::bindings::auto]]
    sp<Integer> rand(const sp<Integer>& a, const sp<Integer>& b);

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
    std::mt19937 _generator;
};

}

#endif
