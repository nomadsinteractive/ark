#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_NINE_PATCH_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_NINE_PATCH_H_

#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterNinePatch : public AtlasImporter {
public:
    AtlasImporterNinePatch(document manifest);

    virtual void import(Atlas& atlas, const sp<Readable>& readable) override;

    class Attachment {
    public:
        const sp<Vertices>& ensureVerticesTriangleStrips(int32_t type) const;
        const sp<Vertices>& ensureVerticesQuads(int32_t type) const;

    private:
        void import(Atlas& atlas, const document& manifest);

        void add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Atlas& atlas);
        void add(int32_t type, uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds);

        sp<Vertices> makeNinePatchVertices(uint32_t textureWidth, uint32_t textureHeight, const Rect& paddings, const Rect& bounds) const;

    private:
        std::unordered_map<int32_t, sp<Vertices>> _vertices_triangle_strips;
        std::unordered_map<int32_t, sp<Vertices>> _vertices_quads;

        friend class AtlasImporterNinePatch;
    };

//  [[plugin::builder("nine-patch")]]
    class BUILDER : public Builder<AtlasImporter> {
    public:
        BUILDER(const document& manifest);

        virtual sp<AtlasImporter> build(const Scope& args) override;

    private:
        document _manifest;
    };

private:
    document _manifest;
};

}

#endif
