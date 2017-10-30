#ifndef ARK_GRAPHICS_IMPL_FRAME_TEXT_H_
#define ARK_GRAPHICS_IMPL_FRAME_TEXT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/util/alphabets.h"

namespace ark {

//[[core::class]]
class Text : public Renderer, public Block {
public:
    Text(const sp<Layer>& layer, const Alphabets::Characters& characters);

    virtual void render(RenderCommandPipeline& pipeline, float x, float y) override;

    virtual const sp<Size>& size() override;

//  [[plugin::builder("text")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;
    private:
        Alphabets::Characters createCharacters(const Atlas& atlas, const String& str, const sp<Size>& size, const sp<AlphabetLayer>& alphabetLayer);

    private:
        float _letter_spacing;
        float _text_scale;
        float _paragraph_spacing;
        float _line_height;
        float _line_indent;

        sp<Builder<Size>> _size;
        sp<Builder<String>> _text;
        sp<Builder<Layer>> _layer;
    };

private:
    sp<Layer> _layer;
    Alphabets::Characters _characters;

};

}

#endif
