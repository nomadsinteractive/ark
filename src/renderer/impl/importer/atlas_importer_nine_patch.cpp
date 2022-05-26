#include "renderer/impl/importer/atlas_importer_nine_patch.h"

#include "core/util/documents.h"

#include "renderer/base/atlas.h"
#include "renderer/base/texture.h"

namespace ark {

AtlasImporterNinePatch::BUILDER::BUILDER(const document& manifest)
    : _manifest(manifest)
{
}

sp<AtlasImporter> AtlasImporterNinePatch::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterNinePatch>::make(_manifest);
}

AtlasImporterNinePatch::AtlasImporterNinePatch(document manifest)
    : _manifest(std::move(manifest))
{
}

void AtlasImporterNinePatch::import(Atlas& atlas, const sp<Readable>& /*readable*/)
{
    const sp<Atlas::AttachmentNinePatch>& vertices = atlas.attachments().ensure<Atlas::AttachmentNinePatch>();
    vertices->import(atlas, _manifest);
}

}
