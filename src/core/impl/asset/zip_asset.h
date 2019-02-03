#ifndef ARK_CORE_IMPL_ASSET_ZIP_ASSET_H_
#define ARK_CORE_IMPL_ASSET_ZIP_ASSET_H_

#include <zip.h>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/asset.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ZipAsset : public Asset {
public:
    ZipAsset(const sp<Readable>& zipReadable, const String& zipLocation);

    virtual sp<Readable> get(const String& name) override;
    virtual sp<Asset> getAsset(const String& path) override;
    virtual String getRealPath(const String& path) override;

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
