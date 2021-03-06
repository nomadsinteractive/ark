#include "graphics/impl/alphabet/alphabet_system.h"

#include "core/base/bean_factory.h"
#include "core/util/documents.h"

#include "graphics/base/font.h"

#include "platform/platform.h"

namespace ark {

AlphabetSystem::BUILDER::BUILDER(BeanFactory& factory, const document manifest)
    : _text_size(Documents::getAttribute<uint32_t>(manifest, Constants::Attributes::TEXT_SIZE, 24)),
      _lang(factory.getBuilder<String>(manifest, "lang"))
{
}

sp<Alphabet> AlphabetSystem::BUILDER::build(const Scope& args)
{
    Font font(Font::FONT_FAMILY_DEFAULT, Font::FONT_STYLE_REGULAR, _text_size);
    const String lang = _lang ? *(_lang->build(args)) : String("");
    return Platform::getSystemAlphabet(font, lang);
}

}
