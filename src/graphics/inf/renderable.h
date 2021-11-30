#ifndef ARK_GRAPHICS_INF_RENDERABLE_H_
#define ARK_GRAPHICS_INF_RENDERABLE_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

#include "renderer/base/varyings.h"

namespace ark {

class ARK_API Renderable {
public:
    virtual ~Renderable() = default;

    struct Snapshot {
        Snapshot(bool disposed = true)
            : _disposed(disposed) {
        }
        Snapshot(bool disposed, bool dirty, bool visible, int32_t type, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings)
            : _disposed(disposed), _dirty(dirty), _visible(visible), _type(type), _position(position), _size(size), _transform(transform), _varyings(varyings) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        bool _disposed;
        bool _dirty;
        bool _visible;
        int32_t _type;
        V3 _position;
        V3 _size;
        Transform::Snapshot _transform;
        Varyings::Snapshot _varyings;
    };

    virtual Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate) = 0;

};

}

#endif
