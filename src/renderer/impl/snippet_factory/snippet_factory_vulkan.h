#pragma once

#include "core/base/api.h"

#include "renderer/inf/snippet_factory.h"

namespace ark {

class ARK_API SnippetFactoryVulkan final : public SnippetFactory {
public:
    sp<Snippet> createCoreSnippet() override;
};

}
