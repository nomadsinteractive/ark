#pragma once

#include "renderer/inf/snippet_factory.h"

namespace ark::plugin::opengl {

class SnippetFactoryGLES30 : public SnippetFactory {
public:
    virtual sp<Snippet> createCoreSnippet() override;
};

}
