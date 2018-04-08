#ifndef ARK_PLUGIN_ASSIMP_IMPL_IO_ARK_IO_SYSTEM_H_
#define ARK_PLUGIN_ASSIMP_IMPL_IO_ARK_IO_SYSTEM_H_

#include <assimp/IOSystem.hpp>

namespace ark {
namespace plugin {
namespace assimp {

class ArkIOSystem : public Assimp::IOSystem {
public:

    virtual bool Exists( const char* pFile) const;

    virtual char getOsSeparator() const;

    virtual Assimp::IOStream* Open(const char* pFile, const char* pMode = "rb");

    virtual void Close(Assimp::IOStream* pFile);
};

}
}
}

#endif
