#include "vorbis/impl/readable/ogg_decoder_readable.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"

#include "core/impl/readable/file_readable.h"
#include "core/util/strings.h"

namespace ark::plugin::vorbis {

OggDecoderReadable::OggDecoderReadable(const sp<Readable>& source)
    : _source(source), _raw_size(source->remaining())
{
    ov_callbacks callbacks = {
        _read_callback,
        _seek_callback,
        nullptr,
        _tell_callback
    };
    int ret = ov_open_callbacks(this, &_ogg_file, nullptr, 0, callbacks);
    DCHECK(ret == 0, "ov_open_callbacks failed.");
}

OggDecoderReadable::~OggDecoderReadable()
{
    ov_clear(&_ogg_file);
}

uint32_t OggDecoderReadable::read(void* buffer, uint32_t length)
{
/*
    int bitstream;
    return ov_read(&_ogg_file, reinterpret_cast<char*>(buffer), length, 0, 2, 1, &bitstream);
/*/
    int bitstream;
    uint8_t* buf = reinterpret_cast<uint8_t*>(buffer);
    uint8_t* end = buf + length;
    while(buf < end)
    {
        long len = ov_read(&_ogg_file, reinterpret_cast<char*>(buf), end - buf, 0, 2, 1, &bitstream);
        if(len <= 0)
            return buf - reinterpret_cast<uint8_t*>(buffer);
        buf += len;
    }
    return length;
/**/
}

int32_t OggDecoderReadable::seek(int32_t position, int32_t whence)
{
    DCHECK(whence == SEEK_SET, "Ogg decoder only supports seeking from beginning.");
    return ov_pcm_seek(&_ogg_file, static_cast<ogg_int64_t>(position));
}

int32_t OggDecoderReadable::remaining()
{
    ogg_int64_t total = ov_pcm_total(&_ogg_file, -1);
    ogg_int64_t tell = ov_pcm_tell(&_ogg_file);
    DCHECK(total != OV_EINVAL && tell != OV_EINVAL, "ov_pcm_total ov_pcm_tell failed.");
    return static_cast<int32_t>(total - tell);
}

uint32_t OggDecoderReadable::position()
{
    return ov_pcm_tell(&_ogg_file);
}

const sp<Readable>& OggDecoderReadable::delegate() const
{
    return _source;
}

uint32_t OggDecoderReadable::rawSize() const
{
    return _raw_size;
}

size_t OggDecoderReadable::_read_callback(void* ptr, size_t size, size_t nmemb, void* datasource)
{
    OggDecoderReadable* oggDecoder = reinterpret_cast<OggDecoderReadable*>(datasource);
    return oggDecoder->delegate()->read(ptr, size * nmemb);
}

int OggDecoderReadable::_seek_callback(void* datasource, ogg_int64_t offset, int whence)
{
    OggDecoderReadable* oggDecoder = reinterpret_cast<OggDecoderReadable*>(datasource);
    return oggDecoder->delegate()->seek(static_cast<int32_t>(offset), whence);
}

long OggDecoderReadable::_tell_callback(void* datasource)
{
    OggDecoderReadable* oggDecoder = static_cast<OggDecoderReadable*>(datasource);
    return oggDecoder->rawSize() - oggDecoder->delegate()->remaining();
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