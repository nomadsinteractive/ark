#pragma once

#include <bgfx/bgfx.h>

#include "renderer/forwarding.h"

#include "bgfx/base/handle.h"

namespace ark::plugin::bgfx {

template<typename T, typename U> class ResourceBase : public U {
public:
    template<typename... Args> ResourceBase(Args&&... args)
        : U(std::forward<Args>(args)...) {
    }

    uint64_t id() override {
        return _handle.id();
    }

    ResourceRecycleFunc recycle() override {
        T handle = _handle.release();
        return [handle](GraphicsContext& context) {
            ::bgfx::destroy(handle);
        };
    }

    T handle() const {
        return _handle;
    }

protected:
    Handle<T> _handle;
};

}