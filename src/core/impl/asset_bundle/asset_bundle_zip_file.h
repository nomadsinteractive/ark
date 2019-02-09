#ifndef ARK_CORE_IMPL_ASSET_BUNDLE_ZIP_FILE_H_
#define ARK_CORE_IMPL_ASSET_BUNDLE_ZIP_FILE_H_

#include <zip.h>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetBundleZipFile : public AssetBundle {
public:
    AssetBundleZipFile(const sp<Readable>& zipReadable, const String& zipLocation);

    virtual sp<Asset> get(const String& name) override;
    virtual sp<AssetBundle> getBundle(const String& path) override;

    bool hasEntry(const String& name) const;

public:
    class Stub {
    public:
        Stub(const sp<Readable>& zipReadable, const String& zipLocation);
        ~Stub();

        const sp<Readable>& readable() const;
        const String& location() const;
        int32_t size() const;
        int32_t position() const;

        zip_t* archive();
        zip_source_t* source();

    private:
        sp<Readable> _zip_readable;
        String _zip_location;
        int32_t _size;

        zip_t* _zip_archive;
        zip_source_t* _zip_source;
    };

    sp<Stub> _stub;

};

}

#endif
