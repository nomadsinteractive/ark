#pragma once

#include "core/base/timestamp.h"
#include "core/base/wrapper.h"
#include "core/inf/uploader.h"
#include "core/types/shared_ptr.h"

namespace ark {

class UploaderWrapper : public Wrapper<Uploader>, public Uploader {
public:
    UploaderWrapper(sp<Uploader> delegate);

    virtual bool update(uint64_t timestamp) override;
    virtual void upload(Writable& buf) override;

    void setDelegate(sp<Uploader> delegate);

private:
    Timestamp _timestamp;
};

}
