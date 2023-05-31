#pragma once

#include "core/ark.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"

#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/mesh.h"
#include "renderer/base/model_bundle.h"
#include "renderer/inf/model_loader.h"


namespace ark {
namespace plugin {
namespace gltf {

class ModelImporterGltf : public ModelLoader::Importer {
public:

    virtual Model import(const Manifest& manifest, MaterialBundle& materialBundle) override;

//  [[plugin::builder::by-value("gltf")]]
    class BUILDER : public Builder<ModelLoader::Importer> {
    public:
        BUILDER() = default;

        virtual sp<ModelLoader::Importer> build(const Scope& args) override;

    };

private:
    std::vector<sp<Texture>> _textures;

};

}
}
}
