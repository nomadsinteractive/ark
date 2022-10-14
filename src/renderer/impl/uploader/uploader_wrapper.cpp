#include "renderer/impl/uploader/uploader_wrapper.h"

#include "core/impl/updatable/updatable_wrapper.h"

namespace ark {

UploaderWrapper::UploaderWrapper(sp<Uploader> delegate)
    : Uploader(delegate->size()), Delegate<Uploader>(std::move(delegate))
{
}

size_t UploaderWrapper::size()
{
    return _delegate->size();
}

void UploaderWrapper::upload(Writable& writable)
{
    _delegate->upload(writable);
}

sp<Updatable> UploaderWrapper::updatable()
{
    return _delegate->updatable();
}

void UploaderWrapper::reset(sp<Uploader> uploader)
{
    Delegate<Uploader>::reset(std::move(uploader));
    _updatable->reset(_delegate->updatable());
}

}
