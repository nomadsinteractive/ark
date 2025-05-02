#include "core/impl/uploader/uploader_snapshot.h"

#include "core/inf/writable.h"
#include "core/types/shared_ptr.h"
#include "core/util/uploader_type.h"

namespace ark {

UploaderSnapshot::UploaderSnapshot(Uploader& delegate)
    : Uploader(0), _strips(UploaderType::record(delegate))
{
    size_t s = 0;
    for(const auto& [k, v] : _strips)
        s += v.size();

    _size = std::max(s, delegate.size());
}

void UploaderSnapshot::upload(Writable& writable)
{
    for(const auto& [i, j] : _strips)
        writable.write(j.data(), j.size(), i);
}

bool UploaderSnapshot::update(uint64_t /*timestamp*/)
{
    return false;
}

}
