#include "graphics/impl/frame/text.h"

#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"

#include "graphics/base/size.h"
#include "graphics/base/render_object.h"
#include "graphics/base/layer_context.h"
#include "graphics/inf/alphabet.h"

#include "renderer/impl/layer/image_layer.h"
#include "renderer/impl/layer/alphabet_layer.h"

namespace ark {

extern bool g_isOriginBottom;

Text::Text(const sp<Layer>& layer, const Alphabets::Characters& characters)
    : _layer(layer), _characters(characters)
{
}

void Text::render(RenderRequest& /*pipeline*/, float x, float y)
{
    const sp<LayerContext>& renderContext = _layer->layerContext();
    for(const sp<RenderObject>& i : _characters._render_objects->items())
        renderContext->draw(x, y, i);
}

const sp<Size>& Text::size()
{
    return _characters._size;
}

Text::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _letter_spacing(Documents::getAttribute<float>(manifest, "letter-spacing", 0)), _text_scale(Documents::getAttribute<float>(manifest, "text-scale", 0)),
      _paragraph_spacing(Documents::getAttribute<float>(manifest, "paragraph-spacing", 0)),
      _line_height(Documents::getAttribute<float>(manifest, "line-height", 0)), _line_indent(Documents::getAttribute<float>(manifest, "line-indent", 0)),
      _size(factory.ensureBuilder<Size>(manifest, Constants::Attributes::SIZE)), _text(Strings::load(manifest, Constants::Attributes::TEXT)),
      _layer(factory.ensureBuilder<Layer>(manifest, Constants::Attributes::LAYER))
{
}

sp<Renderer> Text::BUILDER::build(const sp<Scope>& args)
{
    const sp<String> text = _text->build(args);
    NOT_NULL(_layer);
    const sp<Layer> layer = _layer->build(args);
    const sp<Size> size = _size->build(args);
    if(layer.is<AlphabetLayer>())
    {
        const sp<AlphabetLayer>& alphabetLayer = layer.cast<AlphabetLayer>();
        return sp<Text>::make(layer, createCharacters(alphabetLayer->atlas(), text, size, alphabetLayer));
    }
    DCHECK(layer.is<ImageLayer>(), "Label can only be added to a AlphabetLayer or ImageLayer");
    return sp<Text>::make(layer, createCharacters(layer.cast<ImageLayer>()->atlas(), text, size, nullptr));
}

Alphabets::Characters Text::BUILDER::createCharacters(const Atlas& atlas, const String& text, const sp<Size>& size, const sp<AlphabetLayer>& alphabetLayer)
{
    float width = size->width();
    float y = 0;
    Alphabets::Characters characters(sp<List<sp<RenderObject>>>::make(), size);
    const std::wstring utext = Strings::fromUTF8(text);
    std::wstring::size_type iter = 0;
    while(iter != std::wstring::npos)
    {
        const std::wstring::size_type n = utext.find('\n', iter);
        const std::wstring s = utext.substr(iter, n != std::wstring::npos ? n - iter : std::wstring::npos);
        iter = n != std::wstring::npos ? n + 1 : n;
        Alphabets::Characters cs = alphabetLayer ? Alphabets::create(alphabetLayer->alphabet(), s, _text_scale, _letter_spacing, _line_indent, y, width, _line_height, -_line_indent)
                                                 : Alphabets::create(atlas, s, _letter_spacing, _line_indent, y, width, _line_height, -_line_indent);
        for(const sp<RenderObject>& j : cs._render_objects->items())
            characters._render_objects->push_back(j);
        y -= (g_upDirection * cs._size->height());
    }
    return characters;
}

}
