#include "app/base/level.h"

#include "core/ark.h"

#include "graphics/base/camera.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/transform_3d.h"
#include "graphics/util/vec3_type.h"
#include "graphics/util/vec4_type.h"

#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/base/application_bundle.h"
#include "app/base/level_object.h"
#include "app/base/level_layer.h"
#include "app/base/level_library.h"

namespace ark {

Level::Level(const String& src)
    : _stub(sp<Stub>::make())
{
    doLoad(src);
}

const Map<int32_t, sp<LevelLibrary>>& Level::libraries() const
{
    return _stub->_libraries;
}

sp<LevelLayer> Level::getLayer(const StringView name) const
{
    const auto iter = _layers_by_name.find(name);
    return iter != _layers_by_name.end() ? iter->second : nullptr;
}

const Map<String, sp<Camera>>& Level::cameras() const
{
    return _stub->_cameras;
}

sp<Camera> Level::getCamera(const String& name) const
{
    const auto iter = _stub->_cameras.find(name);
    return iter != _stub->_cameras.end() ? iter->second : nullptr;
}

const Map<String, sp<Vec3>>& Level::lights() const
{
    return _stub->_lights;
}

sp<Vec3> Level::getLight(const String& name) const
{
    const auto iter = _stub->_lights.find(name);
    return iter != _stub->_lights.end() ? iter->second : nullptr;
}

const Vector<sp<LevelLayer>>& Level::layers()
{
    return _layers;
}

void Level::doLoad(const String& src)
{
    const document manifest = Ark::instance().applicationContext()->applicationBundle()->loadDocument(src);
    CHECK(manifest, "Cannot load manifest \"%s\"", src.c_str());

    for(const document& i : manifest->children("library"))
    {
        const String& name = Documents::ensureAttribute(i, constants::NAME);
        const String& dimensions = Documents::ensureAttribute(i, "dimensions");
        const int32_t id = Documents::ensureAttribute<int32_t>(i, constants::ID);
        CHECK_WARN(_stub->_libraries.find(id) == _stub->_libraries.end(), "Overwriting instance library mapping(%d)", id);
        _stub->_libraries.emplace(id, sp<LevelLibrary>::make(id, name, sp<Vec3>::make<Vec3::Const>(Strings::eval<V3>(dimensions))));
    }

    for(const document& i : manifest->children(constants::LAYER))
    {
        String layerName = Documents::getAttribute(i, constants::NAME);
        Vector<sp<LevelObject>> layerObjects;
        for(const document& j : i->children("object"))
        {
            LevelObject obj(_stub, j);
            if(obj._type == LevelObject::TYPE_CAMERA)
            {
                DCHECK_WARN(!getCamera(obj._name), "Overwriting camera(%s) in \"%s\"", obj._name.c_str(), src.c_str());
                const float fovy = Documents::ensureAttribute<float>(j, "fov_y");
                const float clipNear = Documents::ensureAttribute<float>(j, "clip-near");
                const float clipFar = Documents::ensureAttribute<float>(j, "clip-far");
                const Quaternion quaternion(sp<Vec4>::make<Vec4::Const>(obj._rotation ? obj._rotation.value() : constants::QUATERNION_ONE));
                const M4 matrix = quaternion.toMatrix()->val();
                const V3 front = MatrixUtil::mul(matrix, V3(0, -1.0f, 0));
                const V3 up = MatrixUtil::mul(matrix, V3(0, 0, -1.0f));
                Camera c = Ark::instance().createCamera(Ark::COORDINATE_SYSTEM_RHS, false, Ark::instance().renderController()->renderEngine()->isBackendLHS());
                c.perspective(fovy, 16.0f / 9, clipNear, clipFar);
                c.lookAt(obj._position, obj._position + front, up);
                _stub->_cameras[obj._name] = sp<Camera>::make(std::move(c));
            }
            else if(obj._type == LevelObject::TYPE_LIGHT)
            {
                DCHECK_WARN(!getLight(obj._name), "Overwriting light(%s) in \"%s\"", obj._name.c_str(), src.c_str());
                _stub->_lights[obj._name] = sp<Vec3>::make<Vec3::Const>(obj._position);
            }
            layerObjects.push_back(sp<LevelObject>::make(obj));
        }
        _layers.push_back(sp<LevelLayer>::make(_stub, std::move(layerName), std::move(layerObjects)));
    }

    for(const sp<LevelLayer>& i : _layers)
        if(i->name())
            _layers_by_name.emplace(i->name(), i);
}

}
