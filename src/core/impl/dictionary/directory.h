#ifndef ARK_CORE_IMPL_DICTIONARY_DIRECTORY_H_
#define ARK_CORE_IMPL_DICTIONARY_DIRECTORY_H_

#include <map>

#include "core/inf/dictionary.h"
#include "core/types/null.h"
#include "core/util/strings.h"

namespace ark {

template<typename T> class Directory : public Dictionary<T> {
public:

    virtual T get(const String& name) override {
        String filename, dirname;
        Strings::rcut(name, dirname, filename, '/');
        sp<Directory<T>> directory;
        for(const String& i : dirname.split('/')) {
            directory = getDirectory(i);
            if(!directory)
                return Null::ptr<T>();
        }
        return directory ? directory->getFile(filename) : Null::ptr<T>();
    }

    const sp<Directory<T>>& getDirectory(const String& name) const {
        String parentdirname, dirname;
        Strings::rcut(name, parentdirname, dirname, '/');
        const Directory<T>* directory = this;
        for(const String& i : parentdirname.split('/')) {
            const auto iter = directory->_directories.find(i);
            if(iter == directory->_directories.end())
                return sp<Directory<T>>::null();
            directory = iter->second.get();
        }

        const auto iter = directory->_directories.find(dirname);
        if(iter != directory->_directories.end())
            return iter->second;
        return sp<Directory<T>>::null();
    }

    T getFile(const String& name) const {
        auto iter = _files.find(name);
        if(iter != _files.end())
            return iter->second;
        return Null::ptr<T>();
    }

private:
    std::map<String, sp<Directory<T>>> _directories;
    std::map<String, T> _files;

};

}

#endif
