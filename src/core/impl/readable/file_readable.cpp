#include "core/impl/readable/file_readable.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"

namespace ark {

FileReadable::FileReadable(const String& filepath, const String& mode)
    : _filepath(filepath)
{
    _fp = fopen(filepath.c_str(), mode.c_str());
    CHECK(_fp, "Cannot open file \"%s\" for reading", filepath.c_str());
    init();
}

FileReadable::FileReadable(FILE* fp)
    : _fp(fp)
{
    init();
}

FileReadable::~FileReadable()
{
    if(_fp)
        fclose(_fp);
}

uint32_t FileReadable::read(void* buffer, uint32_t length)
{
    return fread(buffer, 1, length, _fp);
}

int32_t FileReadable::seek(int32_t position, int32_t whence)
{
    return fseek(_fp, position, whence);
}

int32_t FileReadable::remaining()
{
    return _size - ftell(_fp);
}

uint32_t FileReadable::position()
{
    return ftell(_fp);
}

void FileReadable::init()
{
    fseek(_fp, 0, SEEK_END);
    _size = ftell(_fp);
    fseek(_fp, 0, SEEK_SET);
}

FileReadable::BUILDER::BUILDER(BeanFactory& factory, const String& src)
    : _src(factory.ensureBuilder<String>(src))
{
}

sp<Readable> FileReadable::BUILDER::build(const Scope& args)
{
    return Ark::instance().openAsset(_src->build(args));
}

}
