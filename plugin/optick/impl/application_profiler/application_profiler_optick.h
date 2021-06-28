#ifndef ARK_PLUGIN_OPTICK_IMPL_APPLICATION_PROFILER_APPLICATION_PROFILER_OPTICK_H_
#define ARK_PLUGIN_OPTICK_IMPL_APPLICATION_PROFILER_APPLICATION_PROFILER_OPTICK_H_

#include "core/inf/builder.h"

#include "app/inf/application_profiler.h"

namespace ark {

class ApplicationProfilerOptick : public ApplicationProfiler {
public:

    virtual op<Tracer> makeTracer(const char* func, const char* filename, int32_t lineno, const char* name, Category category) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<ApplicationProfiler> {
    public:
        BUILDER() = default;

        virtual sp<ApplicationProfiler> build(const Scope& args) override;
    };

private:
    void onRenderFrame(const String& name, Runnable& delegate);
    void onStartThread(const String& name, Runnable& delegate);

    void onEvent(const String& name, Runnable& delegate, ApplicationProfiler::Category category);
};

}

#endif
