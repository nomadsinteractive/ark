#pragma once

#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/model_bundle.h"
#include "renderer/inf/model_loader.h"


namespace ark::plugin::gltf {

class ModelImporterGltf final : public ModelLoader::Importer {
public:

    Model import(const Manifest& manifest, MaterialBundle& materialBundle) override;

//  [[plugin::builder::by-value("gltf")]]
    class BUILDER final : public Builder<ModelLoader::Importer> {
    public:
        BUILDER() = default;

        sp<ModelLoader::Importer> build(const Scope& args) override;
    };

private:
    std::vector<sp<Texture>> _textures;

};

}
