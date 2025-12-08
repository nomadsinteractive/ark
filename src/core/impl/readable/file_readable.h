#pragma once

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

    uint32_t read(void* buffer, uint32_t length) override;
    int32_t seek(int32_t position, int32_t whence) override;
    int32_t remaining() override;
    uint32_t position() override;

    //[[plugin::builder::by-value]]
    class BUILDER final : public Builder<Readable> {
    public:
        BUILDER(BeanFactory& factory, const String& src);

        sp<Readable> build(const Scope& args) override;

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
