#include "optick/impl/application_profiler/application_profiler_optick.h"

#include <unordered_map>

#include <optick.h>

#include "core/base/string.h"

#include "graphics/base/v3.h"

namespace ark {

namespace {

class EventScopeOptick : public ApplicationProfiler::EventScope {
public:
    EventScopeOptick(const ::Optick::EventDescription& description)
        : _event(description) {
    }

private:
    ::Optick::Event _event;
};

class LoggerOptick : public ApplicationProfiler::Logger {
public:
    LoggerOptick(const char* func, const char* filename, int32_t lineno, const char* name)
        : _event_description(::Optick::CreateDescription(func, filename, lineno, name)) {
    }

    virtual void log(const void* data, ApplicationProfiler::DataType dataType) override {
        switch(dataType) {
        case ApplicationProfiler::DATA_TYPE_INT32:
            ::Optick::Tag::Attach(*_event_description, *reinterpret_cast<const int32_t*>(data));
            break;
        case ApplicationProfiler::DATA_TYPE_UINT32:
            ::Optick::Tag::Attach(*_event_description, *reinterpret_cast<const uint32_t*>(data));
            break;
        case ApplicationProfiler::DATA_TYPE_UINT64:
            ::Optick::Tag::Attach(*_event_description, *reinterpret_cast<const uint64_t*>(data));
            break;
        case ApplicationProfiler::DATA_TYPE_FLOAT:
            ::Optick::Tag::Attach(*_event_description, *reinterpret_cast<const float*>(data));
            break;
        case ApplicationProfiler::DATA_TYPE_STRING:
            ::Optick::Tag::Attach(*_event_description, reinterpret_cast<const char*>(data));
            break;
        case ApplicationProfiler::DATA_TYPE_VEC2: {
            const V2* v = reinterpret_cast<const V2*>(data);
            ::Optick::Tag::Attach(*_event_description, v->x(), v->y(), 0);
            break;
        }
        case ApplicationProfiler::DATA_TYPE_VEC3:
        case ApplicationProfiler::DATA_TYPE_VEC4: {
            const V3* v = reinterpret_cast<const V3*>(data);
            ::Optick::Tag::Attach(*_event_description, v->x(), v->y(), v->z());
            break;
        }
        default:
            break;
        }
    }

private:
    ::Optick::EventDescription* _event_description;
};

class OptickEventTracer : public ApplicationProfiler::Tracer {
public:
    OptickEventTracer(const char* func, const char* filename, int32_t lineno, const char* name, ::Optick::Category::Type category = ::Optick::Category::None)
        : _event_description(::Optick::CreateDescription(func, filename, lineno, name, category)) {
    }

    virtual op<ApplicationProfiler::EventScope> trace() override {
        return op<ApplicationProfiler::EventScope>(new EventScopeOptick(*_event_description));
    }

private:
    ::Optick::EventDescription* _event_description;
};

class OptickThreadEventLogger : public ApplicationProfiler::EventScope {
public:
    OptickThreadEventLogger(const char* name)
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

    virtual op<ApplicationProfiler::EventScope> trace() override {
        return op<ApplicationProfiler::EventScope>(new OptickThreadEventLogger(_name.c_str()));
    }

private:
    String _name;
};

class OptickFrameEventTracer : public ApplicationProfiler::Tracer {
public:
    OptickFrameEventTracer(const char* filename, int32_t lineno, const char* name)
        : _frame_thread_scope(name), _frame_tag_event_description(::Optick::EventDescription::Create("Frame", filename, static_cast<uint32_t>(lineno))) {
    }

    virtual op<ApplicationProfiler::EventScope> trace() override {
        ::Optick::EndFrame();
        ::Optick::Update();
        ::Optick::Tag::Attach(*_frame_tag_event_description, ::Optick::BeginFrame());
        return op<ApplicationProfiler::EventScope>(new EventScopeOptick(*::Optick::GetFrameDescription()));
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
        return op<Tracer>(new OptickFrameEventTracer(filename, lineno, name));
    if(category == ApplicationProfiler::CATEGORY_START_THREAD)
        return op<Tracer>(new OptickThreadEventTracer(name));
    return op<Tracer>(new OptickEventTracer(func, filename, lineno, name, toOptickCategory(category)));
}

op<ApplicationProfiler::Logger> ApplicationProfilerOptick::makeLogger(const char* func, const char* filename, int32_t lineno, const char* name)
{
    return op<Logger>(new LoggerOptick(func, filename, lineno, name));
}

sp<ApplicationProfiler> ApplicationProfilerOptick::BUILDER::build(const Scope& /*args*/)
{
    return sp<ApplicationProfilerOptick>::make();
}

}
