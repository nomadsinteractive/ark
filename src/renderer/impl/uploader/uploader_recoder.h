#ifndef ARK_RENDERER_IMPL_UPLOADER_UPLOADER_RECORDER_H_
#define ARK_RENDERER_IMPL_UPLOADER_UPLOADER_RECORDER_H_

#include <vector>

#include "core/inf/array.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/uploader.h"

namespace ark {

class UploaderRecorder : public Uploader {
public:
    UploaderRecorder(Uploader& delegate);

    virtual void upload(Writable& writable) override;

    std::vector<std::pair<size_t, ByteArray::Borrowed>> toStrips() const;

private:
    std::vector<std::pair<size_t, sp<ByteArray>>> _strips;
};

}

#endif
