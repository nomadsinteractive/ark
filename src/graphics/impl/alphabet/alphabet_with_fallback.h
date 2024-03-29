#ifndef ARK_GRAPHICS_IMPL_ALPHABET_ALPHABET_WITH_FALLBACK_H_
#define ARK_GRAPHICS_IMPL_ALPHABET_ALPHABET_WITH_FALLBACK_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/alphabet.h"

namespace ark {

class AlphabetWithFallback : public Alphabet {
public:
    AlphabetWithFallback(const sp<Alphabet>& alphabet, const sp<Alphabet>& fallback);

    virtual void setTextSize(const Font::TextSize& size) override;

    virtual Optional<Metrics> measure(int32_t c) override;
    virtual bool draw(uint32_t c, Bitmap& image, int32_t x, int32_t y) override;

//  [[plugin::style("fallback")]]
    class STYLE : public Builder<Alphabet> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Alphabet>>& delegate, const String& value);

        virtual sp<Alphabet> build(const Scope& args) override;

    private:
        sp<Builder<Alphabet>> _delegate;
        sp<Builder<Alphabet>> _fallback;
    };

private:
    sp<Alphabet> _alphabet;
    sp<Alphabet> _fallback;
};

}

#endif
