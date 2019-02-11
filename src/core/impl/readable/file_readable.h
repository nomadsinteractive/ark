#ifndef ARK_CORE_IMPL_READABLE_FILE_READABLE_H_
#define ARK_CORE_IMPL_READABLE_FILE_READABLE_H_

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/readable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API FileReadable : public Readable {
public:
    FileReadable(const String& filepath, const String& mode);
    FileReadable(FILE* fp);
    ~FileReadable() override;

    virtual uint32_t read(void* buffer, uint32_t length) override;
    virtual int32_t seek(int32_t position, int32_t whence) override;
    virtual int32_t remaining() override;

//[[plugin::builder::by-value]]
    class BUILDER : public Builder<Readable> {
    public:
        BUILDER(BeanFactory& factory, const String& src);

        virtual sp<Readable> build(const sp<Scope>& args) override;

    private:
        sp<Builder<String>> _src;

    };

private:
    void init();

private:
    String _filepath;
    FILE* _fp;
    int32_t _size;
};

}

#endif
