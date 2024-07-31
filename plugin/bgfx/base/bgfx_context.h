#pragma once

#include <bgfx/bgfx.h>

namespace ark::plugin::bgfx {

struct BgfxContext {
    ::bgfx::ViewId _view_id = 0;
};

}