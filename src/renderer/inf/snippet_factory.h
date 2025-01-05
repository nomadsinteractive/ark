#pragma once

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API SnippetFactory {
public:
    virtual ~SnippetFactory() = default;

    virtual sp<Snippet> createCoreSnippet() = 0;
};

}
