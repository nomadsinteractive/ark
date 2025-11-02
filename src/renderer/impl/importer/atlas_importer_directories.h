#pragma once

#include "core/inf/builder.h"
#include "core/inf/storage.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterDirectories final : public AtlasImporter {
public:
    AtlasImporterDirectories(Vector<String> directories);

    void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::builder("directories")]]
    class BUILDER final : public Builder<AtlasImporter> {
    public:
        BUILDER(const document& manifest);

        sp<AtlasImporter> build(const Scope& args) override;

    private:
        Vector<String> _directories;
    };

private:
    Vector<String> _directories;
    sp<ResourceLoaderContext> _resource_loader_context;
};

}
