#pragma once

#include <zip.h>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetBundleZipFile final : public AssetBundle {
public:
    AssetBundleZipFile(sp<Readable> zipReadable, const String& zipLocation);

    sp<Asset> getAsset(const String& name) override;
    sp<AssetBundle> getBundle(const String& path) override;

    Vector<String> listAssets(StringView dirname) override;

    bool hasEntry(const String& name) const;

private:
    class Stub;

    class ReadableZipFile;
    class AssetZipEntry;

    static zip_int64_t _local_zip_source_callback(void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd);

    sp<Stub> _stub;
};

}
