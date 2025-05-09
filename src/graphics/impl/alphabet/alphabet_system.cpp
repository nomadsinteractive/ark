#include "graphics/impl/alphabet/alphabet_system.h"

#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/util/documents.h"

#include "graphics/base/font.h"

#include "platform/platform.h"

namespace ark {

AlphabetSystem::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _text_size(Documents::getAttribute<Font::TextSize>(manifest, constants::TEXT_SIZE, Font::TextSize(24, Font::FONT_SIZE_UNIT_PT))),
      _lang(factory.getBuilder<String>(manifest, "lang"))
{
}

sp<Alphabet> AlphabetSystem::BUILDER::build(const Scope& args)
{
    const Font font(_text_size);
    const String lang = _lang ? *(_lang->build(args)) : String("");
    return Platform::getSystemAlphabet(font, lang);
}

}
