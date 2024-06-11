#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
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

    enum DataType {
        DATA_TYPE_INT32,
        DATA_TYPE_UINT32,
        DATA_TYPE_UINT64,
        DATA_TYPE_FLOAT,
        DATA_TYPE_STRING,
        DATA_TYPE_VEC2,
        DATA_TYPE_VEC3,
        DATA_TYPE_VEC4
    };

    class ARK_API Logger {
    public:
        virtual ~Logger() = default;

        template<typename T> void log(const T& data) {
            log_sfinae(data, nullptr);
        }

        virtual void log(const void* data, DataType dataType) = 0;

    private:
        template<typename T> void log_sfinae(const T& data, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<bool, T>>*) {
            const bool isUnsigned = std::is_unsigned_v<T>;
            const bool isLongLong = sizeof(T) >= sizeof(int64_t);
            const DataType dataType = isUnsigned ? (isLongLong ? DATA_TYPE_UINT64 : DATA_TYPE_UINT32) : (isLongLong ? DATA_TYPE_UINT64 : DATA_TYPE_INT32);
            log(static_cast<const void*>(&data), dataType);
        }

        template<typename T> void log_sfinae(const T& data, std::enable_if_t<std::is_floating_point_v<T>>*) {
            const float logData = data;
            log(&logData, DATA_TYPE_FLOAT);
        }

        template<typename T> void log_sfinae(const T& data, std::enable_if_t<std::is_same_v<bool, T>>*) {
            const uint32_t logData = data ? 1 : 0;
            log(&logData, DATA_TYPE_UINT32);
        }

        void log_sfinae(const String& data, std::nullptr_t) {
            log(data.c_str(), DATA_TYPE_STRING);
        }

        void log_sfinae(const V2& data, std::nullptr_t) {
            log(&data, DATA_TYPE_VEC2);
        }

        void log_sfinae(const V3& data, std::nullptr_t) {
            log(&data, DATA_TYPE_VEC3);
        }

        void log_sfinae(const V4& data, std::nullptr_t) {
            log(&data, DATA_TYPE_VEC4);
        }

    };

    class ARK_API EventScope {
    public:
        virtual ~EventScope() = default;
    };

    class ARK_API Tracer {
    public:
        virtual ~Tracer() = default;

        virtual op<EventScope> trace() = 0;
    };

public:
    virtual ~ApplicationProfiler() = default;

    virtual op<Tracer> makeTracer(const char* func, const char* filename, int32_t lineno, const char* name, Category category) = 0;
    virtual op<Logger> makeLogger(const char* func, const char* filename, int32_t lineno, const char* name) = 0;
};

}
