#include "font_config.h"

#include "core/util/strings.h"
#include "core/util/documents.h"

#include "platform/platform.h"

namespace ark {
namespace platform {
namespace android {

static void _loadSystemFontWithFallback_5x(const Font& font, const String& lang, String& alphabet, String& fallback);
static void _loadSystemFontWithFallback_4x(const Font& font, const String& lang, String& alphabet, String& fallback);

void _loadSystemFontWithFallback(const Font& font, const String& lang, String& alphabet, String& fallback)
{
    if(Platform::isFile("/system/etc/fonts.xml"))
    {
        _loadSystemFontWithFallback_5x(font, lang, alphabet, fallback);
        return;
    }
    _loadSystemFontWithFallback_4x(font, lang, alphabet, fallback);
}

void _loadSystemFontWithFallback_5x(const Font& /*font*/, const String& lang, String& alphabet, String& fallback)
{
    bool isDefault = true;
    const document manifest = Documents::loadFromFile("/system/etc/fonts.xml");
    for(const document& i : manifest->children("family"))
    {
        if(isDefault)
        {
            for(const document& j : i->children("font"))
                if(Documents::getAttribute<uint32_t>(j, "weight", 0) == 400)
                {
                    alphabet = j->value();
                    break;
                }
            isDefault = false;
        }
        else
        {
            const String& fl = Documents::getAttribute(i, "lang");
            if(fl && fl.startsWith(lang))
                for(const document& j : i->children("font"))
                    if(Documents::getAttribute<uint32_t>(j, "weight", 0) == 400)
                    {
                        fallback = j->value();
                        return;
                    }
        }
    }
}

void _loadSystemFontWithFallback_4x(const Font& /*font*/, const String& /*lang*/, String& alphabet, String& fallback)
{
    const String system_fonts_file = "/system/etc/system_fonts.xml";
    const String fallback_fonts_file = "/system/etc/fallback_fonts.xml";
    if(Platform::isFile(system_fonts_file))
    {
        const document manifest = Documents::loadFromFile(system_fonts_file);
        const document family = manifest->getChild("family");
        if(family)
        {
            const document fileset = family->getChild("fileset");
            const document file = fileset ? fileset->getChild("file") : document::null();
            if(file)
                alphabet = file->value();
        }
    }
    if(Platform::isFile(fallback_fonts_file))
    {
        const document manifest = Documents::loadFromFile(fallback_fonts_file);
        const document family = manifest->getChild("family");
        if(family)
        {
            const document fileset = family->getChild("fileset");
            const document file = fileset ? fileset->getChild("file") : document::null();
            if(file)
                fallback = file->value();
        }
    }
}


}
}
}
