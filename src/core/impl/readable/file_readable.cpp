#include "core/impl/readable/file_readable.h"

namespace ark {

FileReadable::FileReadable(const String& file_path, const String& mode)
    : _file_path(file_path)
{
    _fp = fopen(file_path.c_str(), mode.c_str());
    DCHECK(_fp, "Cannot open file \"%s\" for reading", file_path.c_str());
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

const String &FileReadable::filePath() const
{
    return _file_path;
}

void FileReadable::init()
{
    fseek(_fp, 0, SEEK_END);
    _size = ftell(_fp);
    fseek(_fp, 0, SEEK_SET);
}

}
