#ifndef ARK_RENDERER_IMPL_SNIPPET_UCOLOR_H_
#define ARK_RENDERER_IMPL_SNIPPET_UCOLOR_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"

#include "renderer/inf/snippet.h"

namespace ark {

class SnippetUColor : public Snippet {
public:
    SnippetUColor(const sp<Vec4>& color);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context) override;

//  [[plugin::builder::by-value("ucolor")]]
    class DICTIONARY : public Builder<Snippet> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Snippet> build(const sp<Scope>&) override;

    private:
        sp<Builder<Vec4>> _color;

    };

private:
    sp<Vec4> _color;

};

}

#endif
