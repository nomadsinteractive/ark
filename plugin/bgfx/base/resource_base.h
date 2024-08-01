#pragma once

#include <bgfx/bgfx.h>

#include "renderer/forwarding.h"

namespace ark::plugin::bgfx {

template<typename T, typename U> class ResourceBase : public U {
public:
    template<typename... Args> ResourceBase(Args&&... args)
        : U(std::forward<Args>(args)...) {
    }

    uint64_t id() override
    {
        return _handle.idx;
    }

    ResourceRecycleFunc recycle() override
    {
        T handle = _handle;
        _handle = { ::bgfx::kInvalidHandle };
        return [handle](GraphicsContext& context) {
            ::bgfx::destroy(handle);
        };
    }

    T handle() const
    {
        return _handle;
    }

protected:
    T _handle = { ::bgfx::kInvalidHandle };
};

}