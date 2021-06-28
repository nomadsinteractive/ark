#include "optick/impl/application_profiler/application_profiler_optick.h"

#include "optick.h"

#include "core/base/string.h"
#include "core/base/object.h"
#include "core/inf/runnable.h"

namespace ark {

namespace {

class OptickEventObject : public Object {
public:
    OptickEventObject(const ::Optick::EventDescription& description)
        : _event(description) {
    }

private:
    ::Optick::Event _event;
};

class OptickEventTracer : public ApplicationProfiler::Tracer {
public:
    OptickEventTracer(const char* func, const char* filename, int32_t lineno, const char* name, ::Optick::Category::Type category = ::Optick::Category::None)
        : _event_description(::Optick::CreateDescription(func, filename, lineno, name, category)) {
    }

    virtual op<Object> trace() override {
        return op<Object>(new OptickEventObject(*_event_description));
    }

private:
    ::Optick::EventDescription* _event_description;
};

class OptickThreadEventObject : public Object {
public:
    OptickThreadEventObject(const char* name)
        : _thread_scope(name) {
    }

private:
    ::Optick::ThreadScope _thread_scope;
};

class OptickThreadEventTracer : public ApplicationProfiler::Tracer {
public:
    OptickThreadEventTracer(const char* name)
        : _name(name) {
    }

    virtual op<Object> trace() override {
        return op<Object>(new OptickThreadEventObject(_name.c_str()));
    }

private:
    String _name;
};

class OptickFrameEventObject : public Object {
public:
    OptickFrameEventObject(const char* name)
        : _thread_scope(name) {
    }

private:
    ::Optick::ThreadScope _thread_scope;
};

class OptickFrameEventTracer : public ApplicationProfiler::Tracer {
public:
    OptickFrameEventTracer(const char* filename, uint32_t lineno, const char* name)
        : _frame_thread_scope(name), _frame_tag_event_description(::Optick::EventDescription::Create("Frame", filename, lineno)) {
    }

    virtual op<Object> trace() override {
        ::Optick::EndFrame();
        ::Optick::Update();
        ::Optick::Tag::Attach(*_frame_tag_event_description, ::Optick::BeginFrame());
        return op<Object>(new OptickEventObject(*::Optick::GetFrameDescription()));
    }

private:
    ::Optick::ThreadScope _frame_thread_scope;
    ::Optick::EventDescription* _frame_tag_event_description;
};

}

static Optick::Category::Type toOptickCategory(ApplicationProfiler::Category category)
{
    switch(category) {
        case ApplicationProfiler::CATEGORY_PHYSICS:
            return Optick::Category::Physics;
        case ApplicationProfiler::CATEGORY_SCRIPT:
            return Optick::Category::Script;
        default:
            break;
    }
    return Optick::Category::None;
}

op<ApplicationProfiler::Tracer> ApplicationProfilerOptick::makeTracer(const char* func, const char* filename, int32_t lineno, const char* name, Category category)
{
    if(category == ApplicationProfiler::CATEGORY_RENDER_FRAME)
        return op<ApplicationProfiler::Tracer>(new OptickFrameEventTracer(filename, static_cast<uint32_t>(lineno), name));
    if(category == ApplicationProfiler::CATEGORY_START_THREAD)
        return op<ApplicationProfiler::Tracer>(new OptickThreadEventTracer(name));
    return op<ApplicationProfiler::Tracer>(new OptickEventTracer(func, filename, lineno, name, toOptickCategory(category)));
}

sp<ApplicationProfiler> ApplicationProfilerOptick::BUILDER::build(const Scope& /*args*/)
{
    return sp<ApplicationProfilerOptick>::make();
}

}
