#ifndef ARK_CORE_IMPL_ASSET_ZIP_ASSET_H_
#define ARK_CORE_IMPL_ASSET_ZIP_ASSET_H_

#include <zip.h>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"
#include "core/impl/dictionary/directory.h"

#include "core/inf/dictionary.h"

namespace ark {

class ZipAsset : public Asset {
public:
    ZipAsset(const sp<Readable>& zipReadable);

    virtual sp<Readable> get(const String& name) override;

    bool hasEntry(const String& name) const;

public:
    class Stub {
    public:
        Stub(const sp<Readable>& zipReadable);
        ~Stub();

        const sp<Readable>& readable() const;
        int32_t size() const;
        int32_t position() const;

        zip_t* archive();
        zip_source_t* source();

    private:
        sp<Readable> _zip_readable;
        int32_t _size;

        zip_t* _zip_archive;
        zip_source_t* _zip_source;
    };


private:
    sp<Stub> _stub;
};

}

#endif
