#ifndef ARK_APP_BASE_LEVEL_H_
#define ARK_APP_BASE_LEVEL_H_

#include <map>
#include <vector>

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/util/documents.h"
#include "core/util/strings.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Level {
public:
    template<typename T> struct NamedType {
        String _name;
        int32_t _type;
        sp<Builder<T>> _builder;

        struct Instance {
            int32_t _type;
            sp<T> _object;
        };
    };

    typedef NamedType<Layer> RenderObjectLibrary;
    typedef NamedType<Collider> RigidBodyLibrary;

public:
    Level(std::map<String, sp<Camera>> cameras, std::map<String, sp<Vec3>> lights, std::map<String, RenderObjectLibrary::Instance> renderObjects, std::map<String, RigidBodyLibrary::Instance> rigidBodies);

//  [[script::bindings::auto]]
    void load(const String& src);

//  [[script::bindings::auto]]
    sp<Camera> getCamera(const String& name) const;

//  [[script::bindings::auto]]
    sp<Vec3> getLight(const String& name) const;

//  [[script::bindings::auto]]
    sp<RenderObject> getRenderObject(const String& name) const;

//  [[script::bindings::auto]]
    sp<RigidBody> getRigidBody(const String& name) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<Level> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Level> build(const Scope& args) override;

    private:
        template<typename T> std::vector<NamedType<T>> loadNamedTypes(BeanFactory& factory, const document& manifest, const String& name, const String& builderName) const {
            std::vector<NamedType<T>> namedTypes;
            for(const document& i : manifest->children(name))
                namedTypes.push_back({Documents::ensureAttribute(i, Constants::Attributes::NAME), Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE),
                                      factory.ensureBuilder<T>(i, builderName)});
            return namedTypes;
        }

        template<typename T> std::map<String, typename NamedType<T>::Instance> loadNamedTypeInstances(const std::vector<NamedType<T>>& namedTypes, const Scope& args) const {
            std::map<String, typename NamedType<T>::Instance> instances;
            for(const NamedType<T>& i : namedTypes)
                instances[i._name] = {i._type, i._builder->build(args)};
            return instances;
        }

    private:
        std::vector<RenderObjectLibrary> _render_object_libraries;
        std::vector<RigidBodyLibrary> _rigid_object_libraries;
        std::vector<std::pair<String, sp<Builder<Camera>>>> _cameras;
        std::vector<std::pair<String, sp<Builder<Vec3>>>> _lights;
    };

private:
    struct Library {
        Library(const RenderObjectLibrary::Instance& renderObjectInstance, const RigidBodyLibrary::Instance* rigidBodyInstance, const V3& dimensions);

        const RenderObjectLibrary::Instance* _render_object_instance;
        const RigidBodyLibrary::Instance* _rigid_body_instance;
        V3 _dimensions;
    };

    template<typename T> T parseVector(const String& value) const {
        T vector(0);
        std::vector<String> splitted = Strings::unwrap(value, '(', ')').split(',');
        DCHECK(splitted.size() <= sizeof(T) / sizeof(float), "Vector \"%s\" has more components than its target value(Vec%d)", value.c_str(), sizeof(T) / sizeof(float));
        for(size_t i = 0; i < splitted.size(); ++i)
            vector[i] = Strings::parse<float>(splitted.at(i));
        return vector;
    }

    sp<RigidBody> makeRigidBody(const Library& library, const sp<RenderObject>& renderObject) const;
    sp<Transform> makeTransform(const String& rotation, const String& scale) const;

private:
    std::map<String, sp<Camera>> _cameras;
    std::map<String, sp<Vec3>> _lights;
    std::map<String, sp<RenderObject>> _render_objects;

    std::map<String, sp<RigidBody>> _rigid_objects;
    std::vector<sp<RigidBody>> _unnamed_rigid_objects;

    std::map<String, RenderObjectLibrary::Instance> _render_object_libraries;
    std::map<String, RigidBodyLibrary::Instance> _rigid_body_libraries;
};

}

#endif
