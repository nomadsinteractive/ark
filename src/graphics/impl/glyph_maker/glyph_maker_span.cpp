#include "graphics/impl/glyph_maker/glyph_maker_span.h"

#include <sstream>

#include "core/base/string.h"
#include "core/util/strings.h"

#include "graphics/base/glyph.h"

namespace ark {

namespace {

bool compareToken(const std::wstring& text, size_t& pos, const String& token)
{
    for(size_t i = 0; i < token.length(); ++i)
        if(pos + i >= text.length() || text.at(pos + i) != token.at(i))
            return false;
    if(text.length() > pos + token.length() && text.at(pos + token.length()) == '>')
    {
        pos += token.length();
        return true;
    }
    return false;
}

sp<GlyphMaker> parseNextGlyphMakerToken(const std::wstring& text, const Map<String, sp<GlyphMaker>>& spans, size_t& pos)
{
    for(const auto& [name, glyphMaker] : spans)
        if(compareToken(text, pos, name))
            return glyphMaker;
    return nullptr;
}

void appendGlyphs(GlyphMaker& gm, const std::wstring& text, Vector<sp<Glyph>>& glyphs)
{
    for(sp<Glyph>& i : gm.makeGlyphs(text))
        glyphs.push_back(std::move(i));
}

}

GlyphMakerSpan::GlyphMakerSpan(sp<GlyphMaker> glyphMakerDefault, Map<String, sp<GlyphMaker>> spans)
    : _glyph_maker_default(std::move(glyphMakerDefault)), _glyph_maker_spans(std::move(spans))
{
}

Vector<sp<Glyph>> GlyphMakerSpan::makeGlyphs(const std::wstring& text)
{
    Vector<sp<Glyph>> glyphs;
    sp<GlyphMaker> glyphMaker = _glyph_maker_default;

    std::wostringstream sb;

    for(size_t pos = 0; pos < text.size(); ++pos)
    {
        const wchar_t c = text.at(pos);
        if(glyphMaker != _glyph_maker_default)
        {
            if(c == '<' && text.size() > pos + 1 && text.at(pos + 1) == '/')
            {
                pos += 2;
                if(sp<GlyphMaker> closeGlyphMaker = parseNextGlyphMakerToken(text, _glyph_maker_spans, pos))
                {
                    CHECK(closeGlyphMaker == glyphMaker, "Open and close token mismatch in text \"%s\"", Strings::toUTF8(text).c_str());
                    appendGlyphs(closeGlyphMaker, sb.str(), glyphs);
                    glyphMaker = _glyph_maker_default;
                    sb = {};
                    continue;
                }
                pos -= 2;
            }
        }
        else if(c == '<')
        {
            pos += 1;
            if(sp<GlyphMaker> openGlyphMaker = parseNextGlyphMakerToken(text, _glyph_maker_spans, pos))
            {
                appendGlyphs(_glyph_maker_default, sb.str(), glyphs);
                glyphMaker = openGlyphMaker;
                sb = {};
                continue;
            }
            pos -= 1;
        }
        sb << c;
    }

    appendGlyphs(_glyph_maker_default, sb.str(), glyphs);
    return glyphs;
}

}
