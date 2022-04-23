#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_CHARACTERS_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_CHARACTERS_H_

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterCharacters : public AtlasImporter {
public:
    AtlasImporterCharacters(String characters, uint32_t fontWidth, uint32_t fontHeight);

    virtual void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::builder("characters")]]
    class BUILDER : public Builder<AtlasImporter> {
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

#endif
