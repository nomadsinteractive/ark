#include "tracy/impl/application_profiler/application_profiler_tracy.h"

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include "core/base/string.h"

#include "graphics/base/v3.h"
#include "graphics/base/v4.h"

namespace ark::plugin::tracy {

namespace {

class ZoneScopeTracy final : public ApplicationProfiler::EventScope {
public:
    ZoneScopeTracy(const ::tracy::SourceLocationData* sourceLocationData)
        : _scoped_zone(sourceLocationData) {
    }

private:
    ::tracy::ScopedZone _scoped_zone;
};

class LoggerTracy final : public ApplicationProfiler::Logger {
public:
    LoggerTracy(const ::tracy::SourceLocationData& data)
        : _location_data(data), _scoped_zone(&_location_data) {
    }

    void log(const void* data, const ApplicationProfiler::DataType dataType) override
    {
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
            const V2* v = static_cast<const V2*>(data);
            _scoped_zone.TextFmt("(%.1f, %.1f)", v->x(), v->y());
            break;
        }
        case ApplicationProfiler::DATA_TYPE_VEC3: {
            const V3* v = static_cast<const V3*>(data);
            _scoped_zone.TextFmt("(%.1f, %.1f, %.1f)", v->x(), v->y(), v->z());
            break;
        }
        case ApplicationProfiler::DATA_TYPE_VEC4: {
            const V4* v = static_cast<const V4*>(data);
            _scoped_zone.TextFmt("(%.1f, %.1f, %.1f, %.1f)", v->x(), v->y(), v->z(), v->w());
        }
        default:
            break;
        }
    }

private:
    ::tracy::SourceLocationData _location_data;
    ::tracy::ScopedZone _scoped_zone;
};

class TracyZoneTracer final : public ApplicationProfiler::Tracer {
public:
    TracyZoneTracer(const char* func, const char* filename, const int32_t lineno, const char* name)
        : _location_data{ name, func,  filename, static_cast<uint32_t>(lineno), 0 } {
    }

    op<ApplicationProfiler::EventScope> trace() override {
        return op<ApplicationProfiler::EventScope>(new ZoneScopeTracy(&_location_data));
    }

private:
    ::tracy::SourceLocationData _location_data;
};

class TracyThreadEventTracer final : public ApplicationProfiler::Tracer {
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

}

op<ApplicationProfiler::Tracer> ApplicationProfilerTracy::makeTracer(const char* func, const char* filename, const int32_t lineno, const char* name, const Category category)
{
    if(category == CATEGORY_START_THREAD)
        return op<Tracer>(new TracyThreadEventTracer(name));
    return op<Tracer>(new TracyZoneTracer(func, filename, lineno, name));
}

op<ApplicationProfiler::Logger> ApplicationProfilerTracy::makeLogger(const char* func, const char* filename, const int32_t lineno, const char* name)
{
    return op<Logger>(new LoggerTracy({name, func, filename, static_cast<uint32_t>(lineno), 0}));
}

sp<ApplicationProfiler> ApplicationProfilerTracy::BUILDER::build(const Scope& /*args*/)
{
    return sp<ApplicationProfiler>::make<ApplicationProfilerTracy>();
}

}
