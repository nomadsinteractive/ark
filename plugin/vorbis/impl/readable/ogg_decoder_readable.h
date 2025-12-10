#pragma once

#include "core/inf/builder.h"
#include "core/inf/readable.h"
#include "core/types/shared_ptr.h"

#include <vorbis/vorbisfile.h>

namespace ark::plugin::vorbis {

class OggDecoderReadable : public Readable {
public:
    OggDecoderReadable(sp<Readable> source);
    ~OggDecoderReadable() override;

    uint32_t read(void* buffer, uint32_t length) override;
    int32_t seek(int32_t position, int32_t whence) override;
    int32_t remaining() override;
    uint32_t position() override;

    const sp<Readable>& source() const;

//  [[plugin::builder::by-value("ogg")]]
    class BUILDER final : public Builder<Readable> {
    public:
        BUILDER(BeanFactory& factory, const String& src);

        sp<Readable> build(const Scope& args) override;

    private:
        sp<Builder<String>> _src;
    };

private:
    sp<Readable> _source;

    OggVorbis_File _ogg_file;
    uint32_t _raw_size;
};

}
