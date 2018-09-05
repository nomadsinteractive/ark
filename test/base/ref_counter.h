#ifndef ARK_TEST_BASE_REF_COUNTER_H_
#define ARK_TEST_BASE_REF_COUNTER_H_

#include <stdint.h>

#include "core/util/log.h"

namespace ark {

template<typename T> class RefCounter {
public:
    RefCounter() {
        REF_COUNT ++;
        LOGD("RefCount: %d", REF_COUNT);
    }

    ~RefCounter() {
        REF_COUNT --;
        LOGD("RefCount: %d", REF_COUNT);
    }

    static uint32_t refCount() {
        return REF_COUNT;
    }

private:
    static uint32_t REF_COUNT;
};

template<typename T> uint32_t RefCounter<T>::REF_COUNT = 0;

}

#endif
