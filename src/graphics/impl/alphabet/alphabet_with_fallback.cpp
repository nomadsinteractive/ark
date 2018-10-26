#include "graphics/impl/alphabet/alphabet_with_fallback.h"

#include "core/base/bean_factory.h"

namespace ark {

AlphabetWithFallback::AlphabetWithFallback(const sp<Alphabet>& alphabet, const sp<Alphabet>& fallback)
    : _alphabet(alphabet), _fallback(fallback)
{
}

bool AlphabetWithFallback::measure(int32_t c, Metrics& metrics, bool hasFallback)
{
    if(_alphabet->measure(c, metrics, true))
        return true;
    return _fallback->measure(c, metrics, hasFallback);
}

bool AlphabetWithFallback::draw(uint32_t c, const bitmap& image, int32_t x, int32_t y)
{
    if(_alphabet->draw(c, image, x, y))
        return true;
    return _fallback->draw(c, image, x, y);
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
