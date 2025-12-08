#include "core/impl/asset_bundle/asset_bundle_zip_file.h"

#include <filesystem>

#include "core/inf/asset.h"
#include "core/inf/readable.h"
#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"
#include "core/util/strings.h"

#include "platform/platform.h"

namespace ark {

std::pair<Optional<StringView>, StringView> splitpath(const StringView filepath)
{
    if(const auto pos = filepath.rfind('/'); pos != StringView::npos)
        return {filepath.substr(0, pos), filepath.substr(pos + 1)};
    return {{}, filepath};
}

String toZipFilePath(const String& filepath)
{
    String zipFilePath = filepath.replace("\\", "/").rstrip('/');
    return zipFilePath.startsWith("./") ? zipFilePath.substr(2) : zipFilePath;
}

class AssetBundleZipFile::Stub {
public:
    Stub(sp<Readable> zipReadable, const String& zipLocation, const size_t size)
        : _zip_readable(std::move(zipReadable)), _zip_location(Platform::getRealPath(zipLocation)), _size(size)
    {
        CHECK(_zip_readable, "Cannot open file %s", _zip_location.c_str());
        zip_error_t error = {0};
        _zip_source = zip_source_function_create(_local_zip_source_callback, this, &error);
        CHECK(_zip_source, "Zip function create error: %s", zip_error_strerror(&error));
        _zip_archive = zip_open_from_source(_zip_source, 0, &error);
        CHECK(_zip_archive, "Zip open error: %s", zip_error_strerror(&error));
    }
    ~Stub()
    {
        zip_close(_zip_archive);
    }

    int32_t position() const
    {
        return _zip_readable->position();
    }

    sp<Readable> _zip_readable;
    String _zip_location;
    size_t _size;

    zip_t* _zip_archive;
    zip_source_t* _zip_source;

    std::mutex _mutex;
};

class AssetBundleZipFile::ReadableZipFile final : public Readable {
public:
    ReadableZipFile(const sp<Stub>& stub, zip_file_t* zf, const uint64_t size)
        : _stub(stub), _zip_file(zf), _offset(0), _size(size) {
        _buffer.reserve(size);
    }

    ~ReadableZipFile() override
    {
        const std::lock_guard lg(_stub->_mutex);
        zip_fclose(_zip_file);
    }

    uint32_t read(void* buffer, const uint32_t bufferSize) override
    {
        const std::lock_guard lg(_stub->_mutex);

        ASSERT(zip_ftell(_zip_file) == _buffer.size());

        uint32_t sizeReadFromCache = 0;
        if(_buffer.size() > _offset)
        {
            sizeReadFromCache = std::min<uint32_t>(_buffer.size() - _offset, bufferSize);
            memcpy(buffer, &_buffer[_offset], sizeReadFromCache);
            _offset += sizeReadFromCache;
        }

        if(sizeReadFromCache >= bufferSize)
            return bufferSize;

        const zip_int64_t sizeRead = zip_fread(_zip_file, static_cast<uint8_t*>(buffer) + sizeReadFromCache, bufferSize - sizeReadFromCache);
        if(sizeRead > 0)
        {
            _buffer.resize(_offset + sizeRead);
            memcpy(&_buffer[_offset], buffer, sizeRead);
            _offset += sizeRead;
        }
        return sizeRead + sizeReadFromCache;
    }

    int32_t seek(const int32_t position, const int32_t whence) override
    {
        const uint64_t offset = whence == SEEK_SET ? position : static_cast<int64_t>(whence == SEEK_CUR ? _offset : _size) + position;
        const std::lock_guard lg(_stub->_mutex);
        const size_t cacheSize = _buffer.size();
        if(offset > cacheSize)
        {
            ASSERT(zip_ftell(_zip_file) == _buffer.size());
            _buffer.resize(offset);
            const int64_t sizeRead = zip_fread(_zip_file, &_buffer[cacheSize], offset - cacheSize);
            if(sizeRead != static_cast<int64_t>(offset - cacheSize))
            {
                _buffer.resize(cacheSize + sizeRead);
                return -1;
            }
        }
        _offset = offset;
        return 0;
    }

    int32_t remaining() override
    {
        DFATAL("Unimplemented");
        return 0;
    }

    uint32_t position() override
    {
        return zip_ftell(_zip_file);
    }

private:
    sp<Stub> _stub;
    zip_file_t* _zip_file;
    uint64_t _offset;
    uint64_t _size;

