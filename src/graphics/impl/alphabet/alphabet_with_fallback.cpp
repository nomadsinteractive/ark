#include "graphics/impl/alphabet/alphabet_with_fallback.h"

#include "core/base/bean_factory.h"

namespace ark {

AlphabetWithFallback::AlphabetWithFallback(const sp<Alphabet>& alphabet, const sp<Alphabet>& fallback)
    : _alphabet(alphabet), _fallback(fallback)
{
}

void AlphabetWithFallback::setTextSize(const Font::TextSize& size)
{
    _alphabet->setTextSize(size);
    _fallback->setTextSize(size);
}

Optional<Alphabet::Metrics> AlphabetWithFallback::measure(int32_t c)
{
    Optional<Alphabet::Metrics> opt = _alphabet->measure(c);
    return opt ? opt : _fallback->measure(c);
}

bool AlphabetWithFallback::draw(uint32_t c, Bitmap& image, int32_t x, int32_t y)
{
    if(_alphabet->draw(c, image, x, y))
        return true;
    return _fallback->draw(c, image, x, y);
}

AlphabetWithFallback::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Alphabet>>& delegate, const String& value)
    : _delegate(delegate), _fallback(factory.ensureBuilder<Alphabet>(value))
{
}

sp<Alphabet> AlphabetWithFallback::STYLE::build(const Scope& args)
{
    return sp<AlphabetWithFallback>::make(_delegate->build(args), _fallback->build(args));
}

}
