#ifndef ARK_CORE_IMPL_INPUT_INPUT_REPEAT_H_
#define ARK_CORE_IMPL_INPUT_INPUT_REPEAT_H_

#include "core/inf/uploader.h"
#include "core/types/shared_ptr.h"

namespace ark {

class InputRepeat : public Uploader {
public:
    InputRepeat(sp<Uploader> delegate, size_t length, size_t stride = 0);

    virtual bool update(uint64_t timestamp) override;
    virtual void upload(Writable& writable) override;

private:
    sp<Uploader> _delegate;
    size_t _length;
    size_t _stride;

};

}

#endif
