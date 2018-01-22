#ifndef ARK_RENDERER_BASE_CHARACTERS_H_
#define ARK_RENDERER_BASE_CHARACTERS_H_

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/alphabet.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Characters {
public:
//  [[script::bindings::auto]]
    Characters(const sp<Layer>& layer, float textScale = 1.0f, float letterSpacing = 0.0f, float lineHeight = 0.0f, float lineIndent = 0.0f);
    Characters(const sp<Layer>& layer, const sp<ResourceLoaderContext>& resourceLoaderContext, float textScale, float letterSpacing, float lineHeight, float lineIndent);

    const sp<Layer>& layer() const;
//  [[script::bindings::property]]
    const List<sp<RenderObject>>& characters() const;

//  [[script::bindings::property]]
    const sp<Size>& size() const;

//  [[script::bindings::property]]
    const std::wstring& text() const;
//  [[script::bindings::property]]
    void setText(const std::wstring& text);

//  [[script::bindings::auto]]
    void setBounds(float x, float y, float width, float height);

//[[plugin::resource-loader]]
    class BUILDER : public Builder<Characters> {
    public:
        BUILDER(BeanFactory& factory, const document manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Characters> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Layer>> _layer;
        sp<Builder<String>> _text;
        sp<ResourceLoaderContext> _resource_loader_context;

        float _text_scale;
        float _letter_spacing;
        float _line_height;
        float _line_indent;

    };

private:
    void createContent();

    Alphabet::Metrics getItemMetrics(wchar_t c) const;

    void place(float boundary, wchar_t c, float& flowx, float& flowy, float& fontHeight);

private:
    sp<Layer> _layer;
    sp<ObjectPool> _object_pool;

    List<sp<RenderObject>> _characters;
    std::wstring _text;

    float _text_scale;
    float _letter_spacing;
    float _line_height;
    float _line_indent;

    float _x, _y;
    float _width, _height;

    sp<Alphabet> _alphabet;
    sp<AlphabetLayer> _alphabet_layer;

    sp<Atlas> _atlas;

    sp<Size> _size;
};

}

#endif
