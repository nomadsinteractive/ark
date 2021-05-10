#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_CHARACTERS_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_CHARACTERS_H_

#include "core/inf/builder.h"

#include "renderer/base/atlas.h"

namespace ark {

class AtlasImporterCharacters : public Atlas::Importer {
public:
    AtlasImporterCharacters(String characters, uint32_t fontWidth, uint32_t fontHeight);

    virtual void import(Atlas& atlas) override;

//  [[plugin::builder("characters")]]
    class BUILDER : public Builder<Atlas::Importer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Atlas::Importer> build(const Scope& args) override;

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

#endif
