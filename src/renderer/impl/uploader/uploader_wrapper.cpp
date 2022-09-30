#include "renderer/impl/uploader/uploader_wrapper.h"


namespace ark {

size_t UploaderWrapper::size()
{
    return _delegate->size();
}

void UploaderWrapper::upload(Writable& writable)
{
    _delegate->upload(writable);
}

}
