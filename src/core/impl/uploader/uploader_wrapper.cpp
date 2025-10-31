#include "core/impl/uploader/uploader_wrapper.h"

namespace ark {

UploaderWrapper::UploaderWrapper(sp<Uploader> delegate)
    : Wrapper(std::move(delegate)), Uploader(_wrapped->size()) {
}

bool UploaderWrapper::update(uint32_t tick)
{
    return _wrapped->update(tick) || _timestamp.update(tick);
}

void UploaderWrapper::upload(Writable& buf)
{
    _wrapped->upload(buf);
}

void UploaderWrapper::setDelegate(sp<Uploader> delegate)
{
    _size = delegate->size();
    _wrapped = std::move(delegate);
    _timestamp.markDirty();
}

}
