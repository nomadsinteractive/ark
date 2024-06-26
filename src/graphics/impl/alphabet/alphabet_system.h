#pragma once

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/font.h"

namespace ark {

class AlphabetSystem {
public:

//[[plugin::builder("system")]]
    class BUILDER : public Builder<Alphabet> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Alphabet> build(const Scope& args) override;

    private:
        Font::TextSize _text_size;
        sp<Builder<String>> _lang;
    };

};

}
