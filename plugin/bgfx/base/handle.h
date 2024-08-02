#pragma once

#include <bgfx/bgfx.h>

#include "core/base/api.h"

namespace ark::plugin::bgfx {

template<typename T> class Handle {
public:
    Handle()
        :_handle{::bgfx::kInvalidHandle} {
    }
    Handle(Handle&& other)
        :_handle(other.release()) {
    }
    ~Handle() {
        if(isValid())
            ::bgfx::destroy(_handle);
    }
    DISALLOW_COPY_AND_ASSIGN(Handle);

    explicit operator bool() const {
        return isValid();
    }

    Handle& operator=(Handle&& other) {
        _handle = other.release();
        return *this;
    }

    operator T() const {
        return _handle;
    }

    uint64_t id() const {
        return isValid() ? _handle.idx + 1 : 0;
    }

    bool isValid() const {
        return ::bgfx::isValid(_handle);
    }

    void reset(T handle)
    {
        if(isValid())
            ::bgfx::destroy(_handle);
        _handle = handle;
    }

    void destroy() {
        ::bgfx::destroy(release());
    }

    T release() {
        T handle = _handle;
        _handle = {::bgfx::kInvalidHandle};
        return handle;
    }

    ResourceRecycleFunc recycle() {
        const T handle = release();
        return [handle](GraphicsContext& /*context*/) {
            ::bgfx::destroy(handle);
        };
    }

private:
    T _handle;
};

}