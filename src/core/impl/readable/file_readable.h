#ifndef ARK_CORE_IMPL_READABLE_FILE_READABLE_H_
#define ARK_CORE_IMPL_READABLE_FILE_READABLE_H_

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/readable.h"

namespace ark {

class ARK_API FileReadable : public Readable {
public:
    FileReadable(const String& file_path, const String& mode);
    FileReadable(FILE* fp);
    ~FileReadable();

    virtual uint32_t read(void* buffer, uint32_t length) override;
    virtual int32_t seek(int32_t position, int32_t whence) override;
    virtual int32_t remaining() override;

    const String& filePath() const;

private:
    void init();

private:
    String _file_path;
    FILE* _fp;
    int32_t _size;
};

}

#endif
