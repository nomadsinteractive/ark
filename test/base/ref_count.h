#ifndef ARK_TEST_BASE_REF_COUNT_H_
#define ARK_TEST_BASE_REF_COUNT_H_

#include <stdint.h>

#include "core/util/log.h"

namespace ark {

template<typename T> class RefCount {
public:
    RefCount() {
        REF_COUNT ++;
        LOGD("RefCount: %d", REF_COUNT);
    }

    ~RefCount() {
        REF_COUNT --;
        LOGD("RefCount: %d", REF_COUNT);
    }

    static uint32_t refCount() {
        return REF_COUNT;
    }

private:
    static uint32_t REF_COUNT;
};

template<typename T> uint32_t RefCount<T>::REF_COUNT = 0;

}

#endif
