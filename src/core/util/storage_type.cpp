#include "core/util/storage_type.h"

#include "core/inf/storage.h"
#include "core/types/shared_ptr.h"

namespace ark {

void StorageUtil::load(const sp<Storage>& self, const sp<Readable>& src)
{
    self->import(src);
}

void StorageUtil::save(const sp<Storage>& self, const sp<Writable>& out)
{
    self->output(out);
}

void StorageUtil::jsonLoad(const sp<Storage>& self, const Json& json)
{
    self->jsonLoad(json);
}

Json StorageUtil::jsonDump(const sp<Storage>& self)
{
    return self->jsonDump();
}

}
