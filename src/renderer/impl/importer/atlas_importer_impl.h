#pragma once

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/importer.h"

#include "renderer/forwarding.h"

namespace ark {

class AtlasImporterImpl final : public AtlasImporter {
public:
    struct Directory {
        String _src;
    };

    struct File {
        String _manifest;
        String _src;
    };

    AtlasImporterImpl(Vector<Directory> directories, Vector<File> files);

    void import(Atlas& atlas, const sp<Readable>& readable) override;

//  [[plugin::builder]]
    class BUILDER final : public Builder<AtlasImporter> {
    public:
        BUILDER(const document& manifest);

        sp<AtlasImporter> build(const Scope& args) override;

    private:
        Vector<Directory> _directories;
        Vector<File> _files;
    };

private:
    Vector<Directory> _directories;
    Vector<File> _files;
};

}
