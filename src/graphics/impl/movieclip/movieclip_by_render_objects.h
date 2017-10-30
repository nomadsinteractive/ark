#ifndef ARK_GRAPHICS_IMPL_MOVIECLIP_MOIVIECLIP_BY_RENDER_OBJECTS_H_
#define ARK_GRAPHICS_IMPL_MOVIECLIP_MOIVIECLIP_BY_RENDER_OBJECTS_H_

#include <list>

#include "core/inf/builder.h"
#include "core/inf/iterator.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class MovieclipByRenderObjects : public Movieclip {
public:
    MovieclipByRenderObjects(const sp<Layer>& layer, const sp<Transform>& transform, const std::list<sp<Builder<RenderObject>>>& renderObjects, const sp<Scope>& args);

    virtual bool hasNext() override;
    virtual sp<Renderer> next() override;

//  [[plugin::builder("by-render-object")]]
    class BUILDER : public Builder<Movieclip> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Movieclip> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Layer>> _layer;
        sp<Builder<Transform>> _transform;

        std::list<sp<Builder<RenderObject>>> _render_objects;
    };

private:
    std::list<sp<Renderer>> _movieclips;
    std::list<sp<Renderer>>::const_iterator _iterator;

};

}


#endif
