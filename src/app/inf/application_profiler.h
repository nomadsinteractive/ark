#ifndef ARK_APP_INF_APPLICATION_PROFILER_H_
#define ARK_APP_INF_APPLICATION_PROFILER_H_

#include "core/base/api.h"
#include "core/base/object.h"
#include "core/forwarding.h"
#include "core/types/owned_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API ApplicationProfiler {
public:
    enum Category {
        CATEGORY_DEFAULT,
        CATEGORY_RENDER_FRAME,
        CATEGORY_START_THREAD,
        CATEGORY_PHYSICS,
        CATEGORY_SCRIPT,
    };

    class ARK_API Tracer {
    public:
        virtual ~Tracer() = default;

        virtual op<Object> trace() = 0;
    };

public:
    virtual ~ApplicationProfiler() = default;

    virtual op<Tracer> makeTracer(const char* func, const char* filename, int32_t lineno, const char* name, Category category) = 0;
};

}

#endif
