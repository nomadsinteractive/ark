#ifndef ARK_RENDERER_IMPL_UPLOADER_UPLOADER_WRAPPER_H_
#define ARK_RENDERER_IMPL_UPLOADER_UPLOADER_WRAPPER_H_

#include "core/base/delegate.h"
#include "core/inf/updatable.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/uploader.h"

namespace ark {

class UploaderWrapper : public Uploader, public Delegate<Uploader> {
public:
    UploaderWrapper(sp<Uploader> delegate);

    virtual size_t size() override;
    virtual void upload(Writable& writable) override;

    virtual sp<Updatable> updatable() override;

    void reset(sp<Uploader> uploader);

private:
    sp<UpdatableWrapper> _updatable;
};

}

#endif
