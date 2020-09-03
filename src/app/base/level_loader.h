#ifndef ARK_APP_BASE_LEVEL_LOADER_H_
#define ARK_APP_BASE_LEVEL_LOADER_H_

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/util/strings.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API LevelLoader {
public:
    struct InstanceLibrary {
        int32_t _type;
        sp<Layer> _layer;
    };

public:
    LevelLoader(std::map<String, InstanceLibrary> instanceLabraries);

//  [[script::bindings::auto]]
    void load(const String& src);

//  [[plugin::builder]]
    class BUILDER : public Builder<LevelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<LevelLoader> build(const Scope& args) override;

    private:
        struct Library {
            String _name;
            int32_t _type;
            sp<Builder<Layer>> _layer;
        };

    private:
        std::vector<Library> _libraries;
    };

private:
    template<typename T> sp<Variable<T>> parseVector(const String& value) const {
        T vector(0);
        std::vector<String> splitted = Strings::unwrap(value, '(', ')').split(',');
        DCHECK(splitted.size() < sizeof(T) / sizeof(float), "Vector \"\" has more components than its target value(Vec%d)", value.c_str(), sizeof(T) / sizeof(float));
        for(size_t i = 0; i < splitted.size(); ++i)
            vector[i] = Strings::parse<float>(splitted.at(i));
        return sp<typename Variable<T>::Const>::make(vector);
    }

private:
    std::map<String, InstanceLibrary> _instance_libraries;
};

}

#endif
