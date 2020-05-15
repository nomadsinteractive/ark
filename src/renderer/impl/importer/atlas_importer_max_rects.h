#ifndef ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_MAX_RECTS_H_
#define ARK_RENDERER_IMPL_IMPORTER_ATLAS_IMPORTER_MAX_RECTS_H_

#include "core/inf/builder.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"

namespace ark {

class AtlasImporterMaxRects : public Atlas::Importer {
public:
    AtlasImporterMaxRects(const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void import(Atlas& atlas, BeanFactory& factory, const document& manifest) override;

//  [[plugin::resource-loader("max-rects")]]
    class BUILDER : public Builder<Atlas::Importer> {
    public:
        BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Atlas::Importer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    struct PackedBitmap {
        String _src;
        int32_t _x;
        int32_t _y;
    };

    class MaxRectsTextureUploader : public Texture::Uploader {
    public:
        MaxRectsTextureUploader(uint32_t width, uint32_t height, uint8_t channels, const sp<BitmapBundle>& bitmapBundle, std::vector<PackedBitmap> bitmaps);

        void upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate);

    private:
        uint32_t _width;
        uint32_t _height;
        uint8_t _channels;
        sp<BitmapBundle> _bitmap_bundle;
        std::vector<PackedBitmap> _bitmaps;

    };

private:
    sp<ResourceLoaderContext> _resource_loader_context;

};

}

#endif
