#ifndef ARK_GRAPHICS_IMPL_ALPHABET_ALPHABET_WITH_FALLBACK_H_
#define ARK_GRAPHICS_IMPL_ALPHABET_ALPHABET_WITH_FALLBACK_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/alphabet.h"

namespace ark {

class AlphabetWithFallback : public Alphabet {
public:
    AlphabetWithFallback(const sp<Alphabet>& alphabet, const sp<Alphabet>& fallback);

    virtual bool load(uint32_t c, uint32_t& width, uint32_t& height, bool loadGlyph, bool hasFallback) override;
    virtual void draw(const bitmap& image, int32_t x, int32_t y) override;

//  [[plugin::style("fallback")]]
    class STYLE : public Builder<Alphabet> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Alphabet>>& delegate, const String& value);

        virtual sp<Alphabet> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Alphabet>> _delegate;
        sp<Builder<Alphabet>> _fallback;
    };

private:
    sp<Alphabet> _alphabet;
    sp<Alphabet> _fallback;
    bool _load_success;
};

}

#endif
