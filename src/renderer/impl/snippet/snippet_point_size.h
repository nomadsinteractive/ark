#ifndef ARK_RENDERER_IMPL_SNIPPET_POINT_SIZE_H_
#define ARK_RENDERER_IMPL_SNIPPET_POINT_SIZE_H_

#include "core/inf/builder.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetPointSize : public Snippet {
public:
    virtual void preInitialize(PipelineBuildingContext& context) override;

//  [[plugin::builder::by-value("point-size")]]
    class DICTIONARY : public Builder<Snippet> {
    public:
        DICTIONARY() = default;

        virtual sp<Snippet> build(const Scope& args) override;

    };

};

}

#endif
