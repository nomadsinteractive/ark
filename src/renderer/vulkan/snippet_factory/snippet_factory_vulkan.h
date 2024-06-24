#pragma once

#include "renderer/inf/snippet_factory.h"

namespace ark::vulkan {

class SnippetFactoryVulkan final : public SnippetFactory {
public:
    sp<Snippet> createCoreSnippet() override;
};

}
