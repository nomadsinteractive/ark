#include "vorbis/impl/readable/ogg_decoder_readable.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"

namespace ark::plugin::vorbis {

namespace {

size_t _read_callback(void* ptr, const size_t size, const size_t nmemb, void* datasource)
{
    const OggDecoderReadable* oggDecoder = static_cast<OggDecoderReadable*>(datasource);
    return oggDecoder->source()->read(ptr, size * nmemb);
}

int _seek_callback(void* datasource, const ogg_int64_t offset, const int32_t whence)
{
    const OggDecoderReadable* oggDecoder = static_cast<OggDecoderReadable*>(datasource);
    return oggDecoder->source()->seek(static_cast<int32_t>(offset), whence);
}

long _tell_callback(void* datasource)
{
    const OggDecoderReadable* oggDecoder = static_cast<OggDecoderReadable*>(datasource);
    return oggDecoder->source()->position();
}

}

OggDecoderReadable::OggDecoderReadable(sp<Readable> source)
    : _source(std::move(source))
{
    const ov_callbacks callbacks = {
        _read_callback,
        _seek_callback,
        nullptr,
        _tell_callback
    };
    const uint32_t ret = ov_open_callbacks(this, &_ogg_file, nullptr, 0, callbacks);
    CHECK(ret == 0, "ov_open_callbacks failed.");
}

OggDecoderReadable::~OggDecoderReadable()
{
    ov_clear(&_ogg_file);
}

uint32_t OggDecoderReadable::read(void* buffer, const uint32_t length)
{
    int bitstream;
    uint8_t* buf = static_cast<uint8_t*>(buffer);
    uint8_t* end = buf + length;
    while(buf < end)
    {
        const long len = ov_read(&_ogg_file, reinterpret_cast<char*>(buf), end - buf, 0, 2, 1, &bitstream);
        if(len <= 0)
            return buf - static_cast<uint8_t*>(buffer);
        buf += len;
    }
    return length;
}

int32_t OggDecoderReadable::seek(const int32_t position, const int32_t whence)
{
    CHECK(whence == SEEK_SET, "Ogg decoder only supports seeking from beginning.");
    return ov_pcm_seek(&_ogg_file, position);
}

uint32_t OggDecoderReadable::position()
{
    return ov_pcm_tell(&_ogg_file);
}

const sp<Readable>& OggDecoderReadable::source() const
{
    return _source;
}

OggDecoderReadable::BUILDER::BUILDER(BeanFactory& factory, const String& src)
    : _src(factory.ensureBuilder<String>(src))
{
}

sp<Readable> OggDecoderReadable::BUILDER::build(const Scope& args)
{
    const String src = _src->build(args);
    return sp<Readable>::make<OggDecoderReadable>(Ark::instance().openAsset(src));
}

}