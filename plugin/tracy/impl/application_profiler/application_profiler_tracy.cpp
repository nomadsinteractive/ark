#include "tracy/impl/application_profiler/application_profiler_tracy.h"

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include "core/base/string.h"

#include "graphics/base/v3.h"

namespace ark::plugin::tracy {

namespace {

class ZoneScopeTracy : public ApplicationProfiler::EventScope {
public:
    ZoneScopeTracy(const ::tracy::SourceLocationData& data)
        : _location_data(data), _scoped_zone(&_location_data) {
    }

private:
    ::tracy::SourceLocationData _location_data;
    ::tracy::ScopedZone _scoped_zone;
};

class FrameScopeTracy : public ApplicationProfiler::EventScope {
public:
    FrameScopeTracy(const char* frameName)
        : _frame_name(frameName) {
    }
    ~FrameScopeTracy() override {
        ::tracy::Profiler::SendFrameMark(_frame_name, ::tracy::QueueType::FrameMarkMsgEnd);
    }

private:
    const char* _frame_name;
};

class LoggerTracy : public ApplicationProfiler::Logger {
public:
    LoggerTracy(const ::tracy::SourceLocationData& data)
        : _location_data(data), _scoped_zone(&_location_data) {
    }

    void log(const void* data, ApplicationProfiler::DataType dataType) override {
        switch(dataType) {
        case ApplicationProfiler::DATA_TYPE_INT32:
            _scoped_zone.Value(*static_cast<const int32_t*>(data));
            break;
        case ApplicationProfiler::DATA_TYPE_UINT32:
            _scoped_zone.Value(*static_cast<const uint32_t*>(data));
            break;
        case ApplicationProfiler::DATA_TYPE_UINT64:
            _scoped_zone.Value(*static_cast<const uint64_t*>(data));
            break;
        case ApplicationProfiler::DATA_TYPE_FLOAT:
            _scoped_zone.Value(*static_cast<const float*>(data));
            break;
            case ApplicationProfiler::DATA_TYPE_STRING:
            _scoped_zone.Text(static_cast<const char*>(data), std::strlen(static_cast<const char*>(data)));
            break;
        case ApplicationProfiler::DATA_TYPE_VEC2: {
            break;
        }
        case ApplicationProfiler::DATA_TYPE_VEC3:
        case ApplicationProfiler::DATA_TYPE_VEC4: {
            break;
        }
        default:
            break;
        }
    }

private:
    ::tracy::SourceLocationData _location_data;
    ::tracy::ScopedZone _scoped_zone;
};

class TracyZoneTracer : public ApplicationProfiler::Tracer {
public:
    TracyZoneTracer(const char* func, const char* filename, int32_t lineno, const char* name)
        : _location_data{ name, func,  filename, static_cast<uint32_t>(lineno), 0 } {
    }

    op<ApplicationProfiler::EventScope> trace() override {
        return op<ApplicationProfiler::EventScope>(new ZoneScopeTracy(_location_data));
    }

private:
    ::tracy::SourceLocationData _location_data;
};

class TracyThreadEventTracer : public ApplicationProfiler::Tracer {
public:
    TracyThreadEventTracer(const char* name)
        : _name(name) {
    }

    op<ApplicationProfiler::EventScope> trace() override {
        TracyCSetThreadName(_name.c_str());
        return nullptr;
    }

private:
    String _name;
};

class TracyFrameEventTracer : public ApplicationProfiler::Tracer {
public:
    TracyFrameEventTracer(const char* filename, int32_t lineno, const char* name)
        : _frame_name(name) {
    }

    op<ApplicationProfiler::EventScope> trace() override {
        return op<ApplicationProfiler::EventScope>(new FrameScopeTracy(_frame_name));
    }

private:
    const char* _frame_name;
};

}

op<ApplicationProfiler::Tracer> ApplicationProfilerTracy::makeTracer(const char* func, const char* filename, int32_t lineno, const char* name, Category category)
{
    if(category == CATEGORY_RENDER_FRAME)
        return op<Tracer>(new TracyFrameEventTracer(filename, lineno, name));
    if(category == CATEGORY_START_THREAD)
        return op<Tracer>(new TracyThreadEventTracer(name));
    return op<Tracer>(new TracyZoneTracer(func, filename, lineno, name));
}

op<ApplicationProfiler::Logger> ApplicationProfilerTracy::makeLogger(const char* func, const char* filename, int32_t lineno, const char* name)
{
    return op<Logger>(new LoggerTracy({name, func, filename, static_cast<uint32_t>(lineno), 0}));
}

sp<ApplicationProfiler> ApplicationProfilerTracy::BUILDER::build(const Scope& /*args*/)
{
    return sp<ApplicationProfilerTracy>::make();
}

}
