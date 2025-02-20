#pragma once

#include "renderer/inf/snippet_factory.h"

namespace ark::plugin::vulkan {

class SnippetFactoryVulkan final : public SnippetFactory {
public:
    sp<Snippet> createCoreSnippet() override;
};

}
