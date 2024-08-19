#include "core/impl/uploader/uploader_snapshot.h"

#include "core/inf/array.h"
#include "core/inf/writable.h"
#include "core/types/shared_ptr.h"
#include "core/util/uploader_type.h"

namespace ark {

UploaderSnapshot::UploaderSnapshot(Uploader& delegate)
    : Uploader(delegate.size()), _strips(UploaderType::record(delegate))
{
}

void UploaderSnapshot::upload(Writable& writable)
{
    for(const auto& [i, j] : _strips)
        writable.write(j->buf(), j->length(), i);
}

bool UploaderSnapshot::update(uint64_t /*timestamp*/)
{
    return false;
}

}
