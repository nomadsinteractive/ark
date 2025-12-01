#pragma once

#include "renderer/inf/snippet_factory.h"

namespace ark::plugin::opengl {

class SnippetFactoryGLES30 final : public SnippetFactory {
public:
    sp<Snippet> createCoreSnippet() override;
};

}
