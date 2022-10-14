#ifndef ARK_RENDERER_IMPL_UPLOADER_UPLOADER_IMPL_H_
#define ARK_RENDERER_IMPL_UPLOADER_UPLOADER_IMPL_H_

#include <map>
#include <vector>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/uploader.h"

namespace ark {

class UploaderImpl : public Uploader {
public:
    UploaderImpl(std::map<size_t, sp<Input>> inputs, size_t size = 0);

    virtual size_t size() override;
    virtual void upload(Writable& writable) override;

    virtual sp<Updatable> updatable() override;

    void addInput(size_t offset, sp<Input> input);
    void removeInput(size_t offset);

private:
    sp<Updatable> makeUpdatable() const;

private:
    std::map<size_t, sp<Input>> _inputs;
    sp<UpdatableWrapper> _updatable;

    std::vector<uint8_t> _buf;
};

}

#endif
