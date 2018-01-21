#include "graphics/impl/alphabet/alphabet_with_fallback.h"

#include "core/base/bean_factory.h"

namespace ark {

AlphabetWithFallback::AlphabetWithFallback(const sp<Alphabet>& alphabet, const sp<Alphabet>& fallback)
    : _alphabet(alphabet), _fallback(fallback), _load_success(false)
{
}

bool AlphabetWithFallback::load(uint32_t c, Metrics& metrics, bool loadGlyph, bool hasFallback)
{
    _load_success = _alphabet->load(c, metrics, loadGlyph, true);
    if(_load_success)
        return true;
    return _fallback->load(c, metrics, loadGlyph, hasFallback);
}

void AlphabetWithFallback::draw(const bitmap& image, int32_t x, int32_t y)
{
    if(_load_success)
        _alphabet->draw(image, x, y);
    else
        _fallback->draw(image, x, y);
}

AlphabetWithFallback::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Alphabet>>& delegate, const String& value)
    : _delegate(delegate), _fallback(factory.ensureBuilder<Alphabet>(value))
{
}

sp<Alphabet> AlphabetWithFallback::STYLE::build(const sp<Scope>& args)
{
    return sp<AlphabetWithFallback>::make(_delegate->build(args), _fallback->build(args));
}

}
