#include "core/util/storage_type.h"

#include "core/inf/storage.h"
#include "core/types/shared_ptr.h"

namespace ark {

void StorageType::load(const sp<Storage>& self, const sp<Readable>& src)
{
    self->import(src);
}

void StorageType::save(const sp<Storage>& self, const sp<Writable>& out)
{
    self->output(out);
}

void StorageType::jsonLoad(const sp<Storage>& self, const Json& json)
{
    self->jsonLoad(json);
}

Json StorageType::jsonDump(const sp<Storage>& self)
{
    return self->jsonDump();
}

}
