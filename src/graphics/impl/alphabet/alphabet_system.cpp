#include "graphics/impl/alphabet/alphabet_system.h"

#include "core/util/documents.h"

#include "graphics/base/font.h"

#include "platform/platform.h"

namespace ark {

AlphabetSystem::BUILDER::BUILDER(BeanFactory& /*factory*/, const document manifest)
    : _text_size(Documents::getAttribute<uint32_t>(manifest, Constants::Attributes::TEXT_SIZE, 24)),
      _lang(Strings::load(manifest, "lang", ""))
{
}

sp<Alphabet> AlphabetSystem::BUILDER::build(const sp<Scope>& args)
{
    Font font(Font::FONT_FAMILY_DEFAULT, Font::FONT_STYLE_REGULAR, _text_size);
    const String lang = _lang ? *(_lang->build(args)) : String("");
    return Platform::getSystemAlphabet(font, lang);
}

}