    Vector<uint8_t> _buffer;
};

class AssetBundleZipFile::AssetZipEntry final : public Asset {
public:
    AssetZipEntry(const sp<Stub>& stub, String location, const String& entryName, zip_file_t* zf)
        : _stub(stub), _location(std::move(location)),  _zip_file(zf) {
        zip_stat_t zipState = {};
        zip_stat(_stub->_zip_archive, entryName.c_str(), 0, &zipState);
        _size = zipState.size;
    }

    sp<Readable> open() override {
        return sp<Readable>::make<ReadableZipFile>(_stub, _zip_file, _size);
    }

    String location() override {
        return _location;
    }

    size_t size() override
    {
        return _size;
    }

private:
    sp<Stub> _stub;
    String _location;
    uint64_t _size;
    zip_file_t* _zip_file;
};

zip_int64_t AssetBundleZipFile::_local_zip_source_callback(void* userdata, void* data, const zip_uint64_t len, const zip_source_cmd_t cmd)
{
    const Stub* stub = static_cast<Stub*>(userdata);
    switch(cmd)
    {
    case ZIP_SOURCE_TELL:
        return stub->position();
    case ZIP_SOURCE_STAT:
    {
        zip_stat_t* stat = static_cast<zip_stat_t*>(data);
        zip_stat_init(stat);
        stat->valid = ZIP_STAT_SIZE;
        stat->size = stub->_size;
        return sizeof(zip_stat_t);
    }
    case ZIP_SOURCE_OPEN:
        return 0;
    case ZIP_SOURCE_READ:
        return stub->_zip_readable->read(data, static_cast<uint32_t>(len));
    case ZIP_SOURCE_CLOSE:
        return 0;
    case ZIP_SOURCE_FREE:
        return 0;
    case ZIP_SOURCE_ERROR:
        return 0;
    case ZIP_SOURCE_SEEK:
    {
        zip_error_t _zip_error;
        const zip_source_args_seek_t* args = ZIP_SOURCE_GET_ARGS(zip_source_args_seek_t, data, len, &_zip_error);
        return stub->_zip_readable->seek(static_cast<int32_t>(args->offset), args->whence);
    }
    case ZIP_SOURCE_SUPPORTS:
        return ZIP_SOURCE_SUPPORTS_SEEKABLE;
    default:
        break;
    }
    return -1;
}

AssetBundleZipFile::AssetBundleZipFile(Asset& asset)
    : AssetBundleZipFile(asset.open(), asset.location(), asset.size())
{
}

AssetBundleZipFile::AssetBundleZipFile(sp<Readable> zipReadable, const String& zipLocation, const size_t size)
    : _stub(sp<Stub>::make(std::move(zipReadable), zipLocation, size))
{
}

sp<Asset> AssetBundleZipFile::getAsset(const String& name)
{
    String sName = toZipFilePath(name);
    const std::lock_guard lg(_stub->_mutex);
    const zip_int64_t idx = zip_name_locate(_stub->_zip_archive, sName.c_str(), 0);
    if(zip_file_t* zf = idx >= 0 ? zip_fopen_index(_stub->_zip_archive, static_cast<zip_uint64_t>(idx), 0) : nullptr)
    {
        String location = Strings::sprintf("%s/%s", _stub->_zip_location.c_str(), sName.c_str());
        return sp<Asset>::make<AssetZipEntry>(_stub, std::move(location), std::move(sName), zf);
    }
    return nullptr;
}

sp<AssetBundle> AssetBundleZipFile::getBundle(const String& path)
{
    return sp<AssetBundle>::make<AssetBundleWithPrefix>(sp<AssetBundle>::make<AssetBundleZipFile>(*this), path.endsWith("/") ? path : path + "/");
}

Vector<String> AssetBundleZipFile::listAssets(const StringView dirname)
{
    Vector<String> assets;
    const String zipDirname = toZipFilePath(dirname);
    const std::lock_guard lg(_stub->_mutex);
    const int32_t numOfEntries = zip_get_num_entries(_stub->_zip_archive, 0);
    for(int32_t i = 0; i < numOfEntries; ++i)
    {
        const char* filepath = zip_get_name(_stub->_zip_archive, i, 0);
        if(const auto [optDirname, filename] = splitpath(filepath); optDirname)
        {
            if(optDirname.value() == zipDirname)
                assets.emplace_back(filename);
        }
        else if(zipDirname.empty())
            assets.emplace_back(filename);
    }
    return assets;
}

bool AssetBundleZipFile::hasEntry(const String& name) const
{
    const std::lock_guard lg(_stub->_mutex);
    return zip_name_locate(_stub->_zip_archive, name.c_str(), 0) != -1;
}

}
