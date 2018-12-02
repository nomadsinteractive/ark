#ifndef ARK_PLUGIN_VORBIS_IMPL_READABLE_OGG_DECODER_READABLE_H_
#define ARK_PLUGIN_VORBIS_IMPL_READABLE_OGG_DECODER_READABLE_H_

#include <vorbis/vorbisfile.h>

#include "core/inf/builder.h"
#include "core/inf/readable.h"
#include "core/types/shared_ptr.h"

namespace ark {
namespace plugin {
namespace vorbis {

class OggDecoderReadable : public Readable {
public:
    OggDecoderReadable(const sp<Readable>& delegate);
    ~OggDecoderReadable() override;

    virtual uint32_t read(void* buffer, uint32_t length) override;
    virtual int32_t seek(int32_t position, int32_t whence) override;
    virtual int32_t remaining() override;

    const sp<Readable>& delegate() const;
    uint32_t rawSize() const;

//[[plugin::builder::by-value("ogg")]]
    class BUILDER : public Builder<Readable> {
    public:
        BUILDER(BeanFactory& factory, const String& src);

        virtual sp<Readable> build(const sp<Scope>& args) override;

    private:
        sp<Builder<String>> _src;

    };

private:
    static size_t _read_callback(void* ptr, size_t size, size_t nmemb, void* datasource);
    static int _seek_callback(void* datasource, ogg_int64_t offset, int whence);
    static long _tell_callback(void* datasource);

private:
    sp<Readable> _source;

    OggVorbis_File _ogg_file;
    uint32_t _raw_size;
};

}
}
}

#endif
