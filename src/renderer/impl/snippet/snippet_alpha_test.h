#ifndef ARK_RENDERER_IMPL_SNIPPET_ALPHA_TEST_H_
#define ARK_RENDERER_IMPL_SNIPPET_ALPHA_TEST_H_

#include "core/inf/builder.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetAlphaTest : public Snippet {
public:
    SnippetAlphaTest(float threadhold);

    virtual void preInitialize(PipelineBuildingContext& context) override;

//  [[plugin::builder("alpha-test")]]
    class BUILDER : public Builder<Snippet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Snippet> build(const sp<Scope>& args) override;

    private:
        float _threshold;
    };

private:
    float _threshold;

};

}

#endif
