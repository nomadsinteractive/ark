#pragma once

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterCharacters final : public AtlasImporter {
public:
    AtlasImporterCharacters(String characters, uint32_t fontWidth, uint32_t fontHeight);

    void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::builder("characters")]]
    class BUILDER final : public Builder<AtlasImporter> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<AtlasImporter> build(const Scope& args) override;

    private:
        String _characters;
        uint32_t _font_width;
        uint32_t _font_height;
    };

private:
    String _characters;
    uint32_t _font_width;
    uint32_t _font_height;
};

}
