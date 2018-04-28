#include "core/impl/asset/zip_asset.h"

#include "core/inf/readable.h"
#include "core/impl/asset/asset_with_prefix.h"

namespace ark {

namespace {

class ZipFileReadable : public Readable {
public:
    ZipFileReadable(const sp<ZipAsset::Stub>& stub, zip_file_t* zf)
        : _stub(stub), _zip_file(zf){
    }

    ~ZipFileReadable() {
        zip_fclose(_zip_file);
    }

    virtual uint32_t read(void* buffer, uint32_t bufferSize) override {
        return static_cast<uint32_t>(zip_fread(_zip_file, buffer, bufferSize));
    }

    virtual int32_t seek(int32_t /*position*/, int32_t /*whence*/) override {
        DFATAL("Unimplemented");
        return 0;
    }

    virtual int32_t remaining() override {
        DFATAL("Unimplemented");
        return 0;
    }

private:
    sp<ZipAsset::Stub> _stub;
    zip_file_t* _zip_file;
};

}

static zip_int64_t _local_zip_source_callback(void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd)
{
    ZipAsset::Stub* stub = reinterpret_cast<ZipAsset::Stub*>(userdata);
    switch(cmd)
    {
    case ZIP_SOURCE_TELL:
        return stub->position();
    case ZIP_SOURCE_STAT:
    {
        zip_stat_t* stat = reinterpret_cast<zip_stat_t*>(data);
        zip_stat_init(stat);
        stat->valid = ZIP_STAT_SIZE;
        stat->size = stub->size();
        return sizeof(zip_stat_t);
    }
        break;
    case ZIP_SOURCE_OPEN:
        return 0;
    case ZIP_SOURCE_READ:
        return stub->readable()->read(data, static_cast<uint32_t>(len));
    case ZIP_SOURCE_CLOSE:
        return 0;
    case ZIP_SOURCE_FREE:
        return 0;
    case ZIP_SOURCE_SEEK:
    {
        zip_error_t _zip_error;
        zip_source_args_seek_t* args = ZIP_SOURCE_GET_ARGS(zip_source_args_seek_t, data, len, &_zip_error);
        return stub->readable()->seek(static_cast<int32_t>(args->offset), args->whence);
    }
    case ZIP_SOURCE_SUPPORTS:
        return zip_source_make_command_bitmap(ZIP_SOURCE_OPEN, ZIP_SOURCE_READ, ZIP_SOURCE_CLOSE, ZIP_SOURCE_FREE, ZIP_SOURCE_SEEK, ZIP_SOURCE_TELL, ZIP_SOURCE_STAT, -1);
    default:
        break;
    }
    return -1;
}

ZipAsset::ZipAsset(const sp<Readable>& zipReadable)
    : _stub(sp<Stub>::make(zipReadable))
{
}

sp<Readable> ZipAsset::get(const String& name)
{
    zip_int64_t idx = zip_name_locate(_stub->archive(), name.c_str(), 0);
    zip_file_t* zf = idx >= 0 ? zip_fopen_index(_stub->archive(), static_cast<zip_uint64_t>(idx), 0) : nullptr;
    if(zf)
        return sp<ZipFileReadable>::make(_stub, zf);
    return nullptr;
}

sp<Asset> ZipAsset::getAsset(const String& path)
{
    return sp<AssetWithPrefix>::make(sp<ZipAsset>::make(*this), path.endsWith("/") ? path : path + "/");
}

bool ZipAsset::hasEntry(const String& name) const
{
    return zip_name_locate(_stub->archive(), name.c_str(), 0) != -1;
}

ZipAsset::Stub::Stub(const sp<Readable>& zipReadable)
    : _zip_readable(zipReadable), _size(zipReadable->remaining())
{
    zip_error_t error;
    _zip_source = zip_source_function_create(_local_zip_source_callback, this, &error);
    DCHECK(_zip_source, "Zip function create error: %s", zip_error_strerror(&error));
    _zip_archive = zip_open_from_source(_zip_source, 0, &error);
    DCHECK(_zip_archive, "Zip open error: %s", zip_error_strerror(&error));
}

ZipAsset::Stub::~Stub()
{
    zip_close(_zip_archive);
}

const sp<Readable>& ZipAsset::Stub::readable() const
{
    return _zip_readable;
}

int32_t ZipAsset::Stub::size() const
{
    return _size;
}

int32_t ZipAsset::Stub::position() const
{
    return _size - _zip_readable->remaining();
}

zip_t* ZipAsset::Stub::archive()
{
    return _zip_archive;
}

zip_source_t* ZipAsset::Stub::source()
{
    return _zip_source;
}

}
